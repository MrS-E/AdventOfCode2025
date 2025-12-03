//
// Created by sstix on 12/3/25.
//

#include <stdint.h>
#include <string.h>

#define POW10_TABLE \
((const uint64_t[]){ \
1ULL,                     /* 10^0  */ \
10ULL,                    /* 10^1  */ \
100ULL,                   /* 10^2  */ \
1000ULL,                  /* 10^3  */ \
10000ULL,                 /* 10^4  */ \
100000ULL,                /* 10^5  */ \
1000000ULL,               /* 10^6  */ \
10000000ULL,              /* 10^7  */ \
100000000ULL,             /* 10^8  */ \
1000000000ULL,            /* 10^9  */ \
10000000000ULL,           /* 10^10 */ \
100000000000ULL,          /* 10^11 */ \
1000000000000ULL          /* 10^12 */ \
})

#define POW10(n) (POW10_TABLE[(n)])


static inline uint8_t getNum(char ch) {
    unsigned d = (unsigned)(ch - '0');
    return d < 10u ? (uint8_t)d : 0xFF;
}
static inline int getLargestCombination(const char *input, size_t len, size_t numOfPtr, uint64_t *num) {
    if (len < 2) return -1;
    const char *ptr;
    const char *end = input + len;
    const char *start = input;

    for (size_t i = 0; i < numOfPtr; i++) {
        const size_t remaining = numOfPtr - i;
        const char *p = start;
        ptr = p;

        for (; p < (end - remaining + 1); ++p) {
            if (*ptr < *p) {
                ptr = p;
            }
        }

        start = ptr + 1;
        *num += getNum(*ptr) * POW10(remaining-1);
    }

    return 0;
}

int aocday3(char **input, size_t len, uint64_t *sum) {
    for (size_t i = 0; i < len; i++) {
        if (getLargestCombination(input[i], strlen(input[i]), 12, sum)) {
            return -1;
        }
    }
    return 0;
}
