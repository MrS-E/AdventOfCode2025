#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAX_LIGHTS 64
#define MAX_BUTTONS 64

static int popcount64(uint64_t x) {
#if defined(__GNUC__) || defined(__clang__)
   return __builtin_popcountll(x);
#else
   int c = 0;
   while (x) {
      x &= (x - 1);
      c++;
   }
   return c;
#endif
}

static int solve_machine_line(const char *line, uint64_t *out) {
   if (!line)
      return -1;

   uint64_t target_mask = 0;
   uint64_t button_light_mask[MAX_BUTTONS];
   int num_buttons = 0;
   int num_lights = 0;

   for (int i = 0; i < MAX_BUTTONS; i++) {
      button_light_mask[i] = 0;
   }

   const char *p = strchr(line, '[');
   if (!p)
      return -2;
   p++;
   const char *end = strchr(p, ']');
   if (!end)
      return -3;

   num_lights = (int)(end - p);
   if (num_lights <= 0 || num_lights > MAX_LIGHTS)
      return -4;

   for (int i = 0; i < num_lights; i++) {
      char c = p[i];
      if (c == '#') {
         target_mask |= (1ULL << i);
      } else if (c == '.') {
         // off
      } else {
         // invalid character
         return -5;
      }
   }

   const char *q = end + 1;
   const char *brace = strchr(q, '{');
   const char *limit = brace ? brace : (line + strlen(line));

   while (q < limit) {
      const char *open = strchr(q, '(');
      if (!open || open >= limit)
         break;
      const char *close = strchr(open, ')');
      if (!close || close > limit)
         return -6;

      if (num_buttons >= MAX_BUTTONS)
         return -7;

      uint64_t mask = 0;
      const char *r = open + 1;
      while (r < close) {
         while (r < close && !isdigit((unsigned char)*r))
            r++;
         if (r >= close)
            break;

         int val = 0;
         while (r < close && isdigit((unsigned char)*r)) {
            val = val * 10 + (*r - '0');
            r++;
         }
         if (val < 0 || val >= MAX_LIGHTS) {
            return -8;
         }
         mask |= (1ULL << val);
      }

      button_light_mask[num_buttons++] = mask;
      q = close + 1;
   }

   if (num_buttons <= 0) {
      if (target_mask == 0) {
         *out = 0;
         return 0;
      }
      return -9;
   }

   uint64_t rows[MAX_LIGHTS];
   int b_vec[MAX_LIGHTS];

   for (int i = 0; i < num_lights; i++) {
      rows[i] = 0;
      b_vec[i] = ((target_mask >> i) & 1ULL) ? 1 : 0;
   }

   for (int j = 0; j < num_buttons; j++) {
      uint64_t bm = button_light_mask[j];
      for (int i = 0; i < num_lights; i++) {
         if (bm & (1ULL << i)) {
            rows[i] |= (1ULL << j);
         }
      }
   }

   int pivot_col_for_row[MAX_LIGHTS];
   int pivot_row_for_col[MAX_BUTTONS];
   for (int i = 0; i < num_lights; i++)
      pivot_col_for_row[i] = -1;
   for (int j = 0; j < num_buttons; j++)
      pivot_row_for_col[j] = -1;

   int row = 0;
   for (int col = 0; col < num_buttons && row < num_lights; col++) {
      int sel = -1;
      for (int i = row; i < num_lights; i++) {
         if ((rows[i] >> col) & 1ULL) {
            sel = i;
            break;
         }
      }
      if (sel == -1) {
         continue;
      }

      if (sel != row) {
         uint64_t tmp_row = rows[row];
         rows[row] = rows[sel];
         rows[sel] = tmp_row;

         int tmp_b = b_vec[row];
         b_vec[row] = b_vec[sel];
         b_vec[sel] = tmp_b;
      }

      pivot_col_for_row[row] = col;
      pivot_row_for_col[col] = row;

      for (int i = 0; i < num_lights; i++) {
         if (i != row && ((rows[i] >> col) & 1ULL)) {
            rows[i] ^= rows[row];
            b_vec[i] ^= b_vec[row];
         }
      }

      row++;
   }

   int rank = row;

   for (int i = rank; i < num_lights; i++) {
      if (rows[i] == 0 && b_vec[i] == 1) {
         return -10;
      }
   }

   int free_cols[MAX_BUTTONS];
   int num_free = 0;
   for (int col = 0; col < num_buttons; col++) {
      if (pivot_row_for_col[col] == -1) {
         free_cols[num_free++] = col;
      }
   }

   uint64_t x0 = 0;

   for (int col = num_buttons - 1; col >= 0; col--) {
      int prow = pivot_row_for_col[col];
      if (prow == -1) {
         continue;
      }
      int val = b_vec[prow];
      uint64_t rowmask = rows[prow];

      for (int j = col + 1; j < num_buttons; j++) {
         if ((rowmask >> j) & 1ULL) {
            if ((x0 >> j) & 1ULL) {
               val ^= 1;
            }
         }
      }
      if (val & 1) {
         x0 |= (1ULL << col);
      }
   }

   uint64_t basis[MAX_BUTTONS];
   for (int i = 0; i < num_free; i++) {
      int fcol = free_cols[i];
      uint64_t vec = 0;
      vec |= (1ULL << fcol);

      for (int col = num_buttons - 1; col >= 0; col--) {
         int prow = pivot_row_for_col[col];
         if (prow == -1) {
            continue;
         }
         int val = 0;
         uint64_t rowmask = rows[prow];

         for (int j = col + 1; j < num_buttons; j++) {
            if ((rowmask >> j) & 1ULL) {
               if ((vec >> j) & 1ULL) {
                  val ^= 1;
               }
            }
         }
         if (val & 1) {
            vec |= (1ULL << col);
         }
      }
      basis[i] = vec;
   }

   if (num_free > 20) {
      *out = (uint64_t)popcount64(x0);
      return 0;
   }

   uint64_t best = (uint64_t)popcount64(x0);
   uint64_t total_combos = (num_free > 0) ? (1ULL << num_free) : 1ULL;

   for (uint64_t mask = 1; mask < total_combos; mask++) {
      uint64_t x = x0;
      for (int i = 0; i < num_free; i++) {
         if ((mask >> i) & 1ULL) {
            x ^= basis[i];
         }
      }
      int w = popcount64(x);
      if ((uint64_t)w < best) {
         best = (uint64_t)w;
      }
   }
   *out = best;
   return 0;
}

int aocday10(char **lines, size_t n_lines, uint64_t *out) {
   if (!lines || !out)
      return -1;

   for (size_t i = 0; i < n_lines; i++) {
      const char *line = lines[i];
      if (!line) {
         return -1;
      }
      uint64_t r;
      int rc = solve_machine_line(line, &r);
      if (rc != 0) {
         return -1;
      }
      *out += r;
   }
   return 0;
}
