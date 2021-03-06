#include "main.h"

#include <stdio.h>

// register names
char arguments[6][4] = {
    "rdi", "rsi", "rdx", "rcx", "r8", "r9",
};
char arguments_32[6][4] = {
    "edi", "esi", "edx", "ecx", "r8d", "r9d",
};
char arguments_8[6][4] = {
    "dil", "sil", "dl", "cl", "r8b", "r9b",
};

void gen_tree();

// gen_lvalue evaluates the next lvalue (prints error and exists if not), and pushes the address to the stack.
void gen_lvalue(Node *node) {
    switch(node->kind) {
    case ND_LOCAL_VAR:
        // calculate the variable address
        printf("        mov rax, rbp\n");
        printf("        sub rax, %d\n", node->offset);
        printf("        push rax\n");
        return;
    case ND_GLOBAL_VAR:
        printf("        lea rax, %.*s[rip]\n", node->len, node->str);
        printf("        push rax\n");
        return;
    case ND_DEREF:
        // assigning to de-referenced values, e.g. *p = 5;
        gen_tree(node->left);
        return;
    }

    error("expected lvalue, but got node kind %s", node->kind);
}

// load_from_rax_to_rax prints out the assembly "mov rax, [rax]",
// considering the given value size in bytes of the address.
void load_from_rax_to_rax(size_t size) {
    switch (size) {
    case 1:
        printf("        movsx eax, BYTE PTR [rax]\n");
        break;
    case 4:
        // sign extension from double word [rax] to quad word rax
        // since the arithmetic operations are based on 64-bit
        printf("        movsxd rax, DWORD PTR [rax]\n");
        break;
    default:
        printf("        mov rax, [rax]\n");
        break;
    }
}

// multiply_ptr_value multiplies "rdi" register by the type that pointers point to, if the given node represents a pointer.
// e.g. if the node represents a local variable of type int *, then multiply "rdi" by 4.
void multiply_ptr_value(Node *node) {
    Type *type = type_of(node);
    // HACK: ignore pointer to a struct?
    if ((type->ty == PTR && type->ptr_to->ty == STRUCT)) return;
    // left value is a pointer
    if ((type->ty == PTR || type->ty == ARRAY) && type->ptr_to) {
        // which type this pointer points to / this array is composed of
        size_t ptr_to_size = size_of(type->ptr_to);
        if (ptr_to_size == 1) return;
        printf("        imul rdi, %ld\n", ptr_to_size);
    }
}

void _gen_tree(Node *node);

// Current gen_tree call stack: elt: Node*
Vector *gen_tree_stack;

// get_last_loop searches the gen_tree_stack (call stack of gen_tree), and returns the last loop node it found.
// Returns NULL otherwise.
Node *get_last_loop() {
    for (int i = vector_count(gen_tree_stack) - 1; i >= 0; i--) {
        Node *node = (Node*) vector_get(gen_tree_stack, i);
        switch (node->kind) {
        case ND_WHILE:
        case ND_FOR:
            return node;
        }
    }
    return NULL;
}

// separating actual implementation for defer; to search current call stack for "break;" and "continue;"
void gen_tree(Node *node) {
    int count = vector_count(gen_tree_stack);
    vector_add(gen_tree_stack, node);
    _gen_tree(node);
    vector_delete(gen_tree_stack, count);
}

// gen_tree walks the given tree and prints out the assembly calculating the given tree.
void _gen_tree(Node *node) {
    switch (node->kind) {
    case ND_NUM:
        if (size_of(type_of(node)) == 8) {
            printf("        mov rax, %ld\n", node->val);
            printf("        push rax\n");
        } else {
            printf("        push %d\n", (int) node->val);
        }
        return;
    case ND_CHAR:
        printf("        push %d\n", (char) node->val);
        return;
    case ND_STRING:
        printf("        lea rax, .LC%d[rip]\n", node->label);
        printf("        push rax\n");
        return;
    case ND_LOCAL_VAR:
    case ND_GLOBAL_VAR:
        // evaluate the variable
        gen_lvalue(node);

        // If the local var type is an array, leave it as an address
        // i.e. implicit conversion of array to pointer to its first member
        if (node->type && (node->type->ty == ARRAY || node->type->ty == STRUCT)) {
            return;
        }

        printf("        pop rax\n");
        // Load value in the address considering the value size
        load_from_rax_to_rax(size_of(node->type));
        printf("        push rax\n");
        return;
    case ND_ASSIGN:
        gen_lvalue(node->left);
        gen_tree(node->right);

        printf("        pop rdi\n");
        printf("        pop rax\n");
        // Assign value to the address considering the value size
        switch (size_of(type_of(node->left))) {
        case 1:
            printf("        mov [rax], dil\n");
            break;
        case 4:
            printf("        mov [rax], edi\n");
            break;
        default:
            printf("        mov [rax], rdi\n");
            break;
        }
        printf("        push rdi\n");
        return;
    case ND_RETURN:
        gen_tree(node->left);

        // Pop the result to rax
        printf("        pop rax\n");
        // Function epilogue
        printf("        mov rsp, rbp\n");
        printf("        pop rbp\n");
        printf("        ret\n");
        return;
    case ND_ADDR:
        gen_lvalue(node->left);
        return;
    case ND_DEREF:
        gen_tree(node->left);

        printf("        pop rax\n");
        // Load value in the address considering the value size
        Type *ty = type_of(node->left);
        // If the dereference is to an array, implicitly convert it to a pointer
        if (ty->ty == PTR || ty->ty == ARRAY) {
            ty = ty->ptr_to;
        }
        // HACK: ignore pointer to struct
        if (ty->ty == STRUCT) {
            printf("        push rax\n");
            return;
        }
        // load from address only if this is not multi-dimensional array, which is linearly on stack
        if (ty->ty != ARRAY) {
            load_from_rax_to_rax(size_of(ty));
        }
        printf("        push rax\n");
        return;
    case ND_LAND:
        gen_tree(node->left);
        printf("        pop rax\n");
        printf("        cmp rax, 0\n");
        // Minimal evaluation
        printf("        je .Lend%d\n", node->label);

        gen_tree(node->right);
        printf("        pop rax\n");
        printf("        cmp rax, 0\n");
        printf("        je .Lend%d\n", node->label);

        printf(".Lend%d:\n", node->label);
        printf("        setne al\n");
        printf("        movzb rax, al\n");
        printf("        push rax\n");
        return;
    case ND_LOR:
        gen_tree(node->left);
        printf("        pop rax\n");
        printf("        cmp rax, 0\n");
        // Minimal evaluation
        printf("        jne .Lend%d\n", node->label);

        gen_tree(node->right);
        printf("        pop rax\n");
        printf("        cmp rax, 0\n");
        printf("        jne .Lend%d\n", node->label);

        printf(".Lend%d:\n", node->label);
        printf("        setne al\n");
        printf("        movzb rax, al\n");
        printf("        push rax\n");
        return;
    case ND_LNOT:
        gen_tree(node->left);
        printf("        pop rax\n");
        printf("        cmp rax, 0\n");
        // If equals to 0, set 1, otherwise, set 0
        printf("        cmp rax, 0\n");
        printf("        sete al\n");
        printf("        movzb rax, al\n");
        printf("        push rax\n");
        return;
    case ND_IF:
        gen_tree(node->left);

        printf("        pop rax\n");
        printf("        cmp rax, 0\n");
        // If the evaluated condition is 0 (false),
        if (node->third) {
            // Jump to "else" block
            printf("        je .Lelse%d\n", node->label + 1);
        } else {
            // No "else" block, so jump to outside of the "if" statement
            printf("        je .Lend%d\n", node->label);
        }
        // otherwise, evaluate inside "if"
        gen_tree(node->right);
        // pop the result so it doesn't stay on stack
        printf("        pop rax\n");
        if (node->third) {
            // and go to end (if else block exists)
            printf("        jmp .Lend%d\n", node->label);
            // generate "else" block
            printf(".Lelse%d:\n", node->label + 1);
            gen_tree(node->third);
            // pop the result so it doesn't stay on stack
            printf("        pop rax\n");
        }
        // end block (next code block)
        printf(".Lend%d:\n", node->label);
        // leave something on stack (gen func expects each gen_tree to generate a value)
        printf("        push 0\n");
        return;
    case ND_WHILE:
        printf(".Lbegin%d:\n", node->label);

        gen_tree(node->left);
        printf("        pop rax\n");
        printf("        cmp rax, 0\n");
        // If the evaluated condition is false, break the loop
        printf("        je .Lend%d\n", node->label + 1);
        // otherwise, evaluate inside "while"
        gen_tree(node->right);
        // pop the result so it doesn't stay on stack
        printf("        pop rax\n");
        // then return back to the beginning
        printf("        jmp .Lbegin%d\n", node->label);

        // end block (next code block)
        printf(".Lend%d:\n", node->label + 1);
        // leave something on stack (gen func expects each gen_tree to generate a value)
        printf("        push 0\n");
        return;
    case ND_FOR:
        // init
        if (node->left) {
            gen_tree(node->left);
            // pop the result so it doesn't stay on stack
            printf("        pop rax\n");
        }
        // "for" block
        printf(".Lbegin%d:\n", node->label);
        if (node->right) {
            gen_tree(node->right);
        } else {
            printf("        push 1\n");
        }
        printf("        pop rax\n");
        printf("        cmp rax, 0\n");
        // If the evaluated condition is false, break the loop
        printf("        je .Lend%d\n", node->label + 1);
        // otherwise, evaluate inside "for"
        gen_tree(node->fourth);
        // pop the result so it doesn't stay on stack
        printf("        pop rax\n");
        // on continue
        printf(".Lcont%d:\n", node->label + 2);
        if (node->third) {
            gen_tree(node->third);
            // pop the result so it doesn't stay on stack
            printf("        pop rax\n");
        }
        // then return back to the beginning
        printf("        jmp .Lbegin%d\n", node->label);

        // end block (next code block)
        printf(".Lend%d:\n", node->label + 1);
        // leave something on stack (gen func expects each gen_tree to generate a value)
        printf("        push 0\n");
        return;
    case ND_BREAK: ;
        Node *loop = get_last_loop();
        if (loop == NULL) {
            error_at(node->str, "break outside of a loop");
        }
        switch (loop->kind) {
        case ND_WHILE:
            printf("        jmp .Lend%d\n", loop->label + 1);
            break;
        case ND_FOR:
            printf("        jmp .Lend%d\n", loop->label + 1);
            break;
        default:
            error_at(node->str, "unknown loop?");
        }
        return;
    case ND_CONTINUE:
        loop = get_last_loop();
        if (loop == NULL) {
            error_at(node->str, "continue outside of a loop");
        }
        switch (loop->kind) {
        case ND_WHILE:
            printf("        jmp .Lbegin%d\n", loop->label);
            break;
        case ND_FOR:
            printf("        jmp .Lcont%d\n", loop->label + 2);
            break;
        default:
            error_at(node->str, "unknown loop?");
        }
        return;
    case ND_BLOCK:
        for (int i = 0; i < vector_count(node->arguments); i++) {
            Node *next = (Node*) vector_get(node->arguments, i);
            gen_tree(next);
            // pop the result so it doesn't stay on stack
            printf("        pop rax\n");
        }
        printf("        push rax\n");
        return;
    case ND_FUNC_CALL: ;
        // Evaluate arguments
        for (int i = 0; i < vector_count(node->arguments); i++) {
            gen_tree((Node*) vector_get(node->arguments, i));
        }
        // Pop evaluated result into registers, max of 6 results
        for (int i = vector_count(node->arguments) - 1; i >= 0; i--) {
            if (i < 6) {
                printf("        pop %s\n", arguments[i]);
            } else {
                printf("        pop rax\n");
            }
        }
        // 16-byte align rsp
        // 1. push the original rsp two times (which pushes rsp by 16 bytes)
        printf("        push rsp\n");
        printf("        push [rsp]\n");
        // 2. 16-byte align rsp, possibly subtracting 8 bytes.
        printf("        and rsp, -0x10\n");
        // 3. Call function
        printf("        call %.*s\n", node->len, node->str);
        // 4. bring back the original rsp, which is always at [rsp + 8]
        printf("        add rsp, 8\n");
        printf("        mov rsp, [rsp]\n");
        // push the result to stack
        printf("        push rax\n");
        return;
    case ND_FUNC:
        // Function name
        printf("%.*s:\n", node->len, node->str);
        // Function Prologue
        printf("        push rbp\n");
        printf("        mov rbp, rsp\n");
        // allocate local variables
        if (node->offset > 0) {
            // 8-byte align
            int offset = ((node->offset - 1) / 8 + 1) * 8;
            printf("        sub rsp, %d\n", offset);
        }

        // Copy function arguments from registers to stack
        for (int i = 0; i < vector_count(node->arguments); i++) {
            // evaluate address of the local variable in stack
            Node *arg = (Node*) vector_get(node->arguments, i);
            gen_lvalue(arg);
            printf("        pop rax\n");
            // support up to 6 arguments to load from registers
            if (i < 6) {
                // check the argument size in bytes
                switch (size_of(arg->type)) {
                case 1:
                    printf("        mov [rax], %s\n", arguments_8[i]);
                    break;
                case 4:
                    printf("        mov [rax], %s\n", arguments_32[i]);
                    break;
                default:
                    printf("        mov [rax], %s\n", arguments[i]);
                }
            }
        }

        // Function body
        gen_tree(node->left);
        printf("        pop rax\n");

        // Function Epilogue
        printf("        mov rsp, rbp\n");
        printf("        pop rbp\n");
        printf("        ret\n");
        return;
    case ND_ARRAY:
        error("got node array\n");
    }

    // Calculate children and push them onto the 'rsp', register stack pointer.
    gen_tree(node->left);
    gen_tree(node->right);

    // Pop the first result into rax and the second result into rax.
    printf("        pop rdi\n");
    printf("        pop rax\n");

    // Perform the operation
    switch (node->kind) {
    // Basic arithmetic operations
    case ND_ADD:
        multiply_ptr_value(node->left);
        printf("        add rax, rdi\n");
        break;
    case ND_SUB:
        multiply_ptr_value(node->left);
        printf("        sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("        imul rax, rdi\n");
        break;
    case ND_DIV:
        // cqo expands 64-bit rax into 128-bit rdx, rax
        printf("        cqo\n");
        // idiv divides 128-bit rdx, rax by the 64-bit given register (rdi here),
        // and sets the quotient to rax and remainder to rdx.
        printf("        idiv rdi\n");
        break;
    // Comparison operations
    case ND_EQUAL:
        // Compare
        printf("        cmp rax, rdi\n");
        // Set compare result to al (lower 8-bit of rax register)
        printf("        sete al\n");
        // Set rax register from al with zero-extension
        printf("        movzb rax, al\n");
        break;
    case ND_NOT_EQUAL:
        printf("        cmp rax, rdi\n");
        printf("        setne al\n");
        printf("        movzb rax, al\n");
        break;
    case ND_LESS:
        printf("        cmp rax, rdi\n");
        printf("        setl al\n");
        printf("        movzb rax, al\n");
        break;
    case ND_LESS_EQUAL:
        printf("        cmp rax, rdi\n");
        printf("        setle al\n");
        printf("        movzb rax, al\n");
        break;
    case ND_GREATER:
        // opposite of less equal
        printf("        cmp rdi, rax\n");
        printf("        setle al\n");
        printf("        movzb rax, al\n");
        break;
    case ND_GREATER_EQUAL:
        // opposite of less
        printf("        cmp rdi, rax\n");
        printf("        setl al\n");
        printf("        movzb rax, al\n");
        break;
    }

    // Push rax to the stack for the callee.
    printf("        push rax\n");
}

int max(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

void gen_global_node(Node *node, Type *ty) {
    switch (node->kind) {
    case ND_NUM:
    case ND_CHAR:
        // supposing the type is int, for now
        switch (size_of(ty)) {
        case 1:
            printf("        .byte %d\n", (char) node->val);
            break;
        case 2:
            printf("        .short %d\n", (short) node->val);
            break;
        case 4:
            printf("        .long %d\n", (int) node->val);
            break;
        case 8:
            printf("        .quad %ld\n", node->val);
            break;
        default:
            error_at(node->str, "unsupported size: %d", size_of(ty));
        }
        break;
    case ND_GLOBAL_VAR:
        // take address of the global var
        printf("        .quad %.*s\n", node->len, node->str);
        break;
    case ND_STRING:
        // NOTE: does not support non-ascii characters for now
        printf("        .string \"%.*s\"\n", node->len, node->str);
        // zero fill
        if (size_of(type_of(node)) < size_of(ty)) {
            printf("        .zero %ld\n", size_of(ty) - (size_of(type_of(node))));
        }
        break;
    case ND_ADD:
        // expect node->left to be ND_GLOBAL_VAR
        printf("        .quad %.*s+%ld\n", node->left->len, node->left->str, node->right->val);
        break;
    case ND_SUB:
        // expect node->left to be ND_GLOBAL_VAR
        printf("        .quad %.*s+%ld\n", node->left->len, node->left->str, node->right->val);
        break;
    case ND_ARRAY: ;
        if (!ty || ty->ty != ARRAY) {
            error_at(node->str, "expected array type");
        }
        for (int i = 0; i < vector_count(node->arguments); i++) {
            Node *elt = (Node*) vector_get(node->arguments, i);
            gen_global_node(elt, ty->ptr_to);
        }
        // zero fill
        if (vector_count(node->arguments) < ty->array_size) {
            int zero_fill_size = (ty->array_size - vector_count(node->arguments)) * size_of(ty->ptr_to);
            printf("        .zero %d\n", zero_fill_size);
        }
        break;
    default:
        error("unknown initialization");
    }
}

// gen_global generates the assembly for the given global variable.
void gen_global(GlobalVar *var) {
    printf("%.*s:\n", var->len, var->name);
    // If there is an initialization for this global variable
    if (var->init) {
        gen_global_node(var->init, var->type);
    } else {
        printf("        .zero %d\n", var->offset);
    }
}

// gen reads the parsed code in AST, and prints out the assembly to complete the compilation.
void gen() {
    gen_tree_stack = new_vector();

    // Consume tokens to build multiple ASTs (Abstract Syntax Tree)
    program();

    // Base assembly syntax
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");

    // Print out global variables
    if (vector_count(globals) + vector_count(strings) > 0) {
        printf(".data\n");
    }
    for (int i = 0; i < vector_count(globals); i++) {
        GlobalVar *var = (GlobalVar*) vector_get(globals, i);
        gen_global(var);
    }

    // Print out string literals
    for (int i = 0; i < vector_count(strings); i++) {
        Node *literal = (Node*) vector_get(strings, i);
        printf(".LC%d:\n", literal->label);
        printf("        .string \"%.*s\"\n", literal->len, literal->str);
    }

    printf(".text\n");

    // Calculate the result for each functions
    for (int i = 0; i < vector_count(functions); i++) {
        gen_tree(vector_get(functions, i));
    }
}
