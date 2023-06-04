#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <complex.h>
#include "geometry.h"
#include "raytracing.h"

const double complex refractive_index = 2.31 - 0.12 * I; // Indice de réfraction du béton
const double wavelength = 0.3; // Longeur d'onde 900MHz
const double rsrp = -100; // Rapport puissance reçue/puissance émise en dBm
const double rx_size = 0.3; // Taille du récépteur, choix d'une longeur d'onde ici
const double init_pow = 1000; // Puissance totale émise (en milliwatts)

// Charge une scène à partir d'un fichier obj, ne fonctionne qu'avec des triangles
scene* load_scene(FILE* obj_file, bool normals, bool textures, vector tx){   
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

    // Création des tableaux de stockage
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
        } else if (buffer[0] == 'f'){ //On associe les points à leurs triangles respectifs, on ignore les données sur les normales et les textures

            
            triangles[t_i].a = vertices[strtol(buffer+2, &ptr, 10)-1];
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

    // Allocation en mémoire de la scène
    scene* s = malloc(sizeof(scene));
    s->n_triangles = n_triangles;
    s->triangles = triangles;
    s->tx = tx;

    fprintf(stderr, "Fin du chargement d'une scène composée de %d triangles.\n", s->n_triangles);

    free(vertices);
    free(buffer);
    return s;
}


ray* simulate_ray(ray* r, scene* s, int max_ref, vector rx){
    ray** path = malloc((max_ref+1)*sizeof(ray));
    for (int i=0; i<max_ref+1; i++){
        path[i] = NULL;
    }
    path[0] = r;
    
    //Tableaux notant les collisions et la distance du rayon à tous les triangles rencontrés
    vector** collisions = malloc(sizeof(vector*) * s->n_triangles);
    double* distances = malloc(sizeof(double) * s->n_triangles);
    int* collided_triangles = malloc(sizeof(int) * s->n_triangles);
    int last_triangle = -1;

    // On simule n-1 réflexions
    for (int i = 0; i<max_ref-1; i++){
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
        // Dans le cas où le rayon ne rencontre aucun triangle on met fin à la simulation
        if (0 == c){
            break;
        }

        // On trouve le point d'intersection le plus proche mais non nul
        int min_i = 0;

        for (int j = min_i; j<c; j++){
            if (distances[j] < distances[min_i] && distances[j] > 0){
                min_i = j;
            }
        }

        // On note le triangle correspondant comme étant le dernier percuté
        last_triangle = collided_triangles[min_i];

        // On ajoute au chemin le nouveau rayon réfléchi
        path[i+1] = reflect(path[i], s->triangles+last_triangle, refractive_index);

        // On libère les collisions
        for (int j = 0; j<c; j++){
            free(collisions[j]);
        }
    }

    // Libération des tableaux temporaires

    free(distances);
    free(collisions);
    free(collided_triangles);

    // Tests de réception
    int k = 0;
    while(path[k]!=NULL){k++;}
    
    double power_ratio = millis_to_dbm(path[0]->power, path[k-1]->power);

    if(distance(s->tx,rx) > rx_size || power_ratio < rsrp){
      free(path);
      return NULL;
    }
    ray *last_ray = malloc(sizeof(ray));

    last_ray->origin = path[k-1]->origin;
    last_ray->direction = path[k-1]->direction;
    last_ray->E_field = path[k-1]->E_field;
    last_ray->phase = path[k-1]->phase;
    last_ray->power = path[k-1]->power;

    free(path);
    return last_ray;
}

double recieved_power(scene *s, int ray_density, int max_reflections, vector rx){
  double total = 0;
    for(int rot_1 = 0; rot_1<ray_density; rot_1++){
      for(int rot_2 = 0; rot_2<ray_density; rot_2++){
        
        double theta = 2*M_PI*rot_1;
        double phi = M_PI*rot_2;

        vector direction = sph_to_cart(1, theta, phi);
        
        // On utilise une onde en polarisation verticale
        vector E_field = sph_to_cart(1, M_PI-2*theta, phi);

        ray *launched = malloc(sizeof(ray));
        launched->origin = s->tx;
        launched->direction = direction;
        launched->E_field = E_field;
        launched->phase = 0;
        launched->power = init_pow/(4*M_PI); // Puissance surfacique
        
        ray *recieved = simulate_ray(launched, s, max_reflections, rx);
        if(recieved!=NULL){
          total+=recieved->power;
        }
        free(launched);
        free(recieved);
      }
    }
  return total;
}  

void raytrace(scene *s, FILE *write_results, int ray_density, int max_reflections, vector direction, int nb_mesures){
  for(int i = 1; i < nb_mesures + 1; i++){
      // Mesure à distance croissante par rapport à l'emmeteur
      vector rx = vector_add(extern_prod(direction,i), s->tx);
      double recieved = recieved_power(s, ray_density, max_reflections, rx);
      fprintf(write_results, "%d,%lf\n", i, recieved);
  }
}
