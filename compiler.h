#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include <cstdlib>
#include <fstream>
#include <iostream>

// Compile source file to executable
bool compile(const std::string& inputFile, const std::string& outputFile) {
    std::string compileCommand = "g++ \"" + inputFile + "\" -o \"" + outputFile + "\"";
    int result = std::system(compileCommand.c_str());
    return (result == 0);
}

// Run executable in current window
void run(const std::string& executablePath) {
#ifdef _WIN32
    std::string runCommand = "\"" + executablePath + "\"";
#else
    std::string runCommand = "\"./" + executablePath + "\"";
#endif
    std::system(runCommand.c_str());
}

// Run executable in new terminal window
void run_in_another_window(const std::string& executablePath) {
#ifdef _WIN32
    std::string runCommand = "start cmd /c \"\"" + executablePath + "\" & pause\"";
#elif __linux__
    std::string runCommand = "x-terminal-emulator -e \"'" + executablePath + "'; read -n 1 -s -r -p 'Press any key to continue...'\"";
#elif __APPLE__
    std::string runCommand = "osascript -e 'tell application \"Terminal\" to do script \"'" + executablePath + "'\"'";
#endif
    std::system(runCommand.c_str());
}

// Simple debugger showing file contents with line numbers
void debug(const std::string& sourcePath) {
    std::cout << "Debugging " << sourcePath << ":\n";
    std::ifstream file(sourcePath);
    if (file.is_open()) {
        std::string line;
        int lineNumber = 1;
        while (std::getline(file, line)) {
            std::cout << "[" << lineNumber++ << "] " << line << "\n";
        }
        file.close();
    } else {
        std::cerr << "Error opening file: " << sourcePath << "\n";
    }
}

#endif // COMPILER_H