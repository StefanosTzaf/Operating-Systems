typedef void* Pointer;

// We use a function pointer for comparing 2 elements, it returns:
// 0 if they are equal based on the function (not necessarily *a == *b,
// we might be dealing with structs so the compare function will be different and this is its value)
// It is used in listFind
typedef int (*CompareFunc)(Pointer a, Pointer b);
typedef void (*DestroyFunc)(Pointer value);

typedef struct list* List;
typedef struct list_node* ListNode;

typedef struct map* Map;
typedef struct map_node* MapNode;

typedef struct set* Set;
typedef struct set_node* SetNode;

int countDigits(int number);