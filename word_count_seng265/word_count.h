#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>


// Define error codes

#define BAD_ARGS 1
#define BAD_INFILE 2
#define BAD_ALLOCATION 3

//bool values
#define FALSE 0
#define TRUE 1

//type definitions

//node for a doubly-linked word list
//maybe include pointer to head in each node?
typedef struct word_node{
    struct word_node* next;
    struct word_node* prev;
    char* word;
} word_node;

//node for holding a count. Contains a pointer to 1st node of the associated word list
typedef struct count_node{
    word_node* word_list_front;
    word_node* word_list_back;
    struct count_node* next;
    int count;
    int word_len;
} count_node;

//simply a pointer to the first node in a count_list
typedef struct word_histogram{
    count_node* count_list;
} word_histogram;


//function declarations

word_node* new_word_node(char* word);
count_node* new_count_node(int word_len);

void free_histogram(word_histogram* histogram);
void free_count_list(count_node* count_list);
void free_count_node(count_node* node);
void free_word_list(word_node* word_list);
void free_word_node(word_node* node);

word_histogram* new_word_histogram();
void tokenize_line(char* line, word_histogram* histogram);
void place(char* word, word_histogram* histogram);
count_node* add_count_node_front(word_histogram* histogram, count_node* count_curr, int length);
count_node* add_count_node(count_node* count_curr, int length);
void place_word(word_histogram* histogram,count_node* count_curr, char* word);
void print_histogram(word_histogram* histogram, int sort);
void *emalloc(size_t n);

//functions to free memory allocated for a histogram:
void free_histogram(word_histogram* histogram){

    if (histogram->count_list != NULL){
        free_count_list(histogram->count_list);
    }
    free(histogram);
}

void free_count_list(count_node* count_list){
   count_node* temp = NULL;
   if (count_list->next == NULL){
       //end of list
       free_count_node(count_list);

   }
   else {
       temp = count_list;
       count_list = count_list->next;
       temp->next = NULL;
       free_count_node(temp);
       free_count_list(count_list);
   }
}

void free_count_node(count_node* node){
    if (node->word_list_front!=NULL){
        
        free_word_list(node->word_list_front);
    }
    node->word_list_back = NULL;
    free(node);
}

void free_word_list(word_node* word_list){
    word_node* temp = NULL;
    if (word_list->next == NULL){
        //end of list
        free_word_node(word_list);
    }
    else {
        temp = word_list;
        word_list = word_list->next;
        word_list->prev = NULL;
        temp->next = NULL;
        temp->prev = NULL;
        free_word_node(temp);
        free_word_list(word_list);
    }
}

void free_word_node(word_node* node){
    node->next = NULL;
    node->prev = NULL;
    free(node->word);
    free(node);
}
//end of freeing functions

//constructors for struct types
word_node* new_word_node(char* word){
    word_node* node = (word_node*) emalloc(sizeof(word_node));
    node->next = NULL;
    node->prev = NULL;
    node->word = word;
    return node;
}

count_node* new_count_node(int word_len){
    count_node* node = (count_node*) emalloc(sizeof(count_node));
    node->next = NULL;
    node->word_list_front = NULL;
    node->word_list_back = NULL;
    node->word_len = word_len;
    node->count = 0;
    return node;
}

word_histogram* new_word_histogram(){
    word_histogram* histogram = (word_histogram*) emalloc(sizeof(word_histogram));
    histogram->count_list = NULL;
    return histogram;
}
//end of constructors


//tokenize a line from a file, calls place to place each token into the word histogram
void tokenize_line(char* line, word_histogram* histogram){
    char* token;
    char* word;
    char* delims = " .,;()\n\r";
    token = strtok(line,delims);
    while(token){
        word = (char*) emalloc((strlen(token)+1)*sizeof(char));
        
        strncpy(word,token,strlen(token)+1);
        place(word,histogram);
        token = strtok(NULL,delims);
    }
}


//places a word into its proper spot in the word histogram.
//calls add_count_node_front or add_count_node to create a new count_node if necessary
//and calls place_word to place the word in its correct word list.
void place(char* word, word_histogram* histogram){
    int length = strlen(word);
    count_node* count_curr;
    //empty histogram
    if (histogram->count_list == NULL){
        count_node* new_node = new_count_node(length);
        histogram->count_list = new_node;
        place_word(histogram,new_node,word);
    }
    //nonempty histogram
    else {
        count_curr = histogram->count_list;
        while(count_curr != NULL){
            if (count_curr->word_len > length){
                //insert new node at front
                count_curr = add_count_node_front(histogram,count_curr,length);
                place_word(histogram,count_curr,word);
                return;
            }
            else if (count_curr->next == NULL || count_curr->next->word_len > length){ 
                //node already exists, place word into existing list
                if (count_curr->word_len == length){
                    place_word(histogram,count_curr,word);
                    return;
                }
                //node does not exist, insert new node, then add word to new list
                else {
                    count_curr = add_count_node(count_curr,length);
                    place_word(histogram,count_curr,word);
                    return;
                }
            }
            count_curr = count_curr->next;
        }
    }
}


//creates a new count node and adds it to the front of a count list pointed to by histogram.
count_node* add_count_node_front(word_histogram* histogram, count_node* count_curr, int length){
    count_node* new_node = new_count_node(length);
    histogram->count_list = new_node;
    new_node->next = count_curr;
    return new_node;
}

//creates a new count node and adds it to an existing count list after the node count_curr
count_node* add_count_node(count_node* count_curr, int length){
    count_node* new_node = new_count_node(length);
//    new_node->next = count_curr;
    if (count_curr->next != NULL){
        new_node->next = count_curr->next;
    }
    else new_node->next = NULL;
    count_curr->next = new_node;
    return new_node;
}


//places a word into its correct alphabetical spot in the word list pointed to by count_curr.
void place_word(word_histogram* histogram,count_node* count_curr, char* word){
    count_curr->count = count_curr->count+1;
    //create new node for new word
    word_node* new_node = new_word_node(word);
    word_node* word_curr;

    //if the word list is empty:
    if (count_curr->word_list_front == NULL){
        count_curr->word_list_front = new_node;
        count_curr->word_list_back = new_node;
    }
    //word list not empty
    else {
        word_curr = count_curr->word_list_front;
        //insert at front
        if(strcmp(word_curr->word,word)>0){
            count_curr->word_list_front = new_node;
            word_curr->prev = new_node;
            new_node->next = word_curr;
        }
        //insert not at front
        else {
            //find spot
            while(word_curr->next != NULL && strcmp(word_curr->word,word) < 0){
                word_curr = word_curr->next;
            }
            //word is already in the list. Delete the node we have created.
            if (strcmp(word_curr->word,word)==0){
                free_word_node(new_node);
                return;
            }
            //insert word before current word
            else if (strcmp(word_curr->word,word)>0){
                new_node->prev = word_curr->prev;
                new_node->next = word_curr;
                word_curr->prev->next = new_node;
                word_curr->prev = new_node;
            }
            //insert word after last node
            else { //while loop stopped by null: word should be inserted after last node
                count_curr->word_list_back = new_node;
                word_curr->next = new_node;
                new_node->prev = word_curr;
            }
        }
    }
    return;
}


//Function to print a word_histogram.
//Takes the parameter sort to determine the direction of printing the word lists.
void print_histogram(word_histogram* histogram, int sort){
    //check if histogram is empty
    if (histogram->count_list == NULL) {
        return;
    }

    //pointers for traversing the lists
    count_node* count_curr = NULL;
    word_node* word_curr = NULL;

    //traverse through count list
    count_curr = histogram->count_list;
    while (count_curr != NULL){ 
        printf("Count[%02d]=%02d; (words: ",count_curr->word_len,count_curr->count);

        if (sort) { //traverse backward 
        word_curr = count_curr->word_list_back; 
            while (word_curr->prev!= NULL){
                if (word_curr->prev->prev==NULL){
                    printf("\"%s\" and ",word_curr->word);
                }
                else {
                    printf("\"%s\", ",word_curr->word);
                }

                word_curr = word_curr->prev;
            }
            printf("\"%s\")\n",word_curr->word);
            
        }
        else { //traverse forward 
        word_curr = count_curr->word_list_front; 
            while (word_curr->next!= NULL){
                if (word_curr->next->next==NULL){
                    printf("\"%s\" and ",word_curr->word);
                }
                else {
                    printf("\"%s\", ",word_curr->word);
                }

                word_curr = word_curr->next;
            }
            printf("\"%s\")\n",word_curr->word);
            
        }
        if(count_curr->next !=NULL) count_curr = count_curr->next;
        else break;
    }
    return;
}



// Dynamically allocate memory of size_t n and return the 
// pointer to the memory address
//
// exit with return code BAD_ALLOCATION on allocation error 
//
void *emalloc(size_t n) {
    void *p;

    p = malloc(n);
    if (p == NULL) {
        fprintf(stderr, "malloc of %zu bytes failed", n);
        exit(BAD_ALLOCATION);
    }
    return p;
}





