#include <stdlib.h>


#include "geometry.h"
#include "raytracing.h"


ray* simulate_ray(ray* r, scene* s, int n){
    ray* path = malloc(n*sizeof(ray));
    path[0] = r;

    ray** collisions = malloc(sizeof(ray) * s->n_triangles);
    double* distances = malloc(sizeof(double) * s->n_triangles);

    /* On simule n-1 réflexions */
    for (int i = 0; i<n-1; i++){
        /* Pour chaque nouvelle réflexion on teste la collision avec chaque triangle de la scène
        On enregistre les distances des points de réflexions pour ne garder que le plus proche */
        int c = 0;
        for (int j=0; j<s->n_triangles; j++){
            collisions[c] = reflect(r, s->triangles[j]);
            if (collisions[c] != NULL){
                distances[c] = distance(path[i]->origin, collisions[c]->origin);
                c++;
            }
        }

        /* Dans le cas où le rayon ne rencontre aucun triangle on met fin à la simulation */
        if (0 == c){
            break;
        }

        /* On trouve le point d'intersection le plus proche */
        int min_i = 0;
        for (int j = 1; j<c; j++){
            if (distances[j] < distances[min_i]){
                min_i = j;
            }
        }

        /* On ajoute au chemin le nouveau rayon */
        path[i+1] = collisions[min_i];

        /* On libère les collisions ignorées */
        for (int j = 0; j<c; j++){
            if (j != min_i){
                free(collisions[j]);
            }
        }
    }
    free(distances);
    free(collisions);

    return path;
}


uint8_t** render_scene(scene* s, int width, int height, double horizontal_fov, int max_reflexions){
    ray** rays = malloc(height*sizeof(ray*));
    for (int i = 0; i < height; i++){
        rays[i] = malloc(width * ray);
        for (int j = 0; )
    }

}