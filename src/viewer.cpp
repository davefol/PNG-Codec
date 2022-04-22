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

std::vector<std::vector<uint64_t>> pack_image(std::vector<std::vector<std::vector<uint16_t>>> img) {
    std::vector<std::vector<uint64_t>> out;
    for (int y = 0; y < img.size(); y++) {
        std::vector<uint64_t> scanline;
        for (int x = 0; x < img[y].size(); x++) {
            uint64_t pixel = 0;
            for (int c = 0; c < img[y][x].size(); c++) {
                pixel |= img[y][x][c] << (8 * c);
            }
            scanline.push_back(pixel);
        }
        out.push_back(scanline);
    }
    return out;
}

void set_pixel(SDL_Surface *surface, int x, int y, uint32_t pixel)
{
  uint32_t * const target_pixel = (uint32_t *) ((uint8_t *) surface->pixels
                                             + y * surface->pitch
                                             + x * surface->format->BytesPerPixel);
  *target_pixel = pixel;
}

void set_all_scanline(SDL_Surface *surface, vector<uint64_t> img) {
    uint32_t * const target = (uint32_t *) ((uint8_t *) surface->pixels);
    std::copy(img.begin(), img.end(), target);
}

void set_scanline(SDL_Surface *surface, int y, vector<uint64_t> scanline) {
    uint32_t * const target_scanline = (uint32_t *) ((uint8_t *) surface->pixels + y * surface-> pitch);
    std::copy(scanline.begin(), scanline.end(), target_scanline);
}

void set_pixels(SDL_Surface *surface, vector<vector<uint64_t>> img) {
    for (int y = 0; y < img.size(); y++)
        set_scanline(surface, y, img[y]);
        //for (int x = 0; x < img[y].size(); x++)
        //    set_pixel(surface, x, y, img[y][x]);
}




void print_hex(uint8_t c) {
    cout << setw(2) << setfill('0') << hex << (int)c << dec << " ";
} 

unique_ptr<Chunk> consume_chunk(vector<uint8_t>::iterator &buffer_it, vector<uint8_t>& idat_buffer) {
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
    std::cout << chunk_name_str << " " << (int)chunk_size;

    // dispatch on name
    switch (chunk_name) {
        case __IHDR:
            // do stuff
            return make_unique<IHDR>(chunk_size, chunk_name_str, buffer_it);
        case __IDAT:
            idat_buffer.insert(idat_buffer.end(), buffer_it, buffer_it+chunk_size);
            return make_unique<UnknownChunk>(chunk_size, chunk_name_str, buffer_it);
        case __PLTE:
            return make_unique<PLTE>(chunk_size, chunk_name_str, buffer_it);
        //case __IEND:
        //    // do stuff
        //    break;
        default:
            return make_unique<UnknownChunk>(chunk_size, chunk_name_str, buffer_it);

    }
}

void display_image(vector<vector<uint64_t>> img) {
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
        SDL_SetWindowResizable(window, SDL_TRUE);

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
    std::vector<uint8_t> idat_buffer;
    while (buffer_it != buffer.end()) {
        auto chunk_ptr = consume_chunk(buffer_it, idat_buffer);
        cout << "Chunk name: " << chunk_ptr->name << endl;
        cout << "Chunk size: " << chunk_ptr->size << endl;
        chunk_ptr->print();
        chunk_ptr->modify(image_partial);
        cout << endl;
    }
    auto idat_buffer_it = idat_buffer.begin();
    auto idat_chunk = make_unique<IDAT>(idat_buffer.size(), "IDAT", idat_buffer_it);
    idat_chunk->print();
    std::cout << "calling final IDAT modify" << std::endl;
    idat_chunk->modify(image_partial);
    std::cout << "finished final IDAT modify" << std::endl;
    // after the above while loop
    // image partial should be a well behaved in memory image

    std::cout << "Generating image" << std::endl;
    image_partial.generate_image();
    std::cout << "Packing image" << std::endl;
    std::vector<std::vector<uint64_t>> final_img = pack_image(image_partial.get_image());
    std::cout << "Finished packing image" << std::endl;
    //dump_bytes("img.raw", final_img.begin(), final_img.end());
    std::string infgen_true = "";
    if (argc > 2)
        infgen_true = args[2];
    if (infgen_true == "--infgen") {
    } else {
        std::cout << "Displaying image" << std::endl;
        display_image(final_img);
    }
    return 0;
}
