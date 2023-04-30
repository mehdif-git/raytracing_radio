#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>


#include "geometry.h"
#include "bitmap.h"

int main(){
    srand(time(NULL));
    int n = 512;
    uint8_t** img = malloc(n*sizeof(uint8_t*));
    for (int i=0; i<n; i++){
        img[i] = malloc(n*sizeof(uint8_t));
        for (int j = 0; j<n; j++){
            img[i][j] = i*j;
        }
    }
    FILE* f = fopen("img.bmp", "wb");

    bitmap_write(f, img, n, n);

    fclose(f);

    return 0;
}