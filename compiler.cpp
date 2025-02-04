#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <fstream> // For file operations

// Function to compile a C++ source file
bool compile(const std::string &filename)
{
    std::string compileCommand = "g++ " + filename + " -o output"; // Adjust as needed
    int result = std::system(compileCommand.c_str());
    return (result == 0);
}

// Function to run an executable
void run(const std::string &filename)
{
    std::string runCommand = ".\\" + filename + ".exe ";
    std::system(runCommand.c_str());
}

void run_in_another_window(const std::string &filename)
{
#ifdef _WIN32 // Windows
    std::string runCommand = "start cmd /c \"" + filename + " & pause\"";
#elif __linux__ // Linux
    std::string runCommand = "x-terminal-emulator -e \"" + filename + "; read -n 1 -s -r -p 'Press any key to continue...'\"";
#elif __APPLE__ // macOS
    std::string runCommand = "osascript -e 'tell application \"Terminal\" to do script \"" + filename + "\"'";
#else
    std::cerr << "Unsupported operating system." << std::endl;
    return;
#endif

    std::system(runCommand.c_str());
}

// Function to "debug" (simulated basic debugging)
void debug(const std::string &filename)
{
    std::cout << "Debugging " << filename << " (simulated):\n";
    std::ifstream file(filename);
    if (file.is_open())
    {
        std::string line;
        int lineNumber = 1;
        while (std::getline(file, line))
        {
            std::cout << lineNumber << ": " << line << std::endl;
            lineNumber++;
        }
        file.close();
    }
    else
    {
        std::cerr << "Unable to open file for debugging." << std::endl;
    }
}