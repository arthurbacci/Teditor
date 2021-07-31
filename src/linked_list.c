#include "ted.h"

Node *allocate_node(Node n) {
    Node *r = malloc(sizeof(n));
    memcpy(r, &n, sizeof(n));
    return r;
}

void deallocate_node(Node *n) {
    free(n);
}

