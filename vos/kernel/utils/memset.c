#include "utils/memset.h"

void memset(void *dest, char value, uint32_t n){
    char *ptr = (char *)dest;
    for (; n != 0; n--) {
        *ptr++ = value;
    }
}