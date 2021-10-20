//
// Created by Javen on 2021/1/11.
//

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "vm.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "object.h"

VM vm;

static void resetStack(){
    vm.stackTop = vm.stack;
    vm.frameCount = 0;
}

static void runtimeError(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    for (int i = vm.frameCount - 1; i >= 0; i--) {
        CallFrame *frame = &vm.frames[i];
        ObjFunction *function = frame->function;
        size_t instruction = frame->ip - function->chunk.code - 1;
        fprintf(stderr, "[line %d] in ",
                function->chunk.lines[instruction]);
        if (function->name == NULL) {
            fprintf(stderr, "script\n");
        } else {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }

    resetStack();
}

static Value clockNative(int argCount, Value* args) {
    if (argCount != 0) {
        runtimeError("Error on 'clock' function, expect 0 argument, but given %d.", argCount);
    }
    return NUMBER_VALUE((double) clock() / CLOCKS_PER_SEC);
}

static Value inputNative(int argCount, Value* args) {
    if (argCount != 0) {
        runtimeError("Error on 'clock' function, expect 0 argument, but given %d.", argCount);
    }
    char *input = ALLOCATE(char, 5);
    scanf("%s", input);
    char *string = ALLOCATE(char, (int) strlen(input) + 1);
    memcpy(string, input, (int) strlen(input));
    *(string + (int) strlen(input)) = '\0';
    ObjString *result = takeString(string, (int)strlen(string));
    return OBJ_VALUE(result);
}

static void defineNative(const char *name, NativeFn function) {
    push(OBJ_VALUE(copyString(name, (int)strlen(name))));
    push(OBJ_VALUE(newNative(function)));
    tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}

void initVM(){
    resetStack();
    vm.objects = NULL;
    initTable(&vm.globals);
    initTable(&vm.strings);

    defineNative("clock", clockNative);
    defineNative("input", inputNative);
}

void freeVM(){
    freeTable(&vm.globals);
    freeTable(&vm.strings);
    freeObjects();
}

void push(Value value){
    *vm.stackTop++ = value;
}

Value pop(){
    return *(--vm.stackTop);
}

static Value peek(int distance) {
    return vm.stackTop[-1 - distance];
}

static bool call(ObjFunction *function, int argCount) {
    if (argCount != function->arity) {
        runtimeError("Expected %d argument but got %d.",
                     function->arity, argCount);
        return false;
    }

    if (vm.frameCount == FRAMES_MAX) {
        runtimeError("Stack overflow.");
        return false;
    }

    CallFrame *frame = &vm.frames[vm.frameCount++];
    frame->function = function;
    frame->ip = function->chunk.code;
    frame->slots = vm.stackTop - argCount - 1;
    return true;
}

static bool callValue(Value callee, int argCount) {
    if (IS_OBJ(callee)) {
        switch (OBJ_TYPE(callee)) {
            case OBJ_FUNCTION:
                return call(AS_FUNCTION(callee), argCount);
            case OBJ_NATIVE: {
                NativeFn native = AS_NATIVE(callee);
                Value result = native(argCount, vm.stackTop - argCount);
                vm.stackTop -= argCount + 1;
                push(result);
                return true;
            }
            default:
                break;
        }
    }
    runtimeError("Can only call functions ans classes.");
    return false;
}

static bool isFalsey(Value value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate() {
    ObjString *b = AS_STRING(pop());
    ObjString *a = AS_STRING(pop());

    int newLength = a->length + b->length;
    char *chars = ALLOCATE(char, newLength + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[newLength] = '\0';

    ObjString *result = takeString(chars, newLength);
    push(OBJ_VALUE(result));
}

static int lengthOfTheDigits(double number);
static void copyNumber(char* string, int length, double number);

static void concatenateWithNumber() {
    Value b = pop();
    Value a = pop();
    if (IS_STRING(a)) {
        ObjString *string = AS_STRING(a);
        double number = AS_NUMBER(b);
        int numberLength = lengthOfTheDigits(number);
        int newLength = string->length + numberLength;

        char *chars = ALLOCATE(char, newLength + 1);
        memcpy(chars, string->chars, string->length);
        copyNumber(chars + string->length, numberLength, number);
        chars[newLength] = '\0';
        ObjString *result = takeString(chars, newLength);
        push(OBJ_VALUE(result));
    } else {
        double number = AS_NUMBER(a);
        ObjString *string = AS_STRING(b);
        int numberLength = lengthOfTheDigits(number);
        int newLength = numberLength + string->length;

        char *chars = ALLOCATE(char, newLength + 1);
        copyNumber(chars, numberLength, number);
        memcpy(chars + numberLength, string->chars, string->length);
        chars[newLength] = '\0';
        ObjString *result = takeString(chars, newLength);
        push(OBJ_VALUE(result));
    }
}

/** next several functions are turning *double* into character array **/

// reverse the character between *start* and *end*, which is [start, end]
static void reverse(char* string, int start, int end) {
    while (start < end) {
        char temp = *(string + start);
        *(string + start) = *(string + end);
        *(string + end) = temp;
        start++;
        end--;
    }
}

static void copyNumber(char* string, int length, double number) {
    char *numberString = ALLOCATE(char, length);
    char *p;
    p = numberString;
    int integer = (int) number;
    while (integer != 0) {
        *p = '0' + (char) (integer % 10);
        integer /= 10;
        p++;
    }
    reverse(numberString, 0, p - numberString - 1);

    if (length != p - numberString) {
        double decimal = number - integer;
        while (p <= numberString + length) {
            decimal *= 10;
            *p = (int) decimal + '0';
        }

        int i;
        for (i = 0; i < length; i++) {
            *(string + i) = *(numberString + i);
        }
    }

    memcpy(string, numberString, length);
}

static int lengthOfTheDigits(double number) {
    int integer = (int) number;
    double decimal = number - integer;

    int digits = 0;
    while (integer != 0) {
        digits++;
        integer /= 10;
    }

    if (decimal != 0) {
        digits++;
        while (decimal != (int) decimal) {
            decimal *= 10;
            digits++;
        }
    }

    return digits;
}

/************************ end of the block ****************************/

static InterpretResult run() {
    CallFrame *frame = &vm.frames[vm.frameCount - 1];

#define READ_BYTE() (*frame->ip++)
#define READ_CONSTANT() (frame->function->chunk.constants.values[READ_BYTE()])
#define READ_STRING() AS_STRING(READ_CONSTANT())
#define READ_SHORT() (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
#define BINARY_OP(valueType, op)       \
    do{                                \
        if(!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
            runtimeError("Operands must be numbers.");   \
            return INTERPRET_RUNTIME_ERROR; \
        } \
        double b = AS_NUMBER(pop());   \
        double a = AS_NUMBER(pop());   \
        push(valueType(a op b));       \
       } while(false)       \

//    printf("\n== running ==\nv       v     v\n");
    for(;;){
#ifdef DEBUG_TRACE_EXECUTION
        printf("          [ stack ] ");
        for(Value *slot = vm.stack; slot < vm.stackTop; slot++){
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");
        disassembleInstruction(&frame->function->chunk, (int)(frame->ip - frame->function->chunk.code));
#endif
        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT:{
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OP_NIL:    push(NIL_VALUE); break;
            case OP_TRUE:   push(BOOL_VALUE(true)); break;
            case OP_FALSE:  push(BOOL_VALUE(false)); break;
            case OP_POP:    pop(); break;
            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(frame->slots[slot]);
                break;
            }
            case OP_GET_GLOBAL: {
                ObjString *name = READ_STRING();
                Value value;
                if(!tableGet(&vm.globals, name, &value)) {
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(value);
                break;
            }
            case OP_DEFINE_GLOBAL: {
                ObjString *name = READ_STRING();
                tableSet(&vm.globals, name, peek(0));
                pop();
                break;
            }
            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = peek(0);
                break;
            }
            case OP_SET_GLOBAL: {
                ObjString *name = READ_STRING();
                if(tableSet(&vm.globals, name, peek(0))) {
                    tableDelete(&vm.globals, name);
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VALUE(valuesEqual(a, b)));
                break;
            }
            case OP_GREATER: {
                BINARY_OP(BOOL_VALUE, >);
                break;
            }
            case OP_LESS: {
                BINARY_OP(BOOL_VALUE, <);
                break;
            }
            case OP_ADD:{
                if(IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                    concatenate();
                } else if(IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VALUE(a + b));
                } else {
                    concatenateWithNumber();
                }
                break;
            }
            case OP_SUBTRACT: {
                BINARY_OP(NUMBER_VALUE, -);
                break;
            }
            case OP_MULTIPLY: {
                BINARY_OP(NUMBER_VALUE, *);
                break;
            }
            case OP_DIVIDE: {
                BINARY_OP(NUMBER_VALUE, /);
                break;
            }
            case OP_NOT:
                push(BOOL_VALUE(isFalsey(pop())));
                break;
            case OP_NEGATE: {
                if(!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be number.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                push(NUMBER_VALUE(-AS_NUMBER(pop())));
                break;
            }
            case OP_PRINT: {
                printValue(pop());
                printf("\n");
                break;
            }
            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                if(isFalsey(peek(0))) {
                    frame->ip += offset;
                }
                break;
            }
            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                frame->ip += offset;
                break;
            }
            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                frame->ip -= offset;
                break;
            }
            case OP_DUP:
                push(peek(0));
                break;
            case OP_CALL: {
                int argCount = READ_BYTE();
                if (!callValue(peek(argCount), argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
            case OP_RETURN: {
                Value result = pop();
                vm.frameCount--;
                if (vm.frameCount == 0) {
                    pop();
                    return INTERPRET_OK;
                }

                vm.stackTop = frame->slots;
                push(result);
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
        }
    }

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}

InterpretResult interpret(const char *source){
    ObjFunction *function = compile(source);
    if (function == NULL) {
        return INTERPRET_COMPILE_ERROR;
    }

    push(OBJ_VALUE(function));
    call(function, 0);
//    CallFrame *frame = &vm.frames[vm.frameCount++];
//    frame->function = function;
//    frame->ip = function->chunk.code;
//    frame->slots = vm.stack;

    return run();
}
