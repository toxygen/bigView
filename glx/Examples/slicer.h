// SlicerState class
//
// This class computes a series of intersections between an axis-aligned
// parallelpiped and a series of parallel planes.  The vector normal
// to the planes, the distance between the planes, and the distance
// between the first plane and the origin can be specified.  These
// values are saved in the class for further intersections with
// a series of volumes.  The values can be modified with the update_state
// method.
//
// The slices are generated using the calc_slices method.  The method
// takes the bounding box of the parallel piped as incoming parameters,
// and returns a list of slices via the third parameter.  The storage
// for the slices is reused between calls to calc_slices for efficiency.
// The storage is freed when the SlicerState object is destroyed.
// 
// The slices are calculated using an incremental algorithm.  This
// algorithm is most efficient when there are several slices generated
// for each volume.  If there is only a couple slices generated, other
// algorithms would probably be faster.

class SlicerState {
public:
  enum { DIMS = 3 };

  typedef float V3f[DIMS];
  struct Slice {
    int n_verts;
    V3f *vert;
  };

  SlicerState(V3f direction, float spacing, float dist_from_origin);
  ~SlicerState();

  // returns number of slices (polygons)
  int calc_slices(V3f min, V3f max, Slice **slice);
  void update_state(V3f direction, float spacing, float dist_from_origin);

private:
  typedef double V3d[DIMS];

  // private constants
  enum { MAX_VERTS_PER_SLICE = 6, NUM_VERTS = 8, NUM_EDGES = 12 };


  // state that is saved
  V3d dir;
  double spacing;
  double r_spacing;
  double dist_from_origin;

  // Dimension (x, y, or z) to sort vertices by after sorting by
  // distance.  This handles cases when dir is parallel with an axis.
  int sort_dim1, sort_dim2;

  struct ConnInfo { char neighbor, edge; };

  // vertex storage; initialized in constructor
  struct Vert {
    V3d coord;
    double dist;
    Vert* next_slice_vert;
    ConnInfo conn_info[DIMS];
  };
  Vert vert[NUM_VERTS];

  int n_slices_alloced;

  // next 3 have size proportional to n_slice_alloced
  Vert** verts_at_slice;	// n_slices_alloced ints allocated
  Slice* slice_list;		// n_slices_alloced pointers allocated
  V3f* slice_storage;		// MAX_VERTS_PER_SLICE * n_slices_alloced
				// V3f's allocated

  // edge storage; avoids having to call malloc or put it on stack
  struct Edge {
    double inc[DIMS];
    double curr_pos[DIMS];
    Edge* next;
    Edge* prev;
    bool is_in;
  };

  Edge edge[NUM_EDGES];
    
  // the canonical volume
  struct VertInfo {
    V3f coord;  // coord if unit volume
    ConnInfo conn_info[DIMS];
  };
  static const VertInfo vert_info[NUM_VERTS];
  static const char edge_verts[NUM_EDGES][2];
};
