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

#define CG_DEBUG 0
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
    fputs("typedef struct { int start; int end; int step; } range_t;\n", ctx->output);
    fputs("static inline range_t make_range(int s, int e, int st){ range_t r={s,e,st}; return r; }\n", ctx->output);
    fputs("static inline int range_has_next(range_t* r, int i){ return r->step>0 ? i<r->end : i>r->end; }\n", ctx->output);
    fputs("static inline int mathc_floor_div(int a, int b){ if (b == 0) { fprintf(stderr, \"Erro: Divisao por zero\\n\"); exit(1); } int q = a / b; int r = a % b; if ((r != 0) && ((r > 0) != (b > 0))) q--; return q; }\n", ctx->output);
    fputs("static inline double max(double a, double b) { return (a > b) ? a : b; }\n", ctx->output);
    fputs("static inline double min(double a, double b) { return (a < b) ? a : b; }\n", ctx->output);
    fputs("static inline double mathc_sum(int* arr, int len) {\n", ctx->output);
    fputs("    double s = 0.0; int i = 0; goto L_sum_check;\n", ctx->output);
    fputs("L_sum_body: s += arr[i]; i++;\n", ctx->output);
    fputs("L_sum_check: if (i < len) goto L_sum_body; return s;\n", ctx->output);
    fputs("}\n", ctx->output);

    fputs("static inline double mathc_mean(int* arr, int len) {\n", ctx->output);
    fputs("    if (len == 0) return 0.0; return mathc_sum(arr, len) / len;\n", ctx->output);
    fputs("}\n", ctx->output);

    fputs("static inline double mathc_prod(int* arr, int len) {\n", ctx->output);
    fputs("    double p = 1.0; int i = 0; goto L_prod_check;\n", ctx->output);
    fputs("L_prod_body: p *= arr[i]; i++;\n", ctx->output);
    fputs("L_prod_check: if (i < len) goto L_prod_body; return p;\n", ctx->output);
    fputs("}\n", ctx->output);

    fputs("static inline double mathc_variance(int* arr, int len) {\n", ctx->output);
    fputs("    if (len == 0) return 0.0;\n", ctx->output);
    fputs("    double m = mathc_mean(arr, len); double var = 0.0; int i = 0; goto L_var_check;\n", ctx->output);
    fputs("L_var_body: { double diff = arr[i] - m; var += diff * diff; i++; }\n", ctx->output);
    fputs("L_var_check: if (i < len) goto L_var_body; return var / len;\n", ctx->output);
    fputs("}\n", ctx->output);

    fputs("static inline double mathc_std(int* arr, int len) { return sqrt(mathc_variance(arr, len)); }\n", ctx->output);

    fputs("static inline void mathc_sort(int* arr, int len) {\n", ctx->output);
    fputs("    int i = 0; goto L_sort_i_check;\n", ctx->output);
    fputs("L_sort_i_body: { int j = i + 1; goto L_sort_j_check;\n", ctx->output);
    fputs("L_sort_j_body: if (arr[j] < arr[i]) { int tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp; } j++;\n", ctx->output);
    fputs("L_sort_j_check: if (j < len) goto L_sort_j_body; i++; }\n", ctx->output);
    fputs("L_sort_i_check: if (i < len - 1) goto L_sort_i_body;\n", ctx->output);
    fputs("}\n", ctx->output);

    fputs("static inline void mathc_reverse(int* arr, int len) {\n", ctx->output);
    fputs("    int i = 0; int j = len - 1; goto L_rev_check;\n", ctx->output);
    fputs("L_rev_body: { int tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp; i++; j--; }\n", ctx->output);
    fputs("L_rev_check: if (i < j) goto L_rev_body;\n", ctx->output);
    fputs("}\n", ctx->output);

    fputs("static inline int mathc_pop(int* arr, int* len_ptr) {\n", ctx->output);
    fputs("    if (*len_ptr == 0) return 0;\n", ctx->output);
    fputs("    (*len_ptr)--; return arr[*len_ptr];\n", ctx->output);
    fputs("}\n\n", ctx->output);

    // Funções matemáticas avançadas
    fputs("static inline int mathc_factorial(int n) {\n", ctx->output);
    fputs("    if (n < 0) return 0; if (n <= 1) return 1;\n", ctx->output);
    fputs("    int result = 1; int i = 2; goto L_fact_check;\n", ctx->output);
    fputs("L_fact_body: result *= i; i++;\n", ctx->output);
    fputs("L_fact_check: if (i <= n) goto L_fact_body; return result;\n", ctx->output);
    fputs("}\n", ctx->output);

    fputs("static inline int mathc_gcd(int a, int b) {\n", ctx->output);
    fputs("    if (a < 0) a = -a; if (b < 0) b = -b;\n", ctx->output);
    fputs("    goto L_gcd_check;\n", ctx->output);
    fputs("L_gcd_body: { int temp = b; b = a % b; a = temp; }\n", ctx->output);
    fputs("L_gcd_check: if (b != 0) goto L_gcd_body; return a;\n", ctx->output);
    fputs("}\n", ctx->output);

    fputs("static inline int mathc_lcm(int a, int b) {\n", ctx->output);
    fputs("    if (a == 0 || b == 0) return 0;\n", ctx->output);
    fputs("    int g = mathc_gcd(a, b);\n", ctx->output);
    fputs("    if (a < 0) a = -a; if (b < 0) b = -b;\n", ctx->output);
    fputs("    return (a / g) * b;\n", ctx->output);
    fputs("}\n", ctx->output);

    fputs("static inline int mathc_isprime(int n) {\n", ctx->output);
    fputs("    if (n < 2) return 0;\n", ctx->output);
    fputs("    if (n == 2) return 1;\n", ctx->output);
    fputs("    if (n % 2 == 0) return 0;\n", ctx->output);
    fputs("    int i = 3; int limit = (int)sqrt(n); goto L_prime_check;\n", ctx->output);
    fputs("L_prime_body: if (n % i == 0) return 0; i += 2;\n", ctx->output);
    fputs("L_prime_check: if (i <= limit) goto L_prime_body; return 1;\n", ctx->output);
    fputs("}\n", ctx->output);

    fputs("static inline int mathc_combinations(int n, int k) {\n", ctx->output);
    fputs("    if (k > n || k < 0) return 0;\n", ctx->output);
    fputs("    if (k == 0 || k == n) return 1;\n", ctx->output);
    fputs("    if (k > n - k) k = n - k;\n", ctx->output);
    fputs("    int result = 1; int i = 0; goto L_comb_check;\n", ctx->output);
    fputs("L_comb_body: result = result * (n - i) / (i + 1); i++;\n", ctx->output);
    fputs("L_comb_check: if (i < k) goto L_comb_body; return result;\n", ctx->output);
    fputs("}\n", ctx->output);

    fputs("static inline int mathc_permutations(int n, int k) {\n", ctx->output);
    fputs("    if (k > n || k < 0) return 0;\n", ctx->output);
    fputs("    int result = 1; int i = 0; goto L_perm_check;\n", ctx->output);
    fputs("L_perm_body: result *= (n - i); i++;\n", ctx->output);
    fputs("L_perm_check: if (i < k) goto L_perm_body; return result;\n", ctx->output);
    fputs("}\n\n", ctx->output);

    // Funções adicionais para arrays
    fputs("static inline int mathc_min_element(int* arr, int len) {\n", ctx->output);
    fputs("    if (len == 0) return 0;\n", ctx->output);
    fputs("    int min_val = arr[0]; int i = 1; goto L_minelem_check;\n", ctx->output);
    fputs("L_minelem_body: if (arr[i] < min_val) min_val = arr[i]; i++;\n", ctx->output);
    fputs("L_minelem_check: if (i < len) goto L_minelem_body; return min_val;\n", ctx->output);
    fputs("}\n", ctx->output);

    fputs("static inline int mathc_max_element(int* arr, int len) {\n", ctx->output);
    fputs("    if (len == 0) return 0;\n", ctx->output);
    fputs("    int max_val = arr[0]; int i = 1; goto L_maxelem_check;\n", ctx->output);
    fputs("L_maxelem_body: if (arr[i] > max_val) max_val = arr[i]; i++;\n", ctx->output);
    fputs("L_maxelem_check: if (i < len) goto L_maxelem_body; return max_val;\n", ctx->output);
    fputs("}\n", ctx->output);

    fputs("static inline int mathc_find(int* arr, int len, int val) {\n", ctx->output);
    fputs("    int i = 0; goto L_find_check;\n", ctx->output);
    fputs("L_find_body: if (arr[i] == val) return i; i++;\n", ctx->output);
    fputs("L_find_check: if (i < len) goto L_find_body; return -1;\n", ctx->output);
    fputs("}\n", ctx->output);

    fputs("static inline int mathc_count(int* arr, int len, int val) {\n", ctx->output);
    fputs("    int cnt = 0; int i = 0; goto L_count_check;\n", ctx->output);
    fputs("L_count_body: if (arr[i] == val) cnt++; i++;\n", ctx->output);
    fputs("L_count_check: if (i < len) goto L_count_body; return cnt;\n", ctx->output);
    fputs("}\n\n", ctx->output);

    // Wrapper para sqrt que converte resultado complexo quando necessário
    fputs("static inline double complex mathc_sqrt_safe(double x) {\n", ctx->output);
    fputs("    if (x < 0) return csqrt(x);\n", ctx->output);
    fputs("    return sqrt(x) + 0.0 * I;\n", ctx->output);
    fputs("}\n\n", ctx->output);

    // Funções para números complexos
    fputs("static inline double mathc_real(double complex z) { return creal(z); }\n", ctx->output);
    fputs("static inline double mathc_imag(double complex z) { return cimag(z); }\n", ctx->output);
    fputs("static inline double complex mathc_conjugate(double complex z) { return conj(z); }\n", ctx->output);
    fputs("static inline double mathc_phase(double complex z) { return carg(z); }\n", ctx->output);
    fputs("static inline double mathc_magnitude(double complex z) { return cabs(z); }\n\n", ctx->output);

    fputs("static inline int mathc_cardinality(int* set, int len) { return len; }\n\n", ctx->output);

    fputs("typedef struct { int* ptr; int len; } set_result_t;\n", ctx->output);

    // set_union com labels/goto
    fputs("static inline set_result_t set_union(int* a, int a_len, int* b, int b_len) {\n", ctx->output);
    fputs("    int* result = malloc(sizeof(int) * (a_len + b_len));\n", ctx->output);
    fputs("    int result_len = 0;\n", ctx->output);
    fputs("    int i = 0; goto L_u_copy_check;\n", ctx->output);
    fputs("L_u_copy_body: result[result_len++] = a[i]; i++;\n", ctx->output);
    fputs("L_u_copy_check: if (i < a_len) goto L_u_copy_body;\n", ctx->output);
    fputs("    i = 0; goto L_u_bcheck;\n", ctx->output);
    fputs("L_u_bbody: { int found = 0; int j = 0; goto L_u_bjcheck;\n", ctx->output);
    fputs("L_u_bjbody: if (b[i] == a[j]) { found = 1; goto L_u_bjend; } j++;\n", ctx->output);
    fputs("L_u_bjcheck: if (j < a_len) goto L_u_bjbody;\n", ctx->output);
    fputs("L_u_bjend: if (!found) { result[result_len++] = b[i]; } i++; }\n", ctx->output);
    fputs("L_u_bcheck: if (i < b_len) goto L_u_bbody;\n", ctx->output);
    fputs("    set_result_t r = {result, result_len}; return r;\n", ctx->output);
    fputs("}\n", ctx->output);

    // set_difference com labels/goto
    fputs("static inline set_result_t set_difference(int* a, int a_len, int* b, int b_len) {\n", ctx->output);
    fputs("    int* result = malloc(sizeof(int) * a_len);\n", ctx->output);
    fputs("    int result_len = 0;\n", ctx->output);
    fputs("    int i = 0; goto L_d_acheck;\n", ctx->output);
    fputs("L_d_abody: { int found = 0; int j = 0; goto L_d_bcheck;\n", ctx->output);
    fputs("L_d_bbody: if (a[i] == b[j]) { found = 1; goto L_d_bend; } j++;\n", ctx->output);
    fputs("L_d_bcheck: if (j < b_len) goto L_d_bbody;\n", ctx->output);
    fputs("L_d_bend: if (!found) { result[result_len++] = a[i]; } i++; }\n", ctx->output);
    fputs("L_d_acheck: if (i < a_len) goto L_d_abody;\n", ctx->output);
    fputs("    set_result_t r = {result, result_len}; return r;\n", ctx->output);
    fputs("}\n", ctx->output);

    // set_contains com labels/goto
    fputs("static inline int set_contains(int* set, int set_len, int element) {\n", ctx->output);
    fputs("    int i = 0; goto L_c_check;\n", ctx->output);
    fputs("L_c_body: if (set[i] == element) return 1; i++;\n", ctx->output);
    fputs("L_c_check: if (i < set_len) goto L_c_body; return 0;\n", ctx->output);
    fputs("}\n", ctx->output);

    // set_is_subset com labels/goto
    fputs("static inline int set_is_subset(int* a, int a_len, int* b, int b_len) {\n", ctx->output);
    fputs("    int i = 0; goto L_s_check;\n", ctx->output);
    fputs("L_s_body: if (!set_contains(b, b_len, a[i])) return 0; i++;\n", ctx->output);
    fputs("L_s_check: if (i < a_len) goto L_s_body; return 1;\n", ctx->output);
    fputs("}\n\n", ctx->output);
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
    if (!member || member->type != NODE_MEMBER_ACCESS) { codegen_emit(ctx, "/* membro inválido */"); return; }

    ASTNode* obj = member->member_access.object;
    const char* method = member->member_access.member;

    if (obj->inferred_type && obj->inferred_type->base_type == TYPE_ARRAY) {
        if (strcmp(method, "add") == 0 && member->member_access.args) {
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

    if (obj->inferred_type && obj->inferred_type->type_name) {
        if (!member->member_access.args) {
            codegen_expression(ctx, obj);
            if (obj->inferred_type->base_type == TYPE_CUSTOM && obj->type == NODE_IDENTIFIER) {
                codegen_emit(ctx, "->%s", method);
            } else {
                codegen_emit(ctx, ".%s", method);
            }
            return;
        } else {
            codegen_emit(ctx, "%s_%s(", obj->inferred_type->type_name, method);
            codegen_expression(ctx, obj);
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

    codegen_expression(ctx, member->member_access.object);
    codegen_emit(ctx, "->%s", member->member_access.member);
    if (member->member_access.args) {
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
        int continueL = codegen_new_label(ctx);
        codegen_loop_push(ctx, startL, endL, continueL);
        codegen_emit(ctx, "L%d:\n", startL);
        codegen_emit_indent(ctx); codegen_emit(ctx, "if (!range_has_next(&%s_range, %s_index)) goto L%d;\n", iterName, iterName, endL);
        // Bloco por iteração
        codegen_emit_indent(ctx); codegen_emit(ctx, "{\n"); ctx->indent_level++;
        codegen_emit_indent(ctx); codegen_emit(ctx, "int %s = %s_index;\n", iterName, iterName);
        // Corpo
        NodeList* body = for_node->for_stmt.body; while (body){ if (body->node) codegen_statement(ctx, body->node); body=body->next; }
        ctx->indent_level--; codegen_emit_indent(ctx); codegen_emit(ctx, "}\n");
        // Continue + incremento
        codegen_emit(ctx, "L%d:\n", continueL);
        codegen_emit_indent(ctx); codegen_emit(ctx, "%s_index += %s_range.step;\n", iterName, iterName);
        codegen_emit_indent(ctx); codegen_emit(ctx, "goto L%d;\n", startL);
        codegen_emit(ctx, "L%d:\n", endL);
        codegen_loop_pop(ctx);
        return;
    }

    if (iterable->type == NODE_ARRAY_LITERAL) {
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
        // Bloco por iteração
        codegen_emit_indent(ctx); codegen_emit(ctx, "{\n"); ctx->indent_level++;
        codegen_emit_indent(ctx); codegen_emit(ctx, "int %s = _arr%d[_i%d];\n", iterName, arrId, arrId);
        if (idxName) { codegen_emit_indent(ctx); codegen_emit(ctx, "int %s = _i%d;\n", idxName, arrId); }
        NodeList* body = for_node->for_stmt.body; while (body){ if (body->node) codegen_statement(ctx, body->node); body=body->next; }
        ctx->indent_level--; codegen_emit_indent(ctx); codegen_emit(ctx, "}\n");
        codegen_emit(ctx, "L%d:\n", continueL);
        codegen_emit_indent(ctx); codegen_emit(ctx, "_i%d++;\n", arrId);
        codegen_emit_indent(ctx); codegen_emit(ctx, "goto L%d;\n", startL);
        codegen_emit(ctx, "L%d:\n", endL);
        codegen_loop_pop(ctx);
        return;
    }

    if (iterable->type == NODE_IDENTIFIER) {
        int arrId = codegen_new_temp(ctx);
        int continueL = codegen_new_label(ctx);
        codegen_loop_push(ctx, startL, endL, continueL);
        codegen_emit_indent(ctx); codegen_emit(ctx, "int _len%d = %s__len;\n", arrId, iterable->identifier.name);
        codegen_emit_indent(ctx); codegen_emit(ctx, "int _i%d = 0;\n", arrId);
        codegen_emit(ctx, "L%d:\n", startL);
        codegen_emit_indent(ctx); codegen_emit(ctx, "if (_i%d >= _len%d) goto L%d;\n", arrId, arrId, endL);
        // Bloco por iteração
        codegen_emit_indent(ctx); codegen_emit(ctx, "{\n"); ctx->indent_level++;
        codegen_emit_indent(ctx); codegen_emit(ctx, "int %s = %s[_i%d];\n", iterName, iterable->identifier.name, arrId);
        if (idxName) { codegen_emit_indent(ctx); codegen_emit(ctx, "int %s = _i%d;\n", idxName, arrId); }
        NodeList* body = for_node->for_stmt.body; while (body){ if (body->node) codegen_statement(ctx, body->node); body=body->next; }
        ctx->indent_level--; codegen_emit_indent(ctx); codegen_emit(ctx, "}\n");
        codegen_emit(ctx, "L%d:\n", continueL);
        codegen_emit_indent(ctx); codegen_emit(ctx, "_i%d++;\n", arrId);
        codegen_emit_indent(ctx); codegen_emit(ctx, "goto L%d;\n", startL);
        codegen_emit(ctx, "L%d:\n", endL);
        codegen_loop_pop(ctx);
        return;
    }

    // Fallback
    codegen_emit_indent(ctx); codegen_emit(ctx, "/* iterador nao suportado */\n");
}

void codegen_struct(CodeGenContext* ctx, ASTNode* type_def) {
    if (!type_def || type_def->type != NODE_TYPE_DEF) return;
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
    DBG("funcao %s", func && func->type == NODE_FUNCTION_DEF ? func->function_def.name : "<invalida>");
    if (!func) { DBG("null func"); return; }
    if (func->type != NODE_FUNCTION_DEF) { DBG("wrong node type: %d", func->type); return; }

    const char* ret_type = "void";
    int ret_is_custom = 0;
    if (func->function_def.return_type) {
        ret_type = codegen_type_to_c(func->function_def.return_type);
        ret_is_custom = (func->function_def.return_type->base_type == TYPE_CUSTOM);
    }
    const char* func_name = func->function_def.name ? func->function_def.name : "fn";
    
    int is_main = (strcmp(func_name, "main") == 0);
    if (is_main) {
        ret_type = "int";
        ret_is_custom = 0;
    }
    
    DBG("func_name ptr=%p text=%s", (void*)func->function_def.name, func_name);
    if (ret_is_custom) {
        fputs(ret_type, ctx->output);
        fputs("* ", ctx->output);
    } else {
        fputs(ret_type, ctx->output);
        fputc(' ', ctx->output);
    }
    fputs(func_name, ctx->output);
    fputc('(', ctx->output);
    fflush(ctx->output);

    ParamList* params = func->function_def.parameters;

    int first = 1;
    while (params) {
        const char* pname = params->name ? params->name : "arg";

        if (params->type && params->type->base_type == TYPE_ARRAY) {
            const char* elem_type = params->type->element_type ?
                codegen_type_to_c(params->type->element_type) : "int";

            if (!first) codegen_emit(ctx, ", ");
            codegen_emit(ctx, "%s* %s, int %s__len", elem_type, pname, pname);
            first = 0;
        } else {
            const char* ptype = codegen_type_to_c(params->type);
            if (!first) codegen_emit(ctx, ", ");
            if (params->type && params->type->base_type == TYPE_CUSTOM) {
                codegen_emit(ctx, "%s* %s", ptype ? ptype : "void", pname);
            } else {
                codegen_emit(ctx, "%s %s", ptype ? ptype : "int", pname);
            }
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


            // Primeiro trata arrays multidimensionais
            if (var_type && var_type->base_type == TYPE_ARRAY && var_type->element_type &&
                var_type->element_type->base_type == TYPE_ARRAY) {

                if (stmt->declaration.initializer && stmt->declaration.initializer->type == NODE_ARRAY_LITERAL &&
                    stmt->declaration.initializer->array_literal.elements &&
                    stmt->declaration.initializer->array_literal.elements->node &&
                    stmt->declaration.initializer->array_literal.elements->node->type == NODE_ARRAY_LITERAL) {

                    TypeSpec* elem_type = var_type->element_type->element_type;
                    const char* elem_c_type = elem_type ? codegen_type_to_c(elem_type) : "int";

                    int rows = count_literal_elements(stmt->declaration.initializer);
                    int cols = 0;

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
                    if (stmt->declaration.initializer &&
                        stmt->declaration.initializer->type == NODE_BINARY_OP &&
                        (stmt->declaration.initializer->binary_op.op == OP_ADD ||
                         stmt->declaration.initializer->binary_op.op == OP_SUB)) {
                        int temp_id = codegen_new_temp(ctx);
                        codegen_emit(ctx, "set_result_t _tmp%d = ", temp_id);
                        codegen_expression(ctx, stmt->declaration.initializer);
                        codegen_emit(ctx, ";\n");
                        codegen_emit_indent(ctx);
                        codegen_emit(ctx, "%s* %s = _tmp%d.ptr;\n", elem_c_type, stmt->declaration.name, temp_id);
                        codegen_emit_indent(ctx);
                        codegen_emit(ctx, "int %s__len = _tmp%d.len;\n", stmt->declaration.name, temp_id);
                    } else {
                        codegen_emit(ctx, "%s* %s", elem_c_type, stmt->declaration.name);
                        if (stmt->declaration.initializer) {
                            codegen_emit(ctx, " = ");
                            codegen_expression(ctx, stmt->declaration.initializer);
                        }
                        codegen_emit(ctx, ";\n");
                        codegen_emit_indent(ctx);
                        codegen_emit(ctx, "int %s__len = 0;\n", stmt->declaration.name);
                    }
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
                    // Loop de alocação por linhas usando labels/goto
                    {
                        int Ls = codegen_new_label(ctx);
                        int Le = codegen_new_label(ctx);
                        int tid = codegen_new_temp(ctx);
                        codegen_emit(ctx, "int _i%d = 0;\n", tid);
                        codegen_emit_indent(ctx);
                        codegen_emit(ctx, "L%d:\n", Ls);
                        codegen_emit_indent(ctx);
                        codegen_emit(ctx, "if (!(_i%d < %d)) goto L%d;\n", tid, rows, Le);
                        codegen_emit_indent(ctx);
                        codegen_emit(ctx, "%s[_i%d] = malloc(sizeof(%s) * %d);\n", stmt->declaration.name, tid, elem_c_type, cols);
                        codegen_emit_indent(ctx);
                        codegen_emit(ctx, "_i%d++;\n", tid);
                        codegen_emit_indent(ctx);
                        codegen_emit(ctx, "goto L%d;\n", Ls);
                        codegen_emit(ctx, "L%d:\n", Le);
                    }

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
            }

            if (var_type && var_type->base_type == TYPE_CUSTOM) {
                if (stmt->declaration.initializer) {
                    codegen_emit(ctx, "%s* %s = ", type_str, stmt->declaration.name);
                    codegen_expression(ctx, stmt->declaration.initializer);
                    codegen_emit(ctx, ";\n");
                } else {
                    codegen_emit(ctx, "%s* %s = malloc(sizeof(%s));\n", type_str, stmt->declaration.name, type_str);
                    codegen_record_alloc(ctx, stmt->declaration.name);
                }
            } else {
                codegen_emit(ctx, "%s %s", type_str, stmt->declaration.name);
                if (stmt->declaration.initializer) {
                    codegen_emit(ctx, " = ");
                    codegen_expression(ctx, stmt->declaration.initializer);
                }
                codegen_emit(ctx, ";\n");
            }
            break;
        }
        case NODE_FOR_STMT: {
            // Gerar laços usando apenas labels/goto
            codegen_for(ctx, stmt);
            break;
        }

        case NODE_ASSIGNMENT: {
            if (stmt->assignment.op == OP_ASSIGN) {
                codegen_expression(ctx, stmt->assignment.target);
                codegen_emit(ctx, " = ");
                codegen_expression(ctx, stmt->assignment.value);
            } else {
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


        case NODE_IF_STMT: {
            int else_label = codegen_new_label(ctx);
            int end_label = codegen_new_label(ctx);

            codegen_emit(ctx, "if (!(");
            codegen_expression(ctx, stmt->if_stmt.condition);
            codegen_emit(ctx, ")) goto L%d;\n", else_label);

            ctx->indent_level++;
            NodeList* body = stmt->if_stmt.then_block;
            while (body) {
                if (body->node) {
                    codegen_statement(ctx, body->node);
                }
                body = body->next;
            }
            ctx->indent_level--;

            if (stmt->if_stmt.else_block) {
                codegen_emit_indent(ctx);
                codegen_emit(ctx, "goto L%d;\n", end_label);
                codegen_emit(ctx, "L%d:\n", else_label);

                ctx->indent_level++;
                body = stmt->if_stmt.else_block;
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

        case NODE_BREAK_STMT: {
            if (ctx->loop_top >= 0) {
                codegen_emit(ctx, "goto L%d;\n", ctx->loop_end[ctx->loop_top]);
            } else {
                codegen_emit(ctx, "/* break fora do loop */\n");
            }
            break;
        }

        case NODE_CONTINUE_STMT: {
            if (ctx->loop_top >= 0) {
                int continue_target = ctx->loop_continue[ctx->loop_top] ?
                    ctx->loop_continue[ctx->loop_top] : ctx->loop_start[ctx->loop_top];
                codegen_emit(ctx, "goto L%d;\n", continue_target);
            } else {
                codegen_emit(ctx, "/* continue fora do loop */\n");
            }
            break;
        }

        case NODE_EXPR_STMT: {
            codegen_expression(ctx, stmt->expr_stmt.expression);
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
            codegen_emit(ctx, "/* ERRO: comando desconhecido %d */\n", stmt->type);
            break;
    }
}

void codegen_expression(CodeGenContext* ctx, ASTNode* expr) {
    if (!expr) return;

    switch (expr->type) {
        case NODE_INT_LITERAL:
            /* Removido log de debug */
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
                        if (left->type == NODE_IDENTIFIER && left->inferred_type &&
                            (left->inferred_type->base_type == TYPE_SET || left->inferred_type->base_type == TYPE_ARRAY)) {
                            codegen_emit(ctx, "set_is_subset(");
                            codegen_expression(ctx, left);
                            codegen_emit(ctx, ", ");
                            codegen_expression(ctx, left);
                            codegen_emit(ctx, "__len, ");
                            codegen_expression(ctx, right);
                            codegen_emit(ctx, ", ");
                            codegen_expression(ctx, right);
                            codegen_emit(ctx, "__len)");
                        } else {
                            codegen_emit(ctx, "set_contains(");
                            codegen_expression(ctx, right);
                            codegen_emit(ctx, ", ");
                            codegen_expression(ctx, right);
                            codegen_emit(ctx, "__len, ");
                            codegen_expression(ctx, left);
                            codegen_emit(ctx, ")");
                        }
                        break;
                    default:
                        codegen_emit(ctx, "/* operacao de conjuntos nao suportada */");
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

                if (strcmp(func_name, "input") == 0) {
                    // Determinar tipo baseado no contexto ou argumento
                    if (expr->inferred_type) {
                        switch (expr->inferred_type->base_type) {
                            case TYPE_INT: codegen_emit(ctx, "mathc_input_int()"); return;
                            case TYPE_FLOAT: codegen_emit(ctx, "mathc_input_float()"); return;
                            case TYPE_STRING: codegen_emit(ctx, "mathc_input_string()"); return;
                            case TYPE_CHAR: codegen_emit(ctx, "mathc_input_char()"); return;
                            default: codegen_emit(ctx, "mathc_input_string()"); return;
                        }
                    }
                    codegen_emit(ctx, "mathc_input_string()");
                    return;
                }

                if (strcmp(func_name, "print") == 0 || strcmp(func_name, "println") == 0) {
                    int is_println = (strcmp(func_name, "println") == 0);
                    const char* prefix = is_println ? "mathc_println_" : "mathc_print_";

                    NodeList* args = expr->call.arguments;

                    if (!args) {
                        if (is_println) codegen_emit(ctx, "printf(\"\\n\")");
                        return;
                    }

                    ASTNode* arg = args->node;
                    TypeSpec* arg_type = arg->inferred_type;

                    if (arg_type) {
                        switch (arg_type->base_type) {
                            case TYPE_INT:
                            case TYPE_BOOL:
                                codegen_emit(ctx, "%sint(", prefix);
                                codegen_expression(ctx, arg);
                                codegen_emit(ctx, ")");
                                break;
                            case TYPE_FLOAT:
                                codegen_emit(ctx, "%sfloat(", prefix);
                                codegen_expression(ctx, arg);
                                codegen_emit(ctx, ")");
                                break;
                            case TYPE_STRING:
                                codegen_emit(ctx, "%sstring(", prefix);
                                codegen_expression(ctx, arg);
                                codegen_emit(ctx, ")");
                                break;
                            case TYPE_COMPLEX:
                                codegen_emit(ctx, "%scomplex(", prefix);
                                codegen_expression(ctx, arg);
                                codegen_emit(ctx, ")");
                                break;
                            case TYPE_CHAR:
                                codegen_emit(ctx, "%schar(", prefix);
                                codegen_expression(ctx, arg);
                                codegen_emit(ctx, ")");
                                break;
                            case TYPE_ARRAY:
                                codegen_emit(ctx, "%sarray(", prefix);
                                if (arg->type == NODE_IDENTIFIER) {
                                    codegen_expression(ctx, arg);
                                    codegen_emit(ctx, ", %s__len", arg->identifier.name);
                                } else {
                                    codegen_expression(ctx, arg);
                                    codegen_emit(ctx, ", 0");
                                }
                                codegen_emit(ctx, ")");
                                break;
                            case TYPE_SET:
                                codegen_emit(ctx, "%sset(", prefix);
                                if (arg->type == NODE_IDENTIFIER) {
                                    codegen_expression(ctx, arg);
                                    codegen_emit(ctx, ", %s__len", arg->identifier.name);
                                } else {
                                    codegen_expression(ctx, arg);
                                    codegen_emit(ctx, ", 0");
                                }
                                codegen_emit(ctx, ")");
                                break;
                            default:
                                codegen_emit(ctx, "%sint(", prefix);
                                codegen_expression(ctx, arg);
                                codegen_emit(ctx, ")");
                                break;
                        }
                    } else {
                        if (arg->type == NODE_INT_LITERAL || arg->type == NODE_BOOL_LITERAL) {
                            codegen_emit(ctx, "%sint(", prefix);
                        } else if (arg->type == NODE_FLOAT_LITERAL) {
                            codegen_emit(ctx, "%sfloat(", prefix);
                        } else if (arg->type == NODE_STRING_LITERAL) {
                            codegen_emit(ctx, "%sstring(", prefix);
                        } else if (arg->type == NODE_COMPLEX_LITERAL) {
                            codegen_emit(ctx, "%scomplex(", prefix);
                        } else if (arg->type == NODE_CHAR_LITERAL) {
                            codegen_emit(ctx, "%schar(", prefix);
                        } else {
                            codegen_emit(ctx, "%sfloat(", prefix);
                        }
                        codegen_expression(ctx, arg);
                        codegen_emit(ctx, ")");
                    }
                    return;
                } else if (strcmp(func_name, "sum") == 0 ||
                          strcmp(func_name, "mean") == 0 ||
                          strcmp(func_name, "prod") == 0 ||
                          strcmp(func_name, "std") == 0 ||
                          strcmp(func_name, "variance") == 0) {
                    codegen_emit(ctx, "mathc_%s(", func_name);
                    NodeList* args = expr->call.arguments;
                    if (args && args->node && args->node->type == NODE_IDENTIFIER) {
                        codegen_expression(ctx, args->node);
                        codegen_emit(ctx, ", %s__len", args->node->identifier.name);
                    } else {
                        codegen_emit(ctx, "NULL, 0");
                    }
                    codegen_emit(ctx, ")");
                    return;
                } else if (strcmp(func_name, "sort") == 0 || strcmp(func_name, "reverse") == 0) {
                    codegen_emit(ctx, "mathc_%s(", func_name);
                    NodeList* args = expr->call.arguments;
                    if (args && args->node && args->node->type == NODE_IDENTIFIER) {
                        codegen_expression(ctx, args->node);
                        codegen_emit(ctx, ", %s__len", args->node->identifier.name);
                    } else {
                        codegen_emit(ctx, "NULL, 0");
                    }
                    codegen_emit(ctx, ")");
                    return;
                } else if (strcmp(func_name, "pop") == 0) {
                    codegen_emit(ctx, "mathc_pop(");
                    NodeList* args = expr->call.arguments;
                    if (args && args->node && args->node->type == NODE_IDENTIFIER) {
                        codegen_expression(ctx, args->node);
                        codegen_emit(ctx, ", &%s__len", args->node->identifier.name);
                    } else {
                        codegen_emit(ctx, "NULL, NULL");
                    }
                    codegen_emit(ctx, ")");
                    return;
                } else if (strcmp(func_name, "sqrt") == 0 ||
                          strcmp(func_name, "sin") == 0 ||
                          strcmp(func_name, "cos") == 0 ||
                          strcmp(func_name, "tan") == 0 ||
                          strcmp(func_name, "exp") == 0 ||
                          strcmp(func_name, "log") == 0 ||
                          strcmp(func_name, "log10") == 0 ||
                          strcmp(func_name, "log2") == 0 ||
                          strcmp(func_name, "asin") == 0 ||
                          strcmp(func_name, "acos") == 0 ||
                          strcmp(func_name, "atan") == 0 ||
                          strcmp(func_name, "atan2") == 0 ||
                          strcmp(func_name, "sinh") == 0 ||
                          strcmp(func_name, "cosh") == 0 ||
                          strcmp(func_name, "tanh") == 0 ||
                          strcmp(func_name, "floor") == 0 ||
                          strcmp(func_name, "ceil") == 0 ||
                          strcmp(func_name, "round") == 0 ||
                          strcmp(func_name, "trunc") == 0 ||
                          strcmp(func_name, "pow") == 0 ||
                          strcmp(func_name, "fmod") == 0 ||
                          strcmp(func_name, "fabs") == 0 ||
                          strcmp(func_name, "abs") == 0 ||
                          strcmp(func_name, "rand") == 0) {
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
                } else if (strcmp(func_name, "srand") == 0) {
                    codegen_emit(ctx, "srand(");
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
                } else if (strcmp(func_name, "max") == 0 || strcmp(func_name, "min") == 0) {
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
                } else if (strcmp(func_name, "factorial") == 0 ||
                          strcmp(func_name, "gcd") == 0 ||
                          strcmp(func_name, "lcm") == 0 ||
                          strcmp(func_name, "isprime") == 0 ||
                          strcmp(func_name, "combinations") == 0 ||
                          strcmp(func_name, "permutations") == 0) {
                    codegen_emit(ctx, "mathc_%s(", func_name);
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
            if (!expr->array_access.array || !expr->array_access.index) {
                codegen_emit(ctx, "/* acesso invalido de array */");
                break;
            }
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
            codegen_emit(ctx, "/* expressao nao suportada %d */", expr->type);
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
        default:
            break;
    }
}

void codegen_array_literal(CodeGenContext* ctx, ASTNode* literal) {
    codegen_emit(ctx, "/* ERRO: array literal usado em expressão */");
}

void codegen_matrix_literal(CodeGenContext* ctx, ASTNode* literal) {
    codegen_emit(ctx, "/* ERRO: matrix literal usado em expressao */");
}

void codegen_set_literal(CodeGenContext* ctx, ASTNode* literal) {
    codegen_emit(ctx, "/* ERRO: set literal usado em expressao */");
}

void codegen_program(CodeGenContext* ctx, ASTNode* program) {
    if (!program) return;
    DBG("programa iniciado");
    if (!program || program->type != NODE_PROGRAM) { DBG(""); return; }

    // Detectar imports
    int has_random = 0;

    NodeList* imports = program->program.imports;
    while (imports) {
        if (imports->node && imports->node->type == NODE_IMPORT) {
            const char* module = imports->node->import.module_name;
            if (strcmp(module, "random") == 0) has_random = 1;
        }
        imports = imports->next;
    }

    // Headers sempre incluídos (necessários para runtime helpers)
    fputs("#include <stdio.h>\n", ctx->output);
    fputs("#include <stdlib.h>\n", ctx->output);
    fputs("#include <string.h>\n", ctx->output);
    fputs("#include <stdbool.h>\n", ctx->output);
    fputs("#include <math.h>\n", ctx->output);
    fputs("#include <complex.h>\n", ctx->output);

    // Header condicional
    if (has_random) {
        fputs("#include <time.h>\n", ctx->output);
    }

    fputs("\n", ctx->output);

    // Constantes matemáticas sempre disponíveis
    fputs("#ifndef PI\n#define PI 3.14159265358979323846\n#endif\n", ctx->output);
    fputs("#ifndef E\n#define E 2.71828182845904523536\n#endif\n\n", ctx->output);

    codegen_runtime_helpers(ctx);

    fputs("void mathc_print_int(int x) { printf(\"%d\", x); }\n", ctx->output);
    fputs("void mathc_print_float(double x) { printf(\"%g\", x); }\n", ctx->output);
    fputs("void mathc_print_string(char* x) { printf(\"%s\", x); }\n", ctx->output);
    fputs("void mathc_print_char(char x) { printf(\"%c\", x); }\n", ctx->output);
    fputs("void mathc_print_complex(double complex x) { printf(\"%g%+gi\", creal(x), cimag(x)); }\n", ctx->output);

    fputs("void mathc_println_int(int x) { printf(\"%d\\n\", x); }\n", ctx->output);
    fputs("void mathc_println_float(double x) { printf(\"%g\\n\", x); }\n", ctx->output);
    fputs("void mathc_println_string(char* x) { printf(\"%s\\n\", x); }\n", ctx->output);
    fputs("void mathc_println_char(char x) { printf(\"%c\\n\", x); }\n", ctx->output);
    fputs("void mathc_println_complex(double complex x) { printf(\"%g%+gi\\n\", creal(x), cimag(x)); }\n", ctx->output);

    // Funções de input
    fputs("int mathc_input_int() { int x; scanf(\"%d\", &x); return x; }\n", ctx->output);
    fputs("double mathc_input_float() { double x; scanf(\"%lf\", &x); return x; }\n", ctx->output);
    fputs("char* mathc_input_string() { char* s = malloc(256); if(fgets(s, 256, stdin)) { s[strcspn(s, \"\\n\")] = 0; } return s; }\n", ctx->output);
    fputs("char mathc_input_char() { char c; scanf(\" %c\", &c); return c; }\n", ctx->output);

    fputs("void mathc_print_array(int* arr, int len) {\n", ctx->output);
    fputs("    printf(\"[\"); int i = 0; goto L_pa_check;\n", ctx->output);
    fputs("L_pa_body: if (i > 0) printf(\", \" ); printf(\"%d\", arr[i]); i++;\n", ctx->output);
    fputs("L_pa_check: if (i < len) goto L_pa_body; printf(\"]\");\n", ctx->output);
    fputs("}\n", ctx->output);

    fputs("void mathc_println_array(int* arr, int len) {\n", ctx->output);
    fputs("    printf(\"[\"); int i = 0; goto L_pla_check;\n", ctx->output);
    fputs("L_pla_body: if (i > 0) printf(\", \" ); printf(\"%d\", arr[i]); i++;\n", ctx->output);
    fputs("L_pla_check: if (i < len) goto L_pla_body; printf(\"]\\n\");\n", ctx->output);
    fputs("}\n", ctx->output);

    fputs("void mathc_print_set(int* set, int len) {\n", ctx->output);
    fputs("    printf(\"{\"); int i = 0; goto L_ps_check;\n", ctx->output);
    fputs("L_ps_body: if (i > 0) printf(\", \" ); printf(\"%d\", set[i]); i++;\n", ctx->output);
    fputs("L_ps_check: if (i < len) goto L_ps_body; printf(\"}\");\n", ctx->output);
    fputs("}\n", ctx->output);

    fputs("void mathc_println_set(int* set, int len) {\n", ctx->output);
    fputs("    printf(\"{\"); int i = 0; goto L_pls_check;\n", ctx->output);
    fputs("L_pls_body: if (i > 0) printf(\", \" ); printf(\"%d\", set[i]); i++;\n", ctx->output);
    fputs("L_pls_check: if (i < len) goto L_pls_body; printf(\"}\\n\");\n", ctx->output);
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

    codegen_emit_frees(ctx);
    DBG("programa finalizado");
}
