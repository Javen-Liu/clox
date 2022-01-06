//
// Created by Javen on 2021/1/21.
//

#ifndef CLOX_TABLE_H
#define CLOX_TABLE_H

#include "common.h"
#include "value.h"

typedef struct {
    ObjString *key;
    Value value;
} Entry;

typedef struct {
    int count;
    int capacity;
    Entry *entries;
} Table;

void initTable(Table *table);
void freeTable(Table *table);
/**
 * "tableGet" is different from the "tableSet", the last
 * parameter "*value" is a pointer. The memory which "*value"
 * points, will save the result of "tableGet" function.
 * @param table
 * @param key
 * @param value
 * @return
 */
bool tableGet(Table *table, ObjString *key, Value *value);
bool tableSet(Table *table, ObjString *key, Value value);
bool tableDelete(Table *table, ObjString *key);
void tableAddAll(Table *from, Table *to);
ObjString *tableFindString(Table *table, const char *chars, int length, uint32_t hash);
void tableRemoveWhite(Table *table);
void markTable(Table *table);

#endif //CLOX_TABLE_H
