#include "list.h"

void print_list(hashBlock * head) {
    hashBlock * current = head;

    while (current != NULL) {
        printf("Frame is: %d\n", current->frame);
        printf("Page is: %d\n", current->page);
        current = current->next;
    }
}


/* Adding an item to the beginning of the list (pushing to the list) */
void push(hashBlock ** head, int frame, int page) {
    hashBlock * new_node;
    new_node = (hashBlock *) malloc(sizeof(hashBlock));

    new_node->frame = frame;
    new_node->page = page;
    new_node->next = *head;
    *head = new_node;
}



int remove_by_key(hashBlock**head, int key) {

   hashBlock* current = *head;
   hashBlock* previous = NULL;

    if (*head == NULL) {
        return -1;
    }

    while (current->page != key){

        if (current->next == NULL){
            return -1;

        }else{

            previous = current;
            current = current->next;
        }
    }

    if (current == *head){
        *head = (*head)->next;
        free(current);
    }else{
        free(current);
        previous->next = current->next; 
    }

   return 1;
}

bool search_list(hashBlock* head, int page){

    hashBlock* current = head;  // Initialize current 
    while (current != NULL) 
    { 
        if (current->page == page) 
            return true; 
        current = current->next; 
    } 
    return false;

}