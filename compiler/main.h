#include <stddef.h>

// container.c

void error(char *fmt, ...);

// Reports error at the given location
void error_at(char *loc, char *fmt, ...);

char *read_file(char *path);

typedef struct Vector {
    // pointer to the first data
    // assume void* = 8 bytes
    void** data;
    // current data field size
    int size;
    // actual elements count
    int count;
} Vector;

Vector *new_vector();
int vector_count(Vector*);
void vector_add(Vector*, void*);
void vector_set(Vector*, int, void*);
void *vector_get(Vector*, int);
void *vector_get_last(Vector*);
void vector_delete(Vector*, int);
void vector_free(Vector*);

// main.c

// Given file name
extern char *file_name;
// Whole user input
extern char *user_input;

// parse.c

struct Token;
typedef struct Token Token;

// Current token
extern Token *token;

typedef struct Type Type;

typedef enum {
    VOID, // TODO: properly support void type?
    CHAR,
    INT,
    LONG,
    PTR,
    ARRAY,
    FUNC,
    STRUCT,
} TypeKind;

// Variable type
struct Type {
    TypeKind ty;
    // pointer to / array of what type
    // return type of func
    Type *ptr_to;
    size_t array_size;
    // parameters of func, elt type: Type*
    // parameters of struct, elt type: DefinedType*
    Vector *params;
    // identifier, if type is FUNC or STRUCT
    char *str;
    int len;
};

typedef struct LocalVar LocalVar;

// Local variables
struct LocalVar {
    // variable name
    char *name;
    // variable name length
    int len;
    // offset from rbp
    int offset;
    // type
    Type *type;
};

Token *tokenize(char *p);

// Node kind for building AST (Abstract Syntax Tree)
typedef enum {
    ND_ASSIGN, // =
    ND_EQUAL, // ==
    ND_NOT_EQUAL, // !=
    ND_LESS_EQUAL, // <=
    ND_LESS, // <
    ND_GREATER_EQUAL, // >=
    ND_GREATER, // >
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_ADDR, // &
    ND_DEREF, // *
    ND_LAND, // &&
    ND_LOR, // ||
    ND_LNOT, // !
    ND_FUNC, // function
    ND_FUNC_CALL, // function call
    ND_RETURN, // "return" statement
    ND_IF, // "if" statement
    ND_WHILE, // "while" statement
    ND_FOR, // "for" statement
    ND_BREAK, // "break" statement
    ND_CONTINUE, // "continue" statement
    ND_BLOCK, // "{ ~ }" block statement
    ND_LOCAL_VAR, // Local variable
    ND_GLOBAL_VAR, // Global variable
    ND_STRING, // String literal
    ND_NUM, // Number, int
    ND_CHAR, // Number, char
    ND_ARRAY, // Array initializer for variables: e.g. "{1, 2, foo()}"
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    // Global variable initializer node here if the kind is ND_GLOBAL_VAR
    Node *left;
    Node *right;
    Node *third;
    Node *fourth;
    // Value here if the kind is ND_NUM or ND_CHAR
    long val;
    // Offset and type here if the kind is ND_LOCAL_VAR or ND_GLOBAL_VAR,
    // total local vars offset here if the kind is ND_FUNC
    int offset;
    // function return type if the kind is ND_FUNC
    Type *type;
    // Label name sequencing here if the kind is ND_IF, ND_WHILE, or ND_FOR
    // String literal label name here if the kind is ND_STRING
    int label;
    // Function name if the kind is ND_FUNC_CALL or ND_FUNC
    // Variable name here if the kind is ND_LOCAL_VAR or ND_GLOBAL_VAR
    // String literal here if the kind is ND_STRING
    char *str;
    int len;
    // List of statements if the kind is ND_BLOCK
    // List of function arguments if the kind is ND_FUNC or ND_FUNC_CALL, elements: Node*
    // List of array elements if the kind is ND_ARRAY, elements: Node*
    Vector *arguments;
};

// size_of returns the size of the given type.
size_t size_of(Type *ty);
// type_of returns the type of the given node.
Type *type_of(Node *node);

// List of functions, elements: Node*
extern Vector *functions;

typedef struct GlobalVar GlobalVar;

struct GlobalVar {
    // variable name
    char *name;
    // variable name length
    int len;
    // offset from rbp
    int offset;
    // type
    Type *type;
    // initializer
    Node *init;
};

// Global variables, elements: GlobalVar*
extern Vector *globals;

// String literals
extern Vector *strings;

void program();

// codegen.c

void gen();
