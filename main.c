#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "day2.c"
#include "day3.c"

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))
#define DAY2INPUT_DEMO "11-22,95-115,998-1012,1188511880-1188511890,222220-222224,1698522-1698528,446443-446449,38593856-38593862,565653-565659, 824824821-824824827,2121212118-2121212124"
#define DAY3INPUT_DEMO (const char *[]){"987654321111111","811111111111119","234234234234278","818181911112111"}

char **read_lines(const char *filename, size_t *out_len) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("fopen");
        return nullptr;
    }
    size_t cap = 16;
    size_t count = 0;
    char **lines = malloc(cap * sizeof *lines);
    if (!lines) {
        fclose(f);
        return nullptr;
    }
    char buffer[4096];
    while (fgets(buffer, sizeof buffer, f)) {
        size_t len = strcspn(buffer, "\r\n");
        buffer[len] = '\0';
        if (count == cap) {
            cap *= 2;
            char **tmp = realloc(lines, cap * sizeof *lines);
            if (!tmp) {
                for (size_t i = 0; i < count; ++i) {
                    free(lines[i]);
                }
                free(lines);
                fclose(f);
                return nullptr;
            }
            lines = tmp;
        }
        char *line = malloc(len + 1);
        if (!line) {
            for (size_t i = 0; i < count; ++i) {
                free(lines[i]);
            }
            free(lines);
            fclose(f);
            return nullptr;
        }
        memcpy(line, buffer, len + 1);
        lines[count++] = line;
    }
    fclose(f);
    *out_len = count;
    return lines;
}

void free_lines(char **lines, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        free(lines[i]);
    }
    free(lines);
}

int main(void) {
    int rc = 0;

    goto laocday3;

    laocday2:
    {
        unsigned long long day2;
        size_t day2len = 0;
        char **day2lines = read_lines("day2input", &day2len);
        if (day2lines != nullptr && day2len == 1) rc = aocday2(day2lines[0], &day2);
        free_lines(day2lines, day2len);
        printf("Day 2: %llu\n", day2);
    }

    laocday3:
    {
        uint64_t day3;
        size_t day3len = 0;
        char **day3lines = read_lines("day3input", &day3len);
        if (day3lines != nullptr) rc = aocday3(day3lines, day3len, &day3);
        free_lines(day3lines, day3len);
        printf("Day 3: %lu\n", day3);
    }

    return rc;
}