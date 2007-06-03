/* Graph representation and manipulation functions.
   Copyright (C) 2007
   Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "obstack.h"
#include "bitmap.h"
#include "vec.h"
#include "vecprim.h"
#include "graphds.h"

/* Dumps graph G into F.  */

void
dump_graph (FILE *f, struct graph *g)
{
  int i;
  struct edge *e;

  for (i = 0; i < g->n_vertices; i++)
    {
      if (!g->vertices[i].pred
	  && !g->vertices[i].succ)
	continue;

      fprintf (f, "%d (%d)\t<-", i, g->vertices[i].component);
      for (e = g->vertices[i].pred; e; e = e->pred_next)
	fprintf (f, " %d", e->src);
      fprintf (f, "\n");

      fprintf (f, "\t->");
      for (e = g->vertices[i].succ; e; e = e->succ_next)
	fprintf (f, " %d", e->dest);
      fprintf (f, "\n");
    }
}

/* Creates a new graph with N_VERTICES vertices.  */

struct graph *
new_graph (int n_vertices)
{
  struct graph *g = XNEW (struct graph);

  g->n_vertices = n_vertices;
  g->vertices = XCNEWVEC (struct vertex, n_vertices);

  return g;
}

/* Adds an edge from F to T to graph G.  The new edge is returned.  */

struct edge *
add_edge (struct graph *g, int f, int t)
{
  struct edge *e = XNEW (struct edge);
  struct vertex *vf = &g->vertices[f], *vt = &g->vertices[t];


  e->src = f;
  e->dest = t;

  e->pred_next = vt->pred;
  vt->pred = e;

  e->succ_next = vf->succ;
  vf->succ = e;

  return e;
}

/* Moves all the edges incident with U to V.  */

void
identify_vertices (struct graph *g, int v, int u)
{
  struct vertex *vv = &g->vertices[v];
  struct vertex *uu = &g->vertices[u];
  struct edge *e, *next;

  for (e = uu->succ; e; e = next)
    {
      next = e->succ_next;

      e->src = v;
      e->succ_next = vv->succ;
      vv->succ = e;
    }
  uu->succ = NULL;

  for (e = uu->pred; e; e = next)
    {
      next = e->pred_next;

      e->dest = v;
      e->pred_next = vv->pred;
      vv->pred = e;
    }
  uu->pred = NULL;
}

/* Helper function for graphds_dfs.  Returns the source vertex of E, in the
   direction given by FORWARD.  */

static inline int
dfs_edge_src (struct edge *e, bool forward)
{
  return forward ? e->src : e->dest;
}

/* Helper function for graphds_dfs.  Returns the destination vertex of E, in
   the direction given by FORWARD.  */

static inline int
dfs_edge_dest (struct edge *e, bool forward)
{
  return forward ? e->dest : e->src;
}

/* Helper function for graphds_dfs.  Returns the first edge after E (including
   E), in the graph direction given by FORWARD, that belongs to SUBGRAPH.  */

static inline struct edge *
foll_in_subgraph (struct edge *e, bool forward, bitmap subgraph)
{
  int d;

  if (!subgraph)
    return e;

  while (e)
    {
      d = dfs_edge_dest (e, forward);
      if (bitmap_bit_p (subgraph, d))
	return e;

      e = forward ? e->succ_next : e->pred_next;
    }

  return e;
}

/* Helper function for graphds_dfs.  Select the first edge from V in G, in the
   direction given by FORWARD, that belongs to SUBGRAPH.  */

static inline struct edge *
dfs_fst_edge (struct graph *g, int v, bool forward, bitmap subgraph)
{
  struct edge *e;

  e = (forward ? g->vertices[v].succ : g->vertices[v].pred);
  return foll_in_subgraph (e, forward, subgraph);
}

/* Helper function for graphds_dfs.  Returns the next edge after E, in the
   graph direction given by FORWARD, that belongs to SUBGRAPH.  */

static inline struct edge *
dfs_next_edge (struct edge *e, bool forward, bitmap subgraph)
{
  return foll_in_subgraph (forward ? e->succ_next : e->pred_next,
			   forward, subgraph);
}

/* Runs dfs search over vertices of G, from NQ vertices in queue QS.
   The vertices in postorder are stored into QT.  If FORWARD is false,
   backward dfs is run.  If SUBGRAPH is not NULL, it specifies the
   subgraph of G to run DFS on.  Returns the number of the components
   of the graph (number of the restarts of DFS).  */

int
graphds_dfs (struct graph *g, int *qs, int nq, VEC (int, heap) **qt,
	     bool forward, bitmap subgraph)
{
  int i, tick = 0, v, comp = 0, top;
  struct edge *e;
  struct edge **stack = XNEWVEC (struct edge *, g->n_vertices);
  bitmap_iterator bi;
  unsigned av;

  if (subgraph)
    {
      EXECUTE_IF_SET_IN_BITMAP (subgraph, 0, av, bi)
	{
	  g->vertices[av].component = -1;
	  g->vertices[av].post = -1;
	}
    }
  else
    {
      for (i = 0; i < g->n_vertices; i++)
	{
	  g->vertices[i].component = -1;
	  g->vertices[i].post = -1;
	}
    }

  for (i = 0; i < nq; i++)
    {
      v = qs[i];
      if (g->vertices[v].post != -1)
	continue;

      g->vertices[v].component = comp++;
      e = dfs_fst_edge (g, v, forward, subgraph);
      top = 0;

      while (1)
	{
	  while (e)
	    {
	      if (g->vertices[dfs_edge_dest (e, forward)].component
		  == -1)
		break;
	      e = dfs_next_edge (e, forward, subgraph);
	    }

	  if (!e)
	    {
	      if (qt)
		VEC_safe_push (int, heap, *qt, v);
	      g->vertices[v].post = tick++;

	      if (!top)
		break;

	      e = stack[--top];
	      v = dfs_edge_src (e, forward);
	      e = dfs_next_edge (e, forward, subgraph);
	      continue;
	    }

	  stack[top++] = e;
	  v = dfs_edge_dest (e, forward);
	  e = dfs_fst_edge (g, v, forward, subgraph);
	  g->vertices[v].component = comp - 1;
	}
    }

  free (stack);

  return comp;
}

/* Determines the strongly connected components of G, using the algorithm of
   Tarjan -- first determine the postorder dfs numbering in reversed graph,
   then run the dfs on the original graph in the order given by decreasing
   numbers assigned by the previous pass.  If SUBGRAPH is not NULL, it
   specifies the subgraph of G whose strongly connected components we want
   to determine.
   
   After running this function, v->component is the number of the strongly
   connected component for each vertex of G.  Returns the number of the
   sccs of G.  */

int
graphds_scc (struct graph *g, bitmap subgraph)
{
  int *queue = XNEWVEC (int, g->n_vertices);
  VEC (int, heap) *postorder = NULL;
  int nq, i, comp;
  unsigned v;
  bitmap_iterator bi;

  if (subgraph)
    {
      nq = 0;
      EXECUTE_IF_SET_IN_BITMAP (subgraph, 0, v, bi)
	{
	  queue[nq++] = v;
	}
    }
  else
    {
      for (i = 0; i < g->n_vertices; i++)
	queue[i] = i;
      nq = g->n_vertices;
    }

  graphds_dfs (g, queue, nq, &postorder, false, subgraph);
  gcc_assert (VEC_length (int, postorder) == (unsigned) nq);

  for (i = 0; i < nq; i++)
    queue[i] = VEC_index (int, postorder, nq - i - 1);
  comp = graphds_dfs (g, queue, nq, NULL, true, subgraph);

  free (queue);
  VEC_free (int, heap, postorder);

  return comp;
}

/* Runs CALLBACK for all edges in G.  */

void
for_each_edge (struct graph *g, graphds_edge_callback callback)
{
  struct edge *e;
  int i;

  for (i = 0; i < g->n_vertices; i++)
    for (e = g->vertices[i].succ; e; e = e->succ_next)
      callback (g, e);
}

/* Releases the memory occupied by G.  */

void
free_graph (struct graph *g)
{
  struct edge *e, *n;
  struct vertex *v;
  int i;

  for (i = 0; i < g->n_vertices; i++)
    {
      v = &g->vertices[i];
      for (e = v->succ; e; e = n)
	{
	  n = e->succ_next;
	  free (e);
	}
    }
  free (g->vertices);
  free (g);
}

/* Returns the nearest common ancestor of X and Y in tree whose parent
   links are given by PARENT.  MARKS is the array used to mark the
   vertices of the tree, and MARK is the number currently used as a mark.  */

static int
tree_nca (int x, int y, int *parent, int *marks, int mark)
{
  if (x == -1 || x == y)
    return y;

  /* We climb with X and Y up the tree, marking the visited nodes.  When
     we first arrive to a marked node, it is the common ancestor.  */
  marks[x] = mark;
  marks[y] = mark;

  while (1)
    {
      x = parent[x];
      if (x == -1)
	break;
      if (marks[x] == mark)
	return x;
      marks[x] = mark;

      y = parent[y];
      if (y == -1)
	break;
      if (marks[y] == mark)
	return y;
      marks[y] = mark;
    }

  /* If we reached the root with one of the vertices, continue
     with the other one till we reach the marked part of the
     tree.  */
  if (x == -1)
    {
      for (y = parent[y]; marks[y] != mark; y = parent[y])
	continue;

      return y;
    }
  else
    {
      for (x = parent[x]; marks[x] != mark; x = parent[x])
	continue;

      return x;
    }
}

/* Determines the dominance tree of G (stored in the PARENT, SON and BROTHER
   arrays), where the entry node is ENTRY.  */

void
graphds_domtree (struct graph *g, int entry,
		 int *parent, int *son, int *brother)
{
  VEC (int, heap) *postorder = NULL;
  int *marks = XCNEWVEC (int, g->n_vertices);
  int mark = 1, i, v, idom;
  bool changed = true;
  struct edge *e;

  /* We use a slight modification of the standard iterative algorithm, as
     described in
     
     K. D. Cooper, T. J. Harvey and K. Kennedy: A Simple, Fast Dominance
	Algorithm

     sort vertices in reverse postorder
     foreach v
       dom(v) = everything
     dom(entry) = entry;

     while (anything changes)
       foreach v
         dom(v) = {v} union (intersection of dom(p) over all predecessors of v)

     The sets dom(v) are represented by the parent links in the current version
     of the dominance tree.  */

  for (i = 0; i < g->n_vertices; i++)
    {
      parent[i] = -1;
      son[i] = -1;
      brother[i] = -1;
    }
  graphds_dfs (g, &entry, 1, &postorder, true, NULL);
  gcc_assert (VEC_length (int, postorder) == (unsigned) g->n_vertices);
  gcc_assert (VEC_index (int, postorder, g->n_vertices - 1) == entry);

  while (changed)
    {
      changed = false;

      for (i = g->n_vertices - 2; i >= 0; i--)
	{
	  v = VEC_index (int, postorder, i);
	  idom = -1;
	  for (e = g->vertices[v].pred; e; e = e->pred_next)
	    {
	      if (e->src != entry
		  && parent[e->src] == -1)
		continue;

	      idom = tree_nca (idom, e->src, parent, marks, mark++);
	    }

	  if (idom != parent[v])
	    {
	      parent[v] = idom;
	      changed = true;
	    }
	}
    }

  free (marks);
  VEC_free (int, heap, postorder);

  for (i = 0; i < g->n_vertices; i++)
    if (parent[i] != -1)
      {
	brother[i] = son[parent[i]];
	son[parent[i]] = i;
      }
}
