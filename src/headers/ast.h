#ifndef AST_H
    #define AST_H

        #include<iostream>
        #include<memory>
        #include<vector>

        #include "llvm/ADT/APFloat.h"
        #include "llvm/ADT/STLExtras.h"
        #include "llvm/IR/BasicBlock.h"
        #include "llvm/IR/Constants.h"
        #include "llvm/IR/DerivedTypes.h"
        #include "llvm/IR/Function.h"
        #include "llvm/IR/IRBuilder.h"
        #include "llvm/IR/LLVMContext.h"
        #include "llvm/IR/Module.h"
        #include "llvm/IR/Type.h"
        #include "llvm/IR/Verifier.h"

        namespace core{
            // ExprAST - Base class for all expression nodes.
            class ExprAST{
                public:
                    virtual ~ExprAST() = default;
                    virtual llvm::Value *codegen() = 0;
            };

            // NumberExprAST - Expression class for numeric literals like "1.0"
            class NumberExprAST final : public ExprAST{
                double val;

                public:
                    explicit NumberExprAST(const double val): val(val){}
                    llvm::Value *codegen() override;
            };

            // VariableExprAST - Expression class for referencing a variable like "a".
            class VariableExprAST final : public ExprAST{
                std::string name;

                public:
                    explicit VariableExprAST(std::string &name): name(std::move(name)){}
                    llvm::Value *codegen() override;
            };

            // BinaryExprAST - Expression class for binary operator
            class BinaryExprAST final : public ExprAST{
                char op;
                std::unique_ptr<ExprAST> lhs, rhs;

                public:
                    BinaryExprAST(const char op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs): op(op), lhs(std::move(lhs)), rhs(std::move(rhs)){}
                    llvm::Value *codegen() override;
            };

            // CallExprAst - Expression class for function calls.
            class CallExprAST final : public ExprAST{
                std::string callee;
                std::vector<std::unique_ptr<ExprAST>> args;

                public:
                    CallExprAST(std::string callee, std::vector<std::unique_ptr<ExprAST>> args): callee(std::move(callee)), args(std::move(args)){}
            };

            // PrototypeAST - This class represents the "prototype" of a function,
            // which captures it's name, and it's argument names (thus implicitly the number of arguments the function takes).
            class PrototypeAST{
                std::string name;
                std::vector<std::string> args;

                public:
                    PrototypeAST(std::string &name, std::vector<std::string> args): name(std::move(name)), args(std::move(args)){}

                    const std::string &getName() const { return name; }
            };

            /// FunctionAST - This class represents a function definition itself.
            class FunctionAST {
                std::unique_ptr<PrototypeAST> proto;
                std::unique_ptr<ExprAST> body;

                public:
                    FunctionAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body): proto(std::move(proto)), body(std::move(body)) {}

                    llvm::Function *codegen();
            };
        }
#endif