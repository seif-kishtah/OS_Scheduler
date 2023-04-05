#include "headers.h"
#include "TreeQueue.c"
#include<math.h>

void OccupyChildren(struct Tnode * root)
{
    if (root == NULL)
        return;
    root->occupied = 0;
    OccupyChildren(root->left);
    OccupyChildren(root->right);
}

void FreeChildren(struct Tnode * root)
{
    if (root == NULL)
        return;
    root->occupied = -1;
    FreeChildren(root->left);
    FreeChildren(root->right);
}

int GetOccupied(struct Tnode * root)
{
    if (root == NULL)
        return 0;
        if (root!=NULL)
        {
    if (root->occupied == 0)
        return -1;
    if (GetOccupied(root->left) == -1)
        return -1;
    if (GetOccupied(root->right) == -1)
        return -1;
    }
    return 0;
}


void insert(struct Tnode ** binary_tree, int start, int end, struct Tnode* parent, int counter) {
    struct Tnode* tmp = NULL;
    if(!(*binary_tree)) {
        tmp = (struct Tnode *)malloc(sizeof(struct Tnode));
        tmp->left = tmp->right = NULL;
        tmp->start=start;
        tmp->end=end;
        tmp->parent=parent;
        tmp->occupied=-1;
        *binary_tree = tmp;
        tmp->key=-counter+8;
        //printf("here counter now is %d\n",counter);
        //return;
    }
        if(counter <= 0)
        {
            //printf("end of counter\n");
            return;
        }
        counter--;
        insert(&(*binary_tree)->left, start, start+((end-start)/2), tmp, counter);
        insert(&(*binary_tree)->right, (start+(end-start)/2)+1, end, tmp, counter);
}

struct Tnode* initMemory ()
{
    int size=1024;
    struct Tnode* head=NULL;
    insert(&head,0,1023,NULL,7);
    return head;
}

struct Tnode *Treesearch(struct Tnode ** binary_tree, int start, int end) {
    struct Tnode* leftSearch=NULL;
    struct Tnode* rightSearch=NULL;
    if((*binary_tree)==NULL) {
        //printf("Was NULL\n");
        return NULL;
    }
    if(start == (*binary_tree)->start && end == (*binary_tree)->end) {
        // printf("inside condition %d\n",((*binary_tree)->start));
        // printf("inside condition %d\n",((*binary_tree)->end));
        return *binary_tree;
    }
    // printf("%d\n",((*binary_tree)->start));
    // printf("%d\n",((*binary_tree)->end));

    leftSearch = Treesearch(&((*binary_tree)->left), start, end);
    rightSearch = Treesearch(&((*binary_tree)->right), start, end);

    if (rightSearch)
    {
        return rightSearch;
    }
    else
    {
        return leftSearch; 
    }
}

void display_preorder(struct Tnode * binary_tree) {
    if (binary_tree) {
        printf("%d %d %d\n",binary_tree->start,binary_tree->end,binary_tree->occupied);
        display_preorder(binary_tree->left);
        display_preorder(binary_tree->right);
    }
}

void deallocation(struct Tnode** Head,int start, int end)
{
    struct Tnode *deallocated=Treesearch(Head, start, end);
    deallocated->occupied=-1;
    struct Tnode* parent=deallocated->parent;
    FreeChildren(deallocated);
    while(parent != NULL)
    {
        if(parent->left->occupied==-1 && parent->right->occupied==-1)
        {
            parent->occupied=-1;
            parent=parent->parent;
        }
        else
        {
            break;
        }
    }

    
}

int CalculateAlloctionBegin(int level, int counter){
    int total=2;
    int levelWeight=2;
    for (int i=1; i<level-1; i++){
        total=total*2;
    }
   
    total=total-1;
      //printf ("total %d \n" ,total);
    int CurrentNodeNumber= counter - total;
         //printf ("counter %d \n" ,counter);
         //printf ("CurrentNodeNumber %d \n" ,CurrentNodeNumber);

    for (int i=1; i<11-level; i++){
        levelWeight=levelWeight*2;
    }
        //printf ("levelWeight %d \n" ,levelWeight);

    int BeginAt = levelWeight * (CurrentNodeNumber-1);
    //printf ("BEG AT %d \n" ,BeginAt);
    return BeginAt;
}

struct Tnode *Allocate(struct Tnode ** binary_tree, int memoryamount, int *BeginAt) {

    struct Tnode *temp= NULL;
    struct Tnode *aux= NULL;
    int GivenKey;
    int Nodecounter =0;
    temp = *binary_tree;
    *BeginAt=-1;
    GivenKey = 11- ceil(log2(memoryamount));
    if (GivenKey>=9)
        GivenKey=8;
    //printf ("memamount %d \n",GivenKey);

        if (temp->key==1 && temp->occupied==0){
        Emptyit();
        return NULL;
        }


    while (temp != NULL && temp->key <= GivenKey){


        Nodecounter++; 

        if (temp!=NULL && temp->key==GivenKey && GetOccupied(temp)==0){

             if (temp!=NULL)
            temp->occupied=0;

        if (temp!=NULL)
        OccupyChildren(temp);   

         if (temp!=NULL)
        *BeginAt = CalculateAlloctionBegin(temp->key,Nodecounter);

                 if (temp!=NULL)
                //printf("TEMP KWY %d %d\n", temp->occupied,Nodecounter);

        Emptyit();
        return temp;
        }




        if (temp!=NULL && temp->left!=NULL){
          // printf("MMMMMMM %d %d \n",GivenKey, temp->key); fflush(stdout);

        enqueue(temp->left);
        enqueue(temp->right);
        }

        //printf("temp left %d",temp->left->key); fflush(stdout);

        temp =dequeue();
        //printf("temp left %d",temp->left->key);  fflush(stdout);


    }
     Emptyit();
    return NULL;
}





//  int main()
//  {
//     struct Tnode* root=NULL;
//     struct Tnode* search=NULL;
//     int beginAt;
//     root=initMemory();
//     struct Tnode *trial;

    
    
//     trial = Allocate(&root,256,&beginAt);
//     if (trial != NULL)
//     printf ("ALLOCATTED %d \n",1);
//     else 
//     printf ("NOT ALLOCATTED %d \n",1);


//     trial = Allocate(&root,120,&beginAt);
//     if (trial != NULL)
//     printf ("ALLOCATTED %d \n",2);
//     else 
//     printf ("NOT ALLOCATTED %d \n",2);
 


//     trial = Allocate(&root,100,&beginAt);
//     if (trial != NULL)
//     printf ("ALLOCATTED %d \n",3);
//     else 
//     printf ("NOT ALLOCATTED %d \n",3);
 


//      trial = Allocate(&root,50,&beginAt);
//     if (trial != NULL)
//     printf ("ALLOCATTED %d \n",4);
//     else 
//     printf ("NOT ALLOCATTED %d \n",4);


//      trial = Allocate(&root,40,&beginAt);
//     if (trial != NULL)
//     printf ("ALLOCATTED %d \n",5);
//     else 
//     printf ("NOT ALLOCATTED %d \n",5);


//      trial = Allocate(&root,30,&beginAt);
//     if (trial != NULL)
//     printf ("ALLOCATTED %d \n",6);
//     else 
//     printf ("NOT ALLOCATTED %d \n",6);

//     trial = Allocate(&root,30,&beginAt);
//     if (trial != NULL)
//     printf ("ALLOCATTED %d \n",7);
//     else 
//     printf ("NOT ALLOCATTED %d \n",7);

//     trial = Allocate(&root,30,&beginAt);
//     if (trial != NULL)
//     printf ("ALLOCATTED %d \n",8);
//     else 
//     printf ("NOT ALLOCATTED %d \n",8);

//     trial = Allocate(&root,30,&beginAt);
//     if (trial != NULL)
//     printf ("ALLOCATTED %d \n",9);
//     else 
//     printf ("NOT ALLOCATTED %d \n",9);

//     trial = Allocate(&root,16,&beginAt);
//     if (trial != NULL)
//     printf ("ALLOCATTED %d \n",10);
//     else 
//     printf ("NOT ALLOCATTED %d \n",10);

//     trial = Allocate(&root,10,&beginAt);
//     if (trial != NULL)
//     printf ("ALLOCATTED %d \n",11);
//     else 
//     printf ("NOT ALLOCATTED %d \n",11);

//     trial = Allocate(&root,10,&beginAt);
//     if (trial != NULL)
//     printf ("ALLOCATTED %d \n",12);
//     else 
//     printf ("NOT ALLOCATTED %d \n",12);

//     trial = Allocate(&root,12,&beginAt);
//     if (trial != NULL)
//     printf ("ALLOCATTED %d \n",13);
//     else 
//     printf ("NOT ALLOCATTED %d \n",13);

//     trial = Allocate(&root,15,&beginAt);
//     if (trial != NULL)
//     printf ("ALLOCATTED %d \n",14);
//     else 
//     printf ("NOT ALLOCATTED %d \n",14);

//     trial = Allocate(&root,4,&beginAt);
//     if (trial != NULL)
//     printf ("ALLOCATTED 8 %d \n",15);
//     else
//     printf ("NOT ALLOCATTED %d \n",15);

//     //display_preorder(root);

//     trial = Allocate(&root,3,&beginAt);
//     if (trial != NULL)
//     printf ("ALLOCATTED %d \n",16);
//     else 
//     printf ("NOT ALLOCATTED %d \n",16);

//         trial = Allocate(&root,2,&beginAt);
//     if (trial != NULL)
//     printf ("ALLOCATTED %d \n",17);
//     else 
//     printf ("NOT ALLOCATTED %d \n",17);

//         trial = Allocate(&root,1,&beginAt);
//     if (trial != NULL)
//     printf ("ALLOCATTED %d \n",18);
//     else 
//     printf ("NOT ALLOCATTED %d \n",18);






    //insert(&root,0,1023,NULL,3);
    // display_preorder(root);
    // deallocation(&root,0,31);
    //display_preorder(root);
    // search=Treesearch(&root, 0, 511);
    // printf("%d\n",search->parent==NULL);
    //if(search!=NULL)
    //{
        //printf("%d %d\n",search->start,search->end);
    //}
    //else
    //{
       //printf("Not found\n");
    //}
//}

