#include <stdio.h>

#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(int argc, const char* argv[]) {
    initVM();

	Chunk chunk;
    initChunk(&chunk);

    // 写入常数1.2
    int constant = addConstant(&chunk, 1.2);
    writeChunk(&chunk, OP_CONSTANT, 123);
    writeChunk(&chunk, constant, 123);

    constant = addConstant(&chunk, 3.4);
    writeChunk(&chunk, OP_CONSTANT, 124);
    writeChunk(&chunk, constant, 124);

    writeChunk(&chunk, OP_ADD, 124);

    constant = addConstant(&chunk, 5.6);
    writeChunk(&chunk, OP_CONSTANT, 124);
    writeChunk(&chunk, constant, 124);

    writeChunk(&chunk, OP_DIVIDE, 124);

    // 写入负号 "-"
    writeChunk(&chunk, OP_NEGATE, 125);

    // 写入返回操作符
    writeChunk(&chunk, OP_RETURN, 125);

    // 解析chunk里面的内容
    disassembleChunk(&chunk, "test chunk");
    printf("\n");

    // 对chunk字节码里的内容进行解释执行
    interpret(&chunk);

    freeVM();
    freeChunk(&chunk);
    return 0;
}
