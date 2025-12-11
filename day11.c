//
// Created by sstix on 12/11/25.
//

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define NAME_MAX_LEN 64
#define PATHS_OK 0
#define PATHS_ERR_INVALID_INPUT 1
#define PATHS_ERR_NO_SVR 2
#define PATHS_ERR_NO_OUT 3
#define PATHS_ERR_NO_DAC 4
#define PATHS_ERR_NO_FFT 5
#define PATHS_ERR_CYCLE 4
#define PATHS_ERR_OOM 5
#define PATHS_ERR_OVERFLOW 6

typedef struct {
   int *to;
   size_t cnt;
   size_t cap;
} Adj;

typedef struct {
   char name[NAME_MAX_LEN];
} Node;

static void strip_crlf(char *s) {
   for (; *s; s++)
      if (*s == '\r' || *s == '\n') {
         *s = '\0';
         return;
      }
}

static char *trim_ws(char *s) {
   while (*s && isspace((unsigned char)*s))
      s++;
   char *e = s + strlen(s);
   while (e > s && isspace((unsigned char)e[-1]))
      e--;
   *e = '\0';
   return s;
}

static void normalize_name_inplace(char *s) {
   unsigned char *u = (unsigned char *)s;
   if (u[0] == 0xEF && u[1] == 0xBB && u[2] == 0xBF) {
      memmove(s, s + 3, strlen(s + 3) + 1);
   }
   char *w = s;
   for (char *r = s; *r; r++) {
      unsigned char c = (unsigned char)*r;
      if (isalnum(c) || c == '_')
         *w++ = (char)c;
   }
   *w = '\0';
}

static int add_edge(Adj *adj, int from, int to) {
   if (adj[from].cnt == adj[from].cap) {
      size_t nc = adj[from].cap ? adj[from].cap * 2 : 4;
      int *tmp = realloc(adj[from].to, nc * sizeof(int));
      if (!tmp)
         return 0;
      adj[from].to = tmp;
      adj[from].cap = nc;
   }
   adj[from].to[adj[from].cnt++] = to;
   return 1;
}

static int find_or_add(Node **nodes, Adj **adj, size_t *count, size_t *cap,
                       const char *name) {
   for (size_t i = 0; i < *count; i++)
      if (strcmp((*nodes)[i].name, name) == 0)
         return (int)i;

   if (*count == *cap) {
      size_t nc = *cap ? *cap * 2 : 64;
      Node *n_tmp = realloc(*nodes, nc * sizeof(Node));
      Adj *a_tmp = realloc(*adj, nc * sizeof(Adj));
      if (!n_tmp || !a_tmp)
         return -1;
      *nodes = n_tmp;
      *adj = a_tmp;
      for (size_t i = *cap; i < nc; i++) {
         (*adj)[i].to = NULL;
         (*adj)[i].cnt = (*adj)[i].cap = 0;
      }
      *cap = nc;
   }

   strncpy((*nodes)[*count].name, name, NAME_MAX_LEN - 1);
   (*nodes)[*count].name[NAME_MAX_LEN - 1] = '\0';
   return (int)(*count)++;
}

static int dfs_count_state(int u, int out_id, int dac_id, int fft_id, Adj *adj,
                           uint64_t *memo, uint8_t *visiting, uint8_t mask,
                           uint64_t *result) {
   if (u == dac_id)
      mask |= 1;
   if (u == fft_id)
      mask |= 2;

   if (u == out_id) {
      *result = (mask == 3) ? 1u : 0u;
      return PATHS_OK;
   }

   size_t key = (size_t)u * 4u + (size_t)mask;
   if (memo[key] != UINT64_MAX) {
      *result = memo[key];
      return PATHS_OK;
   }

   if (visiting[key])
      return PATHS_ERR_CYCLE;
   visiting[key] = 1;

   uint64_t sum = 0;
   for (size_t i = 0; i < adj[u].cnt; i++) {
      uint64_t sub = 0;
      int err = dfs_count_state(adj[u].to[i], out_id, dac_id, fft_id, adj, memo,
                                visiting, mask, &sub);
      if (err)
         return err;
      if (UINT64_MAX - sum < sub)
         return PATHS_ERR_OVERFLOW;
      sum += sub;
   }

   visiting[key] = 0;
   memo[key] = sum;
   *result = sum;
   return PATHS_OK;
}

int aocday11(char **lines, size_t n_lines, uint64_t *out) {
   if (!lines || !out)
      return PATHS_ERR_INVALID_INPUT;
   *out = 0;

   Node *nodes = NULL;
   Adj *adj = NULL;
   size_t node_count = 0, node_cap = 0;

   for (size_t i = 0; i < n_lines; i++) {
      if (!lines[i])
         continue;

      char *dup = strdup(lines[i]);
      if (!dup)
         goto oom;
      strip_crlf(dup);

      char *whole = trim_ws(dup);
      if (*whole == '\0') {
         free(dup);
         continue;
      }

      char *colon = strchr(whole, ':');
      if (!colon) {
         free(dup);
         goto invalid;
      }
      *colon = '\0';

      char *lhs = trim_ws(whole);
      normalize_name_inplace(lhs);
      if (*lhs == '\0') {
         free(dup);
         goto invalid;
      }

      int from = find_or_add(&nodes, &adj, &node_count, &node_cap, lhs);
      if (from < 0) {
         free(dup);
         goto oom;
      }

      char *rhs = trim_ws(colon + 1);
      char *save = NULL;
      for (char *tok = strtok_r(rhs, " \t,", &save); tok;
           tok = strtok_r(NULL, " \t,", &save)) {
         tok = trim_ws(tok);
         normalize_name_inplace(tok);
         if (*tok == '\0')
            continue;

         int to = find_or_add(&nodes, &adj, &node_count, &node_cap, tok);
         if (to < 0) {
            free(dup);
            goto oom;
         }
         if (!add_edge(adj, from, to)) {
            free(dup);
            goto oom;
         }
      }

      free(dup);
   }

   int svr_id = -1, out_id = -1, dac_id = -1, fft_id = -1;
   for (size_t i = 0; i < node_count; i++) {
      if (strcmp(nodes[i].name, "svr") == 0)
         svr_id = (int)i;
      if (strcmp(nodes[i].name, "out") == 0)
         out_id = (int)i;
      if (strcmp(nodes[i].name, "dac") == 0)
         dac_id = (int)i;
      if (strcmp(nodes[i].name, "fft") == 0)
         fft_id = (int)i;
   }

   if (svr_id < 0) {
      goto no_svr;
   }
   if (out_id < 0) {
      goto no_out;
   }
   if (dac_id < 0) {
      goto no_dac;
   }
   if (fft_id < 0) {
      goto no_fft;
   }

   // memo size: node_count * 4
   uint64_t *memo = malloc(node_count * 4u * sizeof(uint64_t));
   uint8_t *visiting = calloc(node_count * 4u, 1);
   if (!memo || !visiting) {
      free(memo);
      free(visiting);
      goto oom;
   }

   for (size_t i = 0; i < node_count * 4u; i++)
      memo[i] = UINT64_MAX;

   uint64_t ans = 0;
   int err = dfs_count_state(svr_id, out_id, dac_id, fft_id, adj, memo,
                             visiting, 0, &ans);
   if (err == PATHS_OK)
      *out = ans;

   free(memo);
   free(visiting);
   for (size_t i = 0; i < node_count; i++)
      free(adj[i].to);
   free(nodes);
   free(adj);
   return err;

invalid:
   for (size_t i = 0; i < node_count; i++)
      free(adj[i].to);
   free(nodes);
   free(adj);
   return PATHS_ERR_INVALID_INPUT;

oom:
   for (size_t i = 0; i < node_count; i++)
      free(adj[i].to);
   free(nodes);
   free(adj);
   return PATHS_ERR_OOM;

no_svr:
   for (size_t i = 0; i < node_count; i++)
      free(adj[i].to);
   free(nodes);
   free(adj);
   return PATHS_ERR_NO_SVR;

no_out:
   for (size_t i = 0; i < node_count; i++)
      free(adj[i].to);
   free(nodes);
   free(adj);
   return PATHS_ERR_NO_OUT;

no_dac:
   for (size_t i = 0; i < node_count; i++)
      free(adj[i].to);
   free(nodes);
   free(adj);
   return PATHS_ERR_NO_DAC;

no_fft:
   for (size_t i = 0; i < node_count; i++)
      free(adj[i].to);
   free(nodes);
   free(adj);
   return PATHS_ERR_NO_FFT;
}
