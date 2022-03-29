#include <cstdint>
#include <vector>
#include <iostream>
#include <fstream>
#include <SDL2/SDL.h>

using namespace std;

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 640

void set_pixel(SDL_Surface *surface, int x, int y, uint32_t pixel)
{
  uint32_t * const target_pixel = (uint32_t *) ((uint8_t *) surface->pixels
                                             + y * surface->pitch
                                             + x * surface->format->BytesPerPixel);
  *target_pixel = pixel;
}

void set_pixels(SDL_Surface *surface, uint32_t img[][SCREEN_WIDTH]) {
    for (int y = 0; y < SCREEN_HEIGHT; y++)
        for (int x = 0; x < SCREEN_WIDTH; x++)
            set_pixel(surface, x, y, img[y][x]);
}

int main(int argc, char* args[]) {

    ifstream input("/Users/davefol/git/png_decoder/examples/smile.png", ios::binary);
    vector<uint8_t> buffer(std::istreambuf_iterator<char>(input), {});
    cout << "size of image " << buffer.size() << endl;

    // We are expecting the PNG header:
    // 89 
    // 50 4e 47 "PNG"
    // 0d 0a 
    // 1a 0a

    cout << "header: ";
    for (size_t i = 0; i < 8; i++) {
        cout << setw(2) << setfill('0') << hex << (int)buffer[i] << " ";
    }
    cout << endl;


    // pixel value is 32 bits -> 4 bytes
    // 0xFF:: is black
    // 0xFFFF:: is red
    // 0xFF00FF00 is green
    // 0xFF0000FF is blue
    // 0x __ RR GG BB
    // Demo image
    uint32_t img[SCREEN_HEIGHT][SCREEN_WIDTH] = {{0}};

    for (int x = 100; x < 150; x++) 
        for(int y = 100; y < 150; y++)
            img[y][x] = 0xFF0000FF;


    // ======================================
    // --------- Displaying Image -----------
    // ======================================

    SDL_Window* window = NULL;
    SDL_Surface* surface = NULL;
    if (SDL_Init( SDL_INIT_VIDEO) < 0) {
        cout << SDL_GetError() << endl;
    } else {
        window = SDL_CreateWindow(
            "PNG Viewer",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
            SDL_WINDOW_SHOWN
        );

        if (window == NULL) {
            cout << SDL_GetError() << endl;
        } else {
            surface = SDL_GetWindowSurface(window);
            //Fill the surface white
            SDL_FillRect( surface, NULL, SDL_MapRGB( surface->format, 0xFF, 0xFF, 0xFF ) );

            set_pixels(surface, img);
            
            //Update the surface
            SDL_UpdateWindowSurface( window );

            bool quit = false;
            SDL_Event e;
            while (!quit) {
                //Handle events on queue
                while( SDL_PollEvent( &e ) != 0 )
                {
                    //User requests quit
                    if( e.type == SDL_QUIT )
                    {
                        quit = true;
                    }
                }
            }

        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
