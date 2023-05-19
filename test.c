#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include "geometry.h"
#include "raytracing.h"
#include "bitmap.h"

int main(int argc, char *argv[]){
    // On lance test nomFichier.obj resX resY h_fov max_ref
    assert (argc==6);
    FILE* f = fopen(argv[1], "r");
    scene* s = load_scene(f);
    fclose(f);
    
    int resX, resY, max_ref;
    sscanf(argv[2], "%d", &resX);
    sscanf(argv[3], "%d", &resY);
    sscanf(argv[5], "%d", &max_ref);
    double h_fov;
    sscanf(argv[4], "%lf", &h_fov);
    
    //Mise en place de la caméra et du plan d'illumination
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
