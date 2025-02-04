#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include <bits/stdc++.h>
#include <fstream>
#include <windows.h>
#include "compiler.h"
#ifdef _WIN32
#include <SDL2/SDL2_gfx.h>
#else
#include <SDL2/SDL2_gfxPrimitives.h>
#endif
using namespace std;

string path = "C:/Users/Erfan/Dev/Cpp/IDE-Project/";

const int SCREEN_WIDTH = 1400, SCREEN_HEIGHT = 700;
const int minCurrentLine = 3, minCursorPos = 16;
int currentLine = minCurrentLine, cursorPos = minCursorPos;

bool darkMode = false;
bool isMenuOpen = false, isEditOpen = false;
string currentFileName = "", currentProjectName = "";

vector<string> lines = {""};
vector<vector<string>> linesHistory, futureLines;

vector<string> autoCompleteWords = {
        "if", "else", "while", "switch", "case", "for", "try", "catch", "main", "int", "float", "double", "char", "void", "bool", "auto", "const", "static", "return", "namespace", "using", "include", "true", "false", "class", "struct", "template", "signed", "unsigned", "short", "long", "sizeof", "union", "enum","break", "continue","not", "or", "new"
};
vector<string> keywords = autoCompleteWords;

const map<string, vector<string>> libraries ={
        {"iostream", {"cin", "cout", "cerr", "clog", "getline", "put", "get", "ignore"}},
        {"cmath", {"abs", "remainder", "pow", "sqrt", "sin", "cos", "tan", "asin", "acos", "atan", "atan2","sinh", "cosh", "tanh", "asinh", "acosh", "atanh","exp", "log", "log10", "log2", "ceil", "floor", "round"}},
        {"bits/stdc++.h", {"sort", "lower_bound", "upper_bound", "binary_search","unique", "reverse", "partition", "push_back", "pop_back", "insert", "erase", "size", "clear", "begin", "end", "rbegin", "rend", "front", "back","insert", "erase", "find", "count", "lower_bound", "upper_bound", "emplace", "push_back", "pop_back", "push_front", "pop_front", "push", "pop", "top", "front", "back", "empty","substr", "append", "insert", "erase", "replace", "find", "rfind", "compare", "size", "length", "empty", "clear"}}
};

struct Button {
    SDL_Rect rect;
    int radius;
    SDL_Color color;
    SDL_Color textColor;
    string label;
    function<void()> action;

    void render(SDL_Renderer* renderer, TTF_Font* font) {
        // Draw button background as a rounded rectangle
        roundedBoxRGBA(renderer,
                       rect.x, rect.y, rect.x + rect.w, rect.y + rect.h,
                       radius, // Corner radius
                       color.r, color.g, color.b, color.a);

//        // Draw button border as a rounded rectangle
//        roundedRectangleRGBA(renderer,
//                             rect.x, rect.y, rect.x + rect.w, rect.y + rect.h,
//                             radius, // Corner radius
//                             255, 255, 255, 255);

        // Render label
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, label.c_str(), textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        int textWidth, textHeight;
        SDL_QueryTexture(textTexture, NULL, NULL, &textWidth, &textHeight);

        SDL_Rect textRect = {
                rect.x + (rect.w - textWidth) / 2,
                rect.y + (rect.h - textHeight) / 2,
                textWidth, textHeight
        };
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }
    bool isClicked(int x, int y) const {
        return x >= rect.x && x <= rect.x + rect.w && y >= rect.y && y <= rect.y + rect.h;
    }
};
struct ParseState {
    stack<pair<char, size_t>> bracketStack; // Store bracket type and line number
    bool inQuote = false;
    size_t quoteStartLine = 0;
    bool escapeNext = false;
    bool inMultiLineComment = false; // Track if inside a multi-line comment
    size_t commentStartLine = 0; // Track where the comment started
};
struct SwitchCase {
    int startLine;
    int endLine;
    string variable;
    vector<string> constants;
    vector<string> elseLines;
};

vector<string> usedLibraries, includedLibraries, bracketErrors, semicolonErrors, typoErrors, switchErrors;
vector<SwitchCase> switchCases;
vector<Button> buttons, projectsButtons, errorButtons, menuBarButtons, editButtons;
vector<string> projects, files;

void ensureLastLineVisible(int currentLine, int &scrollOffset, int SCREEN_HEIGHT, int LINE_HEIGHT, int totalLines);

void getNameFromUser(SDL_Renderer* renderer, TTF_Font* font, string& name, string content) {
    string inputText = "";
    bool done = false;
    SDL_StartTextInput();

    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done = true;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_BACKSPACE && !inputText.empty()) {
                    inputText.pop_back(); // Remove last character
                } else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                    name = inputText; // Finalize input
                    done = true;
                } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    done = true; // Exit without saving
                }
            } else if (event.type == SDL_TEXTINPUT) {
                inputText += event.text.text; // Append typed character
            }
        }

        // Render the input box and label
        SDL_SetRenderDrawColor(renderer, 0, 80, 160, 255);
        SDL_RenderClear(renderer);

        SDL_Color titleColor = {255, 255, 255, 255};
        SDL_Color textColor = {SDL_Color{0, 80, 160, 255}};
        SDL_Color bgColor = {255, 255, 255, 255};

        string labelText = "Enter the "+ content +":";
        SDL_Surface* labelSurface = TTF_RenderText_Solid(font, labelText.c_str(), titleColor);
        SDL_Texture* labelTexture = SDL_CreateTextureFromSurface(renderer, labelSurface);

        int labelWidth, labelHeight;
        SDL_QueryTexture(labelTexture, NULL, NULL, &labelWidth, &labelHeight);
        SDL_Rect labelRect = {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 3 - 50, labelWidth, labelHeight};
        SDL_RenderCopy(renderer, labelTexture, NULL, &labelRect);

        SDL_FreeSurface(labelSurface);
        SDL_DestroyTexture(labelTexture);

        // Render the input box
        SDL_Rect inputBox = {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 3, SCREEN_WIDTH / 2, 50};
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_RenderFillRect(renderer, &inputBox);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &inputBox);

        SDL_Surface* textSurface = TTF_RenderText_Solid(font, inputText.c_str(), textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        int textWidth, textHeight;
        SDL_QueryTexture(textTexture, NULL, NULL, &textWidth, &textHeight);
        SDL_Rect textRect = {inputBox.x + 10, inputBox.y + (inputBox.h - textHeight) / 2, textWidth, textHeight};
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);

        SDL_RenderPresent(renderer);
    }

    SDL_StopTextInput(); // Disable text input
}

vector<string> getProjects() {
    vector<string> folders;
    WIN32_FIND_DATA fileData;
    HANDLE hFind = FindFirstFile((path + "\\*").c_str(), &fileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                string folderName = fileData.cFileName;
                if (folderName != "." && folderName != ".." &&
                    folderName != "cmake-build-debug" && folderName != ".idea") {
                    folders.push_back(folderName);
                }
            }
        } while (FindNextFile(hFind, &fileData) != 0);
        FindClose(hFind);
    }
    return folders;
}
vector<string> getFiles(string &project) {
    vector<string> cppFiles;
    WIN32_FIND_DATA fileData;
    HANDLE hFind;
    string searchPath = path + "\\" + project + "\\*.cpp";
    hFind = FindFirstFile(searchPath.c_str(), &fileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                cppFiles.push_back(fileData.cFileName);
            }
        } while (FindNextFile(hFind, &fileData) != 0);
        FindClose(hFind);
    }
    searchPath = path + "\\" + project + "\\*.txt";
    hFind = FindFirstFile(searchPath.c_str(), &fileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                cppFiles.push_back(fileData.cFileName);
            }
        } while (FindNextFile(hFind, &fileData) != 0);
        FindClose(hFind);
    }
    return cppFiles;
}

void newProject(SDL_Renderer* renderer, TTF_Font* font, string newPorjectName){
    if(newPorjectName.empty()){
        getNameFromUser(renderer, font, newPorjectName, "new project name");
    }
    if (!newPorjectName.empty()) {
        currentProjectName = newPorjectName;
        cout << currentProjectName;
        if (CreateDirectory((path+newPorjectName).c_str(), NULL)) {
            cout << "Folder created successfully.\n";
            projects.push_back(newPorjectName);
            projectsButtons.push_back({
                                              {2, static_cast<int>(58*(projectsButtons.size())), 202, 50},
                                              20,
                                              SDL_Color{0, 80, 160, 255},
                                              {255, 255, 255, 255},
                                              newPorjectName,[&renderer, &font, newPorjectName]() {
//                         openFile(lines, renderer, font, newPorjectName + ".cpp", newPorjectName);
//                         initializeProjectsButtons(renderer, font);
                    }}
            );
        } else {
            cerr << "Error creating folder: " << GetLastError() << '\n';
        }
    }
    }

void saveAsToFile(const vector<string>& lines, SDL_Renderer* renderer, TTF_Font* font) {
    string projectName;
    getNameFromUser(renderer, font, projectName, "project name");
    newProject(renderer, font, projectName);
    getNameFromUser(renderer, font, currentFileName, "filename");

    if (currentFileName.empty()) {
        cerr << "File name cannot be empty.\n";
        return;
    }

    if (currentFileName.find('.') == string::npos) {
        currentFileName += ".cpp";
    }
    ofstream outFile(path+projectName+"/"+currentFileName);
    if (!outFile) {
        cerr << "Error creating file: " << currentFileName << endl;
        return;
    }
    int iLine = 1;
    for (const string& line : lines) {
        if(iLine>3){
            outFile << line.substr(minCursorPos, line.size()-minCursorPos) << endl;
        }iLine++;
    }

    cout << "The code was saved as " << currentFileName << endl;
    cout << "At " << path << endl;
}
void saveToFile(const vector<string>& lines, SDL_Renderer* renderer, TTF_Font* font) {
    if(currentFileName.empty()){
        saveAsToFile(lines, renderer, font);
    }else {
        if (currentFileName.find('.') == string::npos) {
            currentFileName += ".cpp";
        }
        ofstream outFile(path + currentProjectName + "/" + currentFileName);
        if (!outFile) {
            cerr << "Error creating file: " << path + currentFileName << endl;
            return;
        }
        int iLine = 1;
        for (const string& line : lines) {
            if(iLine>3){
                outFile << line.substr(minCursorPos, line.size()-minCursorPos) << endl;
            }iLine++;
        }

        cout << "The code was saved as" << currentFileName << endl;
        cout << "At" << path << endl;
    }
}

vector<pair<string, SDL_Color>> highlightLine(const string& line) {
    vector<pair<string, SDL_Color>> segments;
    static bool inMultiLineComment = false;
    vector<string> datatypes = {
            "int", "float", "double", "char", "bool", "auto", "const", "static", "short", "long", "unsigned", "signed"
    };

    // The regex is written on one line. Its alternatives are:
    // 1. ([\(\)\{\}\[\]\+\-])                        - Operators/brackets
    // 2. (                                           - String literals (double-,
    //        "([^"\\]|\\.)*"                         -   single- or angle bracket–quoted)
    //      | '([^'\\]|\\.)*'
    //      | <([^>\\]|\\.)*>
    //     )
    // 3. \b(auto|bool|break|case|catch|char|class|const|continue|default|delete|do|double|else|enum|false|float|for|if|int|vector|string|long|namespace|new|nullptr|private|protected|public|return|short|signed|sizeof|static|struct|switch|template|true|try|typedef|union|unsigned|using|void|while)\b
    //                                                   - Keywords (group 6 now)
    // 4. \b([a-zA-Z_][a-zA-Z0-9_]*)\s*(?=\()           - Function names (group 7)
    // 5. \b(\d+(\.\d+)?)\b                             - Numbers (group 8; fractional part in group 9)
    // 6. (//.*$)                                      - Single-line comments (group 10)
    // 7. (/\*.*)                                      - Start of multi-line comment (group 11)
    // 8. (\*/)                                        - End of multi-line comment (group 12)
    // 9. (#\b(include|define|ifdef|else|endif)\b)       - Preprocessor directives (group 13; inner directive in group 14)
    // 10. \b(?!(?:auto|bool|break|case|catch|char|class|const|continue|default|delete|do|double|else|enum|false|float|for|if|int|vector|string|long|namespace|new|nullptr|private|protected|public|return|short|signed|sizeof|static|struct|switch|template|true|try|typedef|union|unsigned|using|void|while)\b)([a-zA-Z_][a-zA-Z0-9_]*)\b
    //                                                   - Variables (group 15), which are identifiers not matching keywords.
    regex combinedRegex(
            R"(([\(\)\{\}\[\]\+\-])|(\"([^\"\\]|\\.)*\"|'([^'\\]|\\.)*'|<([^>\\]|\\.)*>)|\b(auto|bool|break|case|catch|char|class|const|continue|default|delete|do|double|else|enum|std::|false|float|for|if|int|vector|string|long|namespace|new|nullptr|private|protected|public|return|short|signed|sizeof|static|struct|switch|template|true|try|typedef|union|unsigned|using|void|while)\b|\b([a-zA-Z_][a-zA-Z0-9_]*)\s*(?=\()|\b(\d+(\.\d+)?)\b|(//.*$)|(/\*.*)|(\*/)|(#\b(include|define|ifdef|else|endif)\b)|\b(?!(?:auto|bool|break|case|catch|char|class|const|continue|default|delete|do|double|else|enum|false|float|for|if|int|vector|string|long|namespace|new|nullptr|private|protected|public|return|short|signed|sizeof|static|struct|switch|template|true|try|typedef|union|unsigned|using|void|while)\b)([a-zA-Z_][a-zA-Z0-9_]*)\b)"
    );

    sregex_iterator matchesBegin(line.begin(), line.end(), combinedRegex);
    sregex_iterator matchesEnd;

    size_t prevPos = 0;
    for (sregex_iterator it = matchesBegin; it != matchesEnd; ++it) {
        smatch match = *it;

        // Add any text before the match with the normal text color.
        if (match.position() > prevPos) {
            SDL_Color normalColor = darkMode ? SDL_Color{255, 255, 255, 255} : SDL_Color{0, 0, 0, 255};
            if (inMultiLineComment)
                normalColor = darkMode ? SDL_Color{92, 99, 112, 255} : SDL_Color{128, 128, 128, 255}; // Comment color
            segments.push_back({ line.substr(prevPos, match.position() - prevPos), normalColor });
        }

        SDL_Color matchColor = darkMode ? SDL_Color{255, 255, 255, 255} : SDL_Color{0, 0, 0, 255};

        // Updated group index mapping:
        // 1: Operators/brackets
        // 2: String literals (the whole literal; inner groups ignored)
        // 6: Keywords
        // 7: Function names
        // 8: Numbers (group 8; fractional part in group 9)
        // 10: Single-line comments
        // 11: Start of multi-line comment (/*...)
        // 12: End of multi-line comment (...*/)
        // 13: Preprocessor directives (#include/#define) (group 13; inner directive in group 14)
        // 15: Variable names (only if not a keyword)

        if (match[11].matched) { // Start of multi-line comment (/*)
            inMultiLineComment = true;
            matchColor = darkMode ? SDL_Color{92, 99, 112, 255} : SDL_Color{128, 128, 128, 255};
        }
        else if (match[12].matched) { // End of multi-line comment (*/)
            inMultiLineComment = false;
            matchColor = darkMode ? SDL_Color{92, 99, 112, 255} : SDL_Color{128, 128, 128, 255};
        }
        else if (inMultiLineComment || match[10].matched) { // Inside a multi-line comment or single-line comment
            matchColor = darkMode ? SDL_Color{92, 99, 112, 255} : SDL_Color{128, 128, 128, 255};
        }
        else if (match[1].matched) { // Operators/brackets
            matchColor = darkMode ? SDL_Color{171, 178, 191, 255} : SDL_Color{184, 134, 11, 255};
        }
        else if (match[2].matched) { // String literals (including "", '', and <>)
            matchColor = darkMode ? SDL_Color{152, 195, 121, 255} : SDL_Color{0, 100, 0, 255};
        }
        else if (match[6].matched) { // Keywords
            string keyword = match.str();
            // Check if the keyword is a datatype
            if (find(datatypes.begin(), datatypes.end(), keyword) != datatypes.end()) {
                matchColor = darkMode ? SDL_Color{224, 108, 117, 255} : SDL_Color{0, 128, 128, 255}; // Different color for datatypes
            } else {
                matchColor = darkMode ? SDL_Color{198, 120, 221, 255} : SDL_Color{0, 51, 102, 255}; // Default keyword color
            }
        }
        else if (match[7].matched) { // Function names (followed by a parenthesis)
            matchColor = darkMode ? SDL_Color{97, 175, 254, 255} : SDL_Color{255, 140, 0, 255};
        }
        else if (match[8].matched) { // Numbers
            matchColor = darkMode ? SDL_Color{209, 154, 102, 255} : SDL_Color{128, 0, 128, 255};
        }
        else if (match[13].matched) { // Preprocessor directives (#include/#define)
            matchColor = darkMode ? SDL_Color{86, 182, 194, 255} : SDL_Color{0, 139, 139, 255};
        }
        else if (match[15].matched) { // Variables (identifiers that are not keywords)
            matchColor = darkMode ? SDL_Color{229, 192, 123, 255} : SDL_Color{139, 0, 0, 255};
        }

        segments.push_back({ match.str(), matchColor });
        prevPos = match.position() + match.length();
    }

    // Add any remaining text.
    if (prevPos < line.size()) {
        segments.push_back({ line.substr(prevPos), darkMode ? SDL_Color{255, 255, 255, 255} : SDL_Color{0, 0, 0, 255} });
    }

    return segments;
}

void checkSwitchOpportunities(const vector<string>& lines) {
    switchCases.clear();
    switchErrors.clear();

    // More flexible regex patterns
    regex ifRegex(R"(\bif\s*\(\s*([a-zA-Z_]\w*)\s*==\s*([^)]+)\s*\)\s*\{?)");
    regex elseIfRegex(R"(\belse\s+if\s*\(\s*([a-zA-Z_]\w*)\s*==\s*([^)]+)\s*\)\s*\{?)");
    regex elseRegex(R"(\belse\b\s*\{?)");
    regex closeBrace(R"(\})");

    int chainStart = -1;
    string targetVar;
    vector<string> caseValues;
    vector<vector<string>> caseBlocks;
    vector<string> elseBlock;
    int braceDepth = 0;
    bool inChain = false;

    for (size_t i = minCurrentLine; i < lines.size(); ++i) {
        string line = lines[i];
        smatch match;

        // Detect start of chain
        if (regex_search(line, match, ifRegex) && !inChain) {
            chainStart = i;
            targetVar = match[1];
            caseValues.push_back(match[2]);
            inChain = true;
            caseBlocks.emplace_back();  // Start first case block
        }
            // Detect subsequent else-ifs
        else if (inChain && regex_search(line, match, elseIfRegex)) {
            if (match[1] != targetVar) {
                inChain = false;  // Variable mismatch
                continue;
            }
            caseValues.push_back(match[2]);
            caseBlocks.emplace_back();
        }
            // Detect final else
        else if (inChain && regex_search(line, elseRegex)) {
            caseBlocks.emplace_back();  // Start else block
        }

        // Track code blocks
        if (inChain) {
            // Count opening/closing braces
            braceDepth += count(line.begin(), line.end(), '{');
            braceDepth -= count(line.begin(), line.end(), '}');

            // Add line to current block
            if (!caseBlocks.empty()) {
                caseBlocks.back().push_back(line);
            }

            // End of chain detection
            if (braceDepth == 0 && i > chainStart) {
                // Validate minimum chain length
                if (caseValues.size() >= 2) {
                    SwitchCase sc;
                    sc.startLine = chainStart;
                    sc.endLine = i;
                    sc.variable = targetVar;
                    sc.constants = caseValues;
                    sc.elseLines = elseBlock;
                    switchCases.push_back(sc);

                    // Create error message
                    string msg = "Replace if-else chain with switch (" +
                                 targetVar + ") starting at line " +
                                 to_string(chainStart - minCurrentLine + 1);
                    switchErrors.push_back(msg);
                }
                inChain = false;
                targetVar.clear();
                caseValues.clear();
                caseBlocks.clear();
            }
        }
    }
}
void initializeNewLine(int line, vector<string> &lines){
    lines.push_back("");
    for(int i = 0; i<minCursorPos; i++){
        lines[line] += " ";
    }
}

void renderTopBar(SDL_Renderer* renderer, TTF_Font* font, vector<string> &lines) {
    SDL_Texture* darkModeIconTexture = IMG_LoadTexture(renderer, R"(C:\Users\Erfan\Dev\Cpp\IDE-Project\darkMode.png)");
    SDL_Texture* runIconTexture = IMG_LoadTexture(renderer, R"(C:\Users\Erfan\Dev\Cpp\IDE-Project\run.png)");

    SDL_SetRenderDrawColor(renderer, 0, 80, 160, 255); // Lighter dark blue
    SDL_Rect topBarRect = {0, 0, SCREEN_WIDTH, 50};
    SDL_RenderFillRect(renderer, &topBarRect);
    // Render title text
    SDL_Color textColor = {255, 255, 255, 255};
    string title = (currentProjectName==""?"":currentProjectName+'/')+(currentFileName==""?"Eria IDE-Project 2025":currentFileName);
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, title.c_str(), textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    int textWidth, textHeight;
    SDL_QueryTexture(textTexture, NULL, NULL, &textWidth, &textHeight);

    SDL_Rect textRect = {(SCREEN_WIDTH - textWidth) / 2, (50 - textHeight) / 2, textWidth, textHeight};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);

    SDL_SetTextureColorMod(darkModeIconTexture, 255, 255, 255);
    int iconWidth, iconHeight;
    SDL_QueryTexture(darkModeIconTexture, NULL, NULL, &iconWidth, &iconHeight);
    SDL_Rect darkModeIconRect = { SCREEN_WIDTH - iconWidth - 15, 11, iconWidth-2, iconHeight-2 };
    SDL_RenderCopy(renderer, darkModeIconTexture, NULL, &darkModeIconRect);

    SDL_SetTextureColorMod(runIconTexture, 255, 255, 255);
    SDL_QueryTexture(runIconTexture, NULL, NULL, &iconWidth+5, &iconHeight+5);
    SDL_Rect runIconRect = { SCREEN_WIDTH - iconWidth - 355, 6, iconWidth+6, iconHeight+6 };
    SDL_RenderCopy(renderer, runIconTexture, NULL, &runIconRect);

    SDL_DestroyTexture(darkModeIconTexture);
    SDL_DestroyTexture(runIconTexture);
    SDL_SetRenderDrawColor(renderer, 100, 149, 237, 255);

    errorButtons.clear();
    int c = 50;
    const int errorWidth = 500;
    for(string usedLibrary: usedLibraries){
        if(!binary_search(includedLibraries.begin(), includedLibraries.end(), usedLibrary)){
            errorButtons.push_back(
                    {{SCREEN_WIDTH - errorWidth - 15, 10 + c, errorWidth, 50},
                     7,
                     {255, 59, 48, 255},
                     {255, 255, 255, 255},
                     usedLibrary + " must be included",
                     [&lines, usedLibrary]() {
                         string neededLine = "";
                         for(int i=0; i<minCursorPos; i++){
                             neededLine+=" ";
                         }
                         neededLine += ("#include <" + usedLibrary + ">");
                         lines.insert(lines.begin()+minCurrentLine, neededLine);
                         currentLine++;
                     }
                    });

            c+=57;
        }
    }for (const string& error : bracketErrors) {
        errorButtons.push_back({
                                       {SCREEN_WIDTH - errorWidth - 15, 10 + c, errorWidth, 50},
                                       7,
                                       {255, 59, 48, 255},
                                       {255, 255, 255, 255},
                                       error,
                                       []() {}
                               });
        c += 57;
    }
    for (const string& error : typoErrors) {
        errorButtons.push_back({
                                       {SCREEN_WIDTH - errorWidth - 15, 10 + c, errorWidth, 50},
                                       7,
                                       {255, 149, 0, 255}, 
                                       {255, 255, 255, 255},
                                       error,
                                       []() {}
                               });
        c += 57;
    }
    for(const string& error : semicolonErrors) {
        errorButtons.push_back({
                                       {SCREEN_WIDTH - errorWidth - 15, 10 + c, errorWidth, 50},
                                       7,
                                       {255, 59, 48, 255},
                                       {255, 255, 255, 255},
                                       error,
                                       []() {}
                               });
        c += 57;
    }for (const auto& sc : switchCases) {
        string errorMsg = "Replace if-else with switch on '" + sc.variable + "' at line " + to_string(sc.startLine - minCurrentLine + 1);
        errorButtons.push_back({
                                       {SCREEN_WIDTH - 650 - 15, 10 + c, 650, 50},
                                       7,
                                       {255, 149, 0, 255}, // Orange warning color
                                       {255, 255, 255, 255},
                                       errorMsg,
                                           [&lines, sc]() {
                                               vector<string> newLines;
                                               int iNewLine = 0;

                                               initializeNewLine(iNewLine, newLines);
                                               newLines[iNewLine] += ("switch (" + sc.variable + ") {");
                                               size_t j = sc.startLine + 1;
                                               // Add cases
                                               for (size_t i = 0; i < sc.constants.size(); ++i) {
                                                   iNewLine++;
                                                   initializeNewLine(iNewLine, newLines);
                                                   newLines[iNewLine] += ("    case " + sc.constants[i] + ":");

                                                   // Extract the code block from the if-else chain
                                                   for (j ; j < sc.endLine; ++j) {
                                                       if (lines[j].find("}") != string::npos) {j++; break; } // Stop at closing brace
                                                       iNewLine++;
                                                       newLines.push_back("        " + lines[j]); // Preserve indentation
                                                   }

                                                   iNewLine++;
                                                   initializeNewLine(iNewLine, newLines);
                                                   newLines[iNewLine] += ("        break;");
                                               }

                                               // Add the default case
                                               iNewLine++;
                                               initializeNewLine(iNewLine, newLines);
                                               newLines[iNewLine] += ("    default:");
                                               for (j; j < sc.endLine; ++j) {
                                                   if (lines[j].find("}") != string::npos) break; // Stop at closing brace
                                                   iNewLine++;
                                                   newLines.push_back("        " + lines[j]); // Preserve indentation
                                               }
                                               iNewLine++;
                                               initializeNewLine(iNewLine, newLines);
                                               newLines[iNewLine] += ("}");

                                               // Replace the if-else block with the switch statement
                                               lines.erase(lines.begin() + sc.startLine, lines.begin() + sc.endLine + 1);
                                               lines.insert(lines.begin() + sc.startLine, newLines.begin(), newLines.end());
                                           }
                               });
        c += 57;
    }

    for (auto& button : buttons) {
        button.render(renderer, font);
    }
    for (auto& button : errorButtons) {
        button.render(renderer, font);}
    if(!isMenuOpen){
        for (auto& button : projectsButtons) {
            button.render(renderer, font);}
    }else{
        SDL_Color  bgColor = darkMode? SDL_Color{0, 0, 0, 255}:SDL_Color {255, 255, 255, 255};
        for (auto& button : menuBarButtons) {
            button.render(renderer, font);
        }if(isEditOpen){
            for (auto& button : editButtons) {
                button.render(renderer, font);
            }
        }
    }
}

void runCode(SDL_Renderer* renderer, TTF_Font* font){
    if (currentProjectName.empty() || currentFileName.empty()) {
        saveAsToFile(lines, renderer, font);
    }else{
        saveToFile(lines, renderer, font);
    }

    // Build paths
    string sourcePath = path + currentProjectName + "/" + currentFileName;
    string outputPath = path + currentProjectName + "/output.exe";

    // Compile
    if (compile(sourcePath, outputPath)) {
        run_in_another_window(outputPath);
    } else {
        bracketErrors.push_back("Compilation failed! Check errors.");
    }
}

bool handleDarkModeIconClicks(SDL_Event* event) {
    if (event->type == SDL_MOUSEBUTTONDOWN) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        // Check if the mouse click is inside the bounds of the icon
        if (mouseX >= SCREEN_WIDTH - 50 && mouseX <= SCREEN_WIDTH - 10 &&
            mouseY >= 0 && mouseY <= 50) {
            darkMode = !darkMode;
        }
    }
    return true;
}
bool handleRunIconClicks(SDL_Event* event, SDL_Renderer* renderer, TTF_Font* font) {
    if (event->type == SDL_MOUSEBUTTONDOWN) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        // Check if the mouse click is inside the bounds of the icon
        if (mouseX >= SCREEN_WIDTH - 50 - 355 && mouseX <= SCREEN_WIDTH - 355 &&
            mouseY >= 0 && mouseY <= 50) {
            runCode(renderer, font);
        }
    }
    return true;
}

void checkLibrariesInclude(const vector<string>& lines) {
    includedLibraries.clear();
    for (const auto& line : lines) {
        for(auto pair: libraries){
            if (line.find("#include <"+pair.first+">") != string::npos || line.find("#include<"+pair.first+">") != string::npos) {
                includedLibraries.push_back(pair.first);
                break;
            }
        }
    }
}
void checkLibrariesUsage(const vector<string>& lines) {
    usedLibraries.clear();
    regex wordRegex(R"(\b\w+\b)"); // Matches words (sequences of letters, digits, or underscores)

    for (const string& line : lines) {
        smatch match;
        string::const_iterator searchStart(line.cbegin());

        while (regex_search(searchStart, line.cend(), match, wordRegex)) {
            string word = match.str();

            for (const auto& pair : libraries) {
                for (const string& funcName : pair.second) {
                    if (word == funcName) {
                        if (find(usedLibraries.begin(), usedLibraries.end(), pair.first) == usedLibraries.end()) {
                            usedLibraries.push_back(pair.first);
                        }
                        break;
                    }
                }
            }

            searchStart = match.suffix().first;
        }
    }
}
void checkUnclosedCharacters(const vector<string>& lines, vector<string>& errors) {
    bracketErrors.clear();
    ParseState state;

    for (size_t lineNum = 0; lineNum < lines.size(); ++lineNum) {
        const string& line = lines[lineNum];
        for (size_t i = 0; i < line.size(); ++i) {
            char c = line[i];

            if (state.escapeNext) {
                state.escapeNext = false;
                continue;
            }

            // Check if inside a multi-line comment
            if (state.inMultiLineComment) {
                if (c == '*' && i + 1 < line.size() && line[i + 1] == '/') {
                    state.inMultiLineComment = false;
                    i++; // Skip the '/' character
                }
                continue; // Skip processing other characters inside comment
            }

            if (c == '\\') {
                state.escapeNext = true;
                continue;
            }

            if (c == '"') {
                if (!state.inQuote) {
                    state.inQuote = true;
                    state.quoteStartLine = lineNum + 1;
                } else {
                    state.inQuote = false;
                }
            }

            if (state.inQuote) continue;

            // Check for start of multi-line comment
            if (c == '/' && i + 1 < line.size() && line[i + 1] == '*') {
                state.inMultiLineComment = true;
                state.commentStartLine = lineNum + 1;
                i++; // Skip the '*' character
                continue;
            }

            // Handle brackets
            if (c == '(' || c == '{' || c == '[') {
                state.bracketStack.push({c, lineNum + 1});
            }
            else if (c == ')' || c == '}' || c == ']') {
                if (state.bracketStack.empty()) {
                    errors.push_back("Extra '" + string(1, c) + "' at line " + to_string(lineNum + 1 - 3));
                } else {
                    char expected = '\0';
                    switch(c) {
                        case ')': expected = '('; break;
                        case '}': expected = '{'; break;
                        case ']': expected = '['; break;
                    }

                    auto& top = state.bracketStack.top();
                    char actual = top.first;
                    size_t openLine = top.second;

                    if (actual != expected) {
                        errors.push_back("Mismatched '" + string(1, c) + "' at line " +
                                         to_string(lineNum + 1 - 3));
                    }
                    state.bracketStack.pop();
                }
            }
        }
    }

    // Check for unclosed brackets
    while (!state.bracketStack.empty()) {
        auto& top = state.bracketStack.top();
        char bracket = top.first;
        size_t openLine = top.second;
        state.bracketStack.pop();
        errors.push_back("Unclosed '" + string(1, bracket) + "' opened at line " +
                         to_string(openLine - 3));
    }

    // Check for unclosed quote
    if (state.inQuote) {
        errors.push_back("Unclosed '\"' started at line " + to_string(state.quoteStartLine - 3));
    }

    // Check for unclosed multi-line comment
    if (state.inMultiLineComment) {
        errors.push_back("Unclosed comment started at line " + to_string(state.commentStartLine - 3));
    }
}
void checkMissingSemicolons(const vector<string>& lines) {
    semicolonErrors.clear();
    bool inMultiLineComment = false;
    bool inString = false;
    bool escape = false;

    for(size_t lineNum = minCurrentLine; lineNum < lines.size(); lineNum++) {
        const string& line = lines[lineNum];
        bool hasCode = false;
        bool hasSemicolon = false;
        bool isPreprocessor = false;
        bool isBracketOnly = false;

        // Skip empty lines
        if(line.empty()) continue;

        for(size_t i = 0; i < line.size(); i++) {
            char c = line[i];

            // Handle escape sequences in strings
            if(escape) {
                escape = false;
                continue;
            }
            if(c == '\\') {
                escape = true;
                continue;
            }

            // Handle strings
            if(c == '"' && !inMultiLineComment) {
                inString = !inString;
                continue;
            }
            if(inString) continue;

            // Handle comments
            if(c == '/' && i+1 < line.size()) {
                if(line[i+1] == '/') break; // Single-line comment
                if(line[i+1] == '*') inMultiLineComment = true;
            }
            if(c == '*' && i+1 < line.size() && line[i+1] == '/') {
                inMultiLineComment = false;
                i++;
                continue;
            }
            if(inMultiLineComment) continue;

            // Check for preprocessor directives
            if(c == '#') {
                isPreprocessor = true;
                break;
            }

            // Check for meaningful code
            if(!isspace(c)) hasCode = true;
            if(c == ';') hasSemicolon = true;
            if(c == '{' || c == '}') isBracketOnly = true;
        }

        // Skip lines that shouldn't have semicolons
        if(isPreprocessor || isBracketOnly || !hasCode) continue;

        // Check for missing semicolon
        if(!hasSemicolon) {
            semicolonErrors.push_back("Missing semicolon at line " +
                                      to_string(lineNum + 1 - minCurrentLine));
        }
    }
}
void checkUndefinedOperators(const vector<string>& lines, vector<string>& errors) {
    regex undefinedOperatorPattern(R"((\+\+\+|\=\=\+|\+\=\=|\-\-\-|\*\*\*|\/\/\/|\%\=\=|\&\&\&|\|\|\||\<\<\<|\>\>\>|\=\=\=|\!\=\=|\<\=\=|\>\=\=|\+\=\=|\-\=\=|\*\=\=|\/\=\=|\%\=\=|\&\=\=|\|\=\=))");

    for (size_t lineNum = minCurrentLine; lineNum < lines.size(); ++lineNum) {
        const string& line = lines[lineNum];
        smatch match;

        if (regex_search(line, match, undefinedOperatorPattern)) {
            errors.push_back("Undefined operator '" + match.str() + "' at line " + to_string(lineNum + 1 - minCurrentLine));
        }
    }
}
void checkTypos(const vector<string>& lines, vector<string>& typoErrors) {
    typoErrors.clear();
    regex wordRegex(R"(\b\w+\b)");

    for (size_t lineNum = minCurrentLine; lineNum < lines.size(); ++lineNum) {
        const string& line = lines[lineNum];
        smatch match;
        string::const_iterator searchStart(line.cbegin());

        while (regex_search(searchStart, line.cend(), match, wordRegex)) {
            string word = match.str();
            searchStart = match.suffix().first;

            // Skip valid keywords and library functions
            if (find(keywords.begin(), keywords.end(), word) != keywords.end()) continue;

            // Find best match
            string bestMatch;
            float bestScore = 0.0f;

            for (const string& keyword : keywords) {
                int m = word.size(), n = keyword.size();
                vector<int> current(n + 1, 0), previous(n + 1, 0);
                for (int i = 1; i <= m; ++i) {
                    swap(current, previous);
                    for (int j = 1; j <= n; ++j) {
                        current[j] = (word[i-1] == keyword[j-1])
                                     ? previous[j-1] + 1
                                     : max(previous[j], current[j-1]);
                    }
                }

                float similarity = (2.0f * current[n]) / (word.length() + keyword.length());

                if (similarity > bestScore && similarity >= 0.7f) {
                    bestScore = similarity;
                    bestMatch = keyword;
                }
            }

            if (!bestMatch.empty()) {
                typoErrors.push_back("Did you mean '" + bestMatch + "' at line " +
                                     to_string(lineNum + 1 - minCurrentLine) + "?");
            }
        }
    }
}

int getTextWidthUpTo(int position, TTF_Font* font, vector<string> &lines) {
    int width = 0;
    if (position > 0) {
        TTF_SizeText(font, lines[currentLine].substr(0, position).c_str(), &width, nullptr);
    }
    return width + 10; // Add the padding for the left margin
}

void undo(){
    lines = linesHistory[linesHistory.size()-2];
    cursorPos = minCursorPos;
    currentLine = minCurrentLine;
    futureLines.push_back(linesHistory[linesHistory.size()-1]);
    linesHistory.pop_back();
}
void redo(){
    if(!futureLines.empty()){
        lines = futureLines[futureLines.size()-1];
        linesHistory.push_back(futureLines[futureLines.size()-1]);
        futureLines.pop_back();
        cursorPos = minCursorPos;
        currentLine = minCurrentLine;
    }
}

void initializeEditor(){
    lines = {""};
    for(int i=0; i<minCurrentLine; i++){
        lines.push_back("");
    }
    for(int i=0; i<minCursorPos; i++){
        lines[minCurrentLine].push_back(' ');
    }
}

void openFile(vector<string>& lines, SDL_Renderer* renderer, TTF_Font* font,const string &fileName,const string &projectName){
    if(currentFileName != "" && currentProjectName != ""){
        saveToFile(lines, renderer, font);
    }
    currentProjectName = projectName;
    currentFileName = fileName;
    initializeEditor();
    cursorPos = minCursorPos;
    currentLine = minCurrentLine;
    ifstream file(path+currentProjectName+'/'+currentFileName);
    if (!file) {
        printf("Error opening file: %s\n", (path+currentProjectName+'/'+currentFileName).c_str());
        return;
    }
    string line;
    int iLine = 3;
    while (getline(file, line)) {
        if(iLine > 3) {
            lines.push_back("");
            for (int i = 0; i < minCursorPos; i++) {
                lines[iLine].push_back(' ');
            }
        }lines[iLine]+=line;
        iLine++;
    }
    file.close();
}

void initializeProjectsButtons(SDL_Renderer* renderer, TTF_Font* font){
    projects = getProjects();
    if(currentProjectName!=""){
        files = getFiles(currentProjectName);
    }
    projectsButtons.clear();
    int c = 0;
    int g = 0;
    for(string project: projects){
        c+=58;
        if(currentProjectName == project){
            projectsButtons.push_back({
                                              {5, c+g, 200, 50},
                                              20,
                                              SDL_Color{0, 80, 160, 255}, // Lighter dark blue,
                                              {255, 255, 255, 255},
                                              project,[&renderer, &font, project]() {}}
            );
            for(string file: files){
                g+=53;
                projectsButtons.push_back({
                                                  {5, c+g, 200, 50},
                                                  20,
                                                  SDL_Color{100, 149, 237, 255},
                                                  {255, 255, 255, 255},
                                                  file,[&renderer, &font ,file]() {
                            openFile(lines, renderer, font, file, currentProjectName);
                        }}
                );
            }
        }else{
            projectsButtons.push_back({
                                              {5, c+g, 200, 50},
                                              20,
                                              SDL_Color{0, 80, 160, 255},
                                              {255, 255, 255, 255},
                                              project,[&renderer, &font, project]() {
                        openFile(lines, renderer, font, project+".cpp", project);
                        initializeProjectsButtons(renderer, font);
                    }}
            );
        }
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
        return -1;
    }if (TTF_Init() == -1) {
        cerr << "TTF could not initialize! TTF_Error: " << TTF_GetError() << endl;
        SDL_Quit();
        return -1;
    }
    SDL_Window* window = SDL_CreateWindow("Eria IDE-Project 2025",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,SCREEN_WIDTH,SCREEN_HEIGHT,SDL_WINDOW_SHOWN);
    if (!window) {
        cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
        TTF_Quit();
        SDL_Quit();
        return -1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }
    SDL_Surface* icon = IMG_Load((path+"icon.png").c_str());
    if (icon) {
        SDL_SetWindowIcon(window, icon);
        SDL_FreeSurface(icon);
    }
    TTF_Font* font = TTF_OpenFont(R"(C:\Windows\Fonts\consola.ttf)", 24);
    if (!font) {
        cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }


    for (auto it = libraries.begin(); it != libraries.end(); ++it) {
        for(string func: it->second){
            autoCompleteWords.push_back(func);
        }
    }
    keywords = autoCompleteWords;
    sort(keywords.begin(), keywords.end(), [](const string& a, const string& b) {
        return a.length() > b.length();
    });

    initializeEditor();
    linesHistory.push_back(lines);
    int scrollOffset = 0; // Keeps track of scrolling
    const int LINE_HEIGHT = TTF_FontHeight(font); // Height of each line

    // Timer for cursor blinking
    Uint32 lastCursorToggle = SDL_GetTicks();
    bool cursorVisible = true;
    const Uint32 CURSOR_BLINK_INTERVAL = 500;

    buttons.push_back({
                              {SCREEN_WIDTH-175, 7, 100, 36},
                              15,
                              {0, 122, 255, 255},  // Modern blue background
                              {255, 255, 255, 255}, // White text
                              "Save",
                              [&renderer, &font]() { saveToFile(lines, renderer, font); }
                      });

    buttons.push_back({
                              {SCREEN_WIDTH-325, 7, 125, 36},
                              15,
                              {0, 122, 255, 255},  // Modern blue background
                              {255, 255, 255, 255}, // White text
                              "Save As",
                              [&renderer, &font]() { saveAsToFile(lines, renderer, font); }
                      });
    buttons.push_back(
            {{40, 7, 124, 36},
             15,
             {255, 255, 255, 255},
             {0, 0, 0, 255},
             "MENU",
             []() {
                 isMenuOpen = !isMenuOpen;
                 if(!isMenuOpen){
                     isEditOpen = false;
                 }
             }});
    menuBarButtons.push_back(
            {{10, 10+50*1, 186, 36},
             15,
             {0, 122, 255, 255},  // Modern blue background
             {255, 255, 255, 255},
             "New Project",
             [&renderer, &font]() {
                 if(currentFileName != "" && currentProjectName != ""){
                     saveToFile(lines, renderer, font);
                     initializeEditor();
                     cursorPos = minCursorPos;
                     currentLine = minCurrentLine;
                 }
                 newProject(renderer, font,  "");
                 currentFileName = currentProjectName;
                 saveToFile(lines ,renderer, font);
             }});
    menuBarButtons.push_back(
            {{10, 10+50*2, 186, 36},
             15,
             {0, 122, 255, 255},  // Modern blue background
             {255, 255, 255, 255},
             "Save Project",
             [&renderer, &font]() {
                 saveAsToFile(lines, renderer, font);
             }});
    menuBarButtons.push_back(
            {{10, 10+50*3, 186, 36},
             15,
             {0, 122, 255, 255},  // Modern blue background
             {255, 255, 255, 255},
             "Debug&Compile",
             [&renderer, &font]() {// Capture everything by reference
                 runCode(renderer, font);
             }});
    menuBarButtons.push_back(
            {{10, 10+50*4, 186, 36},
             15,
             {0, 122, 255, 255},  // Modern blue background
             {255, 255, 255, 255},
             "Edit",
             []() {
                 isEditOpen = !isEditOpen;
             }});
    editButtons.push_back(
            {{204, 10+50*4-21, 125, 36},
             15,
             {255, 59, 48, 222},
             {255, 255, 255, 255},
             "Undo",
             []() {
                 undo();
             }});
    editButtons.push_back(
            {{204, 10+50*4+21, 125, 36},
             15,
             {255, 59, 48, 222},
             {255, 255, 255, 255},
             "Redo",
             []() {
                 redo();
             }});
    menuBarButtons.push_back(
            {{10, 10+50*5, 186, 36},
             15,
             {0, 122, 255, 255},  // Modern blue background
             {255, 255, 255, 255},
             "Dark/Light",
             []() {
                 darkMode = !darkMode;
             }});
    menuBarButtons.push_back(
            {{10, 10+50*6, 186, 36},
             15,
             {0, 122, 255, 255},  // Modern blue background
             {255, 255, 255, 255},
             "Exit",
             [&renderer, &font, &window]() {
                 TTF_CloseFont(font);
                 SDL_DestroyRenderer(renderer);
                 SDL_DestroyWindow(window);
                 TTF_Quit();
                 SDL_Quit();

                 return 0;
             }});

    SDL_Texture* lightBgTexture = IMG_LoadTexture(renderer, (path + "lightbg.jpg").c_str());
    SDL_Texture* darkBgTexture = IMG_LoadTexture(renderer, (path + "darkbg.jpg").c_str());
    initializeProjectsButtons(renderer, font);
    renderTopBar(renderer, font, lines);

    // Pressed states
    bool ctrlPressed = false, shiftPressed = false;

    // Selection states
    int selectionStart = -1, selectionEnd = -1, selectionStartLine = -1, selectionEndLine = -1;

    SDL_Event e;
    bool quit = false;
    while (!quit) {
        SDL_StartTextInput();

        // Handle cursor blinking
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime > lastCursorToggle + CURSOR_BLINK_INTERVAL) {
            cursorVisible = !cursorVisible;
            lastCursorToggle = currentTime;
        }

        // Event loop
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_MOUSEWHEEL) {
                // Handle scroll
                if (e.wheel.y > 0) { // Scroll up
                    scrollOffset = max(0, scrollOffset - LINE_HEIGHT);
                } else if (e.wheel.y < 0) { // Scroll down
                    scrollOffset += LINE_HEIGHT;
                }
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if(!shiftPressed){
                    selectionStart = -1;
                    selectionEnd = -1;
                    selectionStartLine = -1;
                    selectionEndLine = -1;
                }
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                for (auto& button : buttons) {
                    if (button.isClicked(mouseX, mouseY)) {
                        button.action();
                    }
                }
                for (auto& button : errorButtons) {
                    if (button.isClicked(mouseX, mouseY)) {
                        button.action();
                    }
                }
                if(isMenuOpen){
                    for (auto& button : menuBarButtons) {
                        if (button.isClicked(mouseX, mouseY)) {
                            // isMenuOpen = false;
                            if(button.label!="Edit"){
                                isEditOpen = false;
                            }
                            button.action();
                        }
                        if(isEditOpen){
                            for (auto& button : editButtons) {
                                if (button.isClicked(mouseX, mouseY)) {
                                    isEditOpen = false;
                                    button.action();
                                }
                            }}
                    }}else {
                    for (auto& button : projectsButtons) {
                        if (button.isClicked(mouseX, mouseY)) {
                            button.action();
                        }
                    }}
            }else if (e.type == SDL_KEYDOWN) {
                SDL_Keycode key = e.key.keysym.sym;

                    // Track Control and Shift key states
                if (e.key.keysym.sym == SDLK_LCTRL || e.key.keysym.sym == SDLK_RCTRL) {
                    ctrlPressed = true;
                } else if (e.key.keysym.sym == SDLK_LSHIFT || e.key.keysym.sym == SDLK_RSHIFT) {
                    shiftPressed = true;
                } else if (e.key.keysym.sym == SDLK_s) {
                    // Check for Control + S
                    if (ctrlPressed && !shiftPressed) {
                        saveToFile(lines, renderer, font);
                    }
                    // Check for Control + Shift + S
                    if (ctrlPressed && shiftPressed) {
                        if (ctrlPressed && shiftPressed) {
                            saveAsToFile(lines, renderer, font);
                        }
                    }
                }else if(e.key.keysym.sym == SDLK_v && ctrlPressed){
                    if (SDL_HasClipboardText()) {
                        char* clipboardText = SDL_GetClipboardText();
                        if (clipboardText) {
                            stringstream ss(clipboardText);
                            string line;
                            int g = 0;
                            while (getline(ss, line, '\n')) {
                                // Remove carriage return characters
                                line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

                                if(g>0){
                                    lines.push_back("");
                                    currentLine++;
                                    cursorPos = 0;
                                    for(cursorPos = 0; cursorPos<minCursorPos; cursorPos++){
                                        lines[currentLine].insert(cursorPos, " ");
                                    }
                                }
                                lines[currentLine].insert(cursorPos, line);
                                cursorPos += line.length();
                                g++;
                            }
                            SDL_free(clipboardText);
                        }
                    }
                }else if(e.key.keysym.sym == SDLK_c && ctrlPressed){
                    if(selectionStart != -1){
                        if(selectionStartLine == selectionEndLine){
                            if(selectionStart<selectionEnd){
                                if (SDL_SetClipboardText(lines[selectionStartLine].substr(selectionStart, abs(selectionStart-selectionEnd)).c_str()) == 0) {
                                    printf("Text copied to clipboard successfully!");
                                } else {
                                    printf("Failed to copy text to clipboard: %s\n", SDL_GetError());
                                }
                            }else{
                                if (SDL_SetClipboardText(lines[selectionStartLine].substr(selectionEnd, abs(selectionStart-selectionEnd)).c_str()) == 0) {
                                    printf("Text copied to clipboard successfully!");
                                } else {
                                    printf("Failed to copy text to clipboard: %s\n", SDL_GetError());
                                }
                            }
                        }else {
                            string selectedText;
                            // Copy text from the start line
                            selectedText += lines[selectionStartLine].substr(selectionStart);
                            // Copy full lines in between
                            for (int i = selectionStartLine + 1; i < selectionEndLine; ++i) {
                                selectedText += "\n" + lines[i];
                            }
                            // Copy text from the end line
                            selectedText += "\n" + lines[selectionEndLine].substr(0, selectionEnd);

                            if (SDL_SetClipboardText(selectedText.c_str()) == 0) {
                                printf("Text copied to clipboard successfully!");
                            } else {
                                printf("Failed to copy text to clipboard: %s\n", SDL_GetError());
                            }
                        }
                    }
                }else if(e.key.keysym.sym == SDLK_x && ctrlPressed) {
                    if (selectionStart != -1) {
                        if(selectionStartLine == selectionEndLine){
                            if (selectionStart < selectionEnd) {
                                if (SDL_SetClipboardText(lines[selectionStartLine].substr(selectionStart, abs(selectionStart - selectionEnd)).c_str()) == 0) {
                                    printf("Text copied to clipboard successfully!");
                                    lines[currentLine].erase(selectionStart, abs(selectionEnd - selectionStart));
                                    cursorPos-=abs(selectionEnd - selectionStart);
                                } else {
                                    printf("Failed to copy text to clipboard: %s\n", SDL_GetError());
                                }
                            } else {
                                if (SDL_SetClipboardText(lines[selectionStartLine].substr(selectionEnd, abs(selectionStart - selectionEnd)).c_str()) == 0) {
                                    printf("Text copied to clipboard successfully!");
                                    lines[currentLine].erase(selectionEnd, abs(selectionEnd - selectionStart));
                                } else {
                                    printf("Failed to copy text to clipboard: %s\n", SDL_GetError());
                                }
                            }
                        } else {
                            string selectedText;
                            // Copy text from the start line
                            selectedText += lines[selectionStartLine].substr(selectionStart);
                            // Copy full lines in between
                            for (int i = selectionStartLine + 1; i < selectionEndLine; ++i) {
                                selectedText += "\n" + lines[i];
                            }
                            // Copy text from the end line
                            selectedText += "\n" + lines[selectionEndLine].substr(minCursorPos, selectionEnd);

                            if (SDL_SetClipboardText(selectedText.c_str()) == 0) {
                                printf("Text copied to clipboard successfully!");

                                // Erase selected text from lines
                                lines[selectionStartLine].erase(selectionStart, lines[selectionStartLine].length() - selectionStart);
                                for (int i = selectionStartLine + 1; i < selectionEndLine; ++i) {
                                    lines[i].clear(); // Clear the entire line
                                }
                                lines[selectionEndLine].erase(minCursorPos, selectionEnd);

                                cursorPos = selectionStartLine; // Or update as needed
                            } else {
                                printf("Failed to copy text to clipboard: %s\n", SDL_GetError());
                            }
                        }
                        selectionStart = -1;
                        selectionEnd = -1;
                        selectionStartLine = -1;
                        selectionEndLine = -1;
                    }

                }else if(e.key.keysym.sym == SDLK_z && ctrlPressed){
                    undo();
                }else if(e.key.keysym.sym == SDLK_y && ctrlPressed){
                    redo();
                }else if(e.key.keysym.sym == SDLK_a && ctrlPressed){
                    selectionStart = minCursorPos;
                    selectionEnd = lines[lines.size()-1].size();
                    selectionStartLine = minCurrentLine;
                    selectionEndLine = lines.size();
                }
                else if(e.key.keysym.sym == SDLK_g && ctrlPressed){
                    string lineStr;
                    getNameFromUser(renderer, font, lineStr, "line number");
                    currentLine = stoi(lineStr) + minCurrentLine - 1;
                    cursorPos = minCursorPos;
                    ensureLastLineVisible(currentLine, scrollOffset, SCREEN_HEIGHT, LINE_HEIGHT, lines.size());
                }
                else if (e.key.keysym.sym == SDLK_BACKSPACE) {
                    // Ensure cursorPos is within the valid range
                    if (cursorPos > minCursorPos && cursorPos <= lines[currentLine].size()) {
                        // Remove character before cursor
                        lines[currentLine].erase(cursorPos - 1, 1);
                        cursorPos--;
                    } else if (currentLine > minCurrentLine) {
                        // Merge with previous line
                        cursorPos = lines[currentLine - 1].size();
                        lines[currentLine - 1] += lines[currentLine].substr(minCursorPos, lines[currentLine].size());
                        lines.erase(lines.begin() + currentLine);
                        currentLine--;
                    }
                    // Ensure there's always at least one line
                    if (lines.size()<minCurrentLine) {
                        for(int i=0; i<minCurrentLine; i++){
                            lines.push_back("");
                        }
                        currentLine = 0;
                        cursorPos = 0;
                    }
                } else if (e.key.keysym.sym == SDLK_RETURN) {
                    if (cursorPos <= lines[currentLine].size()) {
                        string remainder = lines[currentLine].substr(cursorPos);
                        lines[currentLine] = lines[currentLine].substr(0, cursorPos);
                        for(int i=0; i<minCursorPos; i++){
                            remainder = " " + remainder;
                        }
                        lines.insert(lines.begin() + currentLine + 1, remainder);
                        currentLine++;
                        for(int i=0; i<minCursorPos; i++){
                            lines[currentLine].push_back(' ');
                        }
                        cursorPos = minCursorPos;
                        ensureLastLineVisible(currentLine, scrollOffset, SCREEN_HEIGHT, LINE_HEIGHT, lines.size());
                    }
                }
                else if (e.key.keysym.sym == SDLK_TAB) {
                    string currentLineText = lines[currentLine];
                    int startPos = cursorPos - 1;
                    // Find the start of the current word
                    while (startPos >= 0 && !isspace(currentLineText[startPos])) {
                        startPos--;
                    }
                    startPos++; // Adjust to the start of the word
                    string partial = currentLineText.substr(startPos, cursorPos - startPos);

                    vector<string> candidates;
                    for (const auto& kw : autoCompleteWords) {
                        if (!partial.empty() && kw.find(partial) == 0) { // Check if partial is a prefix
                            if (partial.length() >= (kw.length() / 2)) { // Check if more than half typed
                                candidates.push_back(kw);
                            }
                        }
                    }
                    if (!candidates.empty()) {
                        // Find the shortest candidate to autocomplete
                        string completion = *min_element(candidates.begin(), candidates.end(),[](const string& a, const string& b) { return a.length() < b.length(); });
                        // Replace the partial word with the completion
                        lines[currentLine].erase(startPos, partial.length());
                        lines[currentLine].insert(startPos, completion);
                        cursorPos = startPos + completion.length();
                    } else {
                        // Default to inserting 4 spaces if no completion
                        lines[currentLine].insert(cursorPos, "    ");
                        cursorPos += 4;
                    }
                } else if(e.key.keysym.sym == SDLK_LEFT || e.key.keysym.sym == SDLK_RIGHT || e.key.keysym.sym == SDLK_UP || e.key.keysym.sym == SDLK_DOWN){
                    if(!shiftPressed){
                        selectionStart = -1; selectionStartLine = -1;
                        selectionEnd = -1; selectionEndLine = -1;
                    }
                    if (e.key.keysym.sym == SDLK_LEFT) {
                        if (shiftPressed) {
                            // Extend selection to the left
                            if (selectionStart == -1) {
                                selectionStart = cursorPos;
                                selectionStartLine = currentLine;
                            }
                        }
                        if (cursorPos > minCursorPos) {
                            cursorPos--;
                        } else if (currentLine > minCurrentLine) {
                            currentLine--;
                            cursorPos = lines[currentLine].size();
                        }
                        if (shiftPressed) {
                            selectionEnd = cursorPos;
                            selectionEndLine = currentLine;
                        }
                    }
                    else if (e.key.keysym.sym == SDLK_RIGHT) {
                        if (shiftPressed) {
                            // Extend selection to the right
                            if (selectionStart == -1) {
                                selectionStart = cursorPos;
                                selectionStartLine = currentLine;
                            }
                        }
                        if (cursorPos < lines[currentLine].size()) {
                            cursorPos++;
                        } else if (currentLine < lines.size() - 1) {
                            currentLine++;
                            cursorPos = minCursorPos;
                        }
                        if (shiftPressed) {
                            selectionEnd = cursorPos;
                            selectionEndLine = currentLine;
                        }
                    }
                    else if (e.key.keysym.sym == SDLK_UP) {
                        if (shiftPressed) {
                            if (selectionStart == -1) {
                                selectionStart = cursorPos;
                                selectionStartLine = currentLine;
                            }
                        }
                        if (currentLine > minCurrentLine) {
                            currentLine--;
                            cursorPos = min(cursorPos, (int)lines[currentLine].size());
                            ensureLastLineVisible(currentLine, scrollOffset, SCREEN_HEIGHT, LINE_HEIGHT, lines.size());
                        }
                        if (shiftPressed) {
                            selectionEnd = cursorPos;
                            selectionEndLine = currentLine;
                        }
                    }
                    else if (e.key.keysym.sym == SDLK_DOWN) {
                        if (shiftPressed) {
                            // Extend selection downwards
                            if (selectionStart == -1) {
                                selectionStart = cursorPos;
                                selectionStartLine = currentLine;
                            }
                            if (currentLine < lines.size() - 1) {
                                currentLine++;
                                cursorPos = min(cursorPos, (int)lines[currentLine].size());
                            }
                            selectionEnd = cursorPos;
                            selectionEndLine = currentLine;
                        } else {
                            // Regular cursor movement
                            if (currentLine < lines.size() - 1) {
                                currentLine++;
                                cursorPos = min(cursorPos, (int)lines[currentLine].size());
                                ensureLastLineVisible(currentLine, scrollOffset, SCREEN_HEIGHT, LINE_HEIGHT, lines.size());
                            }
                        }
                    }}}
            else if (e.type == SDL_KEYUP) {
                // Reset key states when keys are released
                if (e.key.keysym.sym == SDLK_LCTRL || e.key.keysym.sym == SDLK_RCTRL) {
                    ctrlPressed = false;
                } else if (e.key.keysym.sym == SDLK_LSHIFT || e.key.keysym.sym == SDLK_RSHIFT) {
                    shiftPressed = false;
                }
            }else if (e.type == SDL_TEXTINPUT) {
                if (e.text.text) {
                    string inputText = e.text.text;

                    if (inputText == "(") {
                        lines[currentLine].insert(cursorPos, "()");
                        cursorPos++;
                    } else if (inputText == "{") {
                        lines[currentLine].insert(cursorPos, "{}");
                        cursorPos++;
                    } else if (inputText == "[") {
                        lines[currentLine].insert(cursorPos, "[]");
                        cursorPos++;
                    } else {
                        lines[currentLine].insert(cursorPos, inputText);
                        cursorPos += inputText.length();
                    }
                    ensureLastLineVisible(currentLine, scrollOffset, SCREEN_HEIGHT, LINE_HEIGHT, lines.size());
                }
            }
            handleDarkModeIconClicks(&e);
            handleRunIconClicks(&e, renderer, font);
            if(linesHistory[linesHistory.size()-1] != lines){
                linesHistory.push_back(lines);
                futureLines.clear();
            }
            if(selectionStart != -1){
                cout << endl << selectionStart << " " << selectionEnd << " " << selectionStartLine << " " << selectionEndLine;
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, darkMode ? darkBgTexture : lightBgTexture, NULL, NULL);

        // Render text
        int y = -scrollOffset; // Start rendering based on the scroll offset
        for (size_t i = 0; i < lines.size(); ++i) {
            if (y + LINE_HEIGHT > 0 && y < SCREEN_HEIGHT) { // Render only visible lines
                if (lines[i].empty()) {
                    lines[i] = " "; // Show cursor on the current line
                }

                // Get highlighted segments for the line
                auto highlightedSegments = highlightLine(lines[i]);

                int x = 10; // Starting X position
                for (const auto& segment : highlightedSegments) {
                    SDL_Surface* textSurface = TTF_RenderText_Blended(font, segment.first.c_str(), segment.second);
                    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

                    int textWidth = textSurface->w;
                    int textHeight = textSurface->h;
                    SDL_Rect renderQuad = {x, y, textWidth, textHeight};

                    SDL_FreeSurface(textSurface);

                    SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);
                    SDL_DestroyTexture(textTexture);

                    x += textWidth;
                }

                // Render cursor if this is the current line
                if (i == currentLine && cursorVisible) {
                    int cursorX = 0;
                    if (cursorPos > 0) {
                        TTF_SizeText(font, lines[i].substr(0, cursorPos).c_str(), &cursorX, nullptr);
                    }
                    cursorX += 10; // Add padding for the left margin
                    if (darkMode) {
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White cursor in dark mode
                    } else {
                        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black cursor in light mode
                    }
                    SDL_RenderDrawLine(renderer, cursorX, y, cursorX, y + LINE_HEIGHT);
                }
            }
            y += LINE_HEIGHT; // Move to the next line
        }

        checkLibrariesUsage(lines);
        checkLibrariesInclude(lines);
        checkUnclosedCharacters(lines, bracketErrors);
        checkMissingSemicolons(lines);
        checkUndefinedOperators(lines, bracketErrors);
        checkTypos(lines, typoErrors);
        checkSwitchOpportunities(lines);
        renderTopBar(renderer, font, lines);

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 100);

        vector<SDL_Rect> selectedHighlight;
        int linesLength = abs(selectionStartLine - selectionEndLine);
        if(selectionStartLine == selectionEndLine){
            selectedHighlight.push_back({
                                                getTextWidthUpTo(selectionStart, font, lines),
                                                50 + LINE_HEIGHT * (selectionStartLine - minCurrentLine + 1) - 2,
                                                getTextWidthUpTo(selectionEnd, font, lines) - getTextWidthUpTo(selectionStart, font, lines),
                                                LINE_HEIGHT + 2
                                        });
        }else if(selectionStartLine < selectionEndLine){
            for(int i=0; i<=linesLength; i++){
                if(i==0){
                    selectedHighlight.push_back({
                                                        getTextWidthUpTo(selectionStart, font, lines),
                                                        50 + LINE_HEIGHT * (selectionStartLine - minCurrentLine + 1) - 2,
                                                        SCREEN_WIDTH,
                                                        LINE_HEIGHT + 2
                                                });
                }else if(i==linesLength){
                    selectedHighlight.push_back({
                                                        getTextWidthUpTo(minCursorPos, font, lines),
                                                        50 + LINE_HEIGHT * (selectionEndLine - minCurrentLine + 1) - 2,
                                                        getTextWidthUpTo(selectionEnd- minCursorPos-1, font, lines),
                                                        LINE_HEIGHT + 2
                                                });
                }else{
                    selectedHighlight.push_back({
                                                        getTextWidthUpTo(minCursorPos, font, lines),
                                                        50 + LINE_HEIGHT * (selectionStartLine - minCurrentLine + i + 1) - 2,
                                                        SCREEN_WIDTH,
                                                        LINE_HEIGHT + 2
                                                });
                }
            }
        }else{
            for(int i=0; i<=linesLength; i++){
                if(i==0){
                    selectedHighlight.push_back({
                                                        getTextWidthUpTo(selectionEnd, font, lines),
                                                        50 + LINE_HEIGHT * (selectionEndLine - minCurrentLine + 1) - 2,
                                                        SCREEN_WIDTH,
                                                        LINE_HEIGHT + 2
                                                });
                }else if(i==linesLength){
                    selectedHighlight.push_back({
                                                        getTextWidthUpTo(minCursorPos, font, lines),
                                                        50 + LINE_HEIGHT * (selectionStartLine - minCurrentLine + 1) - 2,
                                                        getTextWidthUpTo(selectionStart-minCursorPos, font, lines),
                                                        LINE_HEIGHT + 2
                                                });
                }else{
                    selectedHighlight.push_back({
                                                        getTextWidthUpTo(minCursorPos, font, lines),
                                                        50 + LINE_HEIGHT * (selectionEndLine - minCurrentLine + i + 1) - 2,
                                                        SCREEN_WIDTH,
                                                        LINE_HEIGHT + 2
                                                });
                }
            }
        }

        for(auto selectedLine: selectedHighlight){
            SDL_RenderFillRect(renderer, &selectedLine);
        }
        SDL_RenderPresent(renderer);
    }

    // Clean up and close SDL
    SDL_DestroyTexture(lightBgTexture);
    SDL_DestroyTexture(darkBgTexture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}

void ensureLastLineVisible(int currentLine, int& scrollOffset, int SCREEN_HEIGHT, int LINE_HEIGHT, int totalLines) {
    if (currentLine * LINE_HEIGHT - scrollOffset >= SCREEN_HEIGHT - LINE_HEIGHT) {
        scrollOffset = currentLine * LINE_HEIGHT - SCREEN_HEIGHT + LINE_HEIGHT;
    } else if (currentLine * LINE_HEIGHT - scrollOffset < 0) {
        scrollOffset = currentLine * LINE_HEIGHT;
    }
}