#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <string.h>


#include "geometry.h"
#include "raytracing.h"
#include "bitmap.h"



int main(){

    FILE* f = fopen("models/donut_hd.obj", "r");
    scene* s = load_scene(f, true, true);
    fclose(f);


    fprintf(stderr, "rendering %d triangles", s->n_triangles);

    s->camera.origin.x = -5;
    s->camera.origin.y = 0;
    s->camera.origin.z = 5;

    s->camera.direction.x = 1;
    s->camera.direction.y = 0;
    s->camera.direction.z = -1;

    s->lighting_direction.x = 0;
    s->lighting_direction.y = 0;
    s->lighting_direction.z = -1;


    uint8_t** pixels = render_scene(s, 120, 90, 1.03, 5);

    char* path = malloc(128*sizeof(char));
    time_t t;
    time(&t);
    sprintf(path, "renders/render_%s.bmp", ctime(&t));

    f = fopen(path, "wb");

    bitmap_write(f, pixels, 120, 90);

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