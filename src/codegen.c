#include "codegen.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Declarações antecipadas de funções auxiliares
int count_literal_elements(ASTNode* literal);
void codegen_literal_elements(CodeGenContext* ctx, ASTNode* literal, const char* var_name);
void codegen_array_literal(CodeGenContext* ctx, ASTNode* literal);
void codegen_matrix_literal(CodeGenContext* ctx, ASTNode* literal);
void codegen_set_literal(CodeGenContext* ctx, ASTNode* literal);

#define CG_DEBUG 1
#if CG_DEBUG
  #define DBG(fmt, ...) fprintf(stderr, "[codegen] " fmt "\n", ##__VA_ARGS__)
#else
  #define DBG(fmt, ...) do{}while(0)
#endif

CodeGenContext* codegen_create(FILE* output) {
    DBG("cria contexto");
    CodeGenContext* ctx = malloc(sizeof(CodeGenContext));
    ctx->output = output;
    ctx->label_counter = 0;
    ctx->temp_counter = 0;
    ctx->indent_level = 0;
    ctx->loop_top = -1;
    memset(ctx->loop_continue, 0, sizeof(ctx->loop_continue));
    ctx->alloc_count = 0;
    return ctx;
}

void codegen_free(CodeGenContext* ctx) {
    DBG("libera contexto");
    if (ctx) {
        for (int i = 0; i < ctx->alloc_count; i++) {
            if (ctx->alloc_names[i]) {
                free(ctx->alloc_names[i]);
            }
        }
        free(ctx);
    }
}

int codegen_new_label(CodeGenContext* ctx) {
    return ctx->label_counter++;
}

int codegen_new_temp(CodeGenContext* ctx) {
    return ctx->temp_counter++;
}

void codegen_emit_indent(CodeGenContext* ctx) {
    for (int i = 0; i < ctx->indent_level; i++) {
        fprintf(ctx->output, "    ");
    }
}

void codegen_emit(CodeGenContext* ctx, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(ctx->output, format, args);
    va_end(args);
}

const char* codegen_type_to_c(TypeSpec* type) {
    if (!type) return "void";

    if (type->type_name) {
        return type->type_name;
    }

    switch (type->base_type) {
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "double";
        case TYPE_BOOL: return "int";
        case TYPE_CHAR: return "char";
        case TYPE_STRING: return "char*";
        case TYPE_VOID: return "void";
        case TYPE_COMPLEX: return "double complex";
        case TYPE_ARRAY: {
            static char type_buffer[256];
            if (type->element_type) {
                const char* elem_type = codegen_type_to_c(type->element_type);
                if (type->element_type->base_type == TYPE_ARRAY) {
                    // Array multidimensional
                    snprintf(type_buffer, sizeof(type_buffer), "%s*", elem_type);
                } else {
                    // Array simples
                    snprintf(type_buffer, sizeof(type_buffer), "%s*", elem_type);
                }
                return type_buffer;
            }
            return "void*";
        }
        case TYPE_MATRIX: return "void*";
        case TYPE_SET: return "void*";
        case TYPE_GRAPH: return "void*";
        default: return "void";
    }
}

void codegen_runtime_helpers(CodeGenContext* ctx) {
    fputs("// Runtime helpers (range emulation)\n", ctx->output);
    fputs("typedef struct { int start; int end; int step; } range_t;\n", ctx->output);
    fputs("static inline range_t make_range(int s, int e, int st){ range_t r={s,e,st}; return r; }\n", ctx->output);
    fputs("static inline int range_has_next(range_t* r, int i){ return r->step>0 ? i<r->end : i>r->end; }\n", ctx->output);
    fputs("static inline int mathc_floor_div(int a, int b){ if (b == 0) { fprintf(stderr, \"Erro: Divisao por zero\\n\"); exit(1); } int q = a / b; int r = a % b; if ((r != 0) && ((r > 0) != (b > 0))) q--; return q; }\n", ctx->output);

    // Set operation helpers
    fputs("// Set operation helpers\n", ctx->output);
    fputs("static inline int* set_union(int* a, int a_len, int* b, int b_len) {\n", ctx->output);
    fputs("    int* result = malloc(sizeof(int) * (a_len + b_len));\n", ctx->output);
    fputs("    int result_len = 0;\n", ctx->output);
    fputs("    for (int i = 0; i < a_len; i++) result[result_len++] = a[i];\n", ctx->output);
    fputs("    for (int i = 0; i < b_len; i++) {\n", ctx->output);
    fputs("        int found = 0;\n", ctx->output);
    fputs("        for (int j = 0; j < a_len; j++) if (b[i] == a[j]) { found = 1; break; }\n", ctx->output);
    fputs("        if (!found) result[result_len++] = b[i];\n", ctx->output);
    fputs("    }\n", ctx->output);
    fputs("    // Note: caller should update __len field\n", ctx->output);
    fputs("    return result;\n", ctx->output);
    fputs("}\n", ctx->output);
    fputs("static inline int* set_difference(int* a, int a_len, int* b, int b_len) {\n", ctx->output);
    fputs("    int* result = malloc(sizeof(int) * a_len);\n", ctx->output);
    fputs("    int result_len = 0;\n", ctx->output);
    fputs("    for (int i = 0; i < a_len; i++) {\n", ctx->output);
    fputs("        int found = 0;\n", ctx->output);
    fputs("        for (int j = 0; j < b_len; j++) if (a[i] == b[j]) { found = 1; break; }\n", ctx->output);
    fputs("        if (!found) result[result_len++] = a[i];\n", ctx->output);
    fputs("    }\n", ctx->output);
    fputs("    return result;\n", ctx->output);
    fputs("}\n", ctx->output);
    fputs("static inline int set_contains(int* set, int set_len, int element) {\n", ctx->output);
    fputs("    for (int i = 0; i < set_len; i++) if (set[i] == element) return 1;\n", ctx->output);
    fputs("    return 0;\n", ctx->output);
    fputs("}\n", ctx->output);

    fputc('\n', ctx->output);
}

void codegen_loop_push(CodeGenContext* ctx, int start, int end, int cont) {
    if (ctx->loop_top < 63) {
        ctx->loop_top++;
        ctx->loop_start[ctx->loop_top] = start;
        ctx->loop_end[ctx->loop_top] = end;
        ctx->loop_continue[ctx->loop_top] = cont;
    }
}

void codegen_loop_pop(CodeGenContext* ctx) {
    if (ctx->loop_top >= 0) ctx->loop_top--;
}

void codegen_record_alloc(CodeGenContext* ctx, const char* name) {
    if (ctx->alloc_count < 256 && name) {
        ctx->alloc_names[ctx->alloc_count] = strdup(name);
        ctx->alloc_count++;
    }
}

void codegen_emit_frees(CodeGenContext* ctx) {
    for (int i = 0; i < ctx->alloc_count; i++) {
        if (ctx->alloc_names[i]) {
            codegen_emit_indent(ctx);
            codegen_emit(ctx, "free(%s);\n", ctx->alloc_names[i]);
            free(ctx->alloc_names[i]);
            ctx->alloc_names[i] = NULL;
        }
    }
    ctx->alloc_count = 0;
}

void codegen_member_access(CodeGenContext* ctx, ASTNode* member) {
    if (!member || member->type != NODE_MEMBER_ACCESS) { codegen_emit(ctx, "/* invalid member */"); return; }

    ASTNode* obj = member->member_access.object;
    const char* method = member->member_access.member;

    // Special handling for array methods
    if (obj->inferred_type && obj->inferred_type->base_type == TYPE_ARRAY) {
        if (strcmp(method, "add") == 0 && member->member_access.args) {
            // array.add(element) -> array = realloc(array, sizeof(*array) * (array__len + 1)); array[array__len++] = element;
            codegen_emit(ctx, "(");
            codegen_expression(ctx, obj);
            codegen_emit(ctx, " = realloc(");
            codegen_expression(ctx, obj);
            codegen_emit(ctx, ", sizeof(*");
            codegen_expression(ctx, obj);
            codegen_emit(ctx, ") * (");
            codegen_expression(ctx, obj);
            codegen_emit(ctx, "__len + 1)), ");
            codegen_expression(ctx, obj);
            codegen_emit(ctx, "[");
            codegen_expression(ctx, obj);
            codegen_emit(ctx, "__len++] = ");
            codegen_expression(ctx, member->member_access.args->node);
            codegen_emit(ctx, ")");
            return;
        } else if (strcmp(method, "remove") == 0 && member->member_access.args) {
            // array.remove(index) -> memmove(&array[index], &array[index+1], sizeof(*array) * (array__len - index - 1)); array__len--;
            codegen_emit(ctx, "(memmove(&");
            codegen_expression(ctx, obj);
            codegen_emit(ctx, "[");
            codegen_expression(ctx, member->member_access.args->node);
            codegen_emit(ctx, "], &");
            codegen_expression(ctx, obj);
            codegen_emit(ctx, "[");
            codegen_expression(ctx, member->member_access.args->node);
            codegen_emit(ctx, " + 1], sizeof(*");
            codegen_expression(ctx, obj);
            codegen_emit(ctx, ") * (");
            codegen_expression(ctx, obj);
            codegen_emit(ctx, "__len - ");
            codegen_expression(ctx, member->member_access.args->node);
            codegen_emit(ctx, " - 1)), ");
            codegen_expression(ctx, obj);
            codegen_emit(ctx, "__len--)");
            return;
        } else if (strcmp(method, "clear") == 0) {
            // array.clear() -> array__len = 0;
            codegen_expression(ctx, obj);
            codegen_emit(ctx, "__len = 0");
            return;
        }
    }

    // Special handling for struct field access
    if (obj->inferred_type && obj->inferred_type->type_name) {
        if (!member->member_access.args) {
            // This is a field access: struct.field
            codegen_expression(ctx, obj);
            codegen_emit(ctx, ".%s", method);
            return;
        } else {
            // This is a method call on a user-defined type (struct/class)
            codegen_emit(ctx, "%s_%s(", obj->inferred_type->type_name, method);
            codegen_expression(ctx, obj);  // first argument is the struct itself
            NodeList* a = member->member_access.args;
            while (a) {
                codegen_emit(ctx, ", ");
                codegen_expression(ctx, a->node);
                a = a->next;
            }
            codegen_emit(ctx, ")");
            return;
        }
    }

    // Default member access (for pointers, etc.)
    codegen_expression(ctx, member->member_access.object);
    codegen_emit(ctx, "->%s", member->member_access.member);
    if (member->member_access.args) {
        // call style
        codegen_emit(ctx, "(");
        NodeList* a = member->member_access.args; int first=1;
        while (a) { if (!first) codegen_emit(ctx, ", "); codegen_expression(ctx, a->node); first=0; a=a->next; }
        codegen_emit(ctx, ")");
    }
}

void codegen_for(CodeGenContext* ctx, ASTNode* for_node) {
    if (!for_node || for_node->type != NODE_FOR_STMT) return;
    int startL = codegen_new_label(ctx);
    int endL = codegen_new_label(ctx);
    codegen_loop_push(ctx, startL, endL, startL); // for range, continue goes to start

    ASTNode* iterable = for_node->for_stmt.iterable;
    const char* iterName = for_node->for_stmt.iterator;
    const char* idxName = for_node->for_stmt.index_var;

    // Range-based
    if (iterable->type == NODE_CALL && iterable->call.function->type == NODE_IDENTIFIER && strcmp(iterable->call.function->identifier.name, "range") == 0) {
        NodeList* args = iterable->call.arguments;
        int argc = 0; for (NodeList* t=args;t;t=t->next) argc++;
        codegen_emit_indent(ctx); codegen_emit(ctx, "int %s_index = 0;\n", iterName);
        codegen_emit_indent(ctx);
        if (argc == 1) {
            codegen_emit(ctx, "range_t %s_range = make_range(0, ", iterName); codegen_expression(ctx, args->node); codegen_emit(ctx, ", 1);\n");
        } else {
            codegen_emit(ctx, "range_t %s_range = make_range(", iterName); codegen_expression(ctx, args->node); codegen_emit(ctx, ", "); codegen_expression(ctx, args->next->node); codegen_emit(ctx, ", 1);\n");
        }
        codegen_emit(ctx, "L%d:\n", startL);
        codegen_emit_indent(ctx); codegen_emit(ctx, "if (!range_has_next(&%s_range, %s_index)) goto L%d;\n", iterName, iterName, endL);
        codegen_emit_indent(ctx); codegen_emit(ctx, "int %s = %s_index;\n", iterName, iterName);
        ctx->indent_level++;
        NodeList* body = for_node->for_stmt.body; while (body){ if (body->node) codegen_statement(ctx, body->node); body=body->next; }
        ctx->indent_level--;
        codegen_emit_indent(ctx); codegen_emit(ctx, "%s_index += %s_range.step;\n", iterName, iterName);
        codegen_emit_indent(ctx); codegen_emit(ctx, "goto L%d;\n", startL);
        codegen_emit(ctx, "L%d:\n", endL);
        codegen_loop_pop(ctx);
        return;
    }

    // Array literal iteration (value[, index])
    if (iterable->type == NODE_ARRAY_LITERAL) {
        // create a temp array
        int arrId = codegen_new_temp(ctx);
        int count = 0; NodeList* el = iterable->array_literal.elements; while (el){count++; el=el->next;}
        codegen_emit_indent(ctx); codegen_emit(ctx, "int* _arr%d = malloc(sizeof(int)*%d);\n", arrId, count);
        el = iterable->array_literal.elements; int pos=0;
        while (el){ codegen_emit_indent(ctx); codegen_emit(ctx, "_arr%d[%d] = ", arrId, pos); codegen_expression(ctx, el->node); codegen_emit(ctx, ";\n"); el=el->next; pos++; }
        int continueL = codegen_new_label(ctx);
        codegen_loop_push(ctx, startL, endL, continueL);
        codegen_emit_indent(ctx); codegen_emit(ctx, "int _i%d = 0;\n", arrId);
        codegen_emit(ctx, "L%d:\n", startL);
        codegen_emit_indent(ctx); codegen_emit(ctx, "if (_i%d >= %d) goto L%d;\n", arrId, count, endL);
        // value binding
        codegen_emit_indent(ctx); codegen_emit(ctx, "int %s = _arr%d[_i%d];\n", iterName, arrId, arrId);
        if (idxName) { codegen_emit_indent(ctx); codegen_emit(ctx, "int %s = _i%d;\n", idxName, arrId); }
        ctx->indent_level++;
        NodeList* body = for_node->for_stmt.body; while (body){ if (body->node) codegen_statement(ctx, body->node); body=body->next; }
        ctx->indent_level--;
        codegen_emit(ctx, "L%d:\n", continueL);
        codegen_emit_indent(ctx); codegen_emit(ctx, "_i%d++;\n", arrId);
        codegen_emit_indent(ctx); codegen_emit(ctx, "goto L%d;\n", startL);
        codegen_emit(ctx, "L%d:\n", endL);
        codegen_loop_pop(ctx);
        return;
    }

    // Identifier array (assume int* with len builtin)
    if (iterable->type == NODE_IDENTIFIER) {
        int arrId = codegen_new_temp(ctx);
        int continueL = codegen_new_label(ctx);
        codegen_loop_push(ctx, startL, endL, continueL);
        codegen_emit_indent(ctx); codegen_emit(ctx, "int _len%d = %s__len;\n", arrId, iterable->identifier.name);
        codegen_emit_indent(ctx); codegen_emit(ctx, "int _i%d = 0;\n", arrId);
        codegen_emit(ctx, "L%d:\n", startL);
        codegen_emit_indent(ctx); codegen_emit(ctx, "if (_i%d >= _len%d) goto L%d;\n", arrId, arrId, endL);
        codegen_emit_indent(ctx); codegen_emit(ctx, "int %s = %s[_i%d];\n", iterName, iterable->identifier.name, arrId);
        if (idxName) { codegen_emit_indent(ctx); codegen_emit(ctx, "int %s = _i%d;\n", idxName, arrId); }
        ctx->indent_level++;
        NodeList* body = for_node->for_stmt.body; while (body){ if (body->node) codegen_statement(ctx, body->node); body=body->next; }
        ctx->indent_level--;
        codegen_emit(ctx, "L%d:\n", continueL);
        codegen_emit_indent(ctx); codegen_emit(ctx, "_i%d++;\n", arrId);
        codegen_emit_indent(ctx); codegen_emit(ctx, "goto L%d;\n", startL);
        codegen_emit(ctx, "L%d:\n", endL);
        codegen_loop_pop(ctx);
        return;
    }

    // Fallback
    codegen_emit_indent(ctx); codegen_emit(ctx, "/* unsupported for-loop iterable */\n");
}

void codegen_struct(CodeGenContext* ctx, ASTNode* type_def) {
    if (!type_def || type_def->type != NODE_TYPE_DEF) return;
    // Emit struct with fields (only declarations nodes)
    fputs("typedef struct ", ctx->output);
    fputs(type_def->type_def.name, ctx->output);
    fputs(" {\n", ctx->output);
    NodeList* m = type_def->type_def.members;
    while (m) {
        ASTNode* n = m->node;
        if (n && n->type == NODE_DECLARATION) {
            codegen_emit_indent(ctx);
            fputs(codegen_type_to_c(n->declaration.var_type), ctx->output);
            fputc(' ', ctx->output);
            fputs(n->declaration.name, ctx->output);
            fputs(";\n", ctx->output);
        }
        m = m->next;
    }
    fputs("} ", ctx->output);
    fputs(type_def->type_def.name, ctx->output);
    fputs(";\n\n", ctx->output);
    m = type_def->type_def.members;
    while (m) {
        ASTNode* n = m->node;
        if (n && n->type == NODE_FUNCTION_DEF) {
            const char* ret_type = n->function_def.return_type ? codegen_type_to_c(n->function_def.return_type) : "void";
            fputs(ret_type, ctx->output);
            fputs(" ", ctx->output);
            fputs(n->function_def.name, ctx->output);
            fputs("(", ctx->output);

            fputs(type_def->type_def.name, ctx->output);
            fputs(" this", ctx->output);

            ParamList* p = n->function_def.parameters;
            while (p) {
                fputs(", ", ctx->output);
                fputs(codegen_type_to_c(p->type), ctx->output);
                fputc(' ', ctx->output);
                fputs(p->name, ctx->output);
                p = p->next;
            }
            fputs(") {\n", ctx->output);

            ctx->indent_level++;
            NodeList* b = n->function_def.body;
            while (b) {
                if (b->node) codegen_statement(ctx, b->node);
                b = b->next;
            }
            ctx->indent_level--;

            fputs("}\n\n", ctx->output);
        }
        m = m->next;
    }
}

void codegen_function(CodeGenContext* ctx, ASTNode* func) {
    DBG("function %s", func && func->type == NODE_FUNCTION_DEF ? func->function_def.name : "<invalid>");
    if (!func) { DBG("null func"); return; }
    if (func->type != NODE_FUNCTION_DEF) { DBG("wrong node type: %d", func->type); return; }

    const char* ret_type = "void";
    if (func->function_def.return_type) {
        ret_type = codegen_type_to_c(func->function_def.return_type);
    }
    const char* func_name = func->function_def.name ? func->function_def.name : "fn";
    
    int is_main = (strcmp(func_name, "main") == 0);
    if (is_main) {
        ret_type = "int";
    }
    
    DBG("func_name ptr=%p text=%s", (void*)func->function_def.name, func_name);
    fputs(ret_type, ctx->output);
    fputc(' ', ctx->output);
    fputs(func_name, ctx->output);
    fputc('(', ctx->output);
    fflush(ctx->output);

    ParamList* params = func->function_def.parameters;
    int first = 1;
    while (params) {
        const char* pname = params->name ? params->name : "arg";

        // Para arrays, gerar dois parâmetros: ponteiro e tamanho
        if (params->type && params->type->base_type == TYPE_ARRAY) {
            const char* elem_type = params->type->element_type ?
                codegen_type_to_c(params->type->element_type) : "int";

            if (!first) codegen_emit(ctx, ", ");
            codegen_emit(ctx, "%s* %s, int %s__len", elem_type, pname, pname);
            first = 0;
        } else {
            const char* ptype = codegen_type_to_c(params->type);
            if (!first) codegen_emit(ctx, ", ");
            codegen_emit(ctx, "%s %s", ptype ? ptype : "int", pname);
            first = 0;
        }

        params = params->next;
    }
    if (first) {
        codegen_emit(ctx, "void");
    }

    codegen_emit(ctx, ") {\n");
    fflush(ctx->output);
    ctx->indent_level++;

    NodeList* body = func->function_def.body;
    while (body) {
        DBG("stmt node type=%d", body->node ? body->node->type : -1);
        if (body->node) {
            codegen_statement(ctx, body->node);
        }
        body = body->next;
    }

    ctx->indent_level--;
    codegen_emit(ctx, "\n");
    codegen_emit_indent(ctx);
    codegen_emit(ctx, "/* function allocations cleanup */\n");
    codegen_emit_indent(ctx);
    codegen_emit_frees(ctx);
    
    if (is_main) {
        codegen_emit_indent(ctx);
        codegen_emit(ctx, "return 0;\n");
    }
    
    codegen_emit(ctx, "}\n\n");
    fflush(ctx->output);
}

void codegen_statement(CodeGenContext* ctx, ASTNode* stmt) {
    if (!stmt) return;

    codegen_emit_indent(ctx);

    switch (stmt->type) {
        case NODE_DECLARATION: {
            TypeSpec* var_type = stmt->declaration.var_type;
            const char* type_str = codegen_type_to_c(var_type);

            // Default case for simple declarations
            codegen_emit(ctx, "%s %s", type_str, stmt->declaration.name);
            if (stmt->declaration.initializer) {
                codegen_emit(ctx, " = ");
                codegen_expression(ctx, stmt->declaration.initializer);
            }
            codegen_emit(ctx, ";\n");
            break;

            // Primeiro trata arrays multidimensionais
            if (var_type && var_type->base_type == TYPE_ARRAY && var_type->element_type &&
                var_type->element_type->base_type == TYPE_ARRAY) {

                if (stmt->declaration.initializer && stmt->declaration.initializer->type == NODE_ARRAY_LITERAL &&
                    stmt->declaration.initializer->array_literal.elements &&
                    stmt->declaration.initializer->array_literal.elements->node &&
                    stmt->declaration.initializer->array_literal.elements->node->type == NODE_ARRAY_LITERAL) {

                    // Array multidimensional: [[1,2],[3,4],[5,6]]
                    TypeSpec* elem_type = var_type->element_type->element_type;
                    const char* elem_c_type = elem_type ? codegen_type_to_c(elem_type) : "int";

                    int rows = count_literal_elements(stmt->declaration.initializer);
                    int cols = 0;

                    // Conta colunas da primeira linha
                    if (stmt->declaration.initializer->array_literal.elements &&
                        stmt->declaration.initializer->array_literal.elements->node &&
                        stmt->declaration.initializer->array_literal.elements->node->type == NODE_ARRAY_LITERAL) {
                        cols = count_literal_elements(stmt->declaration.initializer->array_literal.elements->node);
                    }

                    codegen_emit(ctx, "%s** %s = malloc(sizeof(%s*) * %d);\n", elem_c_type, stmt->declaration.name, elem_c_type, rows);
                    codegen_emit_indent(ctx);
                    codegen_emit(ctx, "int %s__len = %d;\n", stmt->declaration.name, rows);
                    codegen_emit_indent(ctx);
                    codegen_emit(ctx, "for(int i = 0; i < %d; i++) %s[i] = malloc(sizeof(%s) * %d);\n",
                                rows, stmt->declaration.name, elem_c_type, cols);

                    // Inicializa elementos
                    NodeList* row_list = stmt->declaration.initializer->array_literal.elements;
                    int row_idx = 0;
                    while (row_list) {
                        if (row_list->node && row_list->node->type == NODE_ARRAY_LITERAL) {
                            NodeList* col_list = row_list->node->array_literal.elements;
                            int col_idx = 0;
                            while (col_list && col_idx < cols) {
                                codegen_emit_indent(ctx);
                                codegen_emit(ctx, "%s[%d][%d] = ", stmt->declaration.name, row_idx, col_idx);
                                codegen_expression(ctx, col_list->node);
                                codegen_emit(ctx, ";\n");
                                col_list = col_list->next;
                                col_idx++;
                            }
                        }
                        row_list = row_list->next;
                        row_idx++;
                    }
                    codegen_record_alloc(ctx, stmt->declaration.name);
                    return;
                }
            }

            if (var_type && var_type->base_type == TYPE_ARRAY) {
                TypeSpec* elem_type = var_type->element_type;
                const char* elem_c_type = elem_type ? codegen_type_to_c(elem_type) : "int";

                if (stmt->declaration.initializer && stmt->declaration.initializer->type == NODE_ARRAY_LITERAL) {
                    int count = 0; NodeList* el = stmt->declaration.initializer->array_literal.elements;
                    while (el) { count++; el = el->next; }
                    codegen_emit(ctx, "%s* %s = malloc(sizeof(%s) * %d);\n", elem_c_type, stmt->declaration.name, elem_c_type, count);
                    codegen_emit_indent(ctx);
                    codegen_emit(ctx, "int %s__len = %d;\n", stmt->declaration.name, count);
                    el = stmt->declaration.initializer->array_literal.elements;
                    int idx = 0;
                    while (el) {
                        codegen_emit_indent(ctx);
                        codegen_emit(ctx, "%s[%d] = ", stmt->declaration.name, idx);
                        codegen_expression(ctx, el->node);
                        codegen_emit(ctx, ";\n");
                        el = el->next;
                        idx++;
                    }
                    codegen_record_alloc(ctx, stmt->declaration.name);
                    return;
                } else {
                    codegen_emit(ctx, "%s* %s", elem_c_type, stmt->declaration.name);
                    if (stmt->declaration.initializer) {
                        codegen_emit(ctx, " = ");
                        codegen_expression(ctx, stmt->declaration.initializer);
                    }
                    codegen_emit(ctx, ";\n");
                    return;
                }
            }
            
            if (var_type && var_type->base_type == TYPE_SET) {
                TypeSpec* elem_type = var_type->element_type;
                const char* elem_c_type = elem_type ? codegen_type_to_c(elem_type) : "int";

                if (stmt->declaration.initializer && stmt->declaration.initializer->type == NODE_SET_LITERAL) {
                    int count = 0; NodeList* el = stmt->declaration.initializer->set_literal.elements;
                    while (el) { count++; el = el->next; }
                    codegen_emit(ctx, "%s* %s = malloc(sizeof(%s) * %d);\n", elem_c_type, stmt->declaration.name, elem_c_type, count);
                    codegen_emit_indent(ctx);
                    codegen_emit(ctx, "int %s__len = %d;\n", stmt->declaration.name, count);
                    el = stmt->declaration.initializer->set_literal.elements;
                    int idx = 0;
                    while (el) {
                        codegen_emit_indent(ctx);
                        codegen_emit(ctx, "%s[%d] = ", stmt->declaration.name, idx);
                        codegen_expression(ctx, el->node);
                        codegen_emit(ctx, ";\n");
                        el = el->next;
                        idx++;
                    }
                    codegen_record_alloc(ctx, stmt->declaration.name);
                    return;
                } else {
                    codegen_emit(ctx, "%s* %s", elem_c_type, stmt->declaration.name);
                    if (stmt->declaration.initializer) {
                        codegen_emit(ctx, " = ");
                        codegen_expression(ctx, stmt->declaration.initializer);
                    }
                    codegen_emit(ctx, ";\n");
                    codegen_emit_indent(ctx);
                    codegen_emit(ctx, "int %s__len = 0;\n", stmt->declaration.name);
                    return;
                }
            }

            if (var_type && var_type->base_type == TYPE_MATRIX) {
                TypeSpec* elem_type = var_type->element_type;
                const char* elem_c_type = elem_type ? codegen_type_to_c(elem_type) : "int";

                if (stmt->declaration.initializer && stmt->declaration.initializer->type == NODE_MATRIX_LITERAL) {
                    int rows = count_literal_elements(stmt->declaration.initializer);
                    int cols = 0;

                    // Conta colunas da primeira linha
                    if (stmt->declaration.initializer->matrix_literal.rows &&
                        stmt->declaration.initializer->matrix_literal.rows->node &&
                        stmt->declaration.initializer->matrix_literal.rows->node->type == NODE_ARRAY_LITERAL) {
                        cols = count_literal_elements(stmt->declaration.initializer->matrix_literal.rows->node);
                    }

                    codegen_emit(ctx, "%s** %s = malloc(sizeof(%s*) * %d);\n", elem_c_type, stmt->declaration.name, elem_c_type, rows);
                    codegen_emit_indent(ctx);
                    codegen_emit(ctx, "int %s__rows = %d;\n", stmt->declaration.name, rows);
                    codegen_emit_indent(ctx);
                    codegen_emit(ctx, "int %s__cols = %d;\n", stmt->declaration.name, cols);
                    codegen_emit_indent(ctx);
                    codegen_emit(ctx, "for(int i = 0; i < %d; i++) %s[i] = malloc(sizeof(%s) * %d);\n",
                                rows, stmt->declaration.name, elem_c_type, cols);

                    codegen_literal_elements(ctx, stmt->declaration.initializer, stmt->declaration.name);
                    codegen_record_alloc(ctx, stmt->declaration.name);
                    return;
                } else {
                    codegen_emit(ctx, "%s** %s", elem_c_type, stmt->declaration.name);
                    if (stmt->declaration.initializer) {
                        codegen_emit(ctx, " = ");
                        codegen_expression(ctx, stmt->declaration.initializer);
                    }
                codegen_emit(ctx, ";\n");
                codegen_emit_indent(ctx);
                codegen_emit(ctx, "int %s__rows = 0;\n", stmt->declaration.name);
                codegen_emit_indent(ctx);
                codegen_emit(ctx, "int %s__cols = 0;\n", stmt->declaration.name);
                return;
            }

            // Default case for simple declarations
            codegen_emit(ctx, "%s %s", type_str, stmt->declaration.name);
            if (stmt->declaration.initializer) {
                codegen_emit(ctx, " = ");
                codegen_expression(ctx, stmt->declaration.initializer);
            }
            codegen_emit(ctx, ";\n");
            break;
        }
        case NODE_FOR_STMT: {
            // Basic for loop implementation
            int startL = codegen_new_label(ctx);
            int endL = codegen_new_label(ctx);

            // For now, assume it's an array iteration
            ASTNode* iterable = stmt->for_stmt.iterable;
            const char* iterName = stmt->for_stmt.iterator;

            if (iterable->type == NODE_IDENTIFIER) {
                // Simple array iteration
                codegen_emit(ctx, "{\n");
                ctx->indent_level++;
                codegen_emit_indent(ctx);
                codegen_emit(ctx, "int _len = %s__len;\n", iterable->identifier.name);
                codegen_emit_indent(ctx);
                codegen_emit(ctx, "int _i = 0;\n");
                codegen_emit_indent(ctx);
                codegen_emit(ctx, "for (_i = 0; _i < _len; _i++) {\n");
                ctx->indent_level++;
                codegen_emit_indent(ctx);
                codegen_emit(ctx, "int %s = %s[_i];\n", iterName, iterable->identifier.name);

                // Generate body
                NodeList* body = stmt->for_stmt.body;
                while (body) {
                    if (body->node) {
                        codegen_statement(ctx, body->node);
                    }
                    body = body->next;
                }

                ctx->indent_level--;
                codegen_emit_indent(ctx);
                codegen_emit(ctx, "}\n");
                ctx->indent_level--;
                codegen_emit_indent(ctx);
                codegen_emit(ctx, "}\n");
            } else {
                codegen_emit(ctx, "/* Unsupported for loop */\n");
            }
            break;
        }

        case NODE_ASSIGNMENT: {
            if (stmt->assignment.op == OP_ASSIGN) {
                codegen_expression(ctx, stmt->assignment.target);
                codegen_emit(ctx, " = ");
                codegen_expression(ctx, stmt->assignment.value);
            } else {
                // Compound assignment
                codegen_expression(ctx, stmt->assignment.target);
                codegen_emit(ctx, " = ");
                codegen_expression(ctx, stmt->assignment.target);
                switch (stmt->assignment.op) {
                    case OP_ADD: codegen_emit(ctx, " + "); break;
                    case OP_SUB: codegen_emit(ctx, " - "); break;
                    case OP_MUL: codegen_emit(ctx, " * "); break;
                    case OP_DIV: codegen_emit(ctx, " / "); break;
                    case OP_MOD: codegen_emit(ctx, " %% "); break;
                    default: codegen_emit(ctx, " /*compound?*/ "); break;
                }
                codegen_expression(ctx, stmt->assignment.value);
            }
                codegen_expression(ctx, stmt->assignment.target);
                codegen_emit(ctx, " = ");
                codegen_expression(ctx, stmt->assignment.target);
                switch (stmt->assignment.op) {
                    case OP_ADD: codegen_emit(ctx, " + "); break;
                    case OP_SUB: codegen_emit(ctx, " - "); break;
                    case OP_MUL: codegen_emit(ctx, " * "); break;
                    case OP_DIV: codegen_emit(ctx, " / "); break;
                    case OP_MOD: codegen_emit(ctx, " %% "); break;
                    default: codegen_emit(ctx, " /*compound?*/ "); break;
                }
                codegen_expression(ctx, stmt->assignment.value);
            }
                codegen_emit(ctx, ";\n");
                break;
            }

            // Default case for simple declarations
            codegen_emit(ctx, "%s %s", type_str, stmt->declaration.name);
            if (stmt->declaration.initializer) {
                codegen_emit(ctx, " = ");
                codegen_expression(ctx, stmt->declaration.initializer);
            }
            codegen_emit(ctx, ";\n");
        }

        case NODE_IF_STMT: {
            int else_label = codegen_new_label(ctx);
            int end_label = codegen_new_label(ctx);

            codegen_emit(ctx, "if (!(");
            codegen_expression(ctx, stmt->if_stmt.condition);
            codegen_emit(ctx, ")) goto L%d;\n", else_label);

            ctx->indent_level++;
            NodeList* body = stmt->if_stmt.then_branch;
            while (body) {
                if (body->node) {
                    codegen_statement(ctx, body->node);
                }
                body = body->next;
            }
            ctx->indent_level--;

            if (stmt->if_stmt.else_branch) {
                codegen_emit_indent(ctx);
                codegen_emit(ctx, "goto L%d;\n", end_label);
                codegen_emit(ctx, "L%d:\n", else_label);

                ctx->indent_level++;
                body = stmt->if_stmt.else_branch;
                while (body) {
                    if (body->node) {
                        codegen_statement(ctx, body->node);
                    }
                    body = body->next;
                }
                ctx->indent_level--;

                codegen_emit(ctx, "L%d:\n", end_label);
            } else {
                codegen_emit(ctx, "L%d:\n", else_label);
            }
            break;
        }
            int end_label = codegen_new_label(ctx);
            int else_label = codegen_new_label(ctx);

            codegen_emit(ctx, "if (!(");
            codegen_expression(ctx, stmt->if_stmt.condition);
            codegen_emit(ctx, ")) goto L%d;\n", else_label);

            ctx->indent_level++;
            NodeList* then_block = stmt->if_stmt.then_block;
            while (then_block) {
                if (then_block->node) {
                    codegen_statement(ctx, then_block->node);
                }
                then_block = then_block->next;
            }
            ctx->indent_level--;

            codegen_emit_indent(ctx);
            codegen_emit(ctx, "goto L%d;\n", end_label);
            codegen_emit(ctx, "L%d:\n", else_label);

            if (stmt->if_stmt.else_block) {
                ctx->indent_level++;
                NodeList* else_block = stmt->if_stmt.else_block;
                while (else_block) {
                    if (else_block->node) {
                        codegen_statement(ctx, else_block->node);
                    }
                    else_block = else_block->next;
                }
                ctx->indent_level--;
            }

            codegen_emit(ctx, "L%d:\n", end_label);
            break;
        }

        case NODE_WHILE_STMT: {
            int start_label = codegen_new_label(ctx);
            int end_label = codegen_new_label(ctx);
            codegen_loop_push(ctx, start_label, end_label, start_label);

            codegen_emit(ctx, "L%d:\n", start_label);
            codegen_emit_indent(ctx);
            codegen_emit(ctx, "if (!(");
            codegen_expression(ctx, stmt->while_stmt.condition);
            codegen_emit(ctx, ")) goto L%d;\n", end_label);

            ctx->indent_level++;
            NodeList* body = stmt->while_stmt.body;
            while (body) {
                if (body->node) {
                    codegen_statement(ctx, body->node);
                }
                body = body->next;
            }
            ctx->indent_level--;

            codegen_emit_indent(ctx);
            codegen_emit(ctx, "goto L%d;\n", start_label);
            codegen_emit(ctx, "L%d:\n", end_label);
            codegen_loop_pop(ctx);
            break;
        }
            int start_label = codegen_new_label(ctx);
            int end_label = codegen_new_label(ctx);
            codegen_loop_push(ctx, start_label, end_label, start_label); // for while, continue goes to start

            codegen_emit(ctx, "L%d:\n", start_label);
            codegen_emit_indent(ctx);
            codegen_emit(ctx, "if (!(");
            codegen_expression(ctx, stmt->while_stmt.condition);
            codegen_emit(ctx, ")) goto L%d;\n", end_label);

            ctx->indent_level++;
            NodeList* body = stmt->while_stmt.body;
            while (body) {
                if (body->node) {
                    codegen_statement(ctx, body->node);
                }
                body = body->next;
            }
            ctx->indent_level--;

            codegen_emit_indent(ctx);
            codegen_emit(ctx, "goto L%d;\n", start_label);
            codegen_emit(ctx, "L%d:\n", end_label);
            codegen_loop_pop(ctx);
            break;
        }

        case NODE_RETURN_STMT: {
            if (stmt->return_stmt.value) {
                codegen_emit(ctx, "return ");
                codegen_expression(ctx, stmt->return_stmt.value);
                codegen_emit(ctx, ";\n");
            } else {
                codegen_emit(ctx, "return;\n");
            }
            break;
        }
            codegen_emit(ctx, "return");
            if (stmt->return_stmt.value) {
                codegen_emit(ctx, " ");
                codegen_expression(ctx, stmt->return_stmt.value);
            }
            codegen_emit(ctx, ";\n");
            break;
        }

        case NODE_BREAK_STMT: {
            if (ctx->loop_top >= 0) {
                codegen_emit(ctx, "goto L%d;\n", ctx->loop_end[ctx->loop_top]);
            } else {
                codegen_emit(ctx, "/* break outside loop */\n");
            }
            break;
        }
            if (ctx->loop_top >= 0) {
                codegen_emit(ctx, "goto L%d;\n", ctx->loop_end[ctx->loop_top]);
            } else {
                codegen_emit(ctx, "/* break outside loop */;\n");
            }
            break;
        }

        case NODE_CONTINUE_STMT: {
            if (ctx->loop_top >= 0) {
                int continue_target = ctx->loop_continue[ctx->loop_top] ?
                    ctx->loop_continue[ctx->loop_top] : ctx->loop_start[ctx->loop_top];
                codegen_emit(ctx, "goto L%d;\n", continue_target);
            } else {
                codegen_emit(ctx, "/* continue outside loop */\n");
            }
            break;
        }
            if (ctx->loop_top >= 0) {
                // Use continue label if available (for for loops), otherwise use start
                int continue_target = ctx->loop_continue[ctx->loop_top] ?
                    ctx->loop_continue[ctx->loop_top] : ctx->loop_start[ctx->loop_top];
                codegen_emit(ctx, "goto L%d;\n", continue_target);
            } else {
                codegen_emit(ctx, "/* continue outside loop */;\n");
            }
            break;
        }

        case NODE_EXPR_STMT: {
            codegen_expression(ctx, stmt->expr_stmt.expression);
            codegen_emit(ctx, ";\n");
            break;
        }
            if (stmt->expr_stmt.expression) {
                codegen_expression(ctx, stmt->expr_stmt.expression);
            }
            codegen_emit(ctx, ";\n");
            break;
        }

        case NODE_TRY_STMT: {
            int try_end = codegen_new_label(ctx);
            int catch_start = codegen_new_label(ctx);
            int finally_start = codegen_new_label(ctx);
            int try_finally_end = codegen_new_label(ctx);

            NodeList* try_block = stmt->try_stmt.try_block;
            while (try_block) {
                if (try_block->node) {
                    codegen_statement(ctx, try_block->node);
                }
                try_block = try_block->next;
            }
            codegen_emit_indent(ctx);
            codegen_emit(ctx, "goto L%d;\n", try_end);

            codegen_emit(ctx, "L%d:\n", catch_start);
            if (stmt->try_stmt.catch_clauses) {
                NodeList* catches = stmt->try_stmt.catch_clauses;
                while (catches) {
                    if (catches->node && catches->node->type == NODE_CATCH_CLAUSE) {
                        ASTNode* catch_clause = catches->node;
                        NodeList* body = catch_clause->catch_clause.body;
                        while (body) {
                            if (body->node) {
                                codegen_statement(ctx, body->node);
                            }
                            body = body->next;
                        }
                    }
                    catches = catches->next;
                }
            }
            codegen_emit_indent(ctx);
            codegen_emit(ctx, "goto L%d;\n", finally_start);

            codegen_emit(ctx, "L%d:\n", finally_start);
            if (stmt->try_stmt.finally_block) {
                NodeList* finally_block = stmt->try_stmt.finally_block;
                while (finally_block) {
                    if (finally_block->node) {
                        codegen_statement(ctx, finally_block->node);
                    }
                    finally_block = finally_block->next;
                }
            }
            codegen_emit(ctx, "L%d:\n", try_finally_end);
            codegen_emit(ctx, "L%d:\n", try_end);
            break;
        }

        default:
            codegen_emit(ctx, "/* ERROR: unhandled statement type %d */\n", stmt->type);
            break;
    }
}

void codegen_expression(CodeGenContext* ctx, ASTNode* expr) {
    if (!expr) return;

    switch (expr->type) {
        case NODE_INT_LITERAL:
            fprintf(stderr, "FOUND NODE_INT_LITERAL: value=%d\n", expr->int_literal.value);
            codegen_emit(ctx, "%d", expr->int_literal.value);
            break;

        case NODE_FLOAT_LITERAL:
            codegen_emit(ctx, "%g", expr->float_literal.value);
            break;

        case NODE_STRING_LITERAL:
            codegen_emit(ctx, "\"%s\"", expr->string_literal.value);
            break;

        case NODE_BOOL_LITERAL:
            codegen_emit(ctx, "%d", expr->bool_literal.value);
            break;

        case NODE_CHAR_LITERAL: {
            char c = expr->char_literal.value;
            if (c == '\n') {
                codegen_emit(ctx, "'\\n'");
            } else if (c == '\t') {
                codegen_emit(ctx, "'\\t'");
            } else if (c == '\r') {
                codegen_emit(ctx, "'\\r'");
            } else if (c == '\\') {
                codegen_emit(ctx, "'\\\\'");
            } else if (c == '\'') {
                codegen_emit(ctx, "'\\''");
            } else if (c == '"') {
                codegen_emit(ctx, "'\\\"'");
            } else if (c >= 32 && c <= 126) {
                codegen_emit(ctx, "'%c'", c);
            } else {
                codegen_emit(ctx, "'\\%03o'", (unsigned char)c);
            }
            break;
        }

        case NODE_COMPLEX_LITERAL: {
            const char* val = expr->complex_literal.value;
            if (!val) {
                codegen_emit(ctx, "(0.0 + 0.0 * I)");
                break;
            }

            char* val_copy = strdup(val);
            if (!val_copy) {
                codegen_emit(ctx, "(0.0 + 0.0 * I)");
                break;
            }

            double real = 0.0, imag = 0.0;
            int len = strlen(val_copy);

            int has_i = 0;
            if (len > 0 && (val_copy[len-1] == 'i' || val_copy[len-1] == 'j')) {
                has_i = 1;
                val_copy[len-1] = '\0';
            }

            char* plus = (len > 1) ? strchr(val_copy + 1, '+') : NULL;
            char* minus = (len > 1) ? strchr(val_copy + 1, '-') : NULL;
            char* sep = NULL;
            int is_plus = 0;

            if (plus && (!minus || plus < minus)) {
                sep = plus;
                is_plus = 1;
            } else if (minus) {
                sep = minus;
                is_plus = 0;
            }

            if (sep && has_i) {
                *sep = '\0';
                real = atof(val_copy);
                imag = atof(sep + 1);
                if (!is_plus) imag = -imag;
            } else if (has_i) {
                imag = atof(val_copy);
            } else {
                real = atof(val_copy);
            }

            free(val_copy);
            codegen_emit(ctx, "(%g + %g * I)", real, imag);
            break;
        }

        case NODE_IDENTIFIER:
            codegen_emit(ctx, "%s", expr->identifier.name);
            break;

        case NODE_BINARY_OP: {
            if (expr->binary_op.op == OP_POWER) {
                if (expr->inferred_type && expr->inferred_type->base_type == TYPE_COMPLEX) {
                    codegen_emit(ctx, "cpow(");
                } else {
                    codegen_emit(ctx, "pow(");
                }
                codegen_expression(ctx, expr->binary_op.left);
                codegen_emit(ctx, ", ");
                codegen_expression(ctx, expr->binary_op.right);
                codegen_emit(ctx, ")");
                break;
            }
            if (expr->binary_op.op == OP_FLOOR_DIV) {
                codegen_emit(ctx, "mathc_floor_div(");
                codegen_expression(ctx, expr->binary_op.left);
                codegen_emit(ctx, ", ");
                codegen_expression(ctx, expr->binary_op.right);
                codegen_emit(ctx, ")");
                break;
            }
            if (expr->binary_op.op == OP_MATMUL) {
                codegen_emit(ctx, "(");
                codegen_expression(ctx, expr->binary_op.left);
                codegen_emit(ctx, " * ");
                codegen_expression(ctx, expr->binary_op.right);
                codegen_emit(ctx, ")");
                break;
            }
            ASTNode* left = expr->binary_op.left;
            ASTNode* right = expr->binary_op.right;
            bool left_is_set = left->inferred_type && left->inferred_type->base_type == TYPE_SET;
            bool right_is_set = right->inferred_type && right->inferred_type->base_type == TYPE_SET;

            if (left_is_set || right_is_set) {
                switch (expr->binary_op.op) {
                    case OP_ADD:
                        codegen_emit(ctx, "set_union(");
                        codegen_expression(ctx, left);
                        codegen_emit(ctx, ", ");
                        codegen_expression(ctx, left);
                        codegen_emit(ctx, "__len, ");
                        codegen_expression(ctx, right);
                        codegen_emit(ctx, ", ");
                        codegen_expression(ctx, right);
                        codegen_emit(ctx, "__len)");
                        break;
                    case OP_SUB:
                        codegen_emit(ctx, "set_difference(");
                        codegen_expression(ctx, left);
                        codegen_emit(ctx, ", ");
                        codegen_expression(ctx, left);
                        codegen_emit(ctx, "__len, ");
                        codegen_expression(ctx, right);
                        codegen_emit(ctx, ", ");
                        codegen_expression(ctx, right);
                        codegen_emit(ctx, "__len)");
                        break;
                    case OP_IN:
                        codegen_emit(ctx, "set_contains(");
                        codegen_expression(ctx, right);
                        codegen_emit(ctx, ", ");
                        codegen_expression(ctx, right);
                        codegen_emit(ctx, "__len, ");
                        codegen_expression(ctx, left);
                        codegen_emit(ctx, ")");
                        break;
                    default:
                        codegen_emit(ctx, "/* unsupported set operation */");
                        break;
                }
            } else {
                codegen_emit(ctx, "(");
                codegen_expression(ctx, expr->binary_op.left);

                switch (expr->binary_op.op) {
                    case OP_ADD: codegen_emit(ctx, " + "); break;
                    case OP_SUB: codegen_emit(ctx, " - "); break;
                    case OP_MUL: codegen_emit(ctx, " * "); break;
                    case OP_DIV: codegen_emit(ctx, " / "); break;
                    case OP_MOD: codegen_emit(ctx, " %% "); break;
                    case OP_FLOOR_DIV: codegen_emit(ctx, " /*floor div already handled*/ "); break;
                    case OP_MATMUL: codegen_emit(ctx, " /*matmul already handled*/ "); break;
                    case OP_EQ: codegen_emit(ctx, " == "); break;
                    case OP_NEQ: codegen_emit(ctx, " != "); break;
                    case OP_LT: codegen_emit(ctx, " < "); break;
                    case OP_GT: codegen_emit(ctx, " > "); break;
                    case OP_LE: codegen_emit(ctx, " <= "); break;
                    case OP_GE: codegen_emit(ctx, " >= "); break;
                    case OP_AND: codegen_emit(ctx, " && "); break;
                    case OP_OR: codegen_emit(ctx, " || "); break;
                    case OP_IN: codegen_emit(ctx, " /*in operator*/ "); break;
                    default: codegen_emit(ctx, " /*op?*/ "); break;
                }

                codegen_expression(ctx, expr->binary_op.right);
                codegen_emit(ctx, ")");
            }
            break;
        }

        case NODE_UNARY_OP: {
            switch (expr->unary_op.op) {
                case OP_UNARY_PLUS:
                    codegen_emit(ctx, "(+");
                    codegen_expression(ctx, expr->unary_op.operand);
                    codegen_emit(ctx, ")");
                    break;
                case OP_UNARY_MINUS:
                    codegen_emit(ctx, "(-");
                    codegen_expression(ctx, expr->unary_op.operand);
                    codegen_emit(ctx, ")");
                    break;
                case OP_NOT:
                    codegen_emit(ctx, "(!");
                    codegen_expression(ctx, expr->unary_op.operand);
                    codegen_emit(ctx, ")");
                    break;
                case OP_PRE_INC:
                    codegen_emit(ctx, "(++");
                    codegen_expression(ctx, expr->unary_op.operand);
                    codegen_emit(ctx, ")");
                    break;
                case OP_PRE_DEC:
                    codegen_emit(ctx, "(--");
                    codegen_expression(ctx, expr->unary_op.operand);
                    codegen_emit(ctx, ")");
                    break;
                case OP_POST_INC:
                    codegen_emit(ctx, "(");
                    codegen_expression(ctx, expr->unary_op.operand);
                    codegen_emit(ctx, "++)");
                    break;
                case OP_POST_DEC:
                    codegen_emit(ctx, "(");
                    codegen_expression(ctx, expr->unary_op.operand);
                    codegen_emit(ctx, "--)");
                    break;
                default:
                    codegen_expression(ctx, expr->unary_op.operand);
                    break;
            }
            break;
        }

        case NODE_CALL: {
            if (expr->call.function && expr->call.function->type == NODE_IDENTIFIER) {
                char* func_name = expr->call.function->identifier.name;
                if (strcmp(func_name, "len") == 0) {
                    if (expr->call.arguments && expr->call.arguments->node && expr->call.arguments->node->type == NODE_IDENTIFIER) {
                        codegen_emit(ctx, "%s__len", expr->call.arguments->node->identifier.name);
                        return;
                    }
                }
                if (strcmp(func_name, "len") == 0) {
                    if (expr->call.arguments && expr->call.arguments->node) {
                        ASTNode* arg = expr->call.arguments->node;
                        if (arg->type == NODE_IDENTIFIER) {
                            codegen_emit(ctx, "%s__len", arg->identifier.name);
                            return;
                        }
                    }
                    codegen_emit(ctx, "0");
                    return;
                }

                if (strcmp(func_name, "print") == 0) {
                    if (expr->call.arguments && expr->call.arguments->node) {
                        ASTNode* arg = expr->call.arguments->node;
                        TypeSpec* arg_type = arg->inferred_type;

                        if (arg_type) {
                            switch (arg_type->base_type) {
                                case TYPE_INT:
                                case TYPE_BOOL:
                                    codegen_emit(ctx, "mathc_print_int(");
                                    break;
                                case TYPE_FLOAT:
                                    codegen_emit(ctx, "mathc_print_float(");
                                    break;
                                case TYPE_STRING:
                                    codegen_emit(ctx, "mathc_print_string(");
                                    break;
                                case TYPE_COMPLEX:
                                    codegen_emit(ctx, "mathc_print_complex(");
                                    break;
                                case TYPE_CHAR:
                                    codegen_emit(ctx, "mathc_print_char(");
                                    break;
                                case TYPE_ARRAY:
                                    codegen_emit(ctx, "mathc_print_array(");
                                    if (arg->type == NODE_IDENTIFIER) {
                                        codegen_expression(ctx, arg);
                                        codegen_emit(ctx, ", %s__len", arg->identifier.name);
                                    } else {
                                        // Para expressões que resultam em arrays, isso é mais complexo
                                        // Por enquanto, apenas o ponteiro
                                        codegen_expression(ctx, arg);
                                        codegen_emit(ctx, ", 0");
                                    }
                                    codegen_emit(ctx, ")");
                                    return;
                                default:
                                    codegen_emit(ctx, "mathc_print_int(");
                                    break;
                            }
                        } else {
                            if (arg->type == NODE_INT_LITERAL || arg->type == NODE_BOOL_LITERAL) {
                                codegen_emit(ctx, "mathc_print_int(");
                            } else if (arg->type == NODE_FLOAT_LITERAL) {
                                codegen_emit(ctx, "mathc_print_float(");
                            } else if (arg->type == NODE_STRING_LITERAL) {
                                codegen_emit(ctx, "mathc_print_string(");
                            } else if (arg->type == NODE_COMPLEX_LITERAL) {
                                codegen_emit(ctx, "mathc_print_complex(");
                            } else if (arg->type == NODE_CHAR_LITERAL) {
                                codegen_emit(ctx, "mathc_print_char(");
                            } else {
                                codegen_emit(ctx, "mathc_print_float(");
                            }
                        }
                        codegen_expression(ctx, arg);
                        codegen_emit(ctx, ")");
                        return;
                    }
                } else if (strcmp(func_name, "sqrt") == 0 ||
                          strcmp(func_name, "sin") == 0 ||
                          strcmp(func_name, "cos") == 0 ||
                          strcmp(func_name, "tan") == 0 ||
                          strcmp(func_name, "exp") == 0 ||
                          strcmp(func_name, "log") == 0) {
                    codegen_emit(ctx, "%s(", func_name);
                    NodeList* args = expr->call.arguments;
                    int first = 1;
                    while (args) {
                        if (!first) codegen_emit(ctx, ", ");
                        codegen_expression(ctx, args->node);
                        first = 0;
                        args = args->next;
                    }
                    codegen_emit(ctx, ")");
                    return;
                }
            }

            codegen_expression(ctx, expr->call.function);
            codegen_emit(ctx, "(");

            NodeList* args = expr->call.arguments;
            int first = 1;
            while (args) {
                if (!first) codegen_emit(ctx, ", ");

                ASTNode* arg = args->node;

                // Se o argumento é um array, passar também o tamanho
                if (arg->type == NODE_IDENTIFIER && arg->inferred_type &&
                    arg->inferred_type->base_type == TYPE_ARRAY) {
                    codegen_expression(ctx, arg);
                    codegen_emit(ctx, ", %s__len", arg->identifier.name);
                } else {
                    codegen_expression(ctx, arg);
                }

                first = 0;
                args = args->next;
            }
            codegen_emit(ctx, ")");
            break;
        }

        case NODE_ARRAY_ACCESS: {
            codegen_expression(ctx, expr->array_access.array);
            codegen_emit(ctx, "[");
            codegen_expression(ctx, expr->array_access.index);
            codegen_emit(ctx, "]");
            break;
        }

        case NODE_MEMBER_ACCESS:
            codegen_member_access(ctx, expr);
            break;

        case NODE_ARRAY_LITERAL:
            codegen_array_literal(ctx, expr);
            break;
        case NODE_MATRIX_LITERAL:
            codegen_matrix_literal(ctx, expr);
            break;
        case NODE_SET_LITERAL:
            codegen_set_literal(ctx, expr);
            break;

        default:
            codegen_emit(ctx, "/* unhandled expr type %d */", expr->type);
            break;
    }
}

// Funções auxiliares para literais compostos
int count_literal_elements(ASTNode* literal) {
    if (!literal) return 0;

    switch (literal->type) {
        case NODE_ARRAY_LITERAL: {
            int count = 0;
            NodeList* el = literal->array_literal.elements;
            while (el) { count++; el = el->next; }
            return count;
        }
        case NODE_MATRIX_LITERAL: {
            int count = 0;
            NodeList* el = literal->matrix_literal.rows;
            while (el) { count++; el = el->next; }
            return count;
        }
        case NODE_SET_LITERAL: {
            int count = 0;
            NodeList* el = literal->set_literal.elements;
            while (el) { count++; el = el->next; }
            return count;
        }
        default:
            return 0;
    }
}

void codegen_literal_elements(CodeGenContext* ctx, ASTNode* literal, const char* var_name) {
    switch (literal->type) {
        case NODE_ARRAY_LITERAL: {
            NodeList* el = literal->array_literal.elements;
            int idx = 0;
            while (el) {
                codegen_emit(ctx, "\n");
                codegen_emit_indent(ctx);
                codegen_emit(ctx, "%s[%d] = ", var_name, idx);
                codegen_expression(ctx, el->node);
                codegen_emit(ctx, ";");
                el = el->next;
                idx++;
            }
            break;
        }
        case NODE_MATRIX_LITERAL: {
            NodeList* row_el = literal->matrix_literal.rows;
            int row = 0;
            while (row_el) {
                if (row_el->node->type == NODE_ARRAY_LITERAL) {
                    NodeList* col_el = row_el->node->array_literal.elements;
                    int col = 0;
                    while (col_el) {
                        codegen_emit(ctx, "\n");
                        codegen_emit_indent(ctx);
                        codegen_emit(ctx, "%s[%d][%d] = ", var_name, row, col);
                        codegen_expression(ctx, col_el->node);
                        codegen_emit(ctx, ";");
                        col_el = col_el->next;
                        col++;
                    }
                }
                row_el = row_el->next;
                row++;
            }
            break;
        }
        case NODE_SET_LITERAL: {
            NodeList* el = literal->set_literal.elements;
            int idx = 0;
            while (el) {
                codegen_emit(ctx, "\n");
                codegen_emit_indent(ctx);
                codegen_emit(ctx, "%s[%d] = ", var_name, idx);
                codegen_expression(ctx, el->node);
                codegen_emit(ctx, ";");
                el = el->next;
                idx++;
            }
            break;
        }
    }
}

void codegen_array_literal(CodeGenContext* ctx, ASTNode* literal) {
    // Isso não deveria ser chamado diretamente - literais são tratados nas declarações
    codegen_emit(ctx, "/* ERROR: array literal used in expression */");
}

void codegen_matrix_literal(CodeGenContext* ctx, ASTNode* literal) {
    // Isso não deveria ser chamado diretamente - literais são tratados nas declarações
    codegen_emit(ctx, "/* ERROR: matrix literal used in expression */");
}

void codegen_set_literal(CodeGenContext* ctx, ASTNode* literal) {
    // Isso não deveria ser chamado diretamente - literais são tratados nas declarações
    codegen_emit(ctx, "/* ERROR: set literal used in expression */");
}

void codegen_program(CodeGenContext* ctx, ASTNode* program) {
    if (!program) return;
    DBG("program start");
    if (!program || program->type != NODE_PROGRAM) { DBG("not a program node"); return; }

    fputs("#include <stdio.h>\n", ctx->output);
    fputs("#include <stdlib.h>\n", ctx->output);
    fputs("#include <string.h>\n", ctx->output);
    fputs("#include <math.h>\n", ctx->output);
    fputs("#include <complex.h>\n\n", ctx->output);
    codegen_runtime_helpers(ctx);
    fputs("// Builtin functions\n", ctx->output);
    fputs("void mathc_print_int(int x) { printf(\"%d\\n\", x); }\n", ctx->output);
    fputs("void mathc_print_float(double x) { printf(\"%g\\n\", x); }\n", ctx->output);
    fputs("void mathc_print_string(char* x) { printf(\"%s\\n\", x); }\n", ctx->output);
    fputs("void mathc_print_char(char x) { printf(\"%c\\n\", x); }\n", ctx->output);
    fputs("void mathc_print_complex(double complex x) { printf(\"%g%+gi\\n\", creal(x), cimag(x)); }\n", ctx->output);
    fputs("void mathc_print_array(int* arr, int len) {\n", ctx->output);
    fputs("    printf(\"[\");\n", ctx->output);
    fputs("    for (int i = 0; i < len; i++) {\n", ctx->output);
    fputs("        if (i > 0) printf(\", \");\n", ctx->output);
    fputs("        printf(\"%d\", arr[i]);\n", ctx->output);
    fputs("    }\n", ctx->output);
    fputs("    printf(\"]\\n\");\n", ctx->output);
    fputs("}\n\n", ctx->output);

    NodeList* defs = program->program.definitions;
    while (defs) { if (defs->node && defs->node->type == NODE_TYPE_DEF) codegen_struct(ctx, defs->node); defs = defs->next; }

    ASTNode* main_func = NULL;
    defs = program->program.definitions;
    while (defs) {
        if (defs->node && defs->node->type == NODE_FUNCTION_DEF) {
            if (defs->node->function_def.name && strcmp(defs->node->function_def.name, "main") == 0) {
                main_func = defs->node;
            } else {
                codegen_function(ctx, defs->node);
            }
        }
        defs = defs->next;
    }

    if (program->program.main_function) {
        codegen_function(ctx, program->program.main_function);
    } else if (main_func) {
        codegen_function(ctx, main_func);
    } else {
        fputs("int main(void) {\n", ctx->output);
        fputs("    return 0;\n", ctx->output);
        fputs("}\n", ctx->output);
    }

    fputs("/* global allocations cleanup */\n", ctx->output);
    codegen_emit_frees(ctx);
    DBG("program end");
}
