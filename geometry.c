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
    vector E_te = extern_prod(plane_normal,te_prop); 
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
