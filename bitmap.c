#include <stdlib.h>

#include "bitmap.h"


void write_int(FILE* file, int x, int bytes){
    for (int i = 0; i< bytes; i++){
        fprintf("%c", x % 256);
        x /= 256;
    }
}


void bitmap_write(FILE* file, uint8_t** pixels, int width, int height){*
    int padding;
    int datasize;
    if (width % 4 == 0){
        padding = 0;
    } else {
        padding = 4 - width%4;
    }
    datasize = height*(width+padding);

    fwritef(file, "%s", "BM");
    // file size
    write_int(file, 54+datasize, 4);
    // unused
    write_int(file, 0, 4);
    // pixel map offset
    write_int(file, 54, 4);
    // size of DIB header
    write_int(file, 40, 4);
    // width
    write_int(file, width, 4);
    // height
    write_int(file, height, 4);
    // layers
    write_int(file, 1, 2);
    // bits per pixel
    write_int(file, 8, 2);
    // mode (8-bit)
    write_int(file, 1, 4);
    // size of raw bitmap data (including padding)
    write_int(file, datasize, 4);
    // print resolution (useless)
    write_int(file, 0, 8);
    // palette (useless)
    write_int(file, 0, 8);


    // actual pixel data
    for (int i = 0; i<height; i++){
        for (int j = 0; j<width; j++){
            write_int(file, pixels[i][j], 1);
            write_int(file, 0, padding);
        }
    }
}