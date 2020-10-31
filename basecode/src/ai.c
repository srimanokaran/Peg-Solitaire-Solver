#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <assert.h>

#include "ai.h"
#include "utils.h"
#include "hashtable.h"
#include "stack.h"

#define POSSIBLE_MOVES 4



void copy_state(state_t* dst, state_t* src){
  //Copy field
  memcpy( dst->field, src->field, SIZE*SIZE*sizeof(int8_t) );

  dst->cursor = src->cursor;
  dst->selected = src->selected;

}

/**
* Saves the path up to the node as the best solution found so far
*/
void save_solution( node_t* solution_node ){
  node_t* n = solution_node;
  while( n->parent != NULL ){

    copy_state( &(solution[n->depth]), &(n->state) );
    solution_moves[n->depth-1] = n->move;

    n = n->parent;
  }
  solution_size = solution_node->depth;

}


node_t* create_init_node( state_t* init_state ){
  node_t * new_n = (node_t *) malloc(sizeof(node_t));
  new_n->parent = NULL;
  new_n->depth = 0;

  copy_state(&(new_n->state), init_state);
  return new_n;
}

/**
* Apply an action to node n and return a new node resulting from executing the action
*/
node_t* applyAction(node_t* n, position_s* selected_peg, move_t action ){

  node_t *new_node = create_init_node( &n->state );

  new_node->parent = n;


  new_node->move = action;
  new_node->depth = n->depth+1;
  new_node->state.cursor.x = selected_peg->x;
  new_node->state.cursor.y = selected_peg->y;

  execute_move_t( &(new_node->state), selected_peg, action );

  return new_node;
}

/**
* Find a solution path as per algorithm description in the handout
*/

void find_solution( state_t* init_state  ){
  int rem = 0;
  HashTable table;

  // Choose initial capacity of PRIME NUMBER
  // Specify the size of the keys and values you want to store once
  ht_setup(&table, sizeof(int8_t) * SIZE * SIZE, sizeof(int8_t) * SIZE * SIZE, 16769023);

  // Initialize Stack
  initialize_stack();

  //Add the initial node
  node_t* n = create_init_node( init_state );
  node_t* newNode;
  //list used to keep track of nodes
  list_t *list;
  //initialise list
  list = make_empty_list();
  //position of cursor
  position_s *p = (position_s *) malloc(sizeof(position_s));



  list = insert_at_head(list, n);
  stack_push(n);

  rem = num_pegs(&n->state);

  while(!is_stack_empty()){
    n = stack_top();
    stack_pop();
    expanded_nodes++;

    if( num_pegs(&n->state) <rem ){
      save_solution(n);
      rem = num_pegs(&n->state);
    }

    for(int i=0;i<SIZE;i++){
      p->x = i;
      for(int j=0;j<SIZE;j++){
        p->y = j;
        for(int k=0;k<POSSIBLE_MOVES;k++){
          if( can_apply(&n->state, p, k) ){
            newNode = applyAction(n, p, k);
            list = insert_at_head(list, newNode);
            generated_nodes++;

            if(won(&newNode->state)){
              save_solution(newNode);
              rem = num_pegs(&newNode->state);
              //free hash table
              ht_destroy(&table);
              //free all nodes
              free_list(list);
              free(p);
              return;
            }
            if(!ht_contains(&table, &newNode->state.field)){
              ht_insert(&table, &newNode->state.field, &newNode->state.field);
              stack_push(newNode);
            }
          }
        }
      }
    }
    if(expanded_nodes >= budget){
      //free hash table
      ht_destroy(&table);
      //free all nodes
      free_list(list);
      free(p);
      return;
    }
  }
}
