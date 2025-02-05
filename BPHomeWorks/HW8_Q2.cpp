#include <SDL2/SDL.h>
#include <iostream>
#include <bits/stdc++.h>
#include <SDL2/SDL_mixer.h>
#include <cmath>

#ifdef _WIN32
#include <SDL2/SDL2_gfx.h>
#else
#include <SDL2/SDL2_gfxPrimitives.h>
#endif
using namespace std;

struct Color {
    int r, g, b, a;
};

enum Shekl {
    None,
    Rectangle_Por,
    Rectangle_Khali,
    Circle_Por,
    Circle_Khali,
    Triangle_Por,
    Triangle_Khali
};

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

    Shekl selectedShekl = None;
    bool isDrawing = false, drawStraightLine = false, isDrawingStraightLine = false, drawChandZeli = false,isDrawingChandZeli = false;
    Color drawColor = {0, 0, 0, 255};
    int startX, startY, endX, endY;
    int KoloftieKhat = 5;

    window = SDL_CreateWindow("Erfan Paint Pro 2025", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    vector<Color> rangHa = {
            {0, 0, 0, 255},
            {255, 0, 0, 255},
            {0, 255, 0, 255},
            {0, 0, 255, 255},
            {255, 255, 0, 255}
    };

    vector<SDL_Rect> colorBarRects;
    for (int i = 0; i < rangHa.size(); i++) {
        colorBarRects.push_back({10, i * 60 + 10, 50, 50});
    }
    vector<SDL_Rect> menuRects;
    for (int i = 0; i < 6; i++) {
        menuRects.push_back({10, i*60 + static_cast<int>(rangHa.size()*60 + 10), 50, 50});
    }

    while (true) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 0;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = e.button.x;
                int mouseY = e.button.y;

                bool buttonPressed = false;
                for (int i = 0; i < 4; i++) {
                    if (mouseX >= colorBarRects[i].x && mouseX <= colorBarRects[i].x + colorBarRects[i].w &&
                        mouseY >= colorBarRects[i].y && mouseY <= colorBarRects[i].y + colorBarRects[i].h) {
                        drawColor = rangHa[i];
                        buttonPressed = true;
                        break;
                    }
                }

                for (int i = 0; i < menuRects.size(); i++) {
                    if (mouseX >= menuRects[i].x && mouseX <= menuRects[i].x + menuRects[i].w &&
                        mouseY >= menuRects[i].y && mouseY <= menuRects[i].y + menuRects[i].h) {
                        selectedShekl = static_cast<Shekl>(i + 1);
                        startX = 0;
                        startY = 0;
                        buttonPressed = true;
                        break;
                    }
                }

                if (!buttonPressed) {
                    if(selectedShekl==None){
                        if (!drawStraightLine && !drawChandZeli) {
                            isDrawing = true;
                        } else {
                            if(drawStraightLine){
                                if (!isDrawingStraightLine) {
                                    isDrawingStraightLine = true;
                                } else {
                                    endX = e.button.x;
                                    endY = e.button.y;
                                    thickLineRGBA(renderer, startX, startY, endX, endY, KoloftieKhat,
                                                  drawColor.r, drawColor.g, drawColor.b, drawColor.a);
                                    SDL_RenderPresent(renderer);
                                    isDrawingStraightLine = false;
                                    drawStraightLine = false;
                                }
                            }
                            else if(drawChandZeli){
                                if (!isDrawingChandZeli) {
                                    isDrawingChandZeli = true;
                                } else {
                                    endX = e.button.x;
                                    endY = e.button.y;
                                    thickLineRGBA(renderer, startX, startY, endX, endY, KoloftieKhat,
                                                  drawColor.r, drawColor.g, drawColor.b, drawColor.a);
                                    SDL_RenderPresent(renderer);
                                }
                            }
                    }}else{
                        if(!(startX == 0 && startX == 0)){
                            endX = e.button.x;
                            endY = e.button.y;
                            SDL_SetRenderDrawColor(renderer, drawColor.r, drawColor.g, drawColor.b, 255);
                            if(selectedShekl == Rectangle_Por){
                                boxRGBA(renderer, startX, startY, endX, endY, 0, 0 ,0, 255);
                            }
                            else if(selectedShekl == Rectangle_Khali){
                                SDL_Rect rect = {startX, startY,
                                                 (startX-endX), abs(startY-endY)};
                                SDL_RenderDrawRect(renderer, &rect);
                            }
                            else if(selectedShekl == Circle_Por){
                                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                                filledCircleRGBA(renderer, startX, startY, sqrt(pow(endX - startX, 2) + pow(endY - startY, 2)), 0, 0 ,0, 255);
                            } else if(selectedShekl == Circle_Khali){
                                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                                circleRGBA(renderer, startX, startY, sqrt(pow(endX - startX, 2) + pow(endY - startY, 2)), 0, 0 ,0, 255);
                            }
                            else if(selectedShekl == Triangle_Por){
                                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                                filledTrigonRGBA(renderer, startX, startY, (startX+endX)/2, startY - abs(startX-endX), endX, startY, 0, 0 ,0, 255);
                            }else if(selectedShekl == Triangle_Khali){
                                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                                trigonRGBA(renderer, startX, startY, (startX+endX)/2, startY - abs(startX-endX), endX, startY, 0, 0 ,0, 255);
                            }
                            selectedShekl = None;
                            SDL_RenderPresent(renderer);
                        }
                    }
                    startX = e.button.x;
                    startY = e.button.y;}
            } else if (e.type == SDL_MOUSEMOTION && isDrawing && !drawStraightLine) {
                endX = e.motion.x;
                endY = e.motion.y;
                thickLineRGBA(renderer, startX, startY, endX, endY, KoloftieKhat,
                              drawColor.r, drawColor.g, drawColor.b, drawColor.a);
                startX = endX;
                startY = endY;
                SDL_RenderPresent(renderer);
            } else if (e.type == SDL_MOUSEBUTTONUP) {
                isDrawing = false;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    return 0;
                } else if (e.key.keysym.sym == SDLK_e) {
                    cout << "Eraser" << endl;
                    if (!(drawColor.r == 255 && drawColor.g == 255 && drawColor.b == 255)) {
                        drawColor = {255, 255, 255, 255};
                        KoloftieKhat += 100;
                    } else {
                        drawColor = {0, 0, 0, 255};
                        KoloftieKhat -= 100;
                    }
                } else if (e.key.keysym.sym == SDLK_s) {
                    drawStraightLine = !drawStraightLine;
                }else if (e.key.keysym.sym == SDLK_c) {
                    drawStraightLine = false;
                    isDrawingStraightLine = false;
                    isDrawing = false;
                    drawChandZeli = !drawChandZeli;
                } else if (e.key.keysym.sym == SDLK_PLUS || e.key.keysym.sym == SDLK_EQUALS) {
                    KoloftieKhat += 5;
                } else if (e.key.keysym.sym == SDLK_MINUS && KoloftieKhat > 1) {
                    KoloftieKhat -= 5;
                }
            }
        }

        for (int i = 0; i < rangHa.size(); i++) {
            SDL_SetRenderDrawColor(renderer, rangHa[i].r, rangHa[i].g, rangHa[i].b, rangHa[i].a);
            SDL_RenderFillRect(renderer, &colorBarRects[i]);
        }
        for (int i = 0; i < menuRects.size(); i++) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &menuRects[i]);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &menuRects[i]);

            if(i == 0){
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_Rect rect = {menuRects[i].x + 10, menuRects[i].y + 10, menuRects[i].w - 20, menuRects[i].h - 20};
                boxRGBA(renderer, rect.x, rect.y, rect.x+rect.w, rect.y+rect.h, 0, 0 ,0, 255);
            }if(i == 1){
                SDL_Rect rect = {menuRects[i].x + 10, menuRects[i].y + 10,
                                 30, 30};
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(renderer, &rect);
            }if(i == 2){
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                filledCircleRGBA(renderer, 35, menuRects[i].y + 25, 17.5, 0, 0 ,0, 255);
            }
            if(i == 3){
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                circleRGBA(renderer, 35, menuRects[i].y + 25, 17.5, 0, 0 ,0, 255);
            }if(i == 4){
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                filledTrigonRGBA(renderer, 20, menuRects[i].y + 40, 50, menuRects[i].y + 40, 35, menuRects[i].y + 10, 0, 0 ,0, 255);
            }if(i == 5){
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                trigonRGBA(renderer, 20, menuRects[i].y + 40, 50, menuRects[i].y + 40, 35, menuRects[i].y + 10, 0, 0 ,0, 255);
            }
        }
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

// Commands:
// e --> Pak kon
// + or = --> Koloft Kardan
// - --> Nazok Kardan
// s --> Khat Saaf
// c --> Chand Zeli Delkhah
