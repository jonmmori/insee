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
#include "ksp_routing.h"
#include "list.h"

path_t *init_path(long max_length){

    path_t *p = alloc(sizeof(path_t));
    p->length = 0; 
    p->next = 0;
    p->nodes = alloc((nswitches) * sizeof(long));
    p->links = alloc((nswitches) * sizeof(long));
    p->links_r = alloc((nswitches) * sizeof(long));

    return(p);
}

void destroy_path(path_t *p){

    free(p->nodes);
    free(p->links);
    free(p->links_r);
    free(p);


}

void print_path(path_t *p){

    long i;

    printf("#######################\n");
    for(i = 0; i < p->length;i++){
        printf("%ld ", p->nodes[i]);
    }
    printf("\n");
    for(i = 0; i < p->length;i++){
        printf("%ld ", p->links[i]);
    }
    printf("\n");

    for(i = 0; i < p->length;i++){
        printf("%ld ", p->links_r[i]);
    }
    printf("\n");
    printf("#######################\n");

}

long find_shortest_path(long switch_src, long *paths, long *paths_dists, long *links, long *links_r, graph_t *graph){

    long i, v, diam_aux;                                                                        
    long *queue;                                                                   
    long *discovered;                                                              
    long *processed;                                                               
    long queue_insert, queue_extract;                                              

    queue = alloc((nswitches + 1) * sizeof(long));                                       
    discovered = alloc((nswitches + 1) * sizeof(long));                                  
    processed = alloc((nswitches + 1) * sizeof(long));                                   

    for(i = 0; i < nswitches; i++) {                                                
        queue[i] = -1;                                                             
        paths[i] = -1;                                                              
        discovered[i] = 0;                                                         
        processed[i] = 0;                                                          
        paths_dists[i] = 0;                                                    
    }                                                                              
    queue_insert = 0;                                                              
    queue_extract = 0;                                                             
    diam_aux = 0;                                                                               
    queue[queue_insert] = switch_src;                                                
    queue_insert++;                                                                
    discovered[switch_src] = 1;                                                      
    paths_dists[switch_src] = 1;                                                 

    while(queue_insert != queue_extract) {                                         
        v = queue[queue_extract];                                                  
        queue_extract++;                                                           
        processed[v] = 1;                                                          
        for(i = 0; i < graph[v].nedges; i++) {                                        
            if(graph[v].edge[i].n_edge != -1 && graph[v].edge[i].active && graph[graph[v].edge[i].n_node].active) {
                if(discovered[graph[v].edge[i].n_node] == 0) {                
                    queue[queue_insert] = graph[v].edge[i].n_node;            
                    queue_insert++;                                                
                    discovered[graph[v].edge[i].n_node] = 1;                  
                    paths_dists[graph[v].edge[i].n_node] = paths_dists[v] + 1;
                    paths[graph[v].edge[i].n_node] = v;                        
                    links_r[graph[v].edge[i].n_node] = graph[v].edge[i].n_edge;
                    links[graph[v].edge[i].n_node] = i;                        
                }                                                                  
            }                                                                      
        }                                                                          
    }                                                                              

    for(i = 0; i < nswitches; i++){
        if((paths_dists[i] - 1) > diam_aux)
            diam_aux = paths_dists[i] - 1;
    }

    free(queue);                                                                   
    free(discovered);                                                              
    free(processed);    
    return(diam_aux);
}

long shortest_path(path_t *l, long *paths, long *links, long *links_r, int start, int end, long length){     

    long done = 0;                                                              
    long current = end;                                                         
    long length_aux = length - 1;                                                   

    if(start == end){                                                           
        l->length = 1;                                                               
        l->nodes[0] = end;                                                             
        done = 1;                                                               
    }                                                                           
    else if(paths[current] != -1){                                              
        l->length = length;                                                      
        while(!done){                                                           
            l->nodes[length_aux] = current;                                            
            l->links[length_aux] = links[current];                                            
            l->links_r[length_aux] = links_r[current];                                            
            current = paths[current];                                           
            length_aux--;                                                       
            if(current == start){                                               
                l->nodes[length_aux] = current;                                        
                done = 1;                                                       
            }                                                                   
        }                                                                       
    }                                                                           
    return(done);                                                               
}  

/**
 * Auxiliar funtion to generate the routing table.
 */

long shortest_path_generic(path_t **k_paths, long *paths, long *links, long *links_r, int start, int end, long length, long K, long ecmp, graph_t *graph){

    long k, i, j, p, eq, spur_node, shortest_path_length, cont, curr_path, n_edges, n_nodes_removed, K_aux;
    // list_t **l_aux;
    path_t *total_path;
    path_t *root_path;
    path_t *spur_path;
    edge_removed_t edges[100];
    long nodes_removed[100];
    long *paths_aux, *paths_aux_dists, *links_aux, *links_r_aux;
    list *B = CreateVoidList();;

    root_path = init_path(nswitches);
    spur_path = init_path(nswitches);

    paths_aux = alloc((nswitches) * sizeof(long));
    paths_aux_dists = alloc((nswitches) * sizeof(long));
    links_aux = alloc((nswitches) * sizeof(long));
    links_r_aux = alloc((nswitches) * sizeof(long));

    curr_path = 0;
    K_aux = K;
    shortest_path(k_paths[0], paths, links, links_r, start, end, length);

    shortest_path_length = k_paths[0]->length;

    if(start == end){
        K_aux=1;
    }

    for(k = 1; k < K_aux; k++){
        for(i = k_paths[k-1]->next; i < k_paths[k-1]->length - 1; i++){
            n_edges = 0;
            n_nodes_removed = 0;
            spur_node = k_paths[k-1]->nodes[i];

            for(j = 0;j <= i;j++){
                root_path->nodes[j] = k_paths[k-1]->nodes[j];
                root_path->links[j] = k_paths[k-1]->links[j];
                root_path->links_r[j] = k_paths[k-1]->links_r[j];
            }
            root_path->length = i + 1;

            for(p = 0; p <= curr_path; p++){

                eq = arrays_eq(root_path, k_paths[p], i);

                if(k_paths[p]->length > i && eq == 1){
                    edges[n_edges].src = k_paths[p]->nodes[i];
                    edges[n_edges].dst = k_paths[p]->nodes[i+1];
                    deactivate_edge(graph,  k_paths[p]->nodes[i], k_paths[p]->nodes[i+1]);
                    n_edges++;
                }
            }
            for(j = 0; j < root_path->length; j++){
                if(root_path->nodes[j] == spur_node)
                    break;
                else{
                    nodes_removed[n_nodes_removed] = root_path->nodes[j];
                    deactivate_node(graph, root_path->nodes[j]);
                    n_nodes_removed++;
                }
            }

            find_shortest_path(spur_node, paths_aux, paths_aux_dists, links_aux, links_r_aux, graph);

            if(shortest_path(spur_path, paths_aux, links_aux, links_r_aux, spur_node, end, paths_aux_dists[end])){
                total_path = init_path(root_path->length + spur_path->length);

                for(j = 0; j < root_path->length - 1; j++){
                    total_path->nodes[j] = root_path->nodes[j];
                    total_path->links[j + 1] = root_path->links[j + 1]; 
                    total_path->links_r[j + 1] = root_path->links_r[j + 1]; 
                }

                for(j = 0; j < spur_path->length - 1; j++){
                    total_path->nodes[root_path->length + j - 1] = spur_path->nodes[j];
                    total_path->links[root_path->length + j] = spur_path->links[j + 1];
                    total_path->links_r[root_path->length + j] = spur_path->links_r[j + 1];
                }
                total_path->nodes[root_path->length + spur_path->length -2] = spur_path->nodes[spur_path->length - 1];
                total_path->length = root_path->length + spur_path->length - 1;

                if(root_path->length - 2 >= 0)
                    total_path->next = root_path->length - 2;
                else
                    total_path->next = 0;

                if(!is_path(B, total_path))
                    AddFirst(B, (void*)total_path);
                else
                    destroy_path(total_path);
            }
            for(j = 0; j < n_edges; j++)
                activate_edge(graph, edges[j].src, edges[j].dst);
            for(j = 0; j < n_nodes_removed; j++)
                activate_node(graph, nodes_removed[j]);
        }
        if(B->emptyList == B_TRUE)
            break;

        cont = extract_shortest(B, &k_paths[k], ecmp, shortest_path_length);

        if(!cont)
            break;
        curr_path++;
    }

    DestroyListData(&B);
    destroy_path(root_path);
    destroy_path(spur_path);
    free(paths_aux);
    free(paths_aux_dists);
    free(links_aux);
    free(links_r_aux);

    return(curr_path + 1);
}


/**
 * Auxiliar funtion to generate the routing table.
 */

long shortest_path_generic_limited(path_t **k_paths, long *paths, long *links, long *links_r, int start, int end, long length, long M, long K, long H, long ths, graph_t *graph){

    long k, i, j, p, eq, spur_node, cont, curr_path, n_edges, n_nodes_removed, K_aux, H_aux;
    path_t *total_path;
    path_t *root_path;
    path_t *spur_path;
    edge_removed_t edges[1000];
    long nodes_removed[1000];
    long *paths_aux, *paths_aux_dists, *links_aux, *links_r_aux;
    list *B = CreateVoidList();;

    root_path = init_path(nswitches);
    spur_path = init_path(nswitches);

    paths_aux = alloc((nswitches) * sizeof(long));
    paths_aux_dists = alloc((nswitches) * sizeof(long));
    links_aux = alloc((nswitches) * sizeof(long));
    links_r_aux = alloc((nswitches) * sizeof(long));

    curr_path = 0;
    K_aux = M;
    shortest_path(k_paths[0], paths, links, links_r, start, end, length);

    H_aux = k_paths[0]->length + H;
    if(start == end){
        K_aux=1;
    }

    k = 1;
    while(k < K_aux){
        for(i = k_paths[k-1]->next; i < k_paths[k-1]->length - 1; i++){
            n_edges = 0;
            n_nodes_removed = 0;
            spur_node = k_paths[k-1]->nodes[i];

            for(j = 0;j <= i;j++){
                root_path->nodes[j] = k_paths[k-1]->nodes[j];
                root_path->links[j] = k_paths[k-1]->links[j];
                root_path->links_r[j] = k_paths[k-1]->links_r[j];
            }
            root_path->length = i + 1;

            for(p = 0; p <= curr_path; p++){

                eq = arrays_eq(root_path, k_paths[p], i);

                if(k_paths[p]->length > i && eq == 1){
                    edges[n_edges].src = k_paths[p]->nodes[i];
                    edges[n_edges].dst = k_paths[p]->nodes[i+1];
                    deactivate_edge(graph,  k_paths[p]->nodes[i], k_paths[p]->nodes[i+1]);
                    n_edges++;
                }
            }
            for(j = 0; j < root_path->length; j++){
                if(root_path->nodes[j] == spur_node)
                    break;
                else{
                    nodes_removed[n_nodes_removed] = root_path->nodes[j];
                    deactivate_node(graph, root_path->nodes[j]);
                    n_nodes_removed++;
                }
            }

            find_shortest_path(spur_node, paths_aux, paths_aux_dists, links_aux, links_r_aux, graph);

            if(shortest_path(spur_path, paths_aux, links_aux, links_r_aux, spur_node, end, paths_aux_dists[end])){
                total_path = init_path(root_path->length + spur_path->length);

                for(j = 0; j < root_path->length - 1; j++){
                    total_path->nodes[j] = root_path->nodes[j];
                    total_path->links[j + 1] = root_path->links[j + 1]; 
                    total_path->links_r[j + 1] = root_path->links_r[j + 1]; 
                }

                for(j = 0; j < spur_path->length - 1; j++){
                    total_path->nodes[root_path->length + j - 1] = spur_path->nodes[j];
                    total_path->links[root_path->length + j] = spur_path->links[j + 1];
                    total_path->links_r[root_path->length + j] = spur_path->links_r[j + 1];
                }
                total_path->nodes[root_path->length + spur_path->length -2] = spur_path->nodes[spur_path->length - 1];
                total_path->length = root_path->length + spur_path->length - 1;

                if(root_path->length - 2 >= 0)
                    total_path->next = root_path->length - 2;
                else
                    total_path->next = 0;

                if(!is_path(B, total_path))
                    AddFirst(B, (void*)total_path);
                else
                    destroy_path(total_path);
            }
            for(j = 0; j < n_edges; j++)
                activate_edge(graph, edges[j].src, edges[j].dst);
            for(j = 0; j < n_nodes_removed; j++)
                activate_node(graph, nodes_removed[j]);
        }
        if(B->emptyList == B_TRUE)
            break;

        cont = extract_shortest_limited(B, &k_paths[k],  k - 1, H_aux, ths);

        if(!cont)
            break;
        else if(cont == 2){               
            K_aux = K;
        }
        curr_path++;
        k++;   
    }

    DestroyListData(&B);
    destroy_path(root_path);
    destroy_path(spur_path);
    free(paths_aux);
    free(paths_aux_dists);
    free(links_aux);
    free(links_r_aux);

    return(curr_path + 1);
}

/**
 * Compares two path until position i.
 *
 * @param a*: Paths to compare
 * @param i: Number of position to compare.
 */
long arrays_eq(path_t *a1, path_t *a2, long i){

    long j;
    long eq = 1;

    if(i <= a1->length && i<=a2->length){
        for(j = 0;j <= i; j++){
            if(a1->nodes[j] != a2->nodes[j]){
                eq = 0;
                break;
            }         
        }
    }
    return(eq);
}

long is_path(list *B, path_t *p){

    long j, ret;
    listElement *ll;
    path_t *p_aux;

    ret = 0;

    ll = B->first;
    while( ll ) {
        for(j = 0; j < p->length; j++){
            p_aux = (path_t*)ll->pointer;
            if(p_aux->nodes[j] != p->nodes[j])
                break;
        }
        if(j == p->length){
            ret = 1;
        }
        ll = ll->next;
    }   
    return(ret);
}

long extract_shortest(list *B, path_t **l_dst, long ecmp, long shortest_path_length){

    long ret = 0;                                                                  
    long min = LONG_MAX;                                                           

    listElement *ll = NULL;
    path_t *path =NULL;
    path_t *path_aux = NULL;

    ll = B->first;

    while(ll){                                                          
        path = (path_t*)ll->pointer;                                             
        if(path->length < min){                                                           
            min = path->length;                                                           
            path_aux = path;                                                            
        }                                                                          
        ll = ll->next;
    }                                                                              
    if(path_aux->length <= shortest_path_length || ecmp == 0 ){                            
        ret = 1;                                                                   
        *l_dst = path_aux;                                                            
        RemoveFromList(B, path_aux);
    }                                                                              
    return(ret);                                                                   
}  

long extract_shortest_limited(list *B, path_t **l_dst, long n_current, long H, long ths){

    long ret = 0;                                                                  
    long min = LONG_MAX;                                                           

    listElement *ll = NULL;
    path_t *path = NULL;
    path_t *path_aux = NULL;

    ll = B->first;

    while(ll){                                                          
        path = (path_t*)ll->pointer;                                             
        if(path->length < min){                                                           
            min = path->length;                                                           
            path_aux = path;                                                            
        }                                                                          
        ll = ll->next;
    }                                                                              
    if(path_aux->length <= H){
        ret = 1;
        *l_dst = path_aux;
        RemoveFromList(B, path_aux);
    }
    else if((path_aux->length > H) && n_current < ths){
        ret = 2;
        *l_dst = path_aux;
        RemoveFromList(B, path_aux);
    }
    else{
        ret = 0;
    }
    return(ret);                                                                   
} 
