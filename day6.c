//
// Created by sstix on 12/6/25.
//

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

static long long toNumber(const char *s) {
    long long n = 0;
    for (int i = 0; s[i]; i++) {
        n = n * 10 + (s[i] - '0');
    }
    return n;
}

static char char_at(char **lines, int *rowLens, int r, int c) {
    if (c >= rowLens[r]) {
        return ' ';
    }
    return lines[r][c];
}

static void process_segment_part2(char **lines, int rows, int *rowLens, int startCol, int endCol, long long *total) {
    long long nums[512];
    int numCount = 0;
    char op = 0;
    int opRow = rows - 1;

    for (int c = startCol; c <= endCol; c++) {
        char ch = char_at(lines, rowLens, opRow, c);
        if (ch == '+' || ch == '*') {
            op = ch;
        }
    }

    if (op == 0) {
        return;
    }

    for (int c = endCol; c >= startCol; c--) {
        char buf[64];
        int bi = 0;
        for (int r = 0; r < opRow; r++) {
            char ch = char_at(lines, rowLens, r, c);
            if (isdigit((unsigned char)ch)) {
                buf[bi++] = ch;
            }
        }
        buf[bi] = '\0';
        if (bi > 0) {
            nums[numCount++] = toNumber(buf);
        }
    }

    if (numCount == 0) {
        return;
    }

    long long result = (op == '+') ? 0 : 1;
    for (int i = 0; i < numCount; i++) {
        if (op == '+') {
            result += nums[i];
        } else {
            result *= nums[i];
        }
    }

    *total += result;
}

void aocday6(char **lines, int rows, long long *outTotal) {
    int rowLens[512];
    int cols = 0;

    for (int r = 0; r < rows; r++) {
        rowLens[r] = (int)strlen(lines[r]);
        if (rowLens[r] > cols) {
            cols = rowLens[r];
        }
    }

    long long total = 0;
    int inSegment = 0;
    int segStart = 0;

    for (int c = 0; c < cols; c++) {
        int emptyCol = 1;
        for (int r = 0; r < rows; r++) {
            char ch = char_at(lines, rowLens, r, c);
            if (ch != ' ') {
                emptyCol = 0;
                break;
            }
        }

        if (!inSegment && !emptyCol) {
            inSegment = 1;
            segStart = c;
        } else if (inSegment && emptyCol) {
            process_segment_part2(lines, rows, rowLens, segStart, c - 1, &total);
            inSegment = 0;
        }
    }

    if (inSegment) {
        process_segment_part2(lines, rows, rowLens, segStart, cols - 1, &total);
    }

    *outTotal = total;
}
