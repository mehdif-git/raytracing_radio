#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "geometry.h"
#include "raytracing.h"
#include "bitmap.h"


int main(int argc, char *argv[]){
    // On lance test nomFichier.obj resX resY h_fov max_ref iter
    assert (argc==7);
    FILE* f = fopen(argv[1], "r");
    scene* s = load_scene(f, true, true);
    fclose(f);
    
    int resX, resY, max_ref, iterations;
    sscanf(argv[2], "%d", &resX);
    sscanf(argv[3], "%d", &resY);
    sscanf(argv[5], "%d", &max_ref);
    sscanf(argv[6], "%d", &iterations);
    double h_fov;
    sscanf(argv[4], "%lf", &h_fov);

    
    //Mise en place de la caméra et du plan d'illumination

    s->camera.origin.x = -360;
    s->camera.origin.y = 111;
    s->camera.origin.z = 163;

    s->camera.direction.x = 0.87;
    s->camera.direction.y = 0;
    s->camera.direction.z = -1/2;

    s->lighting_direction.x = 0.87;
    s->lighting_direction.y = 0;
    s->lighting_direction.z = -1/2;

    srand(time(NULL));

    uint8_t** pixels = render_scene(s, resX, resY, h_fov / 180 * M_PI, iterations, max_ref);

    char* path = malloc(128*sizeof(char));
    time_t t;
    time(&t);
    sprintf(path, "renders/render_%s.bmp", ctime(&t));

    f = fopen(path, "wb");

    bitmap_write(f, pixels, resX, resY);

    fclose(f);

    free(path);

    free(s->triangles);
    free(s);


    for (int i = 0; i < resY; i++){
        free(pixels[i]);
    }

    free(pixels);

    return EXIT_SUCCESS;
}
