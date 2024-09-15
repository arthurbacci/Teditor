#include "ted.h"

Node *allocate_node(Node n) {
    Node *r = malloc(sizeof(n));
    memcpy(r, &n, sizeof(n));
    return r;
}

void deallocate_node(Node *n) {
    free(n);
}

Node *single_buffer(Buffer b) {
    Node n = {b, NULL, NULL};
    Node *r = allocate_node(n);
    r->next = r;
    r->prev = r;
    return r;
}

void buffer_add_next(Node *n, Buffer b) {
    Node new_node = {b, n->next, n};
    n->next->prev = allocate_node(new_node);
    n->next = n->next->prev;
}

void buffer_add_prev(Node *n, Buffer b) {
    buffer_add_next(n->prev, b);
}

void buffer_close(Node *n) {
    if (n->next == n) {
        if (is_jmp_set)
            longjmp(end, TED_LONGJMP_USER_EXIT);
        else
            exit(EXIT_SUCCESS);
    }

    n->prev->next = n->next;
    n->next->prev = n->prev;

    free_buffer(&n->data);

    free(n);
}

void free_buffer_list(Node *n) {
    Node *p = n;
    do {
        Node *next = p->next;
        free_buffer(&p->data);
        free(p);
        p = next;
    } while (p != n);
}
