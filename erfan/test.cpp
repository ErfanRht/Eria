#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <iostream>
#include <vector>
#include <cstring>
#include <regex>
#include <fstream>
#include <windows.h>
#include "compiler.h"
#include <bits/stdc++.h>
#ifdef _WIN32
#include <SDL2/SDL2_gfx.h>
#else
#include <SDL2/SDL2_gfxPrimitives.h>
#endif
using namespace std;

const int SCREEN_WIDTH = 1250;
const int SCREEN_HEIGHT = 700;
const int minCurrentLine = 3; // Track the current line being edited
/* int currentLine = minCurrentLine;
const int minCursorPos = 16; */
int cursorPos = minCursorPos; // Track the cursor position within the current line
vector<string> lines = {""}; // Holds multiple lines of text
vector<vector<string>> linesHistory;
vector<vector<string>> futureLines;

string path = "C:/Users/Erfan/Dev/Cpp/IDE-Project/";
void ensureLastLineVisible(int currentLine, int &scrollOffset, int SCREEN_HEIGHT, int LINE_HEIGHT, int totalLines);
bool darkMode = false;
bool isMenuOpen = false, isEditOpen = false;
string currentFileName = "";
string currentProjectName = "";

// libraries
const map<string, vector<string>> libraries ={
        {"iostream", {"cin", "cout", "cerr", "clog", "getline", "put", "get", "ignore"}},
        {"cmath", {
            "abs", "fabs", "fmod", "remainder", "remquo", "hypot", "pow", "sqrt", "cbrt","sin", "cos", "tan", "asin", "acos", "atan", "atan2","sinh", "cosh", "tanh", "asinh", "acosh", "atanh","exp", "exp2", "log", "log10", "log2", "ceil", "floor", "round", "trunc"
        }},
        {"bits/stdc++.h", {
            "sort", "stable_sort", "partial_sort", "nth_element","lower_bound", "upper_bound", "binary_search","next_permutation", "prev_permutation","transform", "unique", "rotate", "reverse", "partition","accumulate", "partial_sum", "adjacent_difference","set_union", "set_intersection", "set_difference", "set_symmetric_difference","push_back", "pop_back", "insert", "erase", "size", "clear", "resize", "begin", "end", "rbegin", "rend", "front", "back","insert", "erase", "find", "count", "lower_bound", "upper_bound", "emplace", "emplace_hint","push_back", "pop_back", "push_front", "pop_front", "push", "pop", "top", "front", "back", "empty","substr", "append", "insert", "erase", "replace", "find", "rfind", "compare", "size", "length", "empty", "clear"
        }}
};

vector<string> usedLibraries;
vector<string> includedLibraries;

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

        // Draw button border as a rounded rectangle
        roundedRectangleRGBA(renderer,
                             rect.x, rect.y, rect.x + rect.w, rect.y + rect.h,
                             radius, // Corner radius
                             255, 255, 255, 255);

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
vector<Button> buttons, projectsButtons, libraryErrorButtons, menuBarButtons, editButtons;
vector<string> projects, files;

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
        SDL_SetRenderDrawColor(renderer, 0, 122, 255, 255);
        SDL_RenderClear(renderer);

        SDL_Color titleColor = {255, 255, 255, 255};
        SDL_Color textColor = {0, 122, 255, 255};
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
                                      {2, static_cast<int>(50*(buttons.size()-2)), 200, 50},
                                      0,
                                      SDL_Color{0, 122, 255, 255},
                                      {255, 255, 255, 255},
                                      newPorjectName,[]() {  }}
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
    static bool inMultiLineComment = false; // Tracks if we are inside /* ... */
    regex combinedRegex(
            R"(([\(\)\{\}\[\]])|(\"([^\"\\]|\\.)*\"|'([^'\\]|\\.)*')|\b(auto|bool|break|case|catch|char|class|const|continue|default|delete|do|double|else|enum|false|float|for|if|int|long|namespace|new|nullptr|private|protected|public|return|short|signed|sizeof|static|struct|switch|template|true|try|typedef|union|unsigned|using|void|while)\b|\b([a-zA-Z_][a-zA-Z0-9_]*)\s*(?=\()|(\b\d+(\.\d+)?\b)|(//.*$)|(/\*.*)|(\*/)|(#\b(include|define)\b))"
    );
    sregex_iterator matchesBegin(line.begin(), line.end(), combinedRegex);
    sregex_iterator matchesEnd;

    size_t prevPos = 0;
    for (sregex_iterator it = matchesBegin; it != matchesEnd; ++it) {
        smatch match = *it;

        // Text before the match
        if (match.position() > prevPos) {
            SDL_Color normalColor = darkMode ? SDL_Color{255, 255, 255, 255} : SDL_Color{0, 0, 0, 255};
            if (inMultiLineComment) normalColor = darkMode ? SDL_Color{92, 99, 112, 255} : SDL_Color{128, 128, 128, 255}; // Comment color
            segments.push_back({line.substr(prevPos, match.position() - prevPos), normalColor});
        }

        SDL_Color matchColor = darkMode?SDL_Color{255, 255, 255, 255}:SDL_Color{0, 0, 0, 255};

        // Determine the type of match
        if (match[1].matched) {
            matchColor = darkMode?SDL_Color{184, 134, 11, 255}:SDL_Color{184, 134, 11, 255}; // Operators
        } else if (match[2].matched) {
            matchColor = darkMode?SDL_Color{152, 195, 121, 255}:SDL_Color{0, 100, 0, 255}; // Strings
        } else if (match[5].matched) {
            string keyword = match.str();
            set<string> keywords = {
                    "break", "case", "catch", "class", "continue",
                    "default", "delete", "do", "else", "enum", "false",
                    "for", "if", "namespace", "new", "nullptr",
                    "private", "protected", "public", "return", "sizeof", "static", "struct",
                    "switch", "template", "true", "try", "typedef", "union", "using",
                    "void", "while"
            };

            matchColor = (keywords.find(keyword) != keywords.end()) ? darkMode?SDL_Color{198, 120, 221, 255}:SDL_Color{0, 51, 102, 255}
                                                                                     : darkMode?SDL_Color{224, 108, 117, 255}:SDL_Color{0, 128, 128, 255};
        } else if (match[6].matched) {
            matchColor = darkMode?SDL_Color{97, 175, 254, 255}:SDL_Color{255, 140, 0, 255}; // Function names
        } else if (match[7].matched) {
            matchColor = darkMode?SDL_Color{209, 154, 102, 255}:SDL_Color{128, 0, 128, 255}; // Digits
        }
        else if (match[12].matched) {
            matchColor = darkMode?SDL_Color{86, 182, 194, 255}:SDL_Color{0, 139, 139, 255}; // Preprocessor directives
        } else if (match[10].matched) { // Start of /* comment
            inMultiLineComment = true;
            matchColor = darkMode ? SDL_Color{92, 99, 112, 255} : SDL_Color{128, 128, 128, 255};
        }
        else if (match[11].matched) { // End of */ comment
            inMultiLineComment = false;
            matchColor = darkMode ? SDL_Color{92, 99, 112, 255} : SDL_Color{128, 128, 128, 255};
        }
        else if (inMultiLineComment || match[9].matched) { // Inside /* ... */ or // comment
            matchColor = darkMode ? SDL_Color{92, 99, 112, 255} : SDL_Color{128, 128, 128, 255};
        }


        segments.push_back({match.str(), matchColor});
        prevPos = match.position() + match.length();
    }

    // Remaining text
    if (prevPos < line.size()) {
        segments.push_back({line.substr(prevPos), darkMode?SDL_Color{255, 255, 255, 255}:SDL_Color{0, 0, 0, 255}});
    }

    return segments;
}

void renderTopBar(SDL_Renderer* renderer, TTF_Font* font, vector<string> &lines) {
    SDL_Texture* darkModeIconTexture = IMG_LoadTexture(renderer, R"(C:\Users\Erfan\Dev\Cpp\IDE-Project\darkMode.png)");
    SDL_Texture* runIconTexture = IMG_LoadTexture(renderer, R"(C:\Users\Erfan\Dev\Cpp\IDE-Project\run.png)");

    // Set the top bar background color
    SDL_SetRenderDrawColor(renderer, 0, 122, 255, 255);
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
    SDL_Rect runIconRexr = { SCREEN_WIDTH - iconWidth - 355, 6, iconWidth+6, iconHeight+6 };
    SDL_RenderCopy(renderer, runIconTexture, NULL, &runIconRexr);

    SDL_DestroyTexture(darkModeIconTexture);
    SDL_DestroyTexture(runIconTexture);

    SDL_SetRenderDrawColor(renderer, 100, 149, 237, 255);
    SDL_Rect rect = SDL_Rect{0, 50,204,SCREEN_HEIGHT};
    SDL_RenderFillRect(renderer, &rect);
    SDL_RenderDrawRect(renderer, &rect);

    libraryErrorButtons.clear();
    int c = 50;
    for(string usedLibrary: usedLibraries){
        if(!binary_search(includedLibraries.begin(), includedLibraries.end(), usedLibrary)){
            libraryErrorButtons.push_back(
                    {{SCREEN_WIDTH - 425, 10 + c, 415, 50},
                     7,
                     {255, 69, 58, 255},
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
    }

    for (auto& button : buttons) {
        button.render(renderer, font);
    }
    for (auto& button : libraryErrorButtons) {
        button.render(renderer, font);}
    if(!isMenuOpen){
        for (auto& button : projectsButtons) {
        button.render(renderer, font);}
    }else{
        SDL_Color  bgColor = darkMode? SDL_Color{0, 0, 0, 255}:SDL_Color {255, 255, 255, 255};
        roundedBoxRGBA(renderer,
                       0, 50, 204, SCREEN_HEIGHT,
                       0, // Corner radius
                       bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        for (auto& button : menuBarButtons) {
            button.render(renderer, font);
            roundedRectangleRGBA(renderer,
                                 button.rect.x, button.rect.y, button.rect.x + button.rect.w, button.rect.y + button.rect.h,
                                 button.radius, // Corner radius
                                 bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        }if(isEditOpen){
            for (auto& button : editButtons) {
                button.render(renderer, font);
                roundedRectangleRGBA(renderer,
                                     button.rect.x, button.rect.y, button.rect.x + button.rect.w, button.rect.y + button.rect.h,
                                     button.radius, // Corner radius
                                     bgColor.r, bgColor.g, bgColor.b, bgColor.a);
            }
        }
    }
}

bool handlebuttonClicks(SDL_Event* event) {
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
        c+=50;
        if(currentProjectName == project){
            projectsButtons.push_back({
                                      {2, c+g, 200, 50},
                                      0,
                                      SDL_Color{0, 122, 255, 255},
                                      {255, 255, 255, 255},
                                      project,[&renderer, &font, project]() {}}
            );
            for(string file: files){
                g+=50;
                projectsButtons.push_back({
                                          {2, c+g, 200, 50},
                                          0,
                                          SDL_Color{100, 149, 237, 255},
                                          {255, 255, 255, 255},
                                          file,[&renderer, &font ,file]() {
                            openFile(lines, renderer, font, file, currentProjectName);
                                          }}
                );
            }
        }else{
            projectsButtons.push_back({
                                      {2, c+g, 200, 50},
                                      0,
                                      SDL_Color{0, 122, 255, 255},
                                      {255, 255, 255, 255},
                                      project,[&renderer, &font, project]() {
                        openFile(lines, renderer, font, project+".cpp", project);
                        initializeProjectsButtons(renderer, font);
                    }}
        );
    }
}}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
        return -1;
    }if (TTF_Init() == -1) {
        cerr << "TTF could not initialize! TTF_Error: " << TTF_GetError() << endl;
        SDL_Quit();
        return -1;
    }SDL_Window* window = SDL_CreateWindow("Eria IDE-Project 2025",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,SCREEN_WIDTH,SCREEN_HEIGHT,SDL_WINDOW_SHOWN);
    if (!window) {
        cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
        TTF_Quit();
        SDL_Quit();
        return -1;
    }SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }TTF_Font* font = TTF_OpenFont(R"(C:\Windows\Fonts\consola.ttf)", 24); // Replace with the path to your .ttf font
    if (!font) {
        cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

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
        {255, 255, 255, 255},
        {34, 139, 34, 255},
        "Save",
        [&renderer, &font]() {
            saveToFile(lines, renderer, font); }});
    buttons.push_back({
        {SCREEN_WIDTH-325, 7, 125, 36},
        15,
        {255, 255, 255, 255},
        {34, 139, 34, 255},
        "Save As",
        [&renderer, &font]() {
                saveAsToFile(lines, renderer, font);
        }});
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
             {0, 122, 255, 255},
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
             {0, 122, 255, 255},
             {255, 255, 255, 255},
             "Save Project",
             [&renderer, &font]() {
                 saveAsToFile(lines, renderer, font);
             }});
    menuBarButtons.push_back(
            {{10, 10+50*3, 186, 36},
             15,
             {0, 122, 255, 255},
             {255, 255, 255, 255},
             "Debug&Compile",
             [&renderer, &font]() {
             }});
    menuBarButtons.push_back(
            {{10, 10+50*4, 186, 36},
             15,
             {0, 122, 255, 255},
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
             {0, 122, 255, 255},
             {255, 255, 255, 255},
             "Dark/Light",
             []() {
                 darkMode = !darkMode;
             }});
    menuBarButtons.push_back(
            {{10, 10+50*6, 186, 36},
             15,
             {0, 122, 255, 255},
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
    initializeProjectsButtons(renderer, font);
    renderTopBar(renderer, font, lines);

    // Pressed states
    bool ctrlPressed = false, shiftPressed = false;
    bool ctrlGPressed = false;

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
                for (auto& button : libraryErrorButtons) {
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
                if(ctrlGPressed){
                    if (key >= SDLK_0 && key <= SDLK_9) {
                        int digit = key - SDLK_0;
                        currentLine = digit + minCurrentLine - 1;
                        cursorPos = minCursorPos;
                    }else{
                        ctrlGPressed = false;
                    }
                }

                // Track Control and Shift key states
                else if (e.key.keysym.sym == SDLK_LCTRL || e.key.keysym.sym == SDLK_RCTRL) {
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
                            // Insert the clipboard text into the current line
                            stringstream ss(clipboardText);
                            string line;
                            int g = 0;
                            while (getline(ss, line, '\n')) {
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
                            // cursorPos += line.length(); // Move cursor forward
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
                    ctrlGPressed = true;
                    ctrlPressed = false;
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
                    // Add spaces for tab
                    lines[currentLine].insert(cursorPos, "    ");
                    cursorPos += 4;
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
            handlebuttonClicks(&e);
            if(linesHistory[linesHistory.size()-1] != lines){
                linesHistory.push_back(lines);
                futureLines.clear();
            }
            if(selectionStart != -1){
                cout << endl << selectionStart << " " << selectionEnd << " " << selectionStartLine << " " << selectionEndLine;
            }
        }

        // Clear screen
        if(darkMode){
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        }else{
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        }
        SDL_RenderClear(renderer);

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
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    SDL_RenderDrawLine(renderer, cursorX, y, cursorX, y + LINE_HEIGHT);
                }
            }
            y += LINE_HEIGHT; // Move to the next line
        }

        checkLibrariesUsage(lines);
        checkLibrariesInclude(lines);
        renderTopBar(renderer, font, lines);

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 100); // Blue with 50% transparency

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
