//
// Created by Javen on 2021/1/21.
//

#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define TABLE_LOAD_FACTOR 0.75

void initTable(Table *table) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void freeTable(Table *table) {
    FREE_ARRAY(Entry, table->entries, table->capacity);
    initTable(table);
}

static Entry *findEntry(Entry *entries, int capacity, ObjString *key) {
    uint32_t index = key->hash % capacity;
    Entry *tombstone = NULL;

    for(;;) {
        Entry *entry = &entries[index];
        if(entry->key == NULL) {
            if(IS_NIL(entry->value)) {
                return tombstone != NULL ? tombstone : entry;
            } else {
                if(tombstone == NULL) {
                    tombstone = entry;
                }
            }
        } else if(entry->key == key) {
            return entry;
        }

        index = (index + 1) % capacity;
    }
}

bool tableGet(Table *table, ObjString *key, Value *value) {
    if(table->count == 0) {
        return false;
    }

    Entry *entry = findEntry(table->entries, table->capacity, key);
    if(entry->key == NULL) {
        return false;
    }

    *value = entry->value;
    return true;
}

static void adjustCapacity(Table *table, int capacity) {
    // Allocate new entries with the new capacity.
    Entry *newEntries = ALLOCATE(Entry, capacity);
    for(int i = 0; i < capacity; i++) {
        newEntries[i].key = NULL;
        newEntries[i].value = NIL_VALUE;
    }

    // Move the old entry to the new entry, also recompute the
    // index, new index is (ObjString's hash % new capacity)
    table->count = 0;
    for(int i = 0; i < table->capacity; i++) {
        Entry *oldEntry = &table->entries[i];
        if(oldEntry->key == NULL) {
            continue;
        }

        Entry *newEntry = findEntry(newEntries, capacity, oldEntry->key);
        newEntry->key = oldEntry->key;
        newEntry->value = oldEntry->value;
        table->count++;
    }

    // Before updating the "table" entries, free the memory of
    // the old entries.
    FREE_ARRAY(Entry, table->entries, table->capacity);
    table->capacity = capacity;
    table->entries = newEntries;
}

bool tableSet(Table *table, ObjString *key, Value value) {
    if(table->count + 1 > table->capacity * TABLE_LOAD_FACTOR) {
        int capacity = GROW_CAPACITY(table->capacity);
        adjustCapacity(table, capacity);
    }

    Entry *entry = findEntry(table->entries, table->capacity, key);

    bool isNewKey = entry->key == NULL;
    if(isNewKey && IS_NIL(entry->value)) {
        table->count++;
    }

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool tableDelete(Table *table, ObjString *key) {
    if(table->count == 0) {
        return false;
    }

    Entry *entry = findEntry(table->entries, table->capacity, key);
    if(entry == NULL) {
        return false;
    }

    // Set a "tombstone" instead of really deleting the value
    entry->key = NULL;
    entry->value = BOOL_VALUE(true);

    return true;
}

void tableAddAll(Table *from, Table *to) {
    for(int i = 0; i < from->capacity; i++) {
        Entry *entry = &from->entries[i];
        if(entry->key != NULL) {
            tableSet(to, entry->key, entry->value);
        }
    }
}

ObjString *tableFindString(Table *table, const char *chars,
                           int length, uint32_t hash) {
    if(table->count == 0) {
        return NULL;
    }

    Entry *entries = table->entries;
    int capacity = table->capacity;
    uint32_t index = hash % capacity;

    for(;;) {
        Entry *entry = &entries[index];

        if(entry->key == NULL) {
            if(IS_NIL(entry->value)) {
                return NULL;
            }
        } else if(entry->key->length == strlen(chars) &&
            entry->key->hash == hash &&
            memcmp(entry->key->chars, chars, length) == 0) {
            return entry->key;
        }

        index = (index + 1) % capacity;
    }
}