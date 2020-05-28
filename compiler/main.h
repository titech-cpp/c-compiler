struct Token;
typedef struct Token Token;

// Whole user input
extern char *user_input;
// Current token
extern Token *token;

typedef struct LocalVar LocalVar;

// Local variables as linked list
struct LocalVar {
  LocalVar *next;
  // variable name
  char *name;
  // variable name length
  int len;
  // offset from rbp
  int offset;
};

// First local variable
extern LocalVar *locals;

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
    ND_RETURN, // "return" statement
    ND_IF, // "if" statement
    ND_WHILE, // "while" statement
    ND_FOR, // "for" statement
    ND_LOCAL_VAR, // Local variable
    ND_NUM, // Number, node is expected to be leaf if and only if kind == ND_NUM, as of now
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *left;
    Node *right;
    Node *third;
    Node *fourth;
    // Value here if the kind is ND_NUM
    int val;
    // Offset here if the kind is ND_LVAR
    int offset;
    // Label name sequencing here if the kind is ND_IF, ND_WHILE, or ND_FOR
    int label;
};

extern Node *code[100];

void program();

void gen();