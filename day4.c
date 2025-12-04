//
// Created by sstix on 12/4/25.
//

#include <stdint.h>
#include <stddef.h>

void aocday4(char **f, size_t rows, size_t cols, uint16_t *sum) {
    const int dr[8] = { -1, -1, -1,  0, 0, 1, 1, 1 };
    const int dc[8] = { -1,  0,  1, -1, 1,-1, 0, 1 };
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (f[r][c] != '@') continue;
            int n = 0;
            for (int k = 0; k < 8; k++) {
                int rr = r + dr[k];
                int cc = c + dc[k];
                if (rr < 0 || rr >= rows || cc < 0 || cc >= cols) continue;
                if (f[rr][cc] == '@') n++;
            }
            if (n < 4) (*sum)++;
        }
    }
}
