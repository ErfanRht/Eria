#include <SDL2/SDL.h>
#include <iostream>
#include <SDL2/SDL_mixer.h>
#include <cmath>
#ifdef _WIN32
#include <SDL2/SDL2_gfx.h>
#else
#include <SDL2/SDL2_gfxPrimitives.h>
#endif
#include <bits/stdc++.h>

using namespace std;

int main(int argc, char* argv[]) {
    Uint32 SDL_flags = SDL_INIT_VIDEO | SDL_INIT_TIMER;
    Uint32 WND_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Init(SDL_flags);
    SDL_CreateWindowAndRenderer(0, 0, WND_flags, &window, &renderer);
    SDL_RaiseWindow(window);
    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    int SCREEN_WIDTH = DM.w;
    int SCREEN_HEIGHT = DM.h;
    window = SDL_CreateWindow("Erfan Nemoodar Pro 2025", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    ifstream file("C:/Users/Erfan/Dev/Cpp/Fractal/Automobile_data.txt");
    string line;
    map<int, vector<string>> data;
    int chandominLine = 0;
    while (getline(file, line)) {
        stringstream ss(line);
        string value;
        vector<string> datayeNom;
        int i=0;
        int esm;
        if(chandominLine != 0){
            while (getline(ss, value, ',')) {
                if(i == 0){
                    esm = stoi(value);
                }else{
                    datayeNom.push_back(value);
                }
                i++;
            }
            data[esm] = datayeNom;
        }
        chandominLine++;
    }
    file.close();

    vector<string> chartHa = {"Bar", "Pie", "Doughnut", "Polar", "Line"};
    while (true) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 0;
            }else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    SDL_DestroyRenderer(renderer);
                    SDL_DestroyWindow(window);
                    SDL_Quit();
                    return 0;
                }
            }
        }
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
