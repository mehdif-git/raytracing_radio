#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <complex.h>
#include "geometry.h"
#include "raytracing.h"

const double complex refractive_index = 2.31 - 0.12 * I; // Indice de réfraction du béton
const vector vert_pol = (vector) {0,1,0};
const double wavelength = 0.3;

scene* load_scene(FILE* obj_file, bool normals, bool textures){
    // Charge une scène à partir d'un fichier obj
    // Ne fonctionne qu'avec des triangles
    char* buffer = malloc(1024*sizeof(char));
    int n_vertices = 0;
    int n_triangles = 0;
    
    // Comptage du nombre de points et de triangles
    while (fgets(buffer, 1024, obj_file)){
        if (buffer[0] == 'v' && buffer[1] == ' '){
            n_vertices++;
        } else if (buffer[0] == 'f'){
            n_triangles++;
        }
    }

    //Création des tableaux de stockage
    vector* vertices = malloc(n_vertices*sizeof(vector));
    triangle* triangles = malloc(n_triangles*sizeof(triangle));

    rewind(obj_file);

    int v_i = 0;
    int t_i = 0;

    char* ptr;

    while (fgets(buffer, 1024, obj_file)){ //On parse le fichier pour en tirer les coordonnées des points
        if (buffer[0] == 'v' && buffer[1] == ' '){
            vertices[v_i].x = strtof(buffer+2, &ptr);
            vertices[v_i].y = strtof(ptr+1, &ptr);
            vertices[v_i].z = strtof(ptr+1, &ptr);
            v_i++;
        } else if (buffer[0] == 'f'){ //On associe les points à leurs triangles respectifs
            triangles[t_i].a = vertices[strtol(buffer+2, &ptr, 10)-1];
            // skip normals and textures
            if (normals){strtol(ptr+1, &ptr, 10);}
            if (textures){strtol(ptr+1, &ptr, 10);}
            triangles[t_i].b = vertices[strtol(ptr+1, &ptr, 10)-1];
            if (normals){strtol(ptr+1, &ptr, 10);}
            if (textures){strtol(ptr+1, &ptr, 10);}
            triangles[t_i].c = vertices[strtol(ptr+1, &ptr, 10)-1];
            triangles[t_i].n = normalize(cross_product(vector_diff(triangles[t_i].a, triangles[t_i].b),vector_diff(triangles[t_i].a, triangles[t_i].c)));

            t_i++;
        }
    }

    scene* s = malloc(sizeof(scene));
    s->n_triangles = n_triangles;
    s->triangles = triangles;
    fprintf(stderr, "Fin du chargement d'une scène composée de %d triangles.\n", s->n_triangles);
    free(vertices);
    free(buffer);
    return s;
}


ray** simulate_ray(ray* r, scene* s, int n_max){
    ray** path = malloc((n_max+1)*sizeof(ray));
    for (int i=0; i<n_max+1; i++){
        path[i] = NULL;
    }
    path[0] = r;
    //Tableaux notant les collisions et la distance du rayon à tous les triangles rencontrés
    vector** collisions = malloc(sizeof(vector*) * s->n_triangles);
    double* distances = malloc(sizeof(double) * s->n_triangles);
    int* collided_triangles = malloc(sizeof(int) * s->n_triangles);
    int last_triangle = -1;

    /* On simule n-1 réflexions */
    for (int i = 0; i<n_max-1; i++){
        /* Pour chaque nouvelle réflexion on teste la collision avec chaque triangle de la scène
        On enregistre les distances des points de réflexions pour ne garder que le plus proche */
        int c = 0;
        for (int j=0; j<s->n_triangles; j++){
            if (j != last_triangle){
                collisions[c] = intersect(path[i], s->triangles+j);
                if (collisions[c] != NULL){
                    distances[c] = distance(path[i]->origin, *(collisions[c]));
                    collided_triangles[c] = j;
                    c++;
                }
            }
        }
        /* Dans le cas où le rayon ne rencontre aucun triangle on met fin à la simulation */
        if (0 == c){
            // fprintf(stderr, "%s", "no colision\n");
            break;
        }

        /* On trouve le point d'intersection le plus proche mais non nul */
        int min_i = 0;

        for (int j = min_i; j<c; j++){
            if (distances[j] < distances[min_i] && distances[j] > 0){
                min_i = j;
            }
        }

        /* On note le triangle correspondant comme étant le dernier percuté */
        last_triangle = collided_triangles[min_i];

        /* On ajoute au chemin le nouveau rayon réfléchi */
        path[i+1] = reflect(path[i], s->triangles+last_triangle, refractive_index, vert_pol);

        if (path[i+1] == NULL){
            fprintf(stderr,"%s", "bruh");
        }

        /* On libère les collisions */
        for (int j = 0; j<c; j++){
            free(collisions[j]);
        }
    }
    free(distances);
    free(collisions);
    free(collided_triangles);

    return path;
}

// Création d'une matrice d'entiers entre 0 et 255 pour générer un bitmap
// Le champ de vision sur l'horizontale doit être entré en radians
uint8_t** render_scene(scene* s, int width, int height, int rays_per_pixel, int max_reflexions){
    ray r;
    ray** path;
    uint8_t** pixels = malloc(height * sizeof(uint8_t*));

    //Classique arctan(y/x) pour trouver l'angle de la caméra
    double cam_horiz_angle;
    if (s->camera.direction.x > 0){
        cam_horiz_angle = atan(s->camera.direction.y/s->camera.direction.x);
    } else if (s->camera.direction.x < 0){
        cam_horiz_angle = M_PI + atan(s->camera.direction.y/s->camera.direction.x);
    } else {
        if (s->camera.direction.y >= 0){
            cam_horiz_angle = M_PI/2;
        } else {
            cam_horiz_angle = - M_PI/2;
        }
    }
    double cam_vert_angle;
    if (s->camera.direction.x > 0){
        cam_vert_angle = atan(s->camera.direction.z/s->camera.direction.x);
    } else if (s->camera.direction.x < 0){
        cam_vert_angle = M_PI + atan(s->camera.direction.z/s->camera.direction.x);
    } else {
        if (s->camera.direction.z >= 0){
            cam_vert_angle = M_PI/2;
        } else {
            cam_vert_angle = - M_PI/2;
        }
    }

    double window_length = 2*tan(horizontal_fov/2);
    double window_height = (double) height / width * window_length;

    int n;
    
    // Génération du bitmap ligne par ligne
    for (int i = 0; i < height; i++){
        pixels[i] = malloc(width * sizeof(uint8_t));
       fprintf(stderr, "Ligne %d sur %d\n", i, height);
        // Génération des rayons de manière uniforme sur l'aire donnée
        for (int j = 0; j < width; j++){
            r.origin = s->camera.origin;
            r.direction.x = 1;
            r.direction.y = - ((double) j/width - 0.5) * window_length;
            r.direction.z = - ((double) i/height - 0.5) * window_height;

            // rotation verticale autour de l'axe y

            double new_x = cos(cam_vert_angle) * r.direction.x - sin(cam_vert_angle) * r.direction.z;
            double new_z = cos(cam_vert_angle) * r.direction.z + sin(cam_vert_angle) * r.direction.x;

            r.direction.x = new_x;
            r.direction.z = new_z;

            // rotation horizontale autour de l'axe z

            new_x = cos(cam_horiz_angle) * r.direction.x - sin(cam_horiz_angle) * r.direction.y;
            double new_y = cos(cam_horiz_angle) * r.direction.y + sin(cam_horiz_angle) * r.direction.x;

            r.direction.x = new_x;
            r.direction.y = new_y;
            r.power = 1;
            double total = 0;


            for (int k=0; k<rays_per_pixel; k++){

                // fprintf(stderr, "%d %d %d\n", i, j, k);

                path = simulate_ray(&r, s, max_reflexions);

                // On trouve le dernier rayon du chemin
                n = 0;
                while (path[n] != NULL){n++;}

                // On calcule la luminosité
                total += path[n-1]->power;
                // Libération de la mémoire
                for (int i=1; i<n; i++){
                    free(path[i]);
                }
                free(path);
            }
            pixels[i][j] = (uint8_t) (2*total / rays_per_pixel * 255);
        }
    }
    return pixels;
}
