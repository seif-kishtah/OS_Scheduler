#include <stdio.h>
#include <stdlib.h>
#include "headers.h"

struct TQnode {
	struct Tnode* TreeNode;
	struct TQnode *next;
};

struct TQnode *front = NULL;
struct TQnode *rear = NULL;

// Enqueue() operation on a queue
void enqueue(struct Tnode* input) {
	struct TQnode *ptr;
	ptr = (struct TQnode *)malloc(sizeof(struct TQnode));
	ptr->TreeNode = input;
	ptr->next = NULL;
	if ((front == NULL) && (rear == NULL)) {
		front = rear = ptr;
	} else {
		rear->next = ptr;
		rear = ptr;
	}
	//printf("Node is Inserted\n\n");
}

// Dequeue() operation on a queue
struct Tnode* dequeue() {
	if (front == NULL) {
		//printf("\nUnderflow\n");
		return NULL;
        }
	 else {
		struct TQnode *temp = front;
		struct Tnode* temp_Treenode = front->TreeNode;
		front = front->next;
		free(temp);
		return temp_Treenode;
	}
}

void  Emptyit(){
    for(int i=0; i<128; i++){
        dequeue();
        rear = NULL;
    }
    

}
