#include <stdio.h>
#include <stdlib.h>

//creation of LinkedList
typedef struct Node{
    int data;
    struct Node *next;
}Node;

//Creating linked list
Node*  createnode(int val){
    Node *new_node = (Node*)malloc(sizeof(Node));
    if(new_node == NULL){
        printf("Memory allocation failed\n");
        return NULL;
    }
    new_node->data = val;
    new_node->next = NULL;
    return new_node;
}   

//Insert at the begining of the LL
Node *insert_at_beginning(Node *head, int val){
    Node *new_node = createnode(val);
    new_node->next = head;
    return new_node;
}

//Insert at the end of the LL
Node *insert_at_end(Node *head, int val){
    Node *new_node = createnode(val);
    
    if(new_node == NULL){
        return head;
    }

    Node *temp = head;
    while(temp->next != NULL){
        temp = temp->next;
    }

    temp->next = new_node;
    return head;
}

Node *insert_at_pos(Node *head, int pos, int val){
    // Node *new_node = createnode(val);
    
    if(pos < 0){
        printf("Invalid position\n");
        return head;
    }
    if(pos == 1){
        return insert_at_beginning(head, val);
    }

    Node *new_node = createnode(val);
    Node *temp = head;

    for(int i=1; i < pos -1 && temp != NULL; i++){
        temp = temp -> next;
    }

    if(temp == NULL){
        printf("Position out of range!\n");
        free(new_node);
        return head;
    }

    new_node->next = temp->next;
    temp->next = new_node;
    return head;
}

//Delete Node
Node *delete_node(Node *head, int val){
    
    if(head == NULL) return NULL;

    if(head->data == val){
        Node *temp = head;
        head = head->next;
        free(temp);
        return head; 
    }

    Node *current = head;
    Node *prev = NULL;

    while(current != NULL && current->data != val){
        prev = current;
        current = current->next;
    }

     if (current == NULL) {
        printf("Val not found!\n");
        return head;
    }

    prev->next = current->next;
    free(current);
    return head;    
}

//print list
void print_list(Node *head){
    
    Node *temp = head;
    while(temp != NULL){
        printf("%d -> ", temp->data);
        temp = temp->next;
    }
    printf("NULL\n");
}

//Free memory
void free_list(Node *head){
    Node *temp;
    while(head != NULL){
        temp = head;
        head = head -> next;
        free(temp);
    }
}


int main() {
    Node* head = NULL;
    
    head = insert_at_beginning(head, 10);
    head = insert_at_end(head, 20);
    head = insert_at_end(head, 30);
    head = insert_at_beginning(head, 5);
    head = insert_at_pos(head, 1,1 );
    
    printf("Linked List: ");
    print_list(head);
    
    head = delete_node(head, 20);
    printf("After deleting 20: ");
    print_list(head);
    
    free_list(head);
    return 0;
}



