/**
 * Jon Moríñigo Mazo (2023)
 *
 * This file contains all the functions used in the mpa traffic pattern
 */

#include <stdlib.h>
#include <string.h>
#include "globals.h"

appnode createNode();
appnode addapp(appnode head, char* name, long nodes, long *nodelist, long id);
void d_app(long id);
appnode head;
void set_bitmap(long node, long id);
int check_finished_app(long id);
/**
 *Functions to create app node
 * @return appnode
 */
appnode createNode(){
    appnode temp;
    temp = (appnode)malloc(sizeof(struct AppLinkedList));
    temp->next=NULL;
    return temp;
}
/**
 *Function to add app to app linked list with the appnode structure initialized
 * @param head head of app queue
 * @param name name of application
 * @param nodes number of nodes required by application
 * @param nodelist list that contain the node ids
 * @param id Id of app
 * @return head of queue
 */
appnode addapp(appnode head, char* name, long nodes, long *nodelist, long id){
    appnode temp, p;
    temp = createNode();
    temp->id = id; //todo CONSTANT
    temp->filename=strdup(name);
    temp->nodes=nodes;
    temp->running=0;
    temp->node_list = malloc(nodes*sizeof(long));
    temp->node_bmp = calloc(nodes, sizeof(long));
    memcpy(temp->node_list, nodelist, nodes*sizeof(long));
    if(head == NULL){
        head = temp;
    }else{
        p=head;
        while(p->next !=NULL){
            p=p->next;
        }
        p->next = temp;
    }
    return head;
}
/**
 * Function used to delete app from app queue
 * @param id Id of app
 */
void d_app(long id){
    appnode prev;
    appnode actual;
    actual=head;
    int i;
    long temp;


    if(head==NULL){
        panic("appmix: Error deleting node, list is empty!");
    }else{
        while(actual!=NULL){
            if(actual->id==id){
                actual->end_clock=sim_clock;
                printf("App %s with id %lld :Tiempo inicio:%lld, tiempo final: %lld\n",actual->filename, actual->id,actual->ini_clock, actual->end_clock);
                break;
            }else{
                prev=actual;
                actual=actual->next;
            }
        }
        if(actual==NULL){
            panic("appmix: Error deleting app, no app with given id exists!");
        }else{
            for(i=0; i<actual->nodes; i++){

                temp = actual->node_list[i];
                network[temp].source=INDEPENDENT_SOURCE;
                network[temp].appid=0;
                init_event(&network[temp].events);
                init_occur(&network[temp].occurs);
            }
        }
    }
    if(head->id==actual->id){
        head=actual->next;
    }else{
        if (actual->next == NULL) {
            prev->next = NULL;
        } else {
            prev->next = actual->next;
        }
    }
    free(actual);
}
/**
 * Function used to check if all the nodes required by the app are available
 * @param nodelist List of nodes required by the app
 * @param nodes Number of nodes required by the app
 * @return
 */
int check_node_disp(long *nodelist, long nodes){
    int i;
    for (i = 0;  i<nodes ; i++) {
        if(network[nodelist[i]].source==OTHER_SOURCE || network[nodelist[i]].source==FINISHED){
          return 1;
        }
    }
    return 0;
}
/**
 * Function used to set a node of an app as finished using a bitmap to represent it
 * @param node Id of node
 * @param id Id of app
 */
void set_bitmap(long node, long id){
    appnode actual;
    actual = head;
    int i;
    while(actual!=NULL){
        if(actual->id==id){
            break;
        }else{
            actual=actual->next;
        }
    }

    for(i=0; i<actual->nodes; i++){
        if(actual->node_list[i]==node){
            actual->node_bmp[i]=1;
            break;
        }
    }
}

/**
 * Function used to check if an app has finished
 * @param id Id of app
 * @return 1 if app has ended, 0 if not.
 */
int check_finished_app(long id){
    appnode actual;
    actual = head;
    int i;
    while(actual!=NULL){
        if(actual->id==id){
            break;
        }else{
            actual=actual->next;
        }
    }
    for(i=0; i<actual->nodes; i++){
        if(actual->node_bmp[i]==0){
            return 0;
        }
    }
    return 1;
}