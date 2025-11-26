#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdbool.h>
#include "../src/ast.h"

typedef enum {
  SYM_VAR,
  SYM_CONST,
  SYM_FUNC,
  SYM_PARAM,
  SYM_TYPE
} SymbolKind;

typedef struct Symbol {
  char *name;
  TypeSpec *type;
  SymbolKind kind;
  int line;
  ParamList *params;

  struct Symbol *next;
} Symbol;

typedef struct SymbolTable {
  struct Scope *current;
} SymbolTable;

typedef struct Scope {
  Symbol **buckets;      // array de listas encadeadas
  int bucket_count;
  struct Scope *parent;  // escopo superior
} Scope;

extern SymbolTable *symbol_table;

SymbolTable *symbol_table_create();
void symbol_table_enter_scope(SymbolTable *table);
void symbol_table_leave_scope(SymbolTable *table);

bool symbol_table_insert(SymbolTable *table, const char *name, TypeSpec *type, SymbolKind kind, int line, ParamList *params);
Symbol *symbol_table_lookup(SymbolTable *table, const char *name);
Symbol *symbol_table_lookup_current_scope(SymbolTable *table, const char *name);

void symbol_table_print(SymbolTable *table);

void free_symbol_table(SymbolTable *table);

#endif
