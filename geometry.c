#include <stdlib.h>
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

vector* intersect(ray* r, triangle* t){
    /* L'équation du plan est sous la forme (n|v) = cste, cette constante est égale à (a|n)
    En notant la droite associée au rayon A + tu, on souhaite (A + ku | n) = (a|n)
    ie (A|n) + k(u|n) = (a|n)
    ou encore
    k = ( (a|n) - (A|n) ) / (u|n)
    Le cas (u|n) = 0 (rayon tangent au plan) renverra NULL sans autre forme de procès
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

ray* reflect(ray* r, triangle* t, double complex ref_index, vector pola){
    vector* p = intersect(r, t);
    if (NULL == p){
        return NULL;
    }
    
    // on calcule la composante normale du vecteur incident
    double normal_component = dot_product(t->n, r->direction);

    // on inverse la composante normale pour créer le rayon réfléchi
    vector new_dir;
    new_dir.x = r->direction.x - 2*(t->n).x*normal_component;
    new_dir.y = r->direction.y - 2*(t->n).y*normal_component;
    new_dir.z = r->direction.z - 2*(t->n).z*normal_component;
    
    //On calcule la puissance du rayon réfléchi ainsi que la phase du champ E associé

    vector eik = normalize(r->direction);
    double te_prop = length(cross_product(eik, pola));
    double complex cos_in = dot_product(eik, t->n) + 0*I;
    double complex sin_in = sin(acos(cos_in)) + 0*I;
    double complex R_te = (cos_in - csqrt(ref_index - (sin_in * sin_in)))/(cos_in + csqrt(ref_index - (sin_in * sin_in)));
    double complex R_tm = (ref_index * cos_in - csqrt(ref_index - (sin_in * sin_in))) /(ref_index * cos_in + csqrt(ref_index - (sin_in * sin_in)));

    double pow_te = cabs(R_te) * cabs(R_te);
    double pow_tm = cabs(R_tm) * cabs(R_tm);
    double new_power = r->power * (te_prop * pow_te + (1-te_prop) * pow_tm); 
    ray* res = malloc(sizeof(ray));

    res->origin = *p;
    res->direction = new_dir;
    res->power = new_power;
    res-> phase = (2 * M_PI / wavelength) * length(*p) + (M_PI / 2);
    free(p);

    return res;
}


ray* diffuse(ray* r, triangle* t){
    vector* p = intersect(r, t);

    if (NULL == p){
        return NULL;
    }

    ray* res = malloc(sizeof(ray));

    res->origin = *p;

    double theta, phi;


    /* on génère deux angles pour créer un vecteur unitaire aléatoire
    on utilise une méthode de Las Vegas pour que le vecteur soit du bon côté du plan du triangle */
    do{
        theta = (double) rand() / RAND_MAX * 2 * M_PI;
        phi = (double) rand() / RAND_MAX * M_PI;
        res->direction.x = sin(phi)*cos(theta);
        res->direction.y = sin(phi)*sin(theta);
        res->direction.z = cos(phi);
    } while (dot_product(res->direction, r->direction) > 0);

    free(p);

    return res;
}

