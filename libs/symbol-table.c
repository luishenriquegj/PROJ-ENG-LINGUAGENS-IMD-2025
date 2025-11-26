#include "symbol-table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_BUCKET_COUNT 211

static unsigned long hash(const char *str) {
  unsigned long h = 5381;
  int c;
  while ((c = *str++))
    h = ((h << 5) + h) + c;
  return h;
}

SymbolTable *symbol_table_create() {
    SymbolTable *table = malloc(sizeof(SymbolTable));
    table->current = NULL;
    return table;
}

SymbolTable *symbol_table = NULL;

void symbol_table_enter_scope(SymbolTable *table) {
  Scope *scope = malloc(sizeof(Scope));
  scope->bucket_count = DEFAULT_BUCKET_COUNT;
  scope->parent = table->current;

  scope->buckets = calloc(scope->bucket_count, sizeof(Symbol*));

  table->current = scope;
}
void symbol_table_leave_scope(SymbolTable *table) {
  if (!table->current) return;

  Scope *top = table->current;
  // liberar todos os símbolos do bucket
  for (int i = 0; i < top->bucket_count; i++) {
    Symbol *s = top->buckets[i];
    while (s) {
      Symbol *next = s->next;
      free(s->name);
      free(s);
      s = next;
    }
  }

  free(top->buckets);

  table->current = top->parent;
  free(top);
}

bool symbol_table_insert(
    SymbolTable *table,
    const char *name,
    TypeSpec *type,
    SymbolKind kind,
    int line,
    ParamList *params
) {
  if (!table->current) {
    fprintf(stderr, "ERRO: inserir símbolo sem escopo aberto!\n");
    return false;
  }

  if (symbol_table_lookup_current_scope(table, name) != NULL) {
    return false;
  }

  Symbol *sym = malloc(sizeof(Symbol));
  sym->name = strdup(name);
  sym->type = type;
  sym->kind = kind;
  sym->line = line;
  sym->params = params;

  unsigned h = hash(name) % table->current->bucket_count;

  sym->next = table->current->buckets[h];
  table->current->buckets[h] = sym;

  return true;
}

Symbol *symbol_table_lookup(SymbolTable *table, const char *name) {
  for (Scope *scope = table->current; scope != NULL; scope = scope->parent) {
    unsigned h = hash(name) % scope->bucket_count;
    Symbol *s = scope->buckets[h];
    while (s) {
      if (strcmp(s->name, name) == 0)
        return s;
      s = s->next;
    }
  }
  return NULL;
}

Symbol *symbol_table_lookup_current_scope(SymbolTable *table, const char *name) {
  if (!table->current) return NULL;

  unsigned h = hash(name) % table->current->bucket_count;
  Symbol *s = table->current->buckets[h];
  while (s) {
    if (strcmp(s->name, name) == 0)
      return s;
    s = s->next;
  }
  return NULL;
}

void free_symbol_table(SymbolTable *table) {
  while (table->current != NULL) {
    symbol_table_leave_scope(table);
  }
  free(table);
}

void symbol_table_print(SymbolTable *table) {
  printf("\n===== SYMBOL TABLE =====\n");

  int scope_level = 0;
  for (Scope *scope = table->current; scope != NULL; scope = scope->parent) {
    printf("  [SCOPE %d]\n", scope_level);
    for (int i = 0; i < scope->bucket_count; i++) {
      Symbol *s = scope->buckets[i];
      while (s) {
        printf("    %s | kind=%d | line=%d\n", s->name, s->kind, s->line);
        s = s->next;
      }
    }
    scope_level++;
  }

  printf("========================\n");
}