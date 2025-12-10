#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAX_COUNTERS 64
#define MAX_BUTTONS 64

static int g_nC;
static int g_nB;
static uint64_t g_target[MAX_COUNTERS];
static uint64_t g_button_mask[MAX_BUTTONS];
static int g_button_cov[MAX_BUTTONS];
static int g_global_max_cov;
static uint64_t g_best;

typedef struct {
   int nC;
   int nB;
   uint64_t target[MAX_COUNTERS];
   uint64_t button_mask[MAX_BUTTONS];
} Machine;

static uint64_t button_dynamic_ub(int k, const uint64_t *rem) {
   uint64_t mask = g_button_mask[k];
   if (!mask)
      return 0;
   uint64_t ub = UINT64_MAX;
   int has = 0;

   for (int i = 0; i < g_nC; i++) {
      if (mask & (1ULL << i)) {
         has = 1;
         if (rem[i] < ub)
            ub = rem[i];
      }
   }

   if (!has)
      return 0;
   if (ub == UINT64_MAX)
      return 0;
   return ub;
}

static uint64_t lower_bound_remaining(const uint64_t *rem) {
   uint64_t sum_rem = 0;
   uint64_t max_rem = 0;
   for (int i = 0; i < g_nC; i++) {
      uint64_t r = rem[i];
      sum_rem += r;
      if (r > max_rem)
         max_rem = r;
   }
   if (sum_rem == 0)
      return 0;

   uint64_t lb = max_rem;
   if (g_global_max_cov > 0) {
      uint64_t lb2 = (sum_rem + (uint64_t)g_global_max_cov - 1) /
                     (uint64_t)g_global_max_cov;
      if (lb2 > lb)
         lb = lb2;
   }
   return lb;
}

static void dfs_buttons(int k, uint64_t presses, uint64_t *rem) {
   uint64_t lb = lower_bound_remaining(rem);
   if (lb == 0) {
      if (presses < g_best)
         g_best = presses;
      return;
   }
   if (presses + lb >= g_best)
      return;
   if (k == g_nB)
      return;
   uint64_t mask = g_button_mask[k];
   uint64_t ub = button_dynamic_ub(k, rem);
   uint64_t applied = 0;
   for (uint64_t x = 0; x <= ub; x++) {
      if (presses + x >= g_best)
         break;
      if (x > 0) {
         for (int i = 0; i < g_nC; i++) {
            if (mask & (1ULL << i)) {
               rem[i]--;
            }
         }
         applied++;
      }
      dfs_buttons(k + 1, presses + x, rem);
   }
   if (applied > 0) {
      for (int i = 0; i < g_nC; i++) {
         if (mask & (1ULL << i)) {
            rem[i] += applied;
         }
      }
   }
}

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

static void sort_buttons_by_coverage(void) {
   for (int i = 0; i < g_nB; i++) {
      int best = i;
      for (int j = i + 1; j < g_nB; j++) {
         if (g_button_cov[j] > g_button_cov[best]) {
            best = j;
         }
      }
      if (best != i) {
         uint64_t tmp_mask = g_button_mask[i];
         int tmp_cov = g_button_cov[i];
         g_button_mask[i] = g_button_mask[best];
         g_button_cov[i] = g_button_cov[best];
         g_button_mask[best] = tmp_mask;
         g_button_cov[best] = tmp_cov;
      }
   }
}

static int parse_machine_line(const char *line, Machine *m) {
   if (!line || !m)
      return -1;

   m->nC = 0;
   m->nB = 0;
   for (int i = 0; i < MAX_COUNTERS; i++)
      m->target[i] = 0;
   for (int j = 0; j < MAX_BUTTONS; j++)
      m->button_mask[j] = 0;

   const char *brace_l = strchr(line, '{');
   if (!brace_l)
      return -1;
   const char *brace_r = strchr(brace_l + 1, '}');
   if (!brace_r)
      return -1;

   const char *p = brace_l + 1;
   while (p < brace_r) {
      while (p < brace_r && !isdigit((unsigned char)*p) && *p != '-')
         p++;
      if (p >= brace_r)
         break;

      int sign = 1;
      if (*p == '-') {
         sign = -1;
         p++;
      }

      long val = 0;
      while (p < brace_r && isdigit((unsigned char)*p)) {
         val = val * 10 + (*p - '0');
         p++;
      }
      if (m->nC >= MAX_COUNTERS)
         return -1;
      if (val < 0)
         return -1;
      m->target[m->nC++] = (uint64_t)(sign * val);

      while (p < brace_r && (isspace((unsigned char)*p) || *p == ','))
         p++;
   }

   const char *limit = brace_l;
   const char *s = line;
   while (s < limit) {
      const char *open = strchr(s, '(');
      if (!open || open >= limit)
         break;
      const char *close = strchr(open + 1, ')');
      if (!close || close > limit)
         return -1;
      if (m->nB >= MAX_BUTTONS)
         return -1;
      uint64_t mask = 0;

      const char *q = open + 1;
      while (q < close) {
         while (q < close && !isdigit((unsigned char)*q) && *q != '-')
            q++;
         if (q >= close)
            break;
         int sign = 1;
         if (*q == '-') {
            sign = -1;
            q++;
         }

         int val = 0;
         while (q < close && isdigit((unsigned char)*q)) {
            val = val * 10 + (*q - '0');
            q++;
         }
         if (sign < 0)
            return -1;
         if (val < 0 || val >= m->nC)
            return -1;

         mask |= (1ULL << val);

         while (q < close && (isspace((unsigned char)*q) || *q == ','))
            q++;
      }

      m->button_mask[m->nB++] = mask;
      s = close + 1;
   }

   return 0;
}

static int solve_machine_dfs(const Machine *m, uint64_t *out) {
   g_nC = m->nC;
   g_nB = m->nB;

   for (int i = 0; i < g_nC; i++) {
      g_target[i] = m->target[i];
   }
   for (int j = 0; j < g_nB; j++) {
      g_button_mask[j] = m->button_mask[j];
      g_button_cov[j] = popcount64(m->button_mask[j]);
   }

   sort_buttons_by_coverage();
   g_global_max_cov = 0;
   for (int j = 0; j < g_nB; j++) {
      if (g_button_cov[j] > g_global_max_cov) {
         g_global_max_cov = g_button_cov[j];
      }
   }

   uint64_t rem[MAX_COUNTERS];
   for (int i = 0; i < g_nC; i++) {
      rem[i] = g_target[i];
   }
   g_best = UINT64_MAX;
   dfs_buttons(0, 0, rem);

   if (g_best == UINT64_MAX)
      return -1;
   *out = g_best;
   return 0;
}

int aocday10(char **lines, size_t n_lines, uint64_t *out) {
   if (!lines || !out)
      return -1;
   uint64_t total = 0;

   for (size_t i = 0; i < n_lines; i++) {
      Machine m;
      if (parse_machine_line(lines[i], &m) != 0)
         return -1;
      if (m.nB == 0) {
         for (int c = 0; c < m.nC; c++) {
            if (m.target[c] != 0)
               return -1;
         }
         continue;
      }
      uint64_t best_for_machine = 0;
      if (solve_machine_dfs(&m, &best_for_machine) != 0)
         return -1;
      total += best_for_machine;
      printf("%s %llu\n", lines[i], best_for_machine);
   }

   *out += total;
   return 0;
}
