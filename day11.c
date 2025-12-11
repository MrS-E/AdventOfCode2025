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
#define PATHS_ERR_NO_YOU 2
#define PATHS_ERR_NO_OUT 3
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

static char *trim_ws(char *s) {
   while (*s && isspace((unsigned char)*s))
      s++;
   char *end = s + strlen(s);
   while (end > s && isspace((unsigned char)end[-1]))
      end--;
   *end = '\0';
   return s;
}

static void strip_crlf(char *s) {
   for (; *s; s++) {
      if (*s == '\r' || *s == '\n') {
         *s = '\0';
         return;
      }
   }
}

static int add_edge(Adj *adj, int from, int to) {
   if (adj[from].cnt == adj[from].cap) {
      size_t new_cap = adj[from].cap ? adj[from].cap * 2 : 4;
      int *tmp = (int *)realloc(adj[from].to, new_cap * sizeof(int));
      if (!tmp)
         return 0;
      adj[from].to = tmp;
      adj[from].cap = new_cap;
   }
   adj[from].to[adj[from].cnt++] = to;
   return 1;
}

static int find_or_add(Node **nodes, Adj **adj, size_t *count, size_t *cap,
                       const char *name) {
   for (size_t i = 0; i < *count; i++) {
      if (strcmp((*nodes)[i].name, name) == 0)
         return (int)i;
   }

   if (*count == *cap) {
      size_t new_cap = *cap ? *cap * 2 : 64;
      Node *n_tmp = realloc(*nodes, new_cap * sizeof(Node));
      Adj *a_tmp = realloc(*adj, new_cap * sizeof(Adj));
      if (!n_tmp || !a_tmp)
         return -1;
      *nodes = n_tmp;
      *adj = a_tmp;
      for (size_t i = *cap; i < new_cap; i++) {
         (*adj)[i].to = NULL;
         (*adj)[i].cnt = (*adj)[i].cap = 0;
      }
      *cap = new_cap;
   }

   strncpy((*nodes)[*count].name, name, NAME_MAX_LEN - 1);
   (*nodes)[*count].name[NAME_MAX_LEN - 1] = '\0';
   return (int)(*count)++;
}

static int dfs_count(int u, int out_id, Adj *adj, uint64_t *memo,
                     uint8_t *visiting, uint64_t *result) {
   if (u == out_id) {
      *result = 1;
      return PATHS_OK;
   }
   if (memo[u] != UINT64_MAX) {
      *result = memo[u];
      return PATHS_OK;
   }
   if (visiting[u])
      return PATHS_ERR_CYCLE;

   visiting[u] = 1;
   uint64_t sum = 0;
   for (size_t i = 0; i < adj[u].cnt; i++) {
      uint64_t sub = 0;
      int err = dfs_count(adj[u].to[i], out_id, adj, memo, visiting, &sub);
      if (err)
         return err;

      if (UINT64_MAX - sum < sub)
         return PATHS_ERR_OVERFLOW;
      sum += sub;
   }

   visiting[u] = 0;
   memo[u] = sum;
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
      if (!lines[i]) {
         // ignore null line pointers
         continue;
      }

      char *dup = strdup(lines[i]);
      if (!dup) {
         // OOM
         for (size_t k = 0; k < node_count; k++)
            free(adj[k].to);
         free(nodes);
         free(adj);
         return PATHS_ERR_OOM;
      }

      strip_crlf(dup);
      char *whole = trim_ws(dup);
      if (*whole == '\0') {
         // skip empty lines
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
      char *rhs = trim_ws(colon + 1);
      if (*lhs == '\0') {
         free(dup);
         goto invalid;
      }

      int from = find_or_add(&nodes, &adj, &node_count, &node_cap, lhs);
      if (from < 0) {
         free(dup);
         goto oom;
      }

      if (*rhs) {
         char *save = NULL;
         for (char *tok = strtok_r(rhs, " \t", &save); tok;
              tok = strtok_r(NULL, " \t", &save)) {

            tok = trim_ws(tok);
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
      }
      free(dup);
      continue;

   invalid:
      free(dup);
      for (size_t k = 0; k < node_count; k++)
         free(adj[k].to);
      free(nodes);
      free(adj);
      return PATHS_ERR_INVALID_INPUT;

   oom:
      free(dup);
      for (size_t k = 0; k < node_count; k++)
         free(adj[k].to);
      free(nodes);
      free(adj);
      return PATHS_ERR_OOM;
   }

   int you_id = -1, out_id = -1;
   for (size_t i = 0; i < node_count; i++) {
      if (strcmp(nodes[i].name, "you") == 0)
         you_id = (int)i;
      if (strcmp(nodes[i].name, "out") == 0)
         out_id = (int)i;
   }
   if (you_id < 0) {
      for (size_t k = 0; k < node_count; k++)
         free(adj[k].to);
      free(nodes);
      free(adj);
      return PATHS_ERR_NO_YOU;
   }
   if (out_id < 0) {
      for (size_t k = 0; k < node_count; k++)
         free(adj[k].to);
      free(nodes);
      free(adj);
      return PATHS_ERR_NO_OUT;
   }

   uint64_t *memo = malloc(node_count * sizeof(uint64_t));
   uint8_t *visiting = calloc(node_count, 1);
   if (!memo || !visiting) {
      free(memo);
      free(visiting);
      for (size_t k = 0; k < node_count; k++)
         free(adj[k].to);
      free(nodes);
      free(adj);
      return PATHS_ERR_OOM;
   }
   for (size_t i = 0; i < node_count; i++)
      memo[i] = UINT64_MAX;

   int err = dfs_count(you_id, out_id, adj, memo, visiting, out);

   free(memo);
   free(visiting);
   for (size_t k = 0; k < node_count; k++)
      free(adj[k].to);
   free(nodes);
   free(adj);
   return err;
}
