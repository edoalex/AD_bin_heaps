#include <binheap.h>
#include <string.h>
#include <stdio.h>

#define PARENT(node) ((node-1)/2)
#define LEFT_CHILD(node) (2*(node)+1)
#define RIGHT_CHILD(node) (2*(node+1))

#define VALID_NODE(H, node) ((H)->num_of_elem>(node))

// returns the address in A of the key of the node given in input
#define ADDR(H, node) ((H)->A+ ((H)->key_pos)[node]*(H)->key_size)
// returns the node corresponding to a given address in A 
// which is not necessarly the index of that key in A
#define INDEX_OF(H, addr) (H)->rev_pos[(((addr)-((H)->A))/(H)->key_size)]

int is_heap_empty(const binheap_type *H)
{
    return H->num_of_elem==0;
}

const void *min_value(const binheap_type *H)
{
    if (is_heap_empty(H)) {
        return NULL;    
    }

    // the min is stored in the root, aka A[0]
    return ADDR(H, 0);
}

void swap_keys(binheap_type *H, unsigned int n_a, unsigned int n_b)
{
	// swap the infos in key_pos
	unsigned int tmp = H->key_pos[n_a];
	H->key_pos[n_a] = H->key_pos[n_b];
	H->key_pos[n_b] = tmp;

	// and then propagate on rev_pos
	H->rev_pos[H->key_pos[n_a]] = n_a;
	H->rev_pos[H->key_pos[n_b]] = n_b;
}


void heapify(binheap_type *H, unsigned int node)
{
    unsigned int dst_node=node, child;

    do {
        node = dst_node;

        // find the minimum among node
        // and its children
        child = RIGHT_CHILD(node);

        if (VALID_NODE(H,child) && 
                H->leq(ADDR(H, child), ADDR(H, dst_node))) {
            dst_node = child;
        }

        child = LEFT_CHILD(node);

        if (VALID_NODE(H,child) && 
                H->leq(ADDR(H, child), ADDR(H, dst_node))) {
            dst_node = child;
        }

        // if the min is not in node
        // swap the keys
        if (dst_node != node) {
            swap_keys(H, dst_node, node);
        }
    } while (dst_node != node);
}

const void *extract_min(binheap_type *H)
{
    if (is_heap_empty(H)) {
        return NULL;
    }

    // swapping the keys among the root (A[0])
    // and the right.most leaf of the last level
    // A[num_of_elem-1]
    swap_keys(H, 0, H->num_of_elem-1);

    // deleting the right-most leaf of the 
    // last level A[num_of_elem - 1]
    H->num_of_elem--; 

    heapify(H,0);

    return ADDR(H, H->num_of_elem);
}

const void *find_the_max(void *A,
                         const unsigned int num_of_elem,  
                         const size_t key_size, 
                         total_order_type leq)
{
    if (num_of_elem==0){
        return NULL;
    }

    const void *max_value = A;

    // for all values in A
    for(const void *addr = A+key_size;
        addr != A + num_of_elem*key_size;
        addr += key_size)
    {
        // if addr > max_value
        if (!leq(addr, max_value)) {
                max_value = addr;
        }
    }

    return max_value;
}

binheap_type *build_heap(void *A, 
                         const unsigned int num_of_elem,
                         const unsigned int max_size,  
                         const size_t key_size, 
                         total_order_type leq)
{
    binheap_type *H = (binheap_type *)malloc(sizeof(binheap_type));
    H->key_pos = (unsigned int *)malloc(sizeof(unsigned int)*max_size); 
    // returns the position in A of the key corresponding to the input
    H->rev_pos = (unsigned int *)malloc(sizeof(unsigned int)*max_size);
    // reversely wrt key_pos, returns the node which position in A array is given
    H->A = A;
    H->num_of_elem = num_of_elem;
    H->max_size = max_size;
    H->key_size = key_size;
    H->leq = leq;
    H->max_order_value = malloc(key_size);

    if (num_of_elem == 0) {
        return H;
    }

    // initialize key_pos and rev_pos in the simplest way
    for(unsigned int i = 0; i<num_of_elem; i++) {
		H->key_pos[i] = i;
		H->rev_pos[i] = i;
    }

    // get the maximum among A[:num_of_elem-1]
    // and store it in max_order_value
    const void *value = find_the_max(A, num_of_elem,
                                    key_size, leq);
    memcpy(H->max_order_value, value, key_size);

    // fix the heap property from the
    // second last level up to the root
    for(unsigned int i=num_of_elem/2; i>0; i--) {
        heapify(H, i);
    }
    heapify(H, 0);

    return H;
}

void delete_heap(binheap_type *H)
{
	free(H->key_pos);
    free(H->rev_pos);
    free(H->max_order_value);
    free(H);
}


const void *decrease_key(binheap_type *H, void *node, const void *value)
{
    unsigned int node_idx = INDEX_OF(H, node);

    // if node doen't belong to H or *value>=*node
    // return NULL
    if (!VALID_NODE(H, node_idx) || !(H->leq(value, node))){
        return NULL;
    }

    memcpy(node, value, H->key_size);

    // to avoid seg. fault when evaluating ADDR of root's parent
    if (node_idx==0) return node;

    unsigned int parent_idx = PARENT(node_idx);
    void *parent = ADDR(H, parent_idx);

    // while node != root and *parent > *node
    while ((node_idx!=0) && (!H->leq(parent, value))) {
        // swap parent and node keys
        swap_keys(H, parent_idx, node_idx);

        // focus on the node's parent
        node = parent;
        node_idx = parent_idx;
        // to avoid seg. fault when evaluating ADDR of root's parent
        if (node_idx==0) return node;

        parent_idx = PARENT(node_idx);
        parent = ADDR(H, parent_idx);
    }
    return node;
}


const void *insert_value(binheap_type *H, const void *value)
{
    // if the heap is already full
    if (H->max_size == H->num_of_elem){
        return NULL;
    }

    // if value > *max_order_value
    if (H->num_of_elem==0 || !H->leq(value, H->max_order_value)){
        memcpy(H->max_order_value, value, H->key_size);
    }

    // update key_pos and rev_pos
    H->key_pos[H->num_of_elem] = H->num_of_elem;
    H->rev_pos[H->num_of_elem] = H->num_of_elem;

    // get the position of the new node
    void *new_node_addr = ADDR(H, H->num_of_elem);  
    memcpy(new_node_addr, H->max_order_value, H->key_size);

    // increase the size of the heap
    H->num_of_elem++;

    // decrease the key of the new node
    return decrease_key(H, new_node_addr, value);
}



void print_heap(const binheap_type *H, 
                void (*key_printer)(const void *value))
{
    unsigned int next_level_node = 1; // store the index of the
                                      // left-most node of the
                                      // next level
    for (unsigned int node = 0; node < H->num_of_elem; node++){
        if (node == next_level_node){
            printf("\n");
            next_level_node = LEFT_CHILD(node);
        } else {
            printf("\t");
        }

        key_printer(ADDR(H, node));
    }
    printf("\n");
}