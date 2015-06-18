
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>
#include <deque>

using namespace std;

const int w = 480;
const int h = 640;
const int fsize = 32;
const int cgap0 = 15;
const int cgap1 = cgap0 + 180;
const int cgap2 = cgap1 + 160;
const int rgap0 = 10;
const int rgap1 = rgap0 + 50;
const int rgap = 50;
const int hposx = 40;
const int hposy = 520;
const int hh = 5;
const int hrpoy = 590;
const int maxhist = 40*1000;

const int nintevals = 7;
int intervals[nintevals] = { 1000, 2000, 3000, 5000, 10000, 20000, 30000 };
const char* fontfile = "OpenSans-Semibold.ttf";

void rendersurface(SDL_Renderer* renderer, SDL_Surface* s, int x, int y) {
    SDL_Rect dest;
    dest.x = x;
    dest.y = y;
    SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
    SDL_QueryTexture(t, NULL, NULL, &dest.w, &dest.h);
    SDL_RenderCopy(renderer, t, NULL, &dest);
    SDL_DestroyTexture(t);
}

void drawtext(SDL_Renderer* renderer, TTF_Font* font,
              const char* text, int x, int y) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Surface* s = TTF_RenderText_Blended(font, text, { 255, 255, 255, 0 });
    rendersurface(renderer, s, x, y);
    SDL_FreeSurface(s);
}

void drawsep(SDL_Renderer* renderer, SDL_Rect* r) {
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderFillRect(renderer, r);
}

void updatedata(Uint32 tick, deque<Uint32>& data) {
    while(data.size() > 0 && data.back() + maxhist < tick) {
        data.pop_back();
    }
}

int count(deque<Uint32>& data, Uint32 tick, int offset) {
    int c = 0;
    Uint32 t;
    for(; c < data.size(); c++) {
        t = data[c];
        if(t + offset < tick) break;
    }
    return c;
}

int updateresults(Uint32 tick, deque<Uint32>& data,
                  double *currentscores, double *topscores,
                  bool onlytop = true) {
    int times;
    double rate;
    for(int i = 0; i < nintevals; i++) {
        times = count(data, tick, intervals[i]);
        rate = times/(double)(intervals[i]) * 1000.0;
        if(rate > topscores[i]) {
            topscores[i] = rate;
        }
        if(!onlytop) {
            currentscores[i] = rate;
        }
    }
}


void drawhist(SDL_Renderer* renderer, TTF_Font* font,
              deque<Uint32>& data, Uint32 tick) {
    const int linew = 4;
    const int histw = 400;
    const int histt = 1000;
    int t, d;
    SDL_Rect r = { hposx, hposy, linew, hh };
    SDL_SetRenderDrawColor(renderer, 150, 255, 150, 255);
    for(int i = 0; i < data.size(); i++) {
        d = tick - data[i];
        t = max(0, d);
        if( t >= histt ) break;
        r.x = hposx + t * histw / histt;
        SDL_RenderFillRect(renderer, &r);
    }
}

void drawdata(SDL_Renderer* renderer, TTF_Font* font,
              double *currentscores, double *topscores,
              int time) {
    const int seph = 2;
    const int bsz = 1024;
    static char buffer[bsz];
    int posy, i;
    SDL_Rect r;
    r.x = 0;
    r.w = w;
    r.h = seph;
    r.y = rgap1;
    drawsep(renderer, &r);
    drawtext(renderer, font, "Interval", cgap0, rgap0);
    drawtext(renderer, font, "Rate", cgap1, rgap0);
    drawtext(renderer, font, "Max", cgap2, rgap0);
    for(i = 0; i < nintevals; i++) {
        posy = i*rgap + rgap1;
        snprintf(buffer, bsz, "%d", intervals[i]/1000);
        drawtext(renderer, font, buffer, cgap0, posy);
        snprintf(buffer, bsz, "%3.2lf", currentscores[i]);
        drawtext(renderer, font, buffer, cgap1, posy);
        snprintf(buffer, bsz, "%3.2lf", topscores[i]);
        drawtext(renderer, font, buffer, cgap2, posy);
        r.y = posy + rgap - seph;
        drawsep(renderer, &r);
    }
    posy = i*rgap + rgap1;
    int m = time / 60000;
    int sec = (time / 1000) % 60;
    int msec = time % 1000;
    snprintf(buffer, bsz, "%02d:%02d.%02d", m, sec, msec/10);
    drawtext(renderer, font, buffer, cgap0, posy);
    snprintf(buffer, bsz, "%d", (int)currentscores[0]);
    drawtext(renderer, font, buffer, cgap0, hrpoy);
}

void draw(SDL_Renderer* renderer, TTF_Font* font,
          deque<Uint32>& data, Uint32 tick,
          double *currentscores, double *topscores,
          int time) {
    SDL_RenderClear(renderer);
    drawdata(renderer, font, currentscores, topscores, time);
    drawhist(renderer, font, data, tick);
    SDL_RenderPresent(renderer);
}

void drawpaderror(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_RenderClear(renderer);
    drawtext(renderer, font, "No gamepad found :(", cgap0, rgap0);
    SDL_RenderPresent(renderer);
}

SDL_Joystick* getpad(int padnum) {
    int npads = SDL_NumJoysticks();
    if(npads <= 0) return NULL;
    return SDL_JoystickOpen(padnum);
}

int main(int argc, char* argv[]) {
    const int padnum = 0;

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)) {
        cerr << "SDL_Init failed: " << SDL_GetError() << endl;
        exit(-1);
    }
    TTF_Init();

    TTF_Font* font = TTF_OpenFont(fontfile, fsize);
    if(!font) {
        cerr << "TTF_OpenFont failed: " << SDL_GetError() << endl;
        exit(-1);
    }

    SDL_Joystick *joystick = getpad(padnum);
    SDL_JoystickID instanceid = SDL_JoystickInstanceID(joystick);

    SDL_Window *window;
    window = SDL_CreateWindow("Button mashing v0.1 by qety1",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              w, h, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
                                                SDL_RENDERER_ACCELERATED |
                                                SDL_RENDERER_PRESENTVSYNC);

    const int updaterate = 100;
    bool running = true;
    bool hidden = true;
    SDL_Event event;
    Uint32 tick, ticksaved, tickfirst;
    deque<Uint32> data;
    double topscores[nintevals] = { 0, 0, 0, 0, 0, 0, 0 };
    double currentscores[nintevals] = { 0, 0, 0, 0, 0, 0, 0 };
    tickfirst = SDL_GetTicks();
    ticksaved = 0;
    while(running) {
        while(SDL_PollEvent(&event) > 0) {
            switch(event.type) {
            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                case SDL_WINDOWEVENT_SHOWN:
                    hidden = false;
                    break;
                case SDL_WINDOWEVENT_HIDDEN:
                    hidden = true;
                    break;
                }
                break;
            case SDL_JOYBUTTONDOWN:
                data.push_front(event.jbutton.timestamp);
                break;
            case SDL_JOYDEVICEADDED:
                if(!joystick) {
                    joystick = getpad(padnum);
                    instanceid = SDL_JoystickInstanceID(joystick);
                }
                break;
            case SDL_JOYDEVICEREMOVED:
                if(joystick && event.jdevice.which == instanceid) {
                    SDL_JoystickClose(joystick);
                    joystick = NULL;
                    instanceid = -1;
                }
                break;
            case SDL_QUIT:
                running = false;
                break;
            }
        }
        tick = SDL_GetTicks();
        if(tick > ticksaved + updaterate) {
            updateresults(tick, data, currentscores, topscores, false);
            ticksaved = tick;
        } else {
            updateresults(tick, data, currentscores, topscores);
        }
        if(!hidden) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            if(joystick) {
                draw(renderer, font, data, tick,
                     currentscores, topscores, tick-tickfirst);
            } else {
                drawpaderror(renderer, font);
            }
        }
    }
    SDL_DestroyWindow(window); 
    SDL_Quit(); 
    return 0;
}
