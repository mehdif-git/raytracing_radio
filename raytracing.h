#ifndef __RAYTRACING_H
#define __RAYTRACING_H


typedef struct scene_s{
    vector camera_location;
    vector camera_angle;
    vector lighting_angle;
    int n_triangles;
    triangle* triangles;
} scene;


/* Charge une scène depuis un fichier */
scene* load_scene(FILE* f);

/* Simule n réflexions du rayon r dans la scene s et renvoie le tableau des rayons successifs de la trajectoire */
ray* simulate_ray(ray r, scene* s, int n);



#endif