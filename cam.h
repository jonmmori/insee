/**
* @file
* @brief	Declaration of CAM tables (if used).
*/

#ifndef _cam
#define _cam

#include "graph.h"
#include "list.h"
#include "ksp_routing.h"

typedef struct cam_t {

    long n_paths; // Number of paths to one destination
    long l_path;  // Last path used to one destination
    long **ports;  // Port to be used

} cam_t;

void init_cams_rr();

void init_cams_adaptive();

void fill_cam_sp(long switch_id, graph_t *graph);

void fill_cam_ecmp(long switch_id, graph_t *graph);

void fill_cam_ksp(long switch_id, graph_t *graph);

void fill_cam_llskr(long switch_id, graph_t *graph);

void fill_cam_allpath(long switch_id, graph_t *graph);

void set_paths(path_t *path);

void set_paths_reverse(path_t *path);

void set_mpaths_rr(path_t **path, long n_paths);

void set_mpaths_reverse_rr(path_t **path, long n_paths);

void set_mpaths_adaptive(path_t **path, long n_paths);

void set_mpaths_reverse_adaptive(path_t **path, long n_paths);

void print_cams();

void print_vector(long *v, long l);

void finish_cams_rr();

void finish_cams_adaptive();
#endif


