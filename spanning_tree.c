#include "globals.h"
#include "spanning_tree.h"

spanning_tree_routing_table_t *s_t_routing_table;

void create_spanning_trees(long n, long nservers){

    long i, rnd_node;

    s_t_routing_table = alloc(sizeof(spanning_tree_routing_table_t));
    s_t_routing_table->s_t_route = alloc(n * sizeof(s_t_route_t));
    s_t_routing_table->n = n;
    s_t_routing_table->l_used = 0;
    s_t_routing_table->n_servers = nservers;

    for(i = 0; i < n; i++) {
        s_t_routing_table->s_t_route[i].path = alloc((NUMNODES - nprocs) * sizeof(long));
        s_t_routing_table->s_t_route[i].link = alloc((NUMNODES - nprocs) * sizeof(long));
        s_t_routing_table->s_t_route[i].link_r = alloc((NUMNODES - nprocs) * sizeof(long));
        rnd_node = rand() % (NUMNODES - nprocs);
        calc_spanning_tree(rnd_node, i, nservers);
    }
}

void destroy_spanning_trees(){

    long i, n;;

    n = s_t_routing_table->n;

    for(i = 0; i < n; i++) {
        free(s_t_routing_table->s_t_route[i].path);
        free(s_t_routing_table->s_t_route[i].link);
        free(s_t_routing_table->s_t_route[i].link_r);
    }
    free(s_t_routing_table->s_t_route);
    free(s_t_routing_table);
}


routing_r spanning_tree_rr(long source, long destination){

    long path_length = 0;
    long start_switch, end_switch;
    long length = NUMNODES - nprocs;
    routing_r res;

    res.rr = alloc(length * sizeof(long));
    start_switch = source / s_t_routing_table->n_servers;
    end_switch = destination / s_t_routing_table->n_servers;

    res.rr[0] = rand() % s_t_routing_table->n;
    if(start_switch != end_switch)
        path_length = calc_spanning_tree_rr(res.rr, start_switch, end_switch, &s_t_routing_table->s_t_route[res.rr[0]]);
    res.rr[1] = 0;
    res.rr[path_length + 2] = destination % s_t_routing_table->n_servers;
    res.size = path_length + 3;

    return(res);
}

void calc_spanning_tree(long switch_src, long s_t_n, long n_servers){

    long i, v, node;                                                                        
    long *queue;                                                                   
    long *discovered;                                                              
    long *processed;                                                               
    long queue_insert, queue_extract;                                              
    long n_switches = NUMNODES - nprocs;

    queue = alloc((n_switches) * sizeof(long));                                       
    discovered = alloc((n_switches) * sizeof(long));                                  
    processed = alloc((n_switches) * sizeof(long));                                   

    s_t_routing_table->s_t_route[s_t_n].root = switch_src;

    for(i = 0; i < n_switches; i++) {                                                
        s_t_routing_table->s_t_route[s_t_n].path[i] = -1;                                                              
        queue[i] = -1;                                                             
        discovered[i] = 0;                                                         
        processed[i] = 0;                                                          
    }                                                                              
    queue_insert = 0;                                                              
    queue_extract = 0;                                                             
    queue[queue_insert] = switch_src;                                                
    queue_insert++;                                                                
    discovered[switch_src] = 1;                                                      

    while(queue_insert != queue_extract) {                                         
        v = queue[queue_extract];                                                  
        queue_extract++;                                                           
        processed[v] = 1;                                                          
        for(i = n_servers; i < radix; i++) {                                        
            if(network[v + nprocs].nbor[i] != -1){
                node = network[v + nprocs].nbor[i];
                if(discovered[node - nprocs] == 0) {                
                    queue[queue_insert] = node - nprocs;            
                    queue_insert++;                                                
                    discovered[node - nprocs] = 1;                  
                    s_t_routing_table->s_t_route[s_t_n].path[node - nprocs] = v;                        
                    s_t_routing_table->s_t_route[s_t_n].link[node - nprocs] = i;                        
                    s_t_routing_table->s_t_route[s_t_n].link_r[node - nprocs] = network[v + nprocs].nborp[i];                        
                }                           
            }                
        }                                                                          
    }                                                                              

    free(queue);                                                                   
    free(discovered);                                                              
    free(processed);    
}

long calc_spanning_tree_rr(long *rr, long start, long end, s_t_route_t *s_t_route){     

    long i, j;
    long length = NUMNODES - nprocs;
    long done_src = 0;                                                              
    long done_dst = 0;                                                              
    long length_src = 1;                                                   
    long length_dst = 1;                                                   
    long *src_aux = alloc(length * sizeof(long)); 
    long *dst_aux = alloc(length * sizeof(long)); 
    long *src_aux_b = alloc(length * sizeof(long)); 
    long *dst_aux_b = alloc(length * sizeof(long)); 


    for(i = 0; i < length; i++){
        src_aux[i] = -1;
        dst_aux[i] = -1;
        src_aux_b[i] = -1;
        dst_aux_b[i] = -1;
    }

    src_aux[0] = start;
    src_aux_b[start] = 0;
    if(s_t_route->path[start] == -1){
        done_src = 1;
    }
    dst_aux[0] = end;
    dst_aux_b[end] = 0;
    if(s_t_route->path[end] == -1){
        done_dst = 1;
    }

    i = 1;

    while(!done_src || !done_dst){                                                           
        if(!done_src){
            src_aux[i] = s_t_route->path[src_aux[i -1]];
            src_aux_b[src_aux[i]] = i;
            length_src++;
            if(s_t_route->path[src_aux[i]] == -1){                                               
                done_src = 1;                                                       
            }                                                                   
        }                 
        if(!done_dst){
            dst_aux[i] = s_t_route->path[dst_aux[i -1]];
            dst_aux_b[dst_aux[i]] = i;
            length_dst++;
            if(s_t_route->path[dst_aux[i]] == -1){                                               
                done_dst = 1;                                                       
            }                                                                   
        } 
        i++;
    }            

    i = 0;
    while(i < length_src){
        if(dst_aux_b[src_aux[i]] != -1){
            break;
        }
        rr[i + 2] = s_t_route->link_r[src_aux[i]];
        i++;
    }

    j = dst_aux_b[src_aux[i]] - 1;

    while(j >= 0){
        rr[i + 2] = s_t_route->link[dst_aux[j]];
        i++;
        j--;
    }

    free(src_aux);
    free(src_aux_b);
    free(dst_aux);
    free(dst_aux_b);

    return(i);
}  


