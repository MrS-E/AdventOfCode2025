//
// Created by sstix on 12/7/25.
//

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

int aocday7(char **grid, size_t rows, int *result){
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

    char *current = (char *)calloc((size_t)cols, sizeof(char));
    char *next    = (char *)calloc((size_t)cols, sizeof(char));
    if (!current || !next) {
        free(current);
        free(next);
        *result = 0;
        return -1;
    }

    int splits = 0;

    current[start_col] = 1;

    for (int r = start_row; r < rows - 1; r++) {
        int any_next = 0;
        memset(next, 0, (size_t)cols);
        for (int c = 0; c < cols; c++) {
            if (!current[c]) {
                continue;
            }
            char below = grid[r + 1][c];
            if (below == '.' || below == 'S') {
                next[c] = 1;
            } else if (below == '^') {
                splits++;
                if (c - 1 >= 0) {
                    next[c - 1] = 1;
                }
                if (c + 1 < cols) {
                    next[c + 1] = 1;
                }
            }
        }

        for (int c = 0; c < cols; c++) {
            current[c] = next[c];
            if (next[c]) {
                any_next = 1;
            }
        }
        if (!any_next) {
            break;
        }
    }

    free(current);
    free(next);
    *result = splits;
    return 0;
}
