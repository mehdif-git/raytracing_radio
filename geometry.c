#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <complex.h>
#include "geometry.h"


double length(vector v){
    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

vector vector_diff(vector u, vector v){
    vector res;
    res.x = u.x - v.x;
    res.y = u.y - v.y;
    res.z = u.z - v.z;
    return res;
}

double distance(vector u, vector v){
    return length(vector_diff(u, v));
}

vector extern_prod(vector v, double coeff){
  vector res;
  res.x = coeff * v.x;
  res.y = coeff * v.y;
  res.z = coeff * v.z;
  return res;
}

vector vector_add(vector u, vector v){
  vector minus_v = extern_prod(v, -1);
  return vector_diff(u, minus_v);
}

vector normalize(vector v){
  double l = length(v);
  return extern_prod(v, 1/l);
}

double dot_product(vector u, vector v){
    return u.x * v.x + u.y * v.y + u.z * v.z;
}

vector cross_product(vector u, vector v){
    vector res;
    res.x = u.y * v.z - u.z * v.y;
    res.y = v.x * u.z - v.z * u.x;
    res.z = u.x * v.y - u.y * v.x;
    return res;
}

vector sph_to_cart(double r, double theta, double phi){
    vector res;
    res.x = sin(theta)*sin(phi)*r;
    res.y = cos(theta)*r;
    res.z = sin(theta)*cos(phi)*r;
    return res;
}

double millis_to_dbm(double p_0, double p){
  return 10*log10(p/p_0);
}

vector* intersect(ray* r, triangle* t){
    /* L'équation du plan est sous la forme (n|v) = cste, cette constante est égale à (a|n)
    En notant la droite associée au rayon A + tu, on souhaite (A + ku | n) = (a|n)
    ie (A|n) + k(u|n) = (a|n)
    ou encore
    k = ( (a|n) - (A|n) ) / (u|n)
    Le cas (u|n) = 0 (rayon tangent au plan) renverra NULL
    */

    double s = dot_product(r->direction, t->n);

    if (0 == s){
        return NULL;
    }

    double k = (dot_product(t->a, t->n) - dot_product(r->origin, t->n)) / s;

    /* Si k est négatif, le point d'intersection se situe du mauvais côté de la demie-droite
    Si k est nul, l'origine du rayon est confondu avec le point d'intersection, on considère qu'il n'y a pas réflexion puisque le rayon part du triangle 
    */

    if (k<=0){
        return NULL;
    }

    /* On obtient enfin le point d'intersection avec le plan contenant le triangle */
    vector* p = malloc(sizeof(vector));
    p->x = r->origin.x + k * r->direction.x;
    p->y = r->origin.y + k * r->direction.y;
    p->z = r->origin.z + k * r->direction.z;
    
    /* Il reste à vérifier que le point si situe à l'intérieur du triangle
    Pour cela, on vérifie que AP se situe entre AB et AC (produits vectoriels de signe opposé) puis que BP se situe entre BA et BC
    */
    vector AP = vector_diff(*p, t->a);
    vector AB = vector_diff(t->b, t->a);
    vector AC = vector_diff(t->c, t->a);

    if (dot_product(cross_product(AP, AB), cross_product(AP, AC)) > 0){
        free(p);
        return NULL;
    }

    vector BP = vector_diff(*p, t->b);
    vector BC = vector_diff(t->c, t->b);

    /* Sachant BA = -AB, la condition est renversée */
    if (dot_product(cross_product(BP, AB), cross_product(BP, BC)) < 0){
        free(p);
        return NULL;
    }

    return p;
}

