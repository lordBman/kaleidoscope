#include <map>

#include "headers/ast.h"
#include "headers/parser.h"

static std::unique_ptr<llvm::LLVMContext> TheContext;
static std::unique_ptr<llvm::IRBuilder<>> Builder;
static std::unique_ptr<llvm::Module> TheModule;
static std::map<std::string, llvm::Value *> NamedValues;

llvm::Value *LogErrorV(const char *Str) {
    parser::LogError(Str);
    return nullptr;
}

llvm::Value *core::NumberExprAST::codegen() {
    return llvm::ConstantFP::get(*TheContext, llvm::APFloat(val));
}

llvm::Value *core::VariableExprAST::codegen() {
    // Look this variable up in the function.
    llvm::Value *V = NamedValues[name];
    if (!V)
        return LogErrorV("Unknown variable name: ");
    return V;
}

llvm::Value *core::BinaryExprAST::codegen() {
    llvm::Value *L = lhs->codegen();
    llvm::Value *R = rhs->codegen();
    if (!L || !R)
        return nullptr;

    switch (op) {
        case '+':
            return Builder->CreateFAdd(L, R, "addtmp");
        case '-':
            return Builder->CreateFSub(L, R, "subtmp");
        case '*':
            return Builder->CreateFMul(L, R, "multmp");
        case '<':
            L = Builder->CreateFCmpULT(L, R, "cmptmp");
            // Convert bool 0/1 to double 0.0 or 1.0
            return Builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*TheContext), "booltmp");
        default:
            return LogErrorV("invalid binary operator");
    }
}