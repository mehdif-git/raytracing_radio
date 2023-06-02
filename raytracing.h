#ifndef __RAYTRACING_H
#define __RAYTRACING_H

typedef struct scene_s{
    vector tx;
    vector rx;
    int n_triangles;
    triangle* triangles;
    vector* normals;
} scene;

/* Charge une scène depuis un fichier */
scene* load_scene(FILE* obj_file, bool normals, bool textures);

/* Simule n réflexions du rayon r dans la scene s et renvoie le tableau des rayons successifs de la trajectoire, terminé par NULL */
ray** simulate_ray(ray* r, scene* s, int n);

// Donne la matrice des illuminations pour chaque pixel de l'image
uint8_t** render_scene(scene* s, int width, int height, double horizontal_fov, int rays_per_pixel, int max_reflexions);

#endif
