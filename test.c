#include <stdlib.h>
#include <stdio.h>

#include "geometry.h"


int main(){
    vector A = {0, 1, 0};
    vector B = {1, 0, 0};
    vector C = {0, 0, 1};

    triangle T = {A, B, C};

    vector O = {0,2,0};
    vector u = {1,-5,1};

    ray r = {O, u};

    ray* r2 = reflect(&r, &T);

    if (r2 != NULL){
        printf("%f\n%f\n%f", r2->direction.x, r2->direction.y, r2->direction.z);
    }
    return 0;
}