//
// Created by sstix on 12/8/25.
//

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
   int32_t x, y, z;
} Point;

typedef struct {
   uint64_t dist;
   int a;
   int b;
} Edge;

typedef struct {
   int *parent;
   int *sz;
   int n;
} DSU;

static void dsu_init(DSU *d, int n) {
   d->n = n;
   d->parent = (int *)malloc((size_t)n * sizeof(int));
   d->sz = (int *)malloc((size_t)n * sizeof(int));
   for (int i = 0; i < n; ++i) {
      d->parent[i] = i;
      d->sz[i] = 1;
   }
}

static void dsu_free(DSU *d) {
   free(d->parent);
   free(d->sz);
}

static int dsu_find(DSU *d, int x) {
   int root = x;
   while (root != d->parent[root]) {
      root = d->parent[root];
   }
   while (x != root) {
      int p = d->parent[x];
      d->parent[x] = root;
      x = p;
   }
   return root;
}

static void dsu_union(DSU *d, int a, int b) {
   int ra = dsu_find(d, a);
   int rb = dsu_find(d, b);
   if (ra == rb)
      return;
   if (d->sz[ra] < d->sz[rb]) {
      int tmp = ra;
      ra = rb;
      rb = tmp;
   }
   d->parent[rb] = ra;
   d->sz[ra] += d->sz[rb];
}

static int is_line_empty(const char *s) {
   if (!s)
      return 1;
   while (*s) {
      if (!isspace((unsigned char)*s))
         return 0;
      s++;
   }
   return 1;
}

static int parse_point(const char *line, Point *p) {
   int32_t x, y, z;
   if (sscanf(line, "%d,%d,%d", &x, &y, &z) == 3) {
      p->x = x;
      p->y = y;
      p->z = z;
      return 1;
   }
   return 0;
}

static int cmp_edge(const void *a, const void *b) {
   const Edge *ea = (const Edge *)a;
   const Edge *eb = (const Edge *)b;
   if (ea->dist < eb->dist)
      return -1;
   if (ea->dist > eb->dist)
      return 1;
   return 0;
}

void aocday8(char **lines, size_t n_lines, uint64_t *out_answer) {
   if (!out_answer)
      return;
   *out_answer = 0;

   Point *points = malloc(n_lines * sizeof(Point));
   size_t n = 0;
   for (size_t i = 0; i < n_lines; ++i) {
      if (!lines[i] || is_line_empty(lines[i]))
         continue;
      if (parse_point(lines[i], &points[n])) {
         n++;
      }
   }
   if (n == 0) {
      free(points);
      return;
   }

   if (n == 1) {
      free(points);
      *out_answer = 0;
      return;
   }

   size_t edges_count = (n * (n - 1)) / 2;
   Edge *edges = malloc(edges_count * sizeof(Edge));

   size_t idx = 0;
   for (size_t i = 0; i < n; ++i) {
      for (size_t j = i + 1; j < n; ++j) {
         int64_t dx = (int64_t)points[i].x - (int64_t)points[j].x;
         int64_t dy = (int64_t)points[i].y - (int64_t)points[j].y;
         int64_t dz = (int64_t)points[i].z - (int64_t)points[j].z;
         uint64_t dist = (uint64_t)(dx * dx + dy * dy + dz * dz);

         edges[idx].dist = dist;
         edges[idx].a = (int)i;
         edges[idx].b = (int)j;
         idx++;
      }
   }

   qsort(edges, edges_count, sizeof(Edge), cmp_edge);

   DSU dsu;
   dsu_init(&dsu, (int)n);

   int components = (int)n;
   uint64_t result = 0;

   for (size_t i = 0; i < edges_count && components > 1; ++i) {
      Edge *e = &edges[i];

      int ra = dsu_find(&dsu, e->a);
      int rb = dsu_find(&dsu, e->b);
      if (ra == rb)
         continue;

      dsu_union(&dsu, ra, rb);
      components--;

      if (components == 1) {
         int64_t xa = points[e->a].x;
         int64_t xb = points[e->b].x;
         result = (uint64_t)(xa * xb);
         break;
      }
   }

   *out_answer = result;

   dsu_free(&dsu);
   free(edges);
   free(points);
}
