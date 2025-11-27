#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include "ast.h"

typedef struct CodeGenContext {
    FILE* output;
    int label_counter;
    int temp_counter;
    int indent_level;
    int loop_top;
    int loop_start[64];
    int loop_end[64];
    int loop_continue[64];
    int alloc_count;
    char* alloc_names[256];
} CodeGenContext;

CodeGenContext* codegen_create(FILE* output);
void codegen_free(CodeGenContext* ctx);

void codegen_program(CodeGenContext* ctx, ASTNode* program);
void codegen_function(CodeGenContext* ctx, ASTNode* func);
void codegen_statement(CodeGenContext* ctx, ASTNode* stmt);
void codegen_expression(CodeGenContext* ctx, ASTNode* expr);
void codegen_for(CodeGenContext* ctx, ASTNode* for_node);
void codegen_struct(CodeGenContext* ctx, ASTNode* type_def);
void codegen_runtime_helpers(CodeGenContext* ctx);

int codegen_new_label(CodeGenContext* ctx);
int codegen_new_temp(CodeGenContext* ctx);
void codegen_emit(CodeGenContext* ctx, const char* format, ...);
void codegen_emit_indent(CodeGenContext* ctx);

const char* codegen_type_to_c(TypeSpec* type);

void codegen_loop_push(CodeGenContext* ctx, int start, int end, int cont);
void codegen_loop_pop(CodeGenContext* ctx);

void codegen_member_access(CodeGenContext* ctx, ASTNode* member);

const char* codegen_type_to_c(TypeSpec* type);

void codegen_record_alloc(CodeGenContext* ctx, const char* name);
void codegen_emit_frees(CodeGenContext* ctx);

#endif // CODEGEN_H
