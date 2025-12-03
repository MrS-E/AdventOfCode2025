//
// Created by sstix on 12/3/25.
//

#include <stdint.h>
#include <string.h>


static inline uint8_t getNum(char ch) {
    unsigned d = (unsigned)(ch - '0');
    return d < 10u ? (uint8_t)d : 0xFF;
}
static inline int getLargestCombination(const char *input, size_t len, uint8_t *num) {
    if (len < 2) return -1;
    const char *lptr = input;
    const char *rptr = input + 1;
    const char *p = input + 1;
    const char *end = input + len;
    for (; p < end; ++p) {
        if (*p > *lptr && p + 1 < end) {
            lptr = p;
            rptr = p + 1;
        } else if (p > lptr && *p > *rptr) {
            rptr = p;
        }
    }
    uint8_t L = getNum(*lptr);
    uint8_t R = getNum(*rptr);
    if (L == 0xFF || R == 0xFF) return -2;
    *num = (uint8_t)(L * 10u + R);
    return 0;
}

int aocday3(char **input, size_t len, uint32_t *sum) {
    for (size_t i = 0; i < len; i++) {
        uint8_t num = 0;
        if (getLargestCombination(input[i], strlen(input[i]), &num)) {
            return -1;
        }
        *sum += num;
    }
}
