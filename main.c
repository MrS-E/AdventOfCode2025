#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "day10.c"
#include "day2.c"
#include "day3.c"
#include "day4.c"
#include "day6.c"
#include "day7.c"
#include "day8.c"

char **read_lines(const char *filename, size_t *out_len) {
   FILE *f = fopen(filename, "r");
   if (!f) {
      perror("fopen");
      return NULL;
   }
   size_t cap = 16;
   size_t count = 0;
   char **lines = malloc(cap * sizeof *lines);
   if (!lines) {
      fclose(f);
      return NULL;
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
            return NULL;
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
         return NULL;
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

   goto laocday10;

laocday1:

laocday2:
   unsigned long long day2;
   size_t day2len = 0;
   char **day2lines = read_lines("day2input", &day2len);
   if (day2lines != NULL && day2len == 1)
      rc = aocday2(day2lines[0], &day2);
   free_lines(day2lines, day2len);
   printf("Day 2: %llu\n", day2);

laocday3:
   uint64_t day3;
   size_t day3len = 0;
   char **day3lines = read_lines("day3input", &day3len);
   if (day3lines != NULL)
      rc = aocday3(day3lines, day3len, &day3);
   free_lines(day3lines, day3len);
   printf("Day 3: %lu\n", day3);

laocday4:
   uint16_t day4;
   size_t day4len = 0;
   char **day4lines = read_lines("day4input", &day4len);
   aocday4(day4lines, day4len, strlen(day4lines[0]), &day4);
   free_lines(day4lines, day4len);
   printf("Day 4: %u\n", day4);

laocday5:

laocday6:
   long long day6;
   size_t day6len = 0;
   char **day6lines = read_lines("day6input", &day6len);
   aocday6(day6lines, (int)day6len, &day6);
   free_lines(day6lines, day6len);
   printf("Day 6: %lld\n", day6);

laocday7:
   long long day7;
   size_t day7len = 0;
   char **day7lines = read_lines("day7input", &day7len);
   rc = aocday7(day7lines, day7len, &day7);
   free_lines(day7lines, day7len);
   printf("Day 7: %lld\n", day7);

laocday8:
   uint64_t answer;
   size_t day8len = 0;
   char **day8lines = read_lines("day8input", &day8len);
   aocday8(day8lines, day8len, &answer);
   printf("Day 8: %llu\n", (unsigned long long)answer);

laocday9:

laocday10:
   uint64_t day10;
   size_t day10len = 0;
   char **day10lines = read_lines("day10input", &day10len);
   rc = aocday10(day10lines, day10len, &day10);
   printf("Day 10: %llu\n", day10);

   return rc;
}