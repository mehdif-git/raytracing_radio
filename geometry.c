#include <stdlib.h>
#include <math.h>

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

vector normalize(vector v){
    double l = length(v);
    vector res;
    res.x = v.x/l;
    res.y = v.y/l;
    res.z = v.z/l;
    return res;
}


double scalar_product(vector u, vector v){
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
    /* On calcul un vecteur normal au plan */
    vector n = cross_product(vector_diff(t->a, t->b), vector_diff(t->a, t->c));

    /* L'équation du plan est sous la forme (n|v) = cste, cette constante est égale à (a|n)
    En notant la droite associée au rayon A + tu, on souhaite (A + ku | n) = (a|n)
    ie (A|n) + k(u|n) = (a|n)
    ou encore
    k = ( (a|n) - (A|n) ) / (u|n)
    Le cas (u|n) = 0 renverra NULL sans autre forme de procès
    */

    double s = scalar_product(r->direction, n);

    if (0 == s){
        return NULL;
    }

    double k = (scalar_product(t->a, n) - scalar_product(r->origin, n)) / s;

    /* Si t est négatif, le point d'intersection se situe du mauvais côté de la demie-droite
    Si k est nul, l'origine du rayon est confondu avec le point d'intersection, on considère qu'il n'y a pas réflexion puisque le rayon part du triangle */
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

    if (scalar_product(cross_product(AP, AB), cross_product(AP, AC)) > 0){
        free(p);
        return NULL;
    }

    vector BP = vector_diff(*p, t->b);
    vector BC = vector_diff(t->c, t->b);

    /* Sachant BA = -AB, la condition est renversée */
    if (scalar_product(cross_product(BP, AB), cross_product(BP, BC)) < 0){
        free(p);
        return NULL;
    }

    return p;
}

ray* reflect(ray* r, triangle* t){
    vector* p = intersect(r, t);
    if (NULL == p){
        return NULL;
    }
    // on calcule le vecteur normal au triangle
    vector n = normalize(cross_product(vector_diff(t->a, t->b), vector_diff(t->a, t->c)));
    
    // on calcule la composante normale du vecteur incident
    double normal_component = scalar_product(n, r->direction);

    // on inverse la composante normale pour créer le rayon réfléchi
    vector new_dir;
    new_dir.x = r->direction.x - 2*n.x*normal_component;
    new_dir.y = r->direction.y - 2*n.y*normal_component;
    new_dir.z = r->direction.z - 2*n.z*normal_component;

    ray* res = malloc(sizeof(ray));

    res->origin = *p;
    res->direction = new_dir;

    free(p);

    return res;
}

