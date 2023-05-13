#ifndef __RAYTRACING_H
#define __RAYTRACING_H


typedef struct scene_s{
    ray camera;
    vector lighting_direction;
    int n_triangles;
    triangle* triangles;
} scene;


// Charge une scène depuis un fichier */
scene* load_scene(FILE* obj_file);

/* Simule n réflexions du rayon r dans la scene s et renvoie le tableau des rayons successifs de la trajectoire */
ray** simulate_ray(ray* r, scene* s, int n);


uint8_t** render_scene(scene* s, int width, int height, double horizontal_fov, int max_reflexions);



#endif