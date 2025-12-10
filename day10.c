#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAX_COUNTERS 64
#define MAX_BUTTONS 64

static uint64_t g_target[MAX_COUNTERS];
static uint64_t g_svals[MAX_COUNTERS];
static uint64_t g_button_mask[MAX_BUTTONS];
static int g_button_cov[MAX_BUTTONS];
static int g_num_counters;
static int g_num_buttons;
static uint64_t g_best;
static int g_global_max_cov;

static uint64_t button_dynamic_ub(int k) {
   uint64_t mask = g_button_mask[k];
   if (!mask)
      return 0;
   uint64_t ub = UINT64_MAX;
   for (int i = 0; i < g_num_counters; i++) {
      if (mask & (1ULL << i)) {
         uint64_t rem =
             (g_target[i] > g_svals[i]) ? (g_target[i] - g_svals[i]) : 0;
         if (rem < ub)
            ub = rem;
      }
   }
   return ub;
}

static uint64_t lower_bound_remaining(void) {
   uint64_t sum_rem = 0;
   uint64_t max_rem = 0;
   for (int i = 0; i < g_num_counters; i++) {
      if (g_svals[i] > g_target[i]) {
         return 1;
      }
      uint64_t rem = g_target[i] - g_svals[i];
      sum_rem += rem;
      if (rem > max_rem)
         max_rem = rem;
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

static void dfs_joltage(int k, uint64_t presses) {
   uint64_t lb = lower_bound_remaining();
   if (lb == 0) {
      if (presses < g_best)
         g_best = presses;
      return;
   }
   if (presses + lb >= g_best)
      return;

   if (k == g_num_buttons) {
      return;
   }

   uint64_t mask = g_button_mask[k];
   uint64_t ub = button_dynamic_ub(k);
   uint64_t applied = 0;

   for (uint64_t x = 0; x <= ub; x++) {
      if (presses + x >= g_best)
         break;
      if (x > 0) {
         for (int i = 0; i < g_num_counters; i++) {
            if (mask & (1ULL << i)) {
               g_svals[i]++;
            }
         }
         applied++;
      }
      int ok = 1;
      for (int i = 0; i < g_num_counters; i++) {
         if (g_svals[i] > g_target[i]) {
            ok = 0;
            break;
         }
      }
      if (!ok)
         break;
      dfs_joltage(k + 1, presses + x);
   }

   if (applied > 0) {
      for (int i = 0; i < g_num_counters; i++) {
         if (mask & (1ULL << i)) {
            g_svals[i] -= applied;
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
   for (int i = 0; i < g_num_buttons; i++) {
      int best = i;
      for (int j = i + 1; j < g_num_buttons; j++) {
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

static int solve_machine(const char *line, uint64_t *out) {
   if (!line)
      return -1;
   g_num_counters = 0;
   for (int i = 0; i < MAX_COUNTERS; i++)
      g_target[i] = 0;

   const char *p = strchr(line, '{');
   if (!p)
      return -2;
   p++;
   const char *end = strchr(p, '}');
   if (!end)
      return -3;

   const char *q = p;
   while (q < end) {
      while (q < end && !isdigit((unsigned char)*q) && *q != '-')
         q++;
      if (q >= end)
         break;
      int sign = 1;
      if (*q == '-') {
         sign = -1;
         q++;
      }

      long val = 0;
      while (q < end && isdigit((unsigned char)*q)) {
         val = val * 10 + (*q - '0');
         q++;
      }
      if (g_num_counters >= MAX_COUNTERS)
         return -4;
      if (val < 0)
         return -5;
      g_target[g_num_counters++] = (uint64_t)(sign * val);

      while (q < end && (isspace((unsigned char)*q) || *q == ','))
         q++;
   }

   if (g_num_counters == 0) {
      *out = 0;
      return 0;
   }

   g_num_buttons = 0;
   for (int j = 0; j < MAX_BUTTONS; j++) {
      g_button_mask[j] = 0;
      g_button_cov[j] = 0;
   }

   const char *limit = strchr(line, '{');
   if (!limit)
      limit = line + strlen(line);

   const char *s = line;
   while (s < limit) {
      const char *open = strchr(s, '(');
      if (!open || open >= limit)
         break;
      const char *close = strchr(open, ')');
      if (!close || close > limit)
         return -6;

      if (g_num_buttons >= MAX_BUTTONS)
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
         if (val < 0 || val >= MAX_COUNTERS)
            return -8;
         mask |= (1ULL << val);
      }

      g_button_mask[g_num_buttons] = mask;
      g_button_cov[g_num_buttons] = popcount64(mask);
      g_num_buttons++;

      s = close + 1;
   }

   if (g_num_buttons == 0) {
      for (int i = 0; i < g_num_counters; i++)
         if (g_target[i] != 0)
            return -9;
      return 0;
   }

   for (int i = 0; i < g_num_counters; i++) {
      int touched = 0;
      for (int j = 0; j < g_num_buttons; j++) {
         if (g_button_mask[j] & (1ULL << i)) {
            touched = 1;
            break;
         }
      }
      if (!touched && g_target[i] > 0)
         return -10;
   }

   sort_buttons_by_coverage();

   g_global_max_cov = 0;
   for (int j = 0; j < g_num_buttons; j++) {
      if (g_button_cov[j] > g_global_max_cov)
         g_global_max_cov = g_button_cov[j];
   }

   for (int i = 0; i < g_num_counters; i++)
      g_svals[i] = 0;
   g_best = UINT64_MAX;

   dfs_joltage(0, 0);
   *out = g_best;
   return 0;
}

int aocday10(char **lines, size_t n_lines, uint64_t *out) {
   if (!lines || !out)
      return -1;

   for (size_t i = 0; i < n_lines; i++) {
      uint64_t r;
      int rc = solve_machine(lines[i], &r);
      if (rc != 0) {
         return -1;
      }
      *out += r;
      printf("%s %llu\n", lines[i], *out);
   }
   return 0;
}
