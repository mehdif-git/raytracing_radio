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

ray* reflect(ray* r, triangle* t, double complex ref_index){
    vector* p = intersect(r, t);
    if (NULL == p){
        return NULL;
    }

    // Calcul de la composante normale du vecteur incident
    double normal_component = dot_product(t->n, r->direction);

    // On inverse la composante normale pour créer le rayon réfléchi
    vector new_dir;
    new_dir.x = r->direction.x - 2*(t->n).x*normal_component;
    new_dir.y = r->direction.y - 2*(t->n).y*normal_component;
    new_dir.z = r->direction.z - 2*(t->n).z*normal_component;
    
    // Calcul de la puissance du rayon réfléchi
    vector eik = normalize(r->direction);
    vector plane_normal = normalize(cross_product(eik, t->n));

    // Projection du vecteur E sur le plan d'incidence
    double te_component = dot_product(r->E_field, plane_normal);
    vector E_te = extern_prod(plane_normal,te_component); 
    vector E_tm = vector_diff(r->E_field,E_te);

    double n_bet = creal(ref_index);
    double te_prop = abs(length(E_te)/length(r->E_field));
    
    double cos_in = fabs(dot_product(eik, t->n));
    double sin_in = sin(acos(cos_in));
    double cos_tr = sqrt(1 - (1/n_bet)*(1/n_bet)*sin_in*sin_in);
    
    double R_te = (cos_in - n_bet * cos_tr)/(cos_in + n_bet * cos_tr);
    double R_tm = (n_bet * cos_in - cos_tr)/(n_bet * cos_in + cos_tr);

    double pow_te = fabs(R_te) * fabs(R_te);
    double pow_tm = fabs(R_tm) * fabs(R_tm);
   
    double new_power = r->power * (te_prop * pow_te + (1-te_prop) *pow_tm); 
    
    // On donne le nouveau vecteur de polarisation
    vector new_pola;
    new_pola = vector_add(extern_prod(E_te, R_te), extern_prod(E_tm, R_tm));

    // Allocation du nouveau rayon et initialisation de ce dernier
    ray* res = malloc(sizeof(ray));

    res->origin = *p;
    res->direction = new_dir;
    res->power = new_power;
    res->phase = (2 * M_PI / wavelength) * length(*p) + (M_PI / 2);
    res->E_field = new_pola;

    free(p);

    return res;
}

ray** simulate_ray(ray* r, scene* s, int max_ref){
    ray** path = malloc((max_ref+1)*sizeof(ray*));
    for(int i = 0; i<max_ref+1;i++){
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

    return path;
}

ray**** propagate(scene *s, int ray_density, int max_reflections){
  // Initialisation du tableau

  ray**** paths = malloc(sizeof(ray***)*ray_density);
  for(int i = 0;i<ray_density;i++){
    paths[i] = malloc(sizeof(ray**)*ray_density);
    for(int j=0; j<ray_density;j++){
      paths[i][j] = malloc(sizeof(ray*)*max_reflections);
    }
  }

  for(int i = 0; i<ray_density; i++){
    for(int j = 0; j<ray_density; j++){
      for(int k = 0; k<max_reflections;k++){
        paths[i][j][k] = NULL;
      }
    }
  }

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
        
        ray **sim = simulate_ray(launched, s, max_reflections);
        for(int k = 0; k<max_reflections;k++){ 
          paths[rot_1][rot_2][k] = sim[k];
        }
        free(launched);
        free(sim);
      }
    }
  
  return paths;
}  

double recieved_power(ray ****paths, int ray_density, int max_reflections, vector rx){
  double total = 0;
  for(int i = 0; i < ray_density; i++){
    for(int j = 0; j < ray_density; j++){
      for(int k = 0; k < max_reflections; k++){
        if(paths[i][j][k] != NULL){
          vector unit_line = normalize(paths[i][j][k]->direction);
          vector on_line = vector_diff(rx, paths[i][j][k]->origin);
          vector proj_on_line = extern_prod(unit_line, dot_product(on_line,unit_line));
          double distance_to_rx = length(vector_diff(on_line, proj_on_line));
          fprintf(stderr, "Distance: %lf mètres \n", distance_to_rx);
          if(distance_to_rx < rx_size /*&& power_ratio > rsrp*/){
            total += paths[i][j][k]->power;
            fprintf(stderr, "Puissance : %lf mW\n", paths[i][j][k]->power);
          }
        }
      }
    }
  }
  return total;
}

void raytrace(scene *s, FILE *write_results, int ray_density, int max_reflections, vector direction, int nb_mesures){
  ray ****paths = propagate(s, ray_density, max_reflections);
  for(int i = 1; i < nb_mesures + 1; i++){
      // Mesure à distance croissante par rapport à l'emmeteur
      vector rx = vector_add(extern_prod(direction,i), s->tx);
      double recieved = recieved_power(paths, ray_density, max_reflections, rx);
      fprintf(write_results, "%d,%lf\n", i, recieved);
  }
  
  for(int i = 0; i<ray_density; i++){
    for(int j = 0; j<ray_density; j++){
      free(paths[i][j]);
    }
    free(paths[i]);
  }
  free(paths);
}
