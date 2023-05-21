#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
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
    scene* s = load_scene(f, true, true);
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

    s->lighting_direction.x = 0;
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

    /*
    scene s;
    s.n_triangles = 2;
    s.triangles = malloc(2*sizeof(triangle));
    s.triangles[0].a = (vector) {1, 1, 0};
    s.triangles[0].b = (vector) {1, -1, 0};
    s.triangles[0].c = (vector) {1, 0, 1};
    s.triangles[1].a = (vector) {-1, 1, 0};
    s.triangles[1].b = (vector) {-1, -1, 0};
    s.triangles[1].c = (vector) {-1, 0, 1};

    ray r;
    r.origin = (vector) {0,0,0};
    r.direction= (vector) {1,0,0};


    ray** path = simulate_ray(&r, &s, 10);

    for (int i=0; i<10; i++){
        printf("%f %f %f\n", path[i]->origin.x, path[i]->origin.y, path[i]->origin.z);
    }
    */

    return 0;
}
