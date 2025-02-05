#include <SDL2/SDL.h>
#include <iostream>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include <cmath>
#define quitGame() Mix_FreeMusic(music);Mix_Quit();SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);SDL_Quit();return 0;
#ifdef _WIN32
#include <SDL2/SDL2_gfx.h>
#else
#include <SDL2/SDL2_gfxPrimitives.h>
#endif
#include <bits/stdc++.h>

using namespace std;

void reRenderer(SDL_Renderer* renderer, SDL_Texture* toopImg, SDL_Rect& toop,
                     SDL_Texture* livan1Img, SDL_Rect& livan1,
                     SDL_Texture* livan2Img, SDL_Rect& livan2,
                     SDL_Texture* livan3Img, SDL_Rect& livan3) {
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, toopImg, NULL, &toop);
        SDL_RenderCopy(renderer, livan1Img, NULL, &livan1);
        SDL_RenderCopy(renderer, livan2Img, NULL, &livan2);
        SDL_RenderCopy(renderer, livan3Img, NULL, &livan3);
        SDL_RenderPresent(renderer);
        SDL_Delay(0);
}
void animateSwap(SDL_Rect& livan1, SDL_Rect& livan2, SDL_Renderer* renderer, int darajeSakhti,
                 SDL_Texture* toopImg, SDL_Rect& toop,
                 SDL_Texture* livan1Img, SDL_Texture* livan2Img, SDL_Texture* livan3Img, SDL_Rect& livan3) {
    int steps = 30;
    int delay = 20/darajeSakhti;
    int livan1StartX = livan1.x, livan1StartY = livan1.y;
    int livan2StartX = livan2.x, livan2StartY = livan2.y;

    for (int i = 1; i <= steps; ++i) {
        livan1.x = livan1StartX + (livan2StartX - livan1StartX) * i / steps;
        livan1.y = livan1StartY + (livan2StartY - livan1StartY) * i / steps;
        livan2.x = livan2StartX + (livan1StartX - livan2StartX) * i / steps;
        livan2.y = livan2StartY + (livan1StartY - livan2StartY) * i / steps;

        reRenderer(renderer, livan1Img, livan1, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3);
        SDL_Delay(delay);
    }
    livan1.x = livan2StartX;
    livan1.y = livan2StartY;
    livan2.x = livan1StartX;
    livan2.y = livan1StartY;
}


void shuffleLivans(SDL_Rect& livan1, SDL_Rect& livan2, SDL_Rect& livan3, SDL_Renderer* renderer, int darajeSakhti,
                   SDL_Texture* toopImg, SDL_Rect& toop,
                   SDL_Texture* livan1Img, SDL_Texture* livan2Img, SDL_Texture* livan3Img) {
    vector<SDL_Rect*> livans = {&livan1, &livan2, &livan3};
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, 2);

    for (int i = 0; i < 17; i++) {
        int a = dist(gen);
        int b = dist(gen);
        if (a != b) {animateSwap(*livans[a], *livans[b], renderer, darajeSakhti,toopImg, toop, livan1Img, livan2Img, livan3Img, *livans[3 - a - b]);
        }
    }
}

bool livanClick(int mouseX, int mouseY, SDL_Rect& livan, SDL_Rect& toop,int w, int livanId, int& correctLivan,
                      SDL_Renderer* renderer, int darajeSakhti, SDL_Texture* toopImg, SDL_Texture* livan1Img, SDL_Rect& livan1,
                      SDL_Texture* livan2Img, SDL_Rect& livan2, SDL_Texture* livan3Img, SDL_Rect& livan3, bool& gameOn, bool& answerTime) {
    if (mouseX >= livan.x && mouseX <= livan.x + livan.w && mouseY >= livan.y && mouseY <= livan.y + livan.h) {
        if(answerTime){
            answerTime = false;
            livan.y -= 250;
            reRenderer(renderer, toopImg, toop, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3);
            SDL_Delay(750);
            if(livanId != correctLivan){
                if(correctLivan == 1){
                    livan1.y -= 250;
                    reRenderer(renderer, toopImg, toop, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3);
                    SDL_Delay(1250);
                    livan1.y += 250;
                    reRenderer(renderer, toopImg, toop, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3);
                } if(correctLivan == 2){
                    livan2.y -= 250;
                    reRenderer(renderer, toopImg, toop, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3);
                    SDL_Delay(1250);
                    livan2.y += 250;
                    reRenderer(renderer, toopImg, toop, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3);
                } if(correctLivan == 3){
                    livan3.y -= 250;
                    reRenderer(renderer, toopImg, toop, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3);
                    SDL_Delay(1250);
                    livan3.y += 250;
                    reRenderer(renderer, toopImg, toop, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3);
                }
            }
            livan.y += 250;
            reRenderer(renderer, toopImg, toop, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3);
        }else {
            gameOn = true;
            correctLivan = livanId;
            if (livanId == 1) {
                toop.x = livan1.x + livan1.w / 2 - toop.w / 2;
            } else if (livanId == 2) {
                toop.x = livan2.x + livan2.w / 2 - toop.w / 2;
            } else if (livanId == 3) {
                toop.x = livan3.x + livan3.w / 2 - toop.w / 2;
            }
            toop.y = livan.y + livan.h - 1.5 * toop.h;
            livan.y -= 250;
            reRenderer(renderer, toopImg, toop, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3);
            SDL_Delay(1250);
            livan.y += 250;
            reRenderer(renderer, toopImg, toop, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3);
            SDL_Delay(500);
            shuffleLivans(livan1, livan2, livan3, renderer,darajeSakhti, toopImg, toop, livan1Img, livan2Img, livan3Img);
            reRenderer(renderer, toopImg, toop, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3);
            if (correctLivan == 1) {
                toop.x = livan1.x + livan1.w / 2 - toop.w / 2;
            } else if (correctLivan == 2) {
                toop.x = livan2.x + livan2.w / 2 - toop.w / 2;
            } else if (correctLivan == 3) {
                toop.x = livan3.x + livan3.w / 2 - toop.w / 2;
            }
            reRenderer(renderer, toopImg, toop, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3);
            gameOn = false;
            answerTime = true;
        }
        return true;
    }else{
        return false;
    }
}



int main(int argc, char* argv[]) {
    int darajeSakhti = 1;
    // cin >> darajeSakhti;

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
    window = SDL_CreateWindow("Erfan Game Pro 2025", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    SDL_Texture* toopImg = IMG_LoadTexture(renderer, R"(C:\Users\Erfan\Dev\Cpp\Fractal\ball1.png)");

    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
        std::cerr << "Error opening audio device: " << Mix_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    Mix_Music* music = Mix_LoadMUS(R"(C:\Users\Erfan\Dev\Cpp\Fractal\music.mp3)");
    if (Mix_PlayMusic(music, -1) == -1) {
        return -1;
    }

    SDL_Rect toop;
    SDL_QueryTexture(toopImg, NULL, NULL, &toop.w, &toop.h);
    toop.w = 100;
    toop.h = 100;
    toop.x = (SCREEN_WIDTH - toop.w) / 2;
    toop.y = (SCREEN_HEIGHT - toop.h) / 2;

    SDL_Texture* livan1Img = IMG_LoadTexture(renderer, R"(C:\Users\Erfan\Dev\Cpp\Fractal\cup1.png)");
    SDL_Rect livan1;
    SDL_QueryTexture(livan1Img, NULL, NULL, &livan1.w, &livan1.h);
    livan1.x = 100;
    livan1.y = (SCREEN_HEIGHT - livan1.h) / 2;
    SDL_Texture* livan2Img = IMG_LoadTexture(renderer, R"(C:\Users\Erfan\Dev\Cpp\Fractal\cup1.png)");
    SDL_Rect livan2;
    SDL_QueryTexture(livan2Img, NULL, NULL, &livan2.w, &livan2.h);
    livan2.x = (SCREEN_WIDTH - livan2.w) / 2;
    livan2.y = (SCREEN_HEIGHT - livan2.h) / 2;
    SDL_Texture* livan3Img = IMG_LoadTexture(renderer, R"(C:\Users\Erfan\Dev\Cpp\Fractal\cup1.png)");
    SDL_Rect livan3;
    SDL_QueryTexture(livan3Img, NULL, NULL, &livan3.w, &livan3.h);
    livan3.x = SCREEN_WIDTH - livan3.w - 100;
    livan3.y = (SCREEN_HEIGHT - livan3.h) / 2;

    int correctLivan = 0;
    bool gameOn = false, answerTime = false;
    while (true) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quitGame()
            }else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = e.button.x;
                int mouseY = e.button.y;
                if(!gameOn){
                    bool isClicked = false;
                    if(correctLivan == 0 && !answerTime){
                        if(!isClicked){
                            isClicked = livanClick(mouseX, mouseY, livan1, toop, SCREEN_WIDTH, 1, correctLivan, renderer,darajeSakhti, toopImg, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3, gameOn, answerTime);
                        }
                        if(!isClicked){
                            isClicked = livanClick(mouseX, mouseY, livan2, toop, SCREEN_WIDTH, 2, correctLivan, renderer,darajeSakhti, toopImg, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3, gameOn, answerTime);}
                        if(!isClicked){
                            isClicked = livanClick(mouseX, mouseY, livan3, toop, SCREEN_WIDTH, 3, correctLivan, renderer,darajeSakhti, toopImg, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3, gameOn, answerTime);
                        }
                    }else{
                        if(!isClicked){
                            isClicked = livanClick(mouseX, mouseY, livan1, toop, SCREEN_WIDTH, 1, correctLivan, renderer,darajeSakhti, toopImg, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3, gameOn, answerTime);
                        }
                        if(!isClicked){
                            isClicked = livanClick(mouseX, mouseY, livan2, toop, SCREEN_WIDTH, 2, correctLivan, renderer,darajeSakhti, toopImg, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3, gameOn, answerTime);}
                        if(!isClicked){
                            isClicked = livanClick(mouseX, mouseY, livan3, toop, SCREEN_WIDTH, 3, correctLivan, renderer,darajeSakhti, toopImg, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3, gameOn, answerTime);
                        }
                        if(isClicked){
                            SDL_Delay(1750);
                            quitGame();
                        }
                    }
                }
            }else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    quitGame()
                }else if (e.key.keysym.sym == SDLK_1) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                }else if (e.key.keysym.sym == SDLK_2) {
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                }else if (e.key.keysym.sym == SDLK_3) {
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                }else if (e.key.keysym.sym == SDLK_4) {
                    livan1Img = IMG_LoadTexture(renderer, R"(C:\Users\Erfan\Dev\Cpp\Fractal\cup1.png)");
                    livan2Img = IMG_LoadTexture(renderer, R"(C:\Users\Erfan\Dev\Cpp\Fractal\cup1.png)");
                    livan3Img = IMG_LoadTexture(renderer, R"(C:\Users\Erfan\Dev\Cpp\Fractal\cup1.png)");
                }else if (e.key.keysym.sym == SDLK_5) {
                    livan1Img = IMG_LoadTexture(renderer, R"(C:\Users\Erfan\Dev\Cpp\Fractal\cup2.png)");
                    livan2Img = IMG_LoadTexture(renderer, R"(C:\Users\Erfan\Dev\Cpp\Fractal\cup2.png)");
                    livan3Img = IMG_LoadTexture(renderer, R"(C:\Users\Erfan\Dev\Cpp\Fractal\cup2.png)");
                }else if (e.key.keysym.sym == SDLK_6) {
                    livan1Img = IMG_LoadTexture(renderer, R"(C:\Users\Erfan\Dev\Cpp\Fractal\cup3.png)");
                    livan2Img = IMG_LoadTexture(renderer, R"(C:\Users\Erfan\Dev\Cpp\Fractal\cup3.png)");
                    livan3Img = IMG_LoadTexture(renderer, R"(C:\Users\Erfan\Dev\Cpp\Fractal\cup3.png)");
                }else if (e.key.keysym.sym == SDLK_7) {
                    toopImg = IMG_LoadTexture(renderer, R"(C:\Users\Erfan\Dev\Cpp\Fractal\ball1.png)");
                }else if (e.key.keysym.sym == SDLK_8) {
                    toopImg = IMG_LoadTexture(renderer, R"(C:\Users\Erfan\Dev\Cpp\Fractal\ball2.png)");
                }else if (e.key.keysym.sym == SDLK_9) {
                    toopImg = IMG_LoadTexture(renderer, R"(C:\Users\Erfan\Dev\Cpp\Fractal\ball3.png)");
                }
                if(answerTime && !gameOn){
                    if (e.key.keysym.sym == SDLK_SPACE) {
                        if(correctLivan!=1){
                            livan1.y -= 250;
                            reRenderer(renderer, toopImg, toop, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3);
                            SDL_Delay(1250);
                            livan1.y += 250;
                            reRenderer(renderer, toopImg, toop, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3);
                        }else if(correctLivan!=2){
                            livan2.y -= 250;
                            reRenderer(renderer, toopImg, toop, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3);
                            SDL_Delay(1250);
                            livan2.y += 250;
                            reRenderer(renderer, toopImg, toop, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3);
                        }else if(correctLivan!=3){
                            livan3.y -= 250;
                            reRenderer(renderer, toopImg, toop, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3);
                            SDL_Delay(1250);
                            livan3.y += 250;
                            reRenderer(renderer, toopImg, toop, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3);
                        }
                    }
                }
            }
        }

        reRenderer(renderer, toopImg, toop, livan1Img, livan1, livan2Img, livan2, livan3Img, livan3);
    }

    quitGame()
}
