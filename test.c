#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include "geometry.h"
#include "raytracing.h"
#include "bitmap.h"

int main(){
    // On lance test nomFichier.obj resX resY h_fov max_ref
    //assert (argc==6);
    FILE* f = fopen("monke.obj", "r");
    scene* s = load_scene(f);
    fclose(f);
    
    int resX = 360;
    int resY = 240; 
    int max_ref = 3; 
    /*sscanf(argv[2], "%d", &resX);
    sscanf(argv[3], "%d", &resY);
    sscanf(argv[5], "%d", &max_ref);
    */
    double h_fov = 1.03;
    /*
    sscanf(argv[4], "%lf", &h_fov);
    */
    s->camera.origin.x = -5;
    s->camera.origin.y = 0;
    s->camera.origin.z = 5;

    s->camera.direction.x = 1;
    s->camera.direction.y = 0;
    s->camera.direction.z = -1;

    s->lighting_direction.x = 1;
    s->lighting_direction.y = 0;
    s->lighting_direction.z = -1;

    uint8_t** pixels = render_scene(s, resX, resY, h_fov, max_ref);

    char* path = malloc(128*sizeof(char));
    time_t t;
    time(&t);
    sprintf(path, "renders/render_%s.bmp", ctime(&t));

    f = fopen(path, "wb");

    bitmap_write(f, pixels, resX, resY);

    fclose(f);

    return 0;
}
