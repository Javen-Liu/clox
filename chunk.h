#ifndef clox_chunk_h
#define clox_chunk_h

#include <_types/_uint8_t.h>

#include "common.h"
#include "value.h"

typedef enum {
	OP_RETURN,
	OP_NEGATE,
	OP_CONSTANT,
} OpCode;

typedef struct {
	int count;
	int capacity;
	uint8_t *code;
	int *lines;
	ValueArray constants;
} Chunk;

void initChunk(Chunk *chunk);
void freeChunk(Chunk *chunk);
void writeChunk(Chunk *chunk, uint8_t byte, int line);
int addConstant(Chunk *chunk, Value value);

#endif
