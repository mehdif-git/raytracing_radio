#ifndef __RAYTRACING_H
#define __RAYTRACING_H

typedef struct scene_s{
    vector tx;
    int n_triangles;
    triangle* triangles;
    vector* normals;
} scene;

/* Charge une scène depuis un fichier */
scene* load_scene(FILE* obj_file, bool normals, bool textures, vector tx);

/* Simule n réflexions du rayon r dans la scene s et renvoie un pointeur vers le rayon arrivant au récepteur si sa puissance est assez grande, NULL sinon */
ray* simulate_ray(ray* r, scene* s, int max_ref, vector rx);

/* Donne la puissance reçue par un récepteur rx après le lancement des rayons de tx */
double recieved_power(scene *s, int ray_density, int max_reflections, vector rx);

/* Simule une série de mesure et compile les résultats dans un fichier CSV :
 * Une scène,  
 * un pointeur vers le fichier CSV, 
 * le nombre de rayons émis par stéradian, 
 * le nombre de réflexions maximal à considérer,
 * une direction d'éloignement et 
 * un nombre de mesures à faire
 */
void raytrace(scene* s, FILE *write_results, int ray_density, int max_reflexions, vector direction, int nb_mesures);

#endif
