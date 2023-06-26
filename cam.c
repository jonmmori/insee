/**
 * @file
 * @brief	Declaration of CAM tables (if used).

 FSIN Functional Simulator of Interconnection Networks
 Copyright (2003-2017) Jose A. Pascual, J. Navaridas

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "globals.h"
#include "cam.h"
#include "graph.h"

void init_cams_rr(){

    long i, switch_id, size;

    size = cam_policy_params[0];

    if(size <= 0)
        size = 1;

    for(switch_id = nprocs; switch_id < NUMNODES; switch_id++){
        network[switch_id].cam = alloc(nswitches * sizeof(cam_t));
        for(i = 0; i < nswitches; i++){
            network[switch_id].cam[i].n_paths = 0; 
            network[switch_id].cam[i].l_path = 0;
            network[switch_id].cam[i].ports = alloc(size * sizeof(long*));
        }
    }
}

void init_cams_adaptive(){

    long i, switch_id, k;

    for(switch_id = nprocs; switch_id < NUMNODES; switch_id++){
        network[switch_id].cam = alloc(nswitches * sizeof(cam_t));
        for(i = 0; i < nswitches; i++){
            network[switch_id].cam[i].n_paths = 0; 
            network[switch_id].cam[i].l_path = 0;
            network[switch_id].cam[i].ports = alloc( 2 * sizeof(long*));
            network[switch_id].cam[i].ports[0] = alloc(stUp * sizeof(long));
            network[switch_id].cam[i].ports[1] = alloc(stUp * sizeof(long));
            for(k = 0; k < stUp; k++){
                network[switch_id].cam[i].ports[0][k] = -1; // Next port
                network[switch_id].cam[i].ports[1][k] = -1; // Distances
            }
        }
    }
}

void fill_cam_sp(long node_id, graph_t *graph){

    long *paths, *paths_dists, *links, *links_r;
    long switch_id, end, diam_aux;
    path_t *l;

    switch_id = node_id - nprocs;
    paths = alloc(nswitches * sizeof(long));
    paths_dists = alloc(nswitches * sizeof(long));
    links = alloc(nswitches * sizeof(long));
    links_r = alloc(nswitches * sizeof(long));
    l = init_path(nswitches);

    diam_aux = find_shortest_path(switch_id, paths, paths_dists, links, links_r, graph);

    if(diam_aux > diameter_t){
        diameter_t = diam_aux;
        diameter_r = diam_aux;
    }

    for(end = switch_id; end < nswitches; end++){ 
        shortest_path(l, paths, links, links_r, switch_id, end, paths_dists[end]);
        set_paths(l);
        set_paths_reverse(l);
    }
    free(paths);
    free(paths_dists);
    free(links);
    free(links_r);
    destroy_path(l);
}

void fill_cam_ecmp(long node_id, graph_t *graph){

    long i, switch_id, end, m_paths, m_paths_aux, diam_aux;
    long *paths, *paths_dists, *links, *links_r;

    switch_id = node_id - nprocs;
    paths = alloc(nswitches * sizeof(long));
    paths_dists = alloc(nswitches * sizeof(long));
    links = alloc(nswitches * sizeof(long));
    links_r = alloc(nswitches * sizeof(long));
    m_paths = cam_policy_params[0];

    path_t **k_paths= alloc(m_paths * sizeof(path_t*));

    diam_aux = find_shortest_path(switch_id, paths, paths_dists, links, links_r, graph);

    if(diam_aux > diameter_t)
        diameter_t = diam_aux;

    for(end = switch_id; end < nswitches; end++){ 
        k_paths[0] = init_path(nswitches);
        m_paths_aux = shortest_path_generic(k_paths, paths, links, links_r, switch_id, end, paths_dists[end], m_paths, 1, graph);
        set_mpaths(k_paths, m_paths_aux);
        set_mpaths_reverse(k_paths, m_paths_aux);

        for(i = 0; i < m_paths_aux; i++){
            destroy_path(k_paths[i]);
        }

    }
    free(k_paths);
    free(paths);
    free(paths_dists);
    free(links);
    free(links_r);
}

void fill_cam_ksp(long node_id, graph_t *graph){

    long i, switch_id, end, m_paths, m_paths_aux, diam_aux;
    long *paths, *paths_dists, *links, *links_r;

    switch_id = node_id - nprocs;
    paths = alloc(nswitches * sizeof(long));
    paths_dists = alloc(nswitches * sizeof(long));
    links = alloc(nswitches * sizeof(long));
    links_r = alloc(nswitches * sizeof(long));
    m_paths = cam_policy_params[0];

    path_t **k_paths= alloc(m_paths * sizeof(path_t*));

    diam_aux = find_shortest_path(switch_id, paths, paths_dists, links, links_r, graph);

    if(diam_aux > diameter_t)
        diameter_t = diam_aux;

    for(end = switch_id; end < nswitches; end++){ 
        k_paths[0] = init_path(nswitches);
        m_paths_aux = shortest_path_generic(k_paths, paths, links, links_r, switch_id, end, paths_dists[end], m_paths, 0, graph);
        set_mpaths(k_paths, m_paths_aux);
        set_mpaths_reverse(k_paths, m_paths_aux);

        for(i = 0; i < m_paths_aux; i++){
            destroy_path(k_paths[i]);
        }
    }
    free(k_paths);
    free(paths);
    free(paths_dists);
    free(links);
    free(links_r);
}

void fill_cam_llskr(long node_id, graph_t *graph){

    long i, switch_id, end, m_paths, m_paths_aux, diam_aux, thw, ths, M, H, EM;
    long *paths, *paths_dists, *links, *links_r;

    switch_id = node_id - nprocs;
    paths = alloc(nswitches * sizeof(long));
    paths_dists = alloc(nswitches * sizeof(long));
    links = alloc(nswitches * sizeof(long));
    links_r = alloc(nswitches * sizeof(long));
    m_paths = cam_policy_params[0];
    thw = cam_policy_params[1];                                                  
    ths = cam_policy_params[2];                                                  
    M = m_paths * (stDown * stDown);
    H = 0;
    EM = 0;

    // Calculate H                                                                 
    for(i = 0; i < nswitches; i++){                                                 
        EM += stUp * pow(stUp - 1, i); 
        if((EM / nswitches) > thw){                                                 
            H = i + 1;                                                             
            break;                                                                 
        }                                                                          
    } 
    path_t **k_paths= alloc(m_paths * sizeof(path_t*));

    diam_aux = find_shortest_path(switch_id, paths, paths_dists, links, links_r, graph);

    if(diam_aux > diameter_t)
        diameter_t = diam_aux;

    for(end = switch_id; end < nswitches; end++){ 
        k_paths[0] = init_path(nswitches);
        m_paths_aux = shortest_path_generic_limited(k_paths, paths, links, links_r, switch_id, end, paths_dists[end], M, m_paths, H, ths, graph);
        set_mpaths(k_paths, m_paths_aux);
        set_mpaths_reverse(k_paths, m_paths_aux);

        for(i = 0; i < m_paths_aux; i++){
            destroy_path(k_paths[i]);
        }
    }

    free(k_paths);
    free(paths);
    free(paths_dists);
    free(links);
    free(links_r);
}

void fill_cam_allpath(long node_id, graph_t *graph){

    long i, switch_id, end, m_paths, m_paths_aux, diam_aux, M, H;
    long *paths, *paths_dists, *links, *links_r;

    switch_id = node_id - nprocs;
    paths = alloc(nswitches * sizeof(long));
    paths_dists = alloc(nswitches * sizeof(long));
    links = alloc(nswitches * sizeof(long));
    links_r = alloc(nswitches * sizeof(long));
    M = cam_policy_params[0];
    H = cam_policy_params[1];
    m_paths = M;

    path_t **k_paths= alloc(m_paths * sizeof(path_t*));

    diam_aux = find_shortest_path(switch_id, paths, paths_dists, links, links_r, graph);

    if(diam_aux > diameter_t)
        diameter_t = diam_aux;

    for(end = switch_id; end < nswitches; end++){ 
        k_paths[0] = init_path(nswitches);
        m_paths_aux = shortest_path_generic_limited(k_paths, paths, links, links_r, switch_id, end, paths_dists[end], M, m_paths, H, 0, graph);
        set_mpaths(k_paths, m_paths_aux);
        set_mpaths_reverse(k_paths, m_paths_aux);

        for(i = 0; i < m_paths_aux; i++){
            destroy_path(k_paths[i]);
        }
    }

    free(k_paths);
    free(paths);
    free(paths_dists);
    free(links);
    free(links_r);
}

void set_paths(path_t *path){

    long i, src, l, dst;

    src = path->nodes[0] + nprocs;
    dst = path->nodes[path->length - 1];

    network[src].cam[dst].ports[0] = alloc((path->length) * sizeof(long)); 
    network[src].cam[dst].ports[0][0] = path->length -1; 
    network[src].cam[dst].n_paths = 1;
    for(i = 1; i < path->length; i++){
        l = path->links[i] + stDown;
        network[src].cam[dst].ports[0][i] = l; // position 0 is for length  
    }
}

void set_paths_reverse(path_t *path){

    long i, src, l, dst;

    src = path->nodes[path->length - 1] + nprocs;
    dst = path->nodes[0];

    network[src].cam[dst].ports[0] = alloc((path->length) * sizeof(long)); 
    network[src].cam[dst].ports[0][0] = path->length -1; 
    network[src].cam[dst].n_paths = 1;
    for(i = path->length - 2; i >= 0; i--){
        l = path->links_r[i + 1] + stDown;
        network[src].cam[dst].ports[0][path->length - 1 - i] = l; // position 0 is for length  
    }
}

void set_mpaths_rr(path_t **path, long n_paths){

    long i, j, src, l, dst;

    src = path[0]->nodes[0] + nprocs;
    dst = path[0]->nodes[path[0]->length - 1];

    network[src].cam[dst].n_paths = n_paths;

    for(j = 0; j < n_paths; j++){

        network[src].cam[dst].ports[j] = alloc((path[j]->length) * sizeof(long)); 
        network[src].cam[dst].ports[j][0] = path[j]->length -1; 

        if((path[j]->length - 1) > diameter_r)
            diameter_r = (path[j]->length - 1);

        for(i = 1; i < path[j]->length; i++){
            l = path[j]->links[i] + stDown;
            network[src].cam[dst].ports[j][i] = l; // position 0 is for length  
        }
    }
}

void set_mpaths_reverse_rr(path_t **path, long n_paths){

    long i, j, src, l, dst;

    src = path[0]->nodes[path[0]->length - 1] + nprocs; // Do it just once
    dst = path[0]->nodes[0];

    network[src].cam[dst].n_paths = n_paths;

    for(j = 0; j < n_paths; j++){

        network[src].cam[dst].ports[j] = alloc((path[j]->length) * sizeof(long)); 
        network[src].cam[dst].ports[j][0] = path[j]->length -1; 

        for(i = path[j]->length - 2; i >= 0; i--){
            l = path[j]->links_r[i + 1] + stDown;
            network[src].cam[dst].ports[j][path[j]->length - 1 - i] = l; // position 0 is for length  
        }
    }
}


void set_mpaths_adaptive(path_t **path, long n_paths){

    long i, j, k, src, l, dst, max_length, dist;


    max_length = path[n_paths - 1]->length;
    dst = path[n_paths - 1]->nodes[max_length - 1];

    for(i = 1; i < max_length; i++){
        for(j = 0; j < n_paths; j++){
            
            if(i == 1 && ((path[j]->length - 1) > diameter_r))
                diameter_r = (path[j]->length - 1);

            if(i < path[j]->length){
                dist = path[j]->length - i;
                src = path[j]->nodes[i - 1] + nprocs; 
                l = path[j]->links[i] + stDown;
                k = 0;

                while(k < stUp){
                    if(network[src].cam[dst].ports[0][k] == l){
                        break;
                    }
                    if(network[src].cam[dst].ports[0][k] == -1){
                        network[src].cam[dst].ports[0][k] = l;
                        network[src].cam[dst].ports[1][k] = dist;
                        network[src].cam[dst].n_paths++; //In adaptive we use this field to know how many ports are used
                        break;
                    }
                    k++;
                }
            }
        }
    }
}

void set_mpaths_reverse_adaptive(path_t **path, long n_paths){
    
    long i, j, k, src, l, dst, max_length, dist;

    max_length = path[n_paths - 1]->length;
    dst = path[n_paths - 1]->nodes[0];

    for(i = 1; i < max_length; i++){
        for(j = 0; j < n_paths; j++){
           if(i < path[j]->length){
                dist = i;
                src = path[j]->nodes[i] + nprocs; 
                l = path[j]->links_r[i] + stDown;
                k = 0;

                while(k < stUp){
                    if(network[src].cam[dst].ports[0][k] == l)
                        break;
                    if(network[src].cam[dst].ports[0][k] == -1){
                        network[src].cam[dst].ports[0][k] = l;
                        network[src].cam[dst].ports[1][k] = dist;
                        network[src].cam[dst].n_paths++; //In adaptive we use this field to know how many ports are used
                        break;
                    }
                    k++;
                }
            }
        }
    }
}

void print_cams(){

    long i, j, k, switch_id;

    for(switch_id = nprocs; switch_id < NUMNODES; switch_id++){
        for(i = 0; i < nswitches; i++){
            printf("SWICTH: %ld --> %ld #ENTRIES: %ld PORTS: \n", switch_id, i, network[switch_id].cam[i].n_paths);
            //for(j = 0; j < network[switch_id].cam[i].n_paths; j++){
            for(j = 0; j < 1; j++){
                if( switch_id != i + nprocs){
                    //for(k = 0; k < network[switch_id].cam[i].ports[j][0]; k++){
                    for(k = 0; k < stUp; k++){
                        //printf("%ld ", network[switch_id].cam[i].ports[j][k + 1]);
                        printf("%ld ", network[switch_id].cam[i].ports[0][k]);
                    }
                    printf("\n");
                        for(k = 0; k < stUp; k++){
                        //printf("%ld ", network[switch_id].cam[i].ports[j][k + 1]);
                        printf("%ld ", network[switch_id].cam[i].ports[1][k]);
                    }

                printf("\n");
                }
            }
        }
    }
}

void print_vector(long *v, long l){

    long i;

    for(i = 0; i < l; i++)
        printf("%ld ", v[i]);
    printf("\n");
}

void finish_cams_rr(){

    long i, j, switch_id;

    for(switch_id = nprocs; switch_id < NUMNODES; switch_id++){
        for(i = 0; i < nswitches; i++){
            for(j = 0; j < network[switch_id].cam[i].n_paths; j++){
                if(switch_id != (i + nprocs)){
                    free(network[switch_id].cam[i].ports[j]);
                }
            }
            free(network[switch_id].cam[i].ports);
        }
        free(network[switch_id].cam);
    }
}

void finish_cams_adaptive(){

    long i, switch_id;

    for(switch_id = nprocs; switch_id < NUMNODES; switch_id++){
        for(i = 0; i < nswitches; i++){
                if(switch_id != (i + nprocs)){
                    free(network[switch_id].cam[i].ports[0]);
                    free(network[switch_id].cam[i].ports[1]);
                }
            free(network[switch_id].cam[i].ports);
        }
        free(network[switch_id].cam);
    }
}



