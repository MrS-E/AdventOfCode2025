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

static bool in_intervals(const Interval *merged, const size_t m, const ull N) {
    size_t lo = 0, hi = m;
    while (lo < hi) {
        const size_t mid = (lo + hi) / 2;
        if (N < merged[mid].L) {
            hi = mid;
        } else if (N > merged[mid].R) {
            lo = mid + 1;
        } else {
            return true;
        }
    }
    return false;
}

static bool has_smaller_period(ull N, int k) {
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "%llu", N);
    if (len <= 0) return false;

    for (int d = 1; d < k; d++) {
        if (k % d != 0) continue;

        bool ok = true;
        for (int i = d; i < len; i++) {
            if (buf[i] != buf[i % d]) {
                ok = false;
                break;
            }
        }
        if (ok) {
            return true;
        }
    }
    return false;
}

// https://adventofcode.com/2025/day/2 part 2
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
        if (!dash) continue;
        *dash = '\0';
        char *left = t;
        char *right = dash + 1;
        char *end = left + strlen(left) - 1;
        while (end >= left && isspace((unsigned char)*end)) *end-- = '\0';
        while (*right && isspace((unsigned char)*right)) right++;
        end = right + strlen(right) - 1;
        while (end >= right && isspace((unsigned char)*end)) *end-- = '\0';

        if (*left == '\0' || *right == '\0') continue;

        ull L = strtoull(left, NULL, 10);
        ull R = strtoull(right, NULL, 10);
        if (L > R) { ull t2 = L; L = R; R = t2; }

        if (n == cap) {
            cap = cap ? cap * 2 : 16;
            intervals = realloc(intervals, cap * sizeof(Interval));
            if (!intervals) {
                fprintf(stderr, "Out of memory\n");
                free(input);
                return 1;
            }
        }
        intervals[n].L = L;
        intervals[n].R = R;
        n++;
    }

    free(input);
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

    ull maxR = 0;
    for (size_t i = 0; i < m; i++) if (merged[i].R > maxR) maxR = merged[i].R;

    int maxDigits = num_digits(maxR);
    if (maxDigits < 2) {
        *sum_out = 0;
        free(merged);
        return 0;
    }

    ull sum = 0;

    for (int k = 1; k <= maxDigits - 1; k++) {
        ull base = 1;
        for (int i = 0; i < k; i++) base *= 10;
        ull startA = base / 10;
        ull endA   = base - 1;

        int maxRep = maxDigits / k;
        if (maxRep < 2) continue;

        for (int rep = 2; rep <= maxRep; rep++) {
            for (ull A = startA; A <= endA; A++) {
                ull N = 0;
                bool overflow = false;

                for (int r = 0; r < rep; r++) {
                    if (N > (ULLONG_MAX - A) / base) {
                        overflow = true;
                        break;
                    }
                    N = N * base + A;
                }
                if (overflow) break;

                if (N > maxR) {
                    break;
                }

                if (has_smaller_period(N, k)) {
                    continue;
                }

                if (in_intervals(merged, m, N)) {
                    sum += N;
                }
            }
        }
    }

    free(merged);
    *sum_out = sum;
    return 0;
}
