#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include "slicer.h"

using namespace std;

/*
 * The canonical subvolume
 *
 *               ^ y
 *               |    c
 *              2*------------*3
 *              /|           /|
 *            h/ |d        g/ |
 *            /  |   k     /  |b
 *          6*------------*7  |
 *           |   |        |   |1
 *          l|  0*--------|---*----> x
 *           |  /    a    |  / 
 *           | /e        j| /f
 *           |/           |/
 *          4*------------*5
 *          /      i
 *         /
 *      z L
 *
 * Edges lettered a-l are numbered 0-11 in vert_info.
 * Important: for each vertex, the neighboring vertices and connecting
 * edges are listed in counterclockwise order when you are looking at
 * them from outside the cube.  The smallest vertex is listed first, but
 * that doesn't really matter (I think).
 *
 * The neighbor members in vert_info are currently unused...
 */
const SlicerState::VertInfo SlicerState::vert_info[NUM_VERTS] = {
  /* vert 0 */ { { 0.0f, 0.0f, 0.0f }, { { 1,  0 }, { 2,  3 }, { 4,  4 } } },
  /* vert 1 */ { { 1.0f, 0.0f, 0.0f }, { { 0,  0 }, { 5,  5 }, { 3,  1 } } },
  /* vert 2 */ { { 0.0f, 1.0f, 0.0f }, { { 0,  3 }, { 3,  2 }, { 6,  7 } } },
  /* vert 3 */ { { 1.0f, 1.0f, 0.0f }, { { 1,  1 }, { 7,  6 }, { 2,  2 } } },
  /* vert 4 */ { { 0.0f, 0.0f, 1.0f }, { { 0,  4 }, { 6, 11 }, { 5,  8 } } },
  /* vert 5 */ { { 1.0f, 0.0f, 1.0f }, { { 1,  5 }, { 4,  8 }, { 7,  9 } } },
  /* vert 6 */ { { 0.0f, 1.0f, 1.0f }, { { 2,  7 }, { 7, 10 }, { 4, 11 } } },
  /* vert 7 */ { { 1.0f, 1.0f, 1.0f }, { { 3,  6 }, { 5,  9 }, { 6, 10 } } }
};
// vertices at end of each edge (in no particular order)
const char SlicerState::edge_verts[NUM_EDGES][2] = {
  /* a */ { 0, 1 }, /* b */ { 1, 3 }, /* c */ { 3, 2 }, /* d */ { 2, 0 },
  /* e */ { 0, 4 }, /* f */ { 1, 5 }, /* g */ { 3, 7 }, /* h */ { 2, 6 },
  /* i */ { 4, 5 }, /* j */ { 5, 7 }, /* k */ { 7, 6 }, /* l */ { 6, 4 },
};


/* #define DEBUG */
#ifdef DEBUG
#define IF_DB(x) x
#else
#define IF_DB(x)
#endif

SlicerState::SlicerState(float d[DIMS], float s, float dfo)
{
  update_state(d, s, dfo);

  n_slices_alloced = 0;

  // copy connectivity info into vert structure for later use
  for (int v = 0; v < NUM_VERTS; v++) {
    for (int i = 0; i < 3; i++)
      vert[v].conn_info[i] = vert_info[v].conn_info[i];
  }
}

void SlicerState::update_state(float d[DIMS], float s, float dfo)
{
  double len = sqrt((double)d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
  dir[0] = d[0]/len;
  dir[1] = d[1]/len;
  dir[2] = d[2]/len;

  spacing = s;
  r_spacing = 1.0/spacing;
  dist_from_origin = dfo;

  // Set sort_dim1 to dimension of dir that is smallest, sort_dim2 to
  // next smallest.  These correspond to the axes that are the most
  // perpendicular to dir (i.e. have the smallest dot product).
  struct pair { double v; int d; };
  pair p[3];
  // v = dot product of dir and axis vectors [(0,0,1), (0,1,0), & (0,0,1)]
  p[0].v = fabs(dir[0]); p[0].d = 0;
  p[1].v = fabs(dir[1]); p[1].d = 1;
  p[2].v = fabs(dir[2]); p[2].d = 2;

  // sort p's records so smallest is at front (bubble sort!)
  pair tmp;
  if (p[0].v > p[1].v)  { tmp = p[0]; p[0] = p[1]; p[1] = tmp; }
  if (p[1].v > p[2].v)  { tmp = p[1]; p[1] = p[2]; p[2] = tmp; }
  if (p[0].v > p[1].v)  { tmp = p[0]; p[0] = p[1]; p[1] = tmp; }

  sort_dim1 = p[0].d;
  sort_dim2 = p[1].d;

  // compute slopes each edge based on unit volume since they are independent
  // of the actual volume size
  double dist[NUM_VERTS];
  for (int v = 0; v < NUM_VERTS; v++)
    dist[v] = vert_info[v].coord[0] * dir[0] + vert_info[v].coord[1] * dir[1] + 
      vert_info[v].coord[2] * dir[2] + dist_from_origin;

  for (int e = 0; e < NUM_EDGES; e++) {
    int v0 = edge_verts[e][0];
    int v1 = edge_verts[e][1];
    double inc_fract = spacing/(dist[v0] - dist[v1]);
    edge[e].inc[0] = inc_fract *
      (vert_info[v0].coord[0] - vert_info[v1].coord[0]);
    edge[e].inc[1] = inc_fract *
      (vert_info[v0].coord[1] - vert_info[v1].coord[1]);
    edge[e].inc[2] = inc_fract *
      (vert_info[v0].coord[2] - vert_info[v1].coord[2]);
  }
}

SlicerState::~SlicerState()
{
  if (n_slices_alloced > 0) {
    delete[] verts_at_slice;
    delete[] slice_list;
    delete[] slice_storage;
  }
}

int SlicerState::calc_slices(float min[DIMS], float max[DIMS],
  Slice **slices)
{
  // reordered to reuse right-hand-side values
  vert[0].coord[0]=vert[2].coord[0]=vert[4].coord[0]=vert[6].coord[0]=min[0];
  vert[1].coord[0]=vert[3].coord[0]=vert[5].coord[0]=vert[7].coord[0]=max[0];
  vert[0].coord[1]=vert[1].coord[1]=vert[4].coord[1]=vert[5].coord[1]=min[1];
  vert[2].coord[1]=vert[3].coord[1]=vert[6].coord[1]=vert[7].coord[1]=max[1];
  vert[0].coord[2]=vert[1].coord[2]=vert[2].coord[2]=vert[3].coord[2]=min[2];
  vert[4].coord[2]=vert[5].coord[2]=vert[6].coord[2]=vert[7].coord[2]=max[2];

  // compute distance each vertex is from origin along vector, plus min & max
  int v;
  double min_dist = +1e38;
  double max_dist = -1e38;
  for (v = 0; v < NUM_VERTS; v++) {
    vert[v].dist = vert[v].coord[0] * dir[0] + vert[v].coord[1] * dir[1] + 
      vert[v].coord[2] * dir[2] + dist_from_origin;
    if (min_dist > vert[v].dist)
      min_dist = vert[v].dist;
    if (max_dist < vert[v].dist)
      max_dist = vert[v].dist;
  }

  // init edges
  int e;
  for (e = 0; e < NUM_EDGES; e++) {
    edge[e].is_in = false;
  }


  // compute where slices are and how many there will be
  double last_skipped_slice_num = floor(min_dist * r_spacing);
  double last_slice_num = floor(max_dist * r_spacing);
  int n_slices = int(last_slice_num - last_skipped_slice_num);
  double last_skipped_slice_pos = spacing * last_skipped_slice_num;

  assert(last_skipped_slice_pos <= min_dist);

  int slice_slots_needed = n_slices + 1;

  // The cube behind this one will generate the last slice in this case,
  // so skip it to avoid duplicate slices.
  // (this also keeps all the slices before the last vertex is processed)
  if (spacing * last_slice_num == max_dist) {
    n_slices--;
  }
  if (n_slices == 0)
    return 0;

  IF_DB(cout << "dist min " << min_dist << " max " << max_dist <<
    " n_slices " << n_slices << " n_slots " << slice_slots_needed << 
    " slice -1 pos " << last_skipped_slice_pos << endl);

  // allocate more space if necessary
  if (n_slices_alloced < slice_slots_needed) {
    if (n_slices_alloced > 0) {
      delete[] verts_at_slice;
      delete[] slice_list;
      delete[] slice_storage;
    }
    int n_to_alloc = slice_slots_needed;
    if (n_to_alloc < n_slices_alloced*2)
      n_to_alloc = n_slices_alloced*2;
    verts_at_slice = new Vert*[n_to_alloc];
    slice_list = new Slice[n_to_alloc];
    slice_storage = new V3f[MAX_VERTS_PER_SLICE * n_to_alloc];
    n_slices_alloced = n_to_alloc;
  }

  int i;
  // loop was:
  // for (i = 0; i < slice_slots_needed; i++) verts_at_slice[i] = 0;
  int to_clear = slice_slots_needed;   // to avoid ptr aliasing
  Vert** vas_ptr = verts_at_slice;
  for (i = 0; i < to_clear; i++)
    *vas_ptr++ = 0;


  // put vertices in slice where it needs to be processed
  for (v = 0; v < NUM_VERTS; v++) {
    int slice = int((vert[v].dist - last_skipped_slice_pos) * r_spacing);
    assert(slice < slice_slots_needed);

    IF_DB(cout << "vert " << v << " " << vert[v].coord[0] << ',' << 
      vert[v].coord[1] << ',' << vert[v].coord[2] << " slice " << slice <<
      " dist " << vert[v].dist << endl);

    // find place to insert vert so list is sorted by dist
    Vert** prev_ptr_loc = &verts_at_slice[slice];
    Vert* curr_vert = verts_at_slice[slice];
    while (curr_vert) {
      if (vert[v].dist < curr_vert->dist)
	break;
      if (vert[v].dist == curr_vert->dist) {  // break dist ties by coord sorts
	if (vert[v].coord[sort_dim1] < curr_vert->coord[sort_dim1])
	  break;
	if (vert[v].coord[sort_dim1] == curr_vert->coord[sort_dim1]) {
	  if (vert[v].coord[sort_dim2] < curr_vert->coord[sort_dim2])
	    break;
	}
      }
      prev_ptr_loc = &curr_vert->next_slice_vert;
      curr_vert = curr_vert->next_slice_vert;
    }

    assert(*prev_ptr_loc == curr_vert);
    *prev_ptr_loc = &vert[v];
    vert[v].next_slice_vert = curr_vert;
  }

#if 0
  // DEBUG
  for (i = 0; i < slice_slots_needed; i++) {
    cout << "slot " << i << endl;
    Vert* vp = verts_at_slice[i];
    while (vp) {
      cout << "vert " << vp - vert << " " << vp->coord[0] << " " <<
	vp->coord[1] << " " << vp->coord[2] << " dist " << vp->dist << endl;
      vp = vp->next_slice_vert;
    }
  }
#endif

  // now iterate over the slices.  Set up variables:
  Edge* ae_list = 0;
  V3f *next_slice_storage_pos = slice_storage;
  *slices = slice_list;
  double curr_dist = last_skipped_slice_pos + spacing;
  assert(min_dist <= curr_dist);

  for(int s = 0; s < n_slices; s++) {

    IF_DB(cout << "slice " << s << endl);

    // handle any vertices that are between last slice and this one
    Vert* curr_vert = verts_at_slice[s];
    while (curr_vert) {

      IF_DB(cout << "process vert " << curr_vert - vert << endl);

      int n_in_edges = 0;
      Edge* in_edge[3];
      int n_out_edges = 0;
      Edge* out_edge[3];
      int last_out_edge = -1;

      // put edges in in_edge and out_edge lists, plus any other stuff
      for (v = 0; v < 3; v++) {
	Edge* curr_edge = &edge[curr_vert->conn_info[v].edge];
	if (curr_edge->is_in) { // edge was in, mark out
	  in_edge[n_in_edges++] = curr_edge;
	  curr_edge->is_in = false;
	  last_out_edge = v;
	  IF_DB(cout << "vert edge " << int(curr_vert->conn_info[v].edge) <<
	    " was in " << endl);
	}
	else {  // edge not in, compute slopes & mark in
	  double bump_fract = (curr_dist - curr_vert->dist) * r_spacing;
	  curr_edge->curr_pos[0] = curr_vert->coord[0] + bump_fract *
	    curr_edge->inc[0];
	  curr_edge->curr_pos[1] = curr_vert->coord[1] + bump_fract *
	    curr_edge->inc[1];
	  curr_edge->curr_pos[2] = curr_vert->coord[2] + bump_fract *
	    curr_edge->inc[2];
	  out_edge[n_out_edges] = curr_edge;
	  n_out_edges++;
	  curr_edge->is_in = true;
	  IF_DB(cout << "vert edge " << int(curr_vert->conn_info[v].edge) <<
	    " was out" << endl);
	}
      }
      // insert/delete edges from active edge list
      switch (n_in_edges) {
      case 0:
	// All edges out, must be first vertex
	assert(ae_list == 0);
	ae_list = out_edge[0];
	out_edge[0]->prev = out_edge[2];
	out_edge[0]->next = out_edge[1];
	out_edge[1]->prev = out_edge[0];
	out_edge[1]->next = out_edge[2];
	out_edge[2]->prev = out_edge[1];
	out_edge[2]->next = out_edge[0];
	break;
      case 1:
	// Replace old edge with 2 new edges
	{
	  Edge *e0, *e1;
	  if (last_out_edge == 1) {  // handle case when in_edge in the middle
	    e0 = out_edge[1];
	    e1 = out_edge[0];
	  }
	  else {
	    e0 = out_edge[0];
	    e1 = out_edge[1];
	  }
	  Edge* ep = in_edge[0]->prev;
	  Edge* en = in_edge[0]->next;
	  ep->next = e0;
	  e0->prev = ep;
	  e0->next = e1;
	  e1->prev = e0;
	  e1->next = en;
	  en->prev = e1;

	  // maybe always set ae_list to out_edge[0]? it would be faster
	  if (ae_list == in_edge[0])
	    ae_list = out_edge[0];
	  IF_DB(cout << "replace edge " << in_edge[0] - edge <<
	    " with " << e0-edge << " & " << e1-edge << endl);
	}
	break;
      case 2:
	// Replace old edge pair with new edge. Find which in-edge is first.
	{
	  Edge *e0, *e1;
	  if (in_edge[0]->next == in_edge[1]) {
	    e0 = in_edge[0];
	    e1 = in_edge[1];
	  }
	  else {
	    e0 = in_edge[1];
	    e1 = in_edge[0];
	  }
	  Edge* ep = e0->prev;
	  Edge* en = e1->next;
	  ep->next = out_edge[0];
	  en->prev = out_edge[0];
	  out_edge[0]->prev = ep;
	  out_edge[0]->next = en;
	  // maybe always set ae_list to out_edge[0]?
	  if (ae_list == in_edge[0] || ae_list == in_edge[1])
	    ae_list = out_edge[0];
	  IF_DB(cout << "replace edges " << e0-edge << " & " << e1-edge <<
	    " with " << out_edge[0] - edge << endl);
	}
	break;
      case 3:
	// no need to free up the last 3 active edges, it is after the
	// last slice
	if (s != n_slices - 1) {
	  cerr << "processing last vert!" << endl;
	  abort();
	}
	IF_DB(cout << "case #3 found !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
	  << endl);
	break;
      default:
	cerr << "bad n_in_edges!" << endl;
	abort();
	break;
      }

#ifdef DEBUG
      cout << "curr ae via nexts:"; 
      Edge* dbg_edge = ae_list;
      if (dbg_edge) {
	do {
	  cout << ' ' << dbg_edge - edge;
	  dbg_edge = dbg_edge->next;
	} while (dbg_edge != ae_list);
      }
      cout << endl;
      cout << "curr ae via prevs:"; 
      dbg_edge = ae_list;
      if (dbg_edge) {
	do {
	  cout << ' ' << dbg_edge - edge;
	  dbg_edge = dbg_edge->prev;
	} while (dbg_edge != ae_list);
      }
      cout << endl;
#endif
      curr_vert = curr_vert->next_slice_vert;
    }



    // Now active edge list holds polygon for the current slicing plane.
    // Copy vertices into into output list & move vertices to next slice.

    IF_DB(cout << "output:" << endl);
    Edge* curr_edge = ae_list;
    slice_list[s].vert = next_slice_storage_pos;
    int n_verts = 0;
    do {
      IF_DB(cout << curr_edge->curr_pos[0] << ',' << curr_edge->curr_pos[1] <<
	',' << curr_edge->curr_pos[2] << endl);

      // copy edge's vert into poly list & bump edge pos to next slice's pos
      double tmp;  // avoids extra loads
      tmp = curr_edge->curr_pos[0];
      curr_edge->curr_pos[0] += curr_edge->inc[0];
      (*next_slice_storage_pos)[0] = tmp;

      tmp = curr_edge->curr_pos[1];
      curr_edge->curr_pos[1] += curr_edge->inc[1];
      (*next_slice_storage_pos)[1] = tmp;

      tmp = curr_edge->curr_pos[2];
      curr_edge->curr_pos[2] += curr_edge->inc[2];
      (*next_slice_storage_pos)[2] =  tmp;

      next_slice_storage_pos++;


      curr_edge = curr_edge->next;
      n_verts++;
    } while(curr_edge != ae_list);

    slice_list[s].n_verts = n_verts;

    curr_dist += spacing;
  }
  return n_slices;
}
