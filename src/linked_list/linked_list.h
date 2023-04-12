#include <stddef.h>

struct Node {
    void *value;
    struct Node *next;
};

typedef struct Node Node;

typedef struct {
    size_t size;
    Node *head;
    Node *tail;
} LinkedList;

void append(LinkedList*, void*);

void free_list(LinkedList *);

LinkedList *new_list(void);
