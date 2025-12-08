//
// Created by sstix on 12/8/25.
//

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define K_NEAREST 1000

typedef struct {
   int32_t x, y, z;
} Point;

typedef struct {
   uint64_t dist;
   int a;
   int b;
} Edge;

static void heap_swap(Edge *a, Edge *b) {
   Edge tmp = *a;
   *a = *b;
   *b = tmp;
}

static void heap_sift_up(Edge *heap, int idx) {
   while (idx > 0) {
      int parent = (idx - 1) / 2;
      if (heap[parent].dist >= heap[idx].dist)
         break;
      heap_swap(&heap[parent], &heap[idx]);
      idx = parent;
   }
}

static void heap_sift_down(Edge *heap, int size, int idx) {
   while (true) {
      int left = 2 * idx + 1;
      int right = left + 1;
      int largest = idx;

      if (left < size && heap[left].dist > heap[largest].dist)
         largest = left;
      if (right < size && heap[right].dist > heap[largest].dist)
         largest = right;
      if (largest == idx)
         break;
      heap_swap(&heap[idx], &heap[largest]);
      idx = largest;
   }
}

static void heap_insert_topk(Edge *heap, int *heap_size, const Edge *e) {
   if (*heap_size < K_NEAREST) {
      heap[*heap_size] = *e;
      heap_sift_up(heap, *heap_size);
      (*heap_size)++;
   } else if (e->dist < heap[0].dist) {
      heap[0] = *e;
      heap_sift_down(heap, *heap_size, 0);
   }
}

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

static int cmp_int_desc(const void *a, const void *b) {
   int ia = *(const int *)a;
   int ib = *(const int *)b;
   return (ib - ia);
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

   Edge heap[K_NEAREST];
   int heap_size = 0;

   for (size_t i = 0; i < n; ++i) {
      for (size_t j = i + 1; j < n; ++j) {
         int64_t dx = (int64_t)points[i].x - (int64_t)points[j].x;
         int64_t dy = (int64_t)points[i].y - (int64_t)points[j].y;
         int64_t dz = (int64_t)points[i].z - (int64_t)points[j].z;
         uint64_t dist = (uint64_t)(dx * dx + dy * dy + dz * dz);

         Edge e;
         e.dist = dist;
         e.a = (int)i;
         e.b = (int)j;
         heap_insert_topk(heap, &heap_size, &e);
      }
   }

   int edges_count = heap_size;

   Edge *edges = malloc((size_t)edges_count * sizeof(Edge));
   for (int i = 0; i < edges_count; ++i) {
      edges[i] = heap[i];
   }

   qsort(edges, (size_t)edges_count, sizeof(Edge), cmp_edge);

   DSU dsu;
   dsu_init(&dsu, (int)n);
   for (int i = 0; i < edges_count; ++i) {
      dsu_union(&dsu, edges[i].a, edges[i].b);
   }

   int *sizes = malloc(n * sizeof(int));
   int sizes_count = 0;
   for (int i = 0; i < (int)n; ++i) {
      if (dsu_find(&dsu, i) == i) {
         sizes[sizes_count++] = dsu.sz[i];
      }
   }

   if (sizes_count == 0) {
      *out_answer = 0;
   } else {
      qsort(sizes, (size_t)sizes_count, sizeof(int), cmp_int_desc);
      uint64_t prod = 1;
      int limit = sizes_count < 3 ? sizes_count : 3;
      for (int i = 0; i < limit; ++i) {
         prod *= (uint64_t)sizes[i];
      }
      *out_answer = prod;
   }

   free(points);
   free(edges);
   free(sizes);
   dsu_free(&dsu);
}
