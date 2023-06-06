#ifndef __RAYTRACING_H
#define __RAYTRACING_H

typedef struct scene_s{
    vector tx;
    int n_triangles;
    triangle* triangles;
} scene;

/* Charge une scène depuis un fichier */
scene* load_scene(FILE* obj_file, bool normals, bool textures, vector tx);

/* Renvoie, s'il existe, le rayon réfléchi à partir du rayon incident et d'un triangle 
Renvoie NULL si l'intersection n'existe pas */
ray* reflect(ray* r, triangle* t, double _Complex ref_index);

/* Simule n réflexions du rayon r dans la scene s et renvoie le chemin pris par le rayon si sa puissance est assez grande, NULL sinon */
ray** simulate_ray(ray* r, scene* s, int max_ref);

/* Renvoie l'ensemble des chemins pris par tous les rayons émis */
ray ****propagate(scene *s, int ray_density, int max_reflections);

/* Donne la puissance reçue par un récepteur rx après le lancement des rayons de tx */
double recieved_power(ray ****paths, int ray_density, int max_reflections, vector rx);

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
