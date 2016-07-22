#include "people.h"

people::people(){

}
double people::trait1_update(int pi){
    return this->trait1[pi] += 0.1;
}
