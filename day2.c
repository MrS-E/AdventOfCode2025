//
// Created by sstix on 12/2/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>

typedef unsigned long long ull;

typedef struct {
    ull L;
    ull R;
} Interval;

static int cmp_interval(const void *a, const void *b) {
    const Interval *ia = a;
    const Interval *ib = b;
    if (ia->L < ib->L) return -1;
    if (ia->L > ib->L) return 1;
    if (ia->R < ib->R) return -1;
    if (ia->R > ib->R) return 1;
    return 0;
}

static int num_digits(ull x) {
    int d = 0;
    if (x == 0) return 1;
    while (x > 0) {
        x /= 10;
        d++;
    }
    return d;
}

//sum invalid ids: https://adventofcode.com/2025/day/2
int aocday2(const char *input_raw, unsigned long long *sum_out) {
    char *input = strdup(input_raw);
    if (!input) return 0;

    Interval *intervals = NULL;
    size_t cap = 0, n = 0;
    char *p = input;
    char *token;

    while ((token = strsep(&p, ",")) != NULL) {
        char *t = token;
        while (*t && isspace((unsigned char)*t)) t++;
        if (*t == '\0') continue;
        char *dash = strchr(t, '-');
        if (!dash) {
            fprintf(stderr, "Invalid token (no dash): '%s'\n", t);
            continue;
        }
        *dash = '\0';
        char *left = t;
        char *right = dash + 1;
        char *end;

        end = left + strlen(left) - 1;
        while (end >= left && isspace((unsigned char)*end)) {
            *end = '\0';
            end--;
        }

        while (*right && isspace((unsigned char)*right)) right++;
        end = right + strlen(right) - 1;
        while (end >= right && isspace((unsigned char)*end)) {
            *end = '\0';
            end--;
        }

        if (*left == '\0' || *right == '\0') {
            fprintf(stderr, "Invalid range part in token: '%s-%s'\n", left, right);
            continue;
        }

        ull L = strtoull(left, NULL, 10);
        ull R = strtoull(right, NULL, 10);
        if (L > R) {
            ull tmp = L;
            L = R;
            R = tmp;
        }

        if (n == cap) {
            cap = cap ? cap * 2 : 16;
            intervals = realloc(intervals, cap * sizeof(Interval));
            if (!intervals) {
                fprintf(stderr, "Out of memory\n");
                return 1;
            }
        }
        intervals[n].L = L;
        intervals[n].R = R;
        n++;
    }

    if (n == 0) {
        *sum_out = 0;
        free(intervals);
        return 0;
    }

    qsort(intervals, n, sizeof(Interval), cmp_interval);

    Interval *merged = malloc(n * sizeof(Interval));
    if (!merged) {
        fprintf(stderr, "Out of memory\n");
        free(intervals);
        return 1;
    }

    size_t m = 0;
    merged[0] = intervals[0];
    m = 1;

    for (size_t i = 1; i < n; i++) {
        if (intervals[i].L > merged[m - 1].R + 1) {
            merged[m++] = intervals[i];
        } else {
            if (intervals[i].R > merged[m - 1].R) {
                merged[m - 1].R = intervals[i].R;
            }
        }
    }

    free(intervals);
    intervals = NULL;

    ull maxR = 0;
    for (size_t i = 0; i < m; i++) {
        if (merged[i].R > maxR) maxR = merged[i].R;
    }

    int maxDigits = num_digits(maxR);
    int maxHalf = maxDigits / 2;

    if (maxHalf == 0) {
        *sum_out = 0;
        free(merged);
        return 0;
    }

    ull sum = 0;
    size_t j = 0;
    bool done = false;

    ull pow10 = 1;
    for (int i = 0; i < maxHalf; i++) {
        pow10 *= 10;
    }

    for (int k = 1; k <= maxHalf && !done; k++) {
        ull base = 1;
        for (int i = 0; i < k; i++) base *= 10;
        ull startA = base / 10;
        ull endA   = base - 1;

        for (ull A = startA; A <= endA; A++) {
            if (A > (ULLONG_MAX - A) / base) {
                done = true;
                break;
            }
            ull N = A * base + A;
            if (N > maxR) {
                break;
            }

            while (j < m && N > merged[j].R) {
                j++;
            }
            if (j >= m) {
                done = true;
                break;
            }

            if (N >= merged[j].L && N <= merged[j].R) {
                sum += N;
            }
        }
    }

    *sum_out = sum;

    free(merged);
    return 0;
}