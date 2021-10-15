//
// Created by Javen on 2021/1/12.
//

#ifndef CLOX_COMPILER_H
#define CLOX_COMPILER_H

#include "chunk.h"
#include "object.h"
#include "vm.h"
#include "scanner.h"

typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,    // =
    PREC_OR,            // or
    PREC_AND,           // and
    PREC_EQUALITY,      // == !=
    PREC_COMPARISON,    // < > <= >=
    PREC_TERM,          // + -
    PREC_FACTOR,        // * /
    PREC_UNARY,         // ! -
    PREC_CALL,          // . ()
    PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(bool canAssign);

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

typedef struct {
    Token name;
    int depth;
} Local;

typedef struct {
    int loopStart;
    int loopEnds[50];
    int loopEndsCount;
} Loop;

typedef enum {
    TYPE_FUNCTION,
    TYPE_SCRIPT,
} FunctionType;

typedef struct {
    ObjFunction *function;
    FunctionType type;

    // Loop array is for jumps of "while" and "for", and
    // only support 50 nested loops.
    Loop loops[50];
    int loopCount;

    Local locals[UINT8_COUNT];
    int localCount;
    int scopeDepth;
} Compiler;

bool compile(const char *source);

#endif //CLOX_COMPILER_H
