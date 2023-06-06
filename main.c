#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>

#include "geometry.h"
#include "raytracing.h"

// On lance test nomFichier.obj ray_density max_reflextions nb_mesures txX txY txZ dirX dirY dirZ
int main(int argc, char *argv[]){
     
    int ray_density, max_reflexions, nb_mesures; 
    sscanf(argv[2], "%d", &ray_density);
    sscanf(argv[3], "%d", &max_reflexions);
    sscanf(argv[4], "%d", &nb_mesures);

    // Les coordonnées du fichier obj sont dans l'ordre y z x
    vector tx;
    double txX, txY, txZ;
    /*sscanf(argv[5], "%lf", &txY);
    sscanf(argv[6], "%lf", &txZ);
    sscanf(argv[7], "%lf", &txX);
    */
    tx.x = 608;
    tx.y = 5;
    tx.z = -1000;

    vector dir;
    double dirX, dirY, dirZ;
    /*sscanf(argv[8], "%lf", &dirY);
    sscanf(argv[9], "%lf", &dirZ);
    sscanf(argv[10],"%lf", &dirX);
    */
    dir.x = 1/8;
    dir.y = 0;
    dir.z = 1;

    // Mise en place de la scène
    FILE* obj_file = fopen(argv[1], "r");
    scene* s = load_scene(obj_file, true, true, tx);
    fclose(obj_file);
    
    // Création du fichier de sortie
    char* path = malloc(128*sizeof(char));
    time_t t;
    time(&t);
    sprintf(path, "renders/mesures_%s.csv", ctime(&t));
    FILE* csv_write = fopen(path, "w");
    fprintf(csv_write, "Distance (mètres), Puissance (mW)\n");
    
    raytrace(s, csv_write, ray_density, max_reflexions, dir, nb_mesures);

    fclose(csv_write);

    free(path);
    free(s->triangles);
    free(s);
    return EXIT_SUCCESS;
}
