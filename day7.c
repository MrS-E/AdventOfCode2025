//
// Created by sstix on 12/7/25.
//

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

int aocday7(char **grid, size_t rows, long long *result){
    if (!grid || !result || rows < 1) {
        return -1;
    }

    int cols = (int)strlen(grid[0]);
    if (cols == 0) {
        return -2;
    }

    int start_row = -1;
    int start_col = -1;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (grid[r][c] == 'S') {
                start_row = r;
                start_col = c;
                break;
            }
        }
        if (start_row != -1) {
            break;
        }
    }

    if (start_row == -1 || start_col == -1) {
        return -3;
    }

    long long *current = (long long *)calloc((size_t)cols, sizeof(long long));
    long long *next    = (long long *)calloc((size_t)cols, sizeof(long long));
    if (!current || !next) {
        free(current);
        free(next);
        *result = 0;
        return -1;
    }
    current[start_col] = 1;

    for (int r = start_row; r < rows - 1; r++) {
        int any_next = 0;
        memset(next, 0, (size_t)cols * sizeof(long long));
        for (int c = 0; c < cols; c++) {
            long long ways = current[c];
            if (ways == 0) {
                continue;
            }
            char below = grid[r + 1][c];
            if (below == '.' || below == 'S') {
                next[c] += ways;
            } else if (below == '^') {
                if (c - 1 >= 0) {
                    next[c - 1] += ways;
                }
                if (c + 1 < cols) {
                    next[c + 1] += ways;
                }
            }
        }

        for (int c = 0; c < cols; c++) {
            current[c] = next[c];
            if (next[c] != 0) {
                any_next = 1;
            }
        }
        if (!any_next) {
            break;
        }
    }

    long long total_timelines = 0;
    for (int c = 0; c < cols; c++) {
        total_timelines += current[c];
    }

    free(current);
    free(next);
    *result = total_timelines;
    return 0;
}
