#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "day2.c"

#define DAY2INPUT_DEMO "11-22,95-115,998-1012,1188511880-1188511890,222220-222224,1698522-1698528,446443-446449,38593856-38593862,565653-565659, 824824821-824824827,2121212118-2121212124"
#define DAY2INPUT "492410748-492568208,246-390,49-90,16-33,142410-276301,54304-107961,12792-24543,3434259704-3434457648,848156-886303,152-223,1303-1870,8400386-8519049,89742532-89811632,535853-567216,6608885-6724046,1985013826-1985207678,585591-731454,1-13,12067202-12233567,6533-10235,6259999-6321337,908315-972306,831-1296,406-824,769293-785465,3862-5652,26439-45395,95-136,747698990-747770821,984992-1022864,34-47,360832-469125,277865-333851,2281-3344,2841977-2953689,29330524-29523460"
char **read_lines(const char *filename, size_t *out_len) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("fopen");
        return nullptr;
    }
    size_t cap   = 16;
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
    int rc;

    unsigned long long day2;
    size_t day2len = 0;
    char **day2lines = read_lines("day2input", &day2len);
    rc = aocday2(day2lines[0], &day2);
    printf("Day 2: %llu\n", day2);
    laocday2:
    {
        unsigned long long day2;
        size_t day2len = 0;
        char **day2lines = read_lines("day2input", &day2len);
        if (day2lines != nullptr && day2len == 1) rc = aocday2(day2lines[0], &day2);
        free_lines(day2lines, day2len);
        printf("Day 2: %llu\n", day2);
    }
    return rc;
}