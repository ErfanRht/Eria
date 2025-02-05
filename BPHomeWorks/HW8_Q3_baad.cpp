#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <cmath>
using namespace std;

struct Noghte {
    double x, y;
};

Noghte charkheshLine(const Noghte start, const Noghte end, double zavie) {
    Noghte newEnd;
    newEnd.x = start.x + (end.x - start.x) * cos(zavie) - (end.y - start.y) * sin(zavie);
    newEnd.y = start.y + (end.x - start.x) * sin(zavie) + (end.y - start.y) * cos(zavie);
    return newEnd;
}

SDL_Color calculateRang(int iterations, int maxIterations) {
    float ratio = static_cast<float>(iterations) / maxIterations;

    int red = static_cast<int>(255 * ratio);
    int green = static_cast<int>(20 + (ratio * 150));
    int blue = static_cast<int>(147 + (ratio * 100));

    red = min(max(red, 0), 255);
    green = min(max(green, 0), 170);
    blue = min(max(blue, 0), 247);

    return SDL_Color{static_cast<Uint8>(red), static_cast<Uint8>(green), static_cast<Uint8>(blue), 255};
}

void Shakhe(SDL_Renderer *m_renderer, double x, double y, int len, double zavie, int iterations, int maxIterations, Uint32 ekhtelafZaman) {
    if (iterations >= maxIterations || len < 1) return;

    SDL_Color rangeShakhe = calculateRang(iterations, maxIterations);
    SDL_SetRenderDrawColor(m_renderer, rangeShakhe.r, rangeShakhe.g, rangeShakhe.b, rangeShakhe.a);

    Noghte start = {x, y};
    Noghte end = {x, y - len};

    // zavie += 0.1 * sin(0.005*ekhtelafZaman);

    Noghte rotatedEnd = charkheshLine(start, end, zavie);

    int koloftieLine = 12 - iterations;
    if (koloftieLine < 2) koloftieLine = 2;

    for (int i = -koloftieLine / 2; i <= koloftieLine / 2; i++) {
        SDL_RenderDrawLine(
                m_renderer,
                static_cast<int>(start.x + i), static_cast<int>(start.y),
                static_cast<int>(rotatedEnd.x + i), static_cast<int>(rotatedEnd.y)
        );
    }

    Shakhe(m_renderer, rotatedEnd.x, rotatedEnd.y, len * 0.75, zavie + M_PI/5, iterations + 1, maxIterations, ekhtelafZaman);
    Shakhe(m_renderer, rotatedEnd.x, rotatedEnd.y, len * 0.75, zavie - M_PI/5, iterations + 1, maxIterations, ekhtelafZaman);
}

int main(int argc, char *argv[]) {
    int maxIterations;
    cout << "Enter the number of iterations (n): ";
    cin >> maxIterations;

    Uint32 SDL_flags = SDL_INIT_VIDEO | SDL_INIT_TIMER;
    Uint32 WND_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP;
    SDL_Window *m_window;
    SDL_Renderer *m_renderer;

    SDL_Init(SDL_flags);
    SDL_CreateWindowAndRenderer(0, 0, WND_flags, &m_window, &m_renderer);
    SDL_RaiseWindow(m_window);

    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    int W = DM.w;
    int H = DM.h;

    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
    SDL_RenderClear(m_renderer);

    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
        std::cerr << "Error opening audio device: " << Mix_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

//    Mix_Music* music = Mix_LoadMUS(R"(C:\Users\Erfan\Dev\Cpp\Fractal\music.mp3)");
//    if (Mix_PlayMusic(music, -1) == -1) {
//        return -1;
//    }


    Uint32 startTime = SDL_GetTicks();

    bool running = true;
    SDL_Event event;
    while (running) {
        Uint32 ekhtelafZaman = SDL_GetTicks() - startTime;

        SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
        SDL_RenderClear(m_renderer);

        Shakhe(m_renderer, W / 2, H, 250, 0, 0, maxIterations, ekhtelafZaman);

        SDL_RenderPresent(m_renderer);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN)) {
                running = false;
            }
        }
    }

    SDL_DestroyWindow(m_window);
    SDL_DestroyRenderer(m_renderer);
    SDL_Quit();
    return 0;
}
