//
// Created by sstix on 12/4/25.
//

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

void aocday4(char **f, size_t rows, size_t cols, uint16_t *sum) {
    const int dr[8] = { -1, -1, -1,  0, 0, 1, 1, 1 };
    const int dc[8] = { -1,  0,  1, -1, 1,-1, 0, 1 };
    bool *rm = malloc(rows * cols * sizeof(bool));

    for (;;) {
        for (size_t i = 0; i < rows * cols; i++) {
            rm[i] = false;
        }

        uint32_t rrow = 0;

        for (size_t r = 0; r < rows; r++) {
            for (size_t c = 0; c < cols; c++) {
                if (f[r][c] != '@') continue;
                int n = 0;
                for (int k = 0; k < 8; k++) {
                    int rr = (int)r + dr[k];
                    int cc = (int)c + dc[k];
                    if (rr < 0 || rr >= (int)rows || cc < 0 || cc >= (int)cols) continue;
                    if (f[rr][cc] == '@') n++;
                }
                if (n < 4) rm[r * cols + c] = true;
            }
        }

        for (size_t r = 0; r < rows; r++) {
            for (size_t c = 0; c < cols; c++) {
                if (rm[r * cols + c]) {
                    f[r][c] = '.';
                    rrow++;
                }
            }
        }

        if (rrow == 0) break;
        *sum += rrow;
    }
    free(rm);
}
