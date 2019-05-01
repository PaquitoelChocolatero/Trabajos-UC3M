#include "../include/concurrency_layer.h"
#include "../lib/parser.c"
#include "../lib/operations_queue.c"

void init_concurrency_mechanisms(){

}

void destroy_concurrency_mechanisms(){

}

void* broker(void * args){
  struct broker_info info;
  args = &info;
  //Definiciones
  char file[256] = args->batch_file;
  struct stock_market * myMarket = args->market;
  struct operations_queue * myQueue = myMarket->stock_operations;
  //Creacion de puntero
  iterator * it = new_iterator(file);
  //Variables de la operacion
  int operation;
  char id; //Accion sobre la que se hace la operacion
  int type; //0->BUY  1->SELL
  int num_shares; //Numero de acciones
  int price; //Precio por accion
  //Lectura reiterada del archivo
  do{
    int next_op = next_operation(it, &id, &type, &num_shares, &price);
    struct operation * op;
    new_operation(op, &id, type, num_shares, price);
    enqueue_operation(myQueue, op);
  }while(operation != -1);
  //Destruccion del puntero
  destroy_iterator(it);
}

void* operation_executer(void * args){

}

void* stats_reader(void * args){

}
