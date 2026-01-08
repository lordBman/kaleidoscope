#include "headers/parser.h"
#include <llvm/ADT/StringRef.h>

int main(){
    // Install standard binary operators.
    // 1 is lowest precedence.
    parser::BinopPrecedence['<'] = 10;
    parser::BinopPrecedence['+'] = 20;
    parser::BinopPrecedence['-'] = 20;
    parser::BinopPrecedence['*'] = 40;  // highest.

    std::cout<<"Hello Kaleidoscope"<<std::endl;
    // Prime the first token.
    fprintf(stderr, "ready> ");
    parser::getNextToken();

    // Run the main "interpreter loop" now.
    parser::MainLoop();

    return 0;
}