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
    // On lance test nomFichier.obj
    assert(argc==2);
    FILE* f = fopen(argv[1], "r");
    scene* s = load_scene(f);
    fclose(f);


    fprintf(stderr, "%d", s->n_triangles);

    s->camera.origin.x = -5;
    s->camera.origin.y = 0;
    s->camera.origin.z = 5;

    s->camera.direction.x = 1;
    s->camera.direction.y = 0;
    s->camera.direction.z = -1;

    s->lighting_direction.x = 1;
    s->lighting_direction.y = 0;
    s->lighting_direction.z = -1;


    uint8_t** pixels = render_scene(s, 360, 240, 1.03, 3);

    char* path = malloc(128*sizeof(char));
    time_t t;
    time(&t);
    sprintf(path, "renders/render_%s.bmp", ctime(&t));

    f = fopen(path, "wb");

    bitmap_write(f, pixels, 360, 240);

    fclose(f);

    return 0;
}
