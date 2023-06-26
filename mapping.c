#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include "globals.h"
#include "misc.h"


long g_nodes;

long *g_sol;

long **g_comm_matrix;
long **g_traffic_matrix;
long *part;
int g_iter;

void gen_topo_matrix2D(long **m){

    long i,j,row_s,column_s,row_d,column_d,resx,resy;

    for(i=0;i<nprocs;i++){
        for(j=0;j<nprocs;j++){
            if(i!=j){
                row_s=i/nodes_y;
                column_s=i%nodes_x;
                row_d=j/nodes_y;
                column_d=j%nodes_x;
                resx=fabs(row_d-row_s);
                resy=fabs(column_d-column_s);
                //printf("%d\n",nodes_x);

                m[i][j]=resx+resy+fabs(resy-resx);
            }
            else{
                m[i][j]=0;
            }
        }

    }
    /*
       for(i=0;i<nprocs;i++){
       for(j=0;j<nprocs;j++){
       printf("%d ",m[i][j]);
       }
       printf("\n");
//printf("%d\n",nodes_y);
}
*/
}

void gen_topo_matrix3D(long **m){

    long i,j,row_s,column_s,row_d,column_d,plane_s,plane_d;
    long resx,resy,resz;

    for(i=0;i<nprocs;i++){
        for(j=0;j<nprocs;j++){
            if(i!=j){

                row_s=(i%(nodes_x*nodes_y))/(nodes_y);
                column_s=(i%(nodes_x*nodes_y))%(nodes_x);
                plane_s=i/(nodes_x*nodes_y);

                row_d=(j%(nodes_x*nodes_y))/(nodes_y);
                column_d=(j%(nodes_x*nodes_y)%nodes_x);
                plane_d=j/(nodes_x*nodes_y);

                resx=fabs(row_d-row_s);
                resy=fabs(column_d-column_s);
                resz=fabs(plane_d-plane_s);
                m[i][j]=resx+resy+resz+fabs(resx-resy)+fabs(resz-resx)+fabs(resy-resz);
            }
            else{
                m[i][j]=0;
            }
        }

    }
}


void gen_comm_matrix(const char *comms, long **comm_matrix){

    FILE *tf;
    long i,j,n1,n2,packet_size;
    char buffer[512];
    char *tok;
    char sep[]=" \t";

    n1 = -1;
    n2 = -1;
    packet_size = 0;

    if((tf = fopen(comms, "r")) == NULL){
        perror("Cannot open trace file");
        exit(-1);
    }

    for(i=0;i<nprocs;i++){
        for(j=0;j<nprocs;j++){
            comm_matrix[i][j]=0;
        }
    }

    while(fgets(buffer, 512, tf) != NULL) {
        tok = strtok( buffer, sep);
        if (strcmp(tok, "-101")==0 || strcmp(tok, "-102")==0) {
            if(strcmp(tok, "-101")==0){
                tok=strtok(NULL, sep);
                n1=atol(tok); // Node to add event
                tok=strtok(NULL, sep); // Task: Not in Use
                tok=strtok(NULL, sep);
                n2=atol(tok);
            }
            /*
               else if(strcmp(tok, "-102")==0){
               tok=strtok(NULL, sep);
               n1=atol(tok); // Node to add event
               tok=strtok(NULL, sep); // Task: Not in Use
               tok=strtok(NULL, sep);
               n2=atol(tok);
        //printf("%d receives from %d \n",n2,n1);
        }
        */
            if (n1!=n2){
                tok=strtok(NULL, sep);
                tok=strtok(NULL, sep);
                tok=strtok(NULL, sep);
                tok=strtok(NULL, sep); //length of message
                packet_size=atol(tok);
                comm_matrix[n1][n2]+=packet_size;

            }
        }

    }
    /*
       for(i=0;i<nprocs;i++){
       for(j=0;j<nprocs;j++){
       printf("%ld ",comm_matrix[i][j]);
       }
       printf("\n");
       }
       */
}

void readMatrix(int **r,int x, int y,const char *filename){

    int i, j, n;
    FILE *fd;

    fd=fopen(filename,"r");
    for(i=0;i<x;i++){
        for(j=0;j<y;j++){
            if((n = fscanf(fd,"%d\t",&r[i][j])) == 0)
                    panic("Error reading mapping matrix from file");
        }
    }
    fclose(fd);
}


void init_structures(long *partition, const char *comm){
    int i;

    part=alloc(sizeof(long)*g_nodes);
    memcpy(part,partition,sizeof(long)*g_nodes);


    g_traffic_matrix=alloc(sizeof(long*)*nprocs);
    for(i=0;i<nprocs;i++)
        g_traffic_matrix[i]=alloc(sizeof(long)*nprocs);
    if(ndim==2)
        gen_topo_matrix2D(g_traffic_matrix);
    else
        gen_topo_matrix3D(g_traffic_matrix);
    //readMatrix((int**)g_traffic_matrix    ,1024,1024,traffic);

    g_comm_matrix=alloc(sizeof(long*)*g_nodes);
    for(i=0;i<g_nodes;i++)
        g_comm_matrix[i]=alloc(sizeof(long)*g_nodes);

    gen_comm_matrix(comm, g_comm_matrix);
    //readMatrix((int**)g_comm_matrix,g_nodes,g_nodes,comm);
}

void random_solution(){
    long i,x,y;

    for(i=0;i<g_nodes;i++){
        g_sol[i]=i;
    }
    for(i=0;i<g_nodes;i++){
        x=rand()%g_nodes;
        y=g_sol[i];
        g_sol[i]=g_sol[x];
        g_sol[x]=y;
    }
}

unsigned long long calc_cost_traffic(long *sol,unsigned long long opt){
    long i,j,pi,pj;
    unsigned long  long a_c;

    a_c=0;
    for(i=0;i<g_nodes;i++){
        pi=sol[i];
        for(j=0;j<g_nodes;j++){
            pj=sol[j];
            //printf("[(%d,%d) %d] ",part[pi],part[pj],g_traffic_matrix[part[pi]][part[pj]]);
            if(i!=j)
                a_c+=g_comm_matrix[i][j]*g_traffic_matrix[pi][pj];
            if(a_c>opt && opt!=0){
                break;
            }
        }
        //	printf("\n");
    }
    return(a_c);
}

unsigned long long calc_cost_traffic_opt(long *sol, long i, long j, unsigned long long opt){
    long f1,f2,f3,f4;
    long pi,pj,pk;
    long pj2;
    unsigned long long v_c=0;
    unsigned long long v_n=0;
    unsigned long long a_c=0;
    int x;

    pi=sol[i];
    pk=sol[j];


    for(x=0;x<g_nodes;x++){
        pj=sol[x];

        if(x==i){
            pj2=pk;
        }
        else if(x==j){
            pj2=pi;
        }
        else{
            pj2=pj;
        }
        f1=((g_comm_matrix[i][x]));
        f2=((g_comm_matrix[j][x]));
        f3=((g_comm_matrix[x][i]));;
        f4=((g_comm_matrix[x][j]));;

        v_c+=(f1*g_traffic_matrix[pi][pj]);
        v_c+=(f2*g_traffic_matrix[pk][pj]);
        v_c+=(f3*g_traffic_matrix[pj][pi]);
        v_c+=(f4*g_traffic_matrix[pj][pk]);

        v_n+=(f1*g_traffic_matrix[pk][pj2]);
        v_n+=(f2*g_traffic_matrix[pi][pj2]);
        v_n+=(f3*g_traffic_matrix[pj2][pk]);
        v_n+=(f4*g_traffic_matrix[pj2][pi]);


    }
    a_c=opt+v_n-v_c;
    return(a_c);
}


unsigned long long opt2_total_t(){
    long sol_part[g_nodes];
    long i,j,h,aux;
    unsigned long long cost,cost_opt,costaux;
    int mejor=1;

    cost_opt=calc_cost_traffic(g_sol,0);
    while(mejor){
        mejor=0;
        j=g_nodes-2;
        memcpy(sol_part,g_sol,sizeof(long)*g_nodes);
        costaux=cost_opt;
        for(h=0;h<g_nodes;h++){
            for(i=0;i<=j;i++){
                //aux=sol_part[j+1];
                //sol_part[j+1]=sol_part[i];
                //sol_part[i]=aux;
                //cost=calc_cost_traffic(sol_part,cost_opt);
                cost=calc_cost_traffic_opt(sol_part,i,j+1,costaux);
                if(cost<cost_opt){
                    //printf("%llu\n",cost);
                    aux=sol_part[j+1];
                    sol_part[j+1]=sol_part[i];
                    sol_part[i]=aux;
                    memcpy(g_sol,sol_part,sizeof(long)*g_nodes);
                    aux=sol_part[j+1];
                    sol_part[j+1]=sol_part[i];
                    sol_part[i]=aux;
                    cost_opt=cost;
                    mejor=1;
                    h=g_nodes;
                    break;
                }
                //aux=sol_part[j+1];
                //sol_part[j+1]=sol_part[i];
                //sol_part[i]=aux;
            }
            j--;
        }
    }
    //printf("%llu\n",cost_opt);
    return(cost_opt);
}


void grasp(int iter,long *g_sol_total){
    unsigned long long loc_opt;
    unsigned long long global_opt=1600000000000LL;
    int k=0;
    while(k<iter) {
        //printf("Generating mapping %d of %d ... \n",k+1,iter);
        random_solution();
        // for(i=0;i<g_nodes;i++)
        //   printf("%d ",g_sol[i]);
        //printf("\n");
        loc_opt=opt2_total_t();
        //for(i=0;i<g_nodes;i++)
        //  printf("%d ",g_sol[i]);
        //printf("\n");
        if(loc_opt<global_opt){
            global_opt=loc_opt;
            memcpy(g_sol_total,g_sol,sizeof(long)*g_nodes);
            //printf("\n Partial Score of the selected mapping: %llu\n",global_opt);
        }
        k++;
    }
    /*
       for(i=0;i<g_nodes;i++)
       printf("%d ",g_sol_total[i]);
       printf("\n");
       printf("\n Score of the selected mapping: %llu\n",global_opt);
       */
}


void mapping_f (long *g_sol_total,long *partition, const char *comms) {
    g_iter=10;
    g_nodes=nprocs;
    init_structures(partition,comms);
    g_sol=alloc(sizeof(long)*g_nodes);
    grasp(g_iter,g_sol_total);
}




