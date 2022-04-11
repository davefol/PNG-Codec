#include <cstdint>
#include <vector>
#include <iostream>
#include <fstream>
#include "SDL.h"
#include <string>

#include "Chunk.h"
#include "IHDR.h"
#include "IDAT.h"
#include "PLTE.h"
#include "UnknownChunk.h"
#include "consume.h"
#include "huffmanutils.h"
#include "ImagePartial.h"
#include "dumputils.h"

using namespace std;

#define SCREEN_WIDTH 440
#define SCREEN_HEIGHT 440
#define DEBUG 1

// Define Chunk types
#define __IHDR 0x49484452
#define __PLTE 0x504c5445
#define __IDAT 0x49444154
#define __IEND 0x49454e44

void set_pixel(SDL_Surface *surface, int x, int y, uint32_t pixel)
{
  uint32_t * const target_pixel = (uint32_t *) ((uint8_t *) surface->pixels
                                             + y * surface->pitch
                                             + x * surface->format->BytesPerPixel);
  *target_pixel = pixel;
}

void set_pixels(SDL_Surface *surface, vector<vector<uint32_t>> img) {
    for (int y = 0; y < img.size(); y++)
        for (int x = 0; x < img[y].size(); x++)
            set_pixel(surface, x, y, img[y][x]);
}



void print_hex(uint8_t c) {
    cout << setw(2) << setfill('0') << hex << (int)c << dec << " ";
} 

unique_ptr<Chunk> consume_chunk(vector<uint8_t>::iterator &buffer_it) {
    // read size
    uint32_t chunk_size = consume_uint32_t(buffer_it);
    // read name
    uint32_t chunk_name = consume_uint32_t(buffer_it);
    // reinterpretting 32bit integer
    // as 4 char bytes
    vector<char> vec_char_name {
        (char)((chunk_name & (0xFF << 8 * 3)) >> 24), // first byte
        (char)((chunk_name & (0xFF << 8 * 2)) >> 16), // second byte
        (char)((chunk_name & (0xFF << 8 * 1)) >> 8), // third byte
        (char)((chunk_name & (0xFF << 8 * 0)) >> 0), // fourth byte
    };
    string chunk_name_str = string(vec_char_name.begin(), vec_char_name.end());

    // dispatch on name
    switch (chunk_name) {
        case __IHDR:
            // do stuff
            return make_unique<IHDR>(chunk_size, chunk_name_str, buffer_it);
            break;
        case __IDAT:
            return make_unique<IDAT>(chunk_size, chunk_name_str, buffer_it);
            break;
        case __PLTE:
            return make_unique<PLTE>(chunk_size, chunk_name_str, buffer_it);
            break;
        //case __IEND:
        //    // do stuff
        //    break;
        default:
            return make_unique<UnknownChunk>(chunk_size, chunk_name_str, buffer_it);

    }
}

void display_image(vector<vector<uint32_t>> img) {
    // ======================================
    // --------- Displaying Image -----------
    // ======================================
    // pixel value is 32 bits -> 4 bytes
    // 0xFF:: is black
    // 0xFFFF:: is red
    // 0xFF00FF00 is green
    // 0xFF0000FF is blue
    // 0x __ RR GG BB
    // Demo image
    SDL_Window* window = NULL;
    SDL_Surface* surface = NULL;
    if (SDL_Init( SDL_INIT_VIDEO) < 0) {
        cout << SDL_GetError() << endl;
    } else {
        window = SDL_CreateWindow(
            "PNG Viewer",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            img[0].size(),
            img.size(),
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
}

int main(int argc, char* args[]) {

    ifstream input(args[1], ios::binary);
    vector<uint8_t> buffer(std::istreambuf_iterator<char>(input), {});
    cout << "size of image " << buffer.size() << endl;

    // We are expecting the PNG header:
    // 89 
    // 50 4e 47 "PNG"
    // 0d 0a 
    // 1a 0a


    vector<uint8_t>::iterator buffer_it = buffer.begin();
    cout << "header: ";
    for (int i = 0; i < 8; i++) {
        print_hex(*buffer_it + i);
    }
    cout << endl;
    advance(buffer_it, 8);
#if DEBUG 
        cout << "Consumed PNG header (8 bytes)\n";
#endif

    ImagePartial image_partial = {
        
    };
    while (buffer_it != buffer.end()) {
        auto chunk_ptr = consume_chunk(buffer_it);
        cout << "Chunk name: " << chunk_ptr->name << endl;
        cout << "Chunk size: " << chunk_ptr->size << endl;
        chunk_ptr->print();
        chunk_ptr->modify(image_partial);
        cout << endl;
    }
    // after the above while loop
    // image partial should be a well behaved in memory image

    std::vector<std::vector<uint32_t>> final_img = image_partial.get_image();
    //dump_bytes("img.raw", final_img.begin(), final_img.end());
    display_image(image_partial.get_image());
    return 0;
}
