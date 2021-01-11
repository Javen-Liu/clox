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

    // 写入负号 "-"
    writeChunk(&chunk, OP_NEGATE, 123);

    // 写入返回操作符
    writeChunk(&chunk, OP_RETURN, 123);

    // 解析chunk里面的内容
    disassembleChunk(&chunk, "test chunk");

    // 对chunk字节码里的内容进行解释执行
    interpret(&chunk);

    freeVM();
    freeChunk(&chunk);
    return 0;
}
