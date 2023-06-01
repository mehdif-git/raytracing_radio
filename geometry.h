#ifndef __GEOMETRY_H
#define __GEOMETRY_H

extern const double wavelength;

typedef struct vector_s{
    double x;
    double y;
    double z;
} vector;

typedef struct triangle_s{
    vector a;
    vector b;
    vector c;
    vector n;
} triangle;

typedef struct ray_s{
    vector origin;
    vector direction;
    double phase;
    double power;
} ray;

/* Renvoie la norme du vecteur v */
double length(vector v);

/* Renvoie v normalisé */
vector normalize(vector v);

/* Renvoie la distance entre u et v */
double distance(vector u, vector v);

/* Renvoie la différence entre le vecteur u et le vecteur v */
vector vector_diff(vector u, vector v);

/* Renvoie le produit scalaire de u et v */
double dot_product(vector u, vector v);

/* Renvoie le produit vectoriel de u et v */
vector cross_product(vector u, vector v);

/* Renvoie, s'il existe, le point d'intersection entre une demie droite (rayon) et un triangle
Renvoie NULL si l'intersection n'existe pas */
vector* intersect(ray* r, triangle* t);

/* Renvoie, s'il existe, le rayon réfléchi à partir du rayon incident et d'un triangle 
Renvoie NULL si l'intersection n'existe pas */
ray* reflect(ray* r, triangle* t, double _Complex ref_index, vector pola);

/* Renvoie en cas de collision un rayon réfléchi dans une direction aléatoire */
ray* diffuse(ray* r, triangle* t);


#endif
