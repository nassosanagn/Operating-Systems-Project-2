#ifndef LIST_H_
#define LIST_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct node{
   int frame;
   int page;
   struct node* next;
}hashBlock;

void print_list(hashBlock * head);
void push(hashBlock ** head, int frame, int page);

int remove_by_key(hashBlock**head,int key) ;
bool search_list(hashBlock* head, int page);

#endif /* LIST_H_ */