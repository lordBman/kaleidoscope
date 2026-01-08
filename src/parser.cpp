#include "headers/parser.h"

static int parser::getNextToken(){
    return CurTok = lexer::gettok();
}

static int parser::getTokPrecedence(){
    if(!isascii(CurTok)){
        return -1;
    }

    // make sure it's a declared binop
    int TokPrec = BinopPrecedence[CurTok];
    if(TokPrec <= 0){
        return -1;
    }
    return TokPrec;
}

std::unique_ptr<core::ExprAST> parser::LogError(const char *Str) {
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr;
}

std::unique_ptr<core::PrototypeAST> parser::LogErrorP(const char *Str) {
    LogError(Str);
    return nullptr;
}

static std::unique_ptr<core::ExprAST> parser::ParseNumberExpr() {
    auto Result = std::make_unique<core::NumberExprAST>(lexer::NumVal);
    getNextToken(); // consume the number
    return std::move(Result);
}

static std::unique_ptr<core::ExprAST> parser::ParseParenExpr() {
    getNextToken(); // eat (.
    auto V = ParseExpression();
    if (!V)
        return nullptr;

    if (parser::CurTok != ')')
        return LogError("expected ')'");
    getNextToken(); // eat ).
    return V;
}

static std::unique_ptr<core::ExprAST> parser::ParseIdentifierExpr() {
    std::string IdName = lexer::IdentifierStr;

    getNextToken(); // eat identifier.

    if (parser::CurTok != '(') // Simple variable ref.
        return std::make_unique<core::VariableExprAST>(IdName);

    // Call.
    getNextToken(); // eat (
    std::vector<std::unique_ptr<core::ExprAST>> Args;
    if (parser::CurTok != ')') {
        while (true) {
            if (auto Arg = ParseExpression())
                Args.push_back(std::move(Arg));
            else
                return nullptr;

            if (CurTok == ')')
                break;

            if (CurTok != ',')
                return LogError("Expected ')' or ',' in argument list");
            getNextToken();
        }
    }

    // Eat the ')'.
    getNextToken();

    return std::make_unique<core::CallExprAST>(IdName, std::move(Args));
}

static std::unique_ptr<core::ExprAST> parser::ParsePrimary() {
    switch (CurTok) {
        default:
            return LogError("unknown token when expecting an expression");
        case lexer::tok_identifier:
            return ParseIdentifierExpr();
        case lexer::tok_number:
            return ParseNumberExpr();
        case '(':
            return ParseParenExpr();
    }
}

static std::unique_ptr<core::ExprAST> parser::ParseBinOpRHS(int ExprPrec, std::unique_ptr<core::ExprAST> LHS) {
    // If this is a binop, find its precedence.
    while (true) {
        int TokPrec = getTokPrecedence();

        // If this is a binop that binds at least as tightly as the current binop,
        // consume it, otherwise we are done.
        if (TokPrec < ExprPrec)
            return LHS;

        // Okay, we know this is a binop.
        int BinOp = CurTok;
        getNextToken(); // eat binop

        // Parse the primary expression after the binary operator.
        auto RHS = ParsePrimary();
        if (!RHS)
            return nullptr;

        // If BinOp binds less tightly with RHS than the operator after RHS, let
        // the pending operator take RHS as its LHS.
        int NextPrec = getTokPrecedence();
        if (TokPrec < NextPrec) {
            RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
            if (!RHS)
                return nullptr;
        }

        // Merge LHS/RHS.
        LHS = std::make_unique<core::BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
    }
}

static std::unique_ptr<core::ExprAST> parser::ParseExpression() {
    auto LHS = ParsePrimary();
    if (!LHS)
        return nullptr;

    return ParseBinOpRHS(0, std::move(LHS));
}

static std::unique_ptr<core::PrototypeAST> parser::ParsePrototype() {
    if (CurTok != lexer::tok_identifier)
        return LogErrorP("Expected function name in prototype");

    std::string FnName = lexer::IdentifierStr;
    getNextToken();

    if (CurTok != '(')
        return LogErrorP("Expected '(' in prototype");

    std::vector<std::string> ArgNames;
    while (getNextToken() == lexer::tok_identifier)
        ArgNames.push_back(lexer::IdentifierStr);
    if (CurTok != ')')
        return LogErrorP("Expected ')' in prototype");

    // success.
    getNextToken(); // eat ')'.

    return std::make_unique<core::PrototypeAST>(FnName, std::move(ArgNames));
}

static std::unique_ptr<core::FunctionAST> parser::ParseDefinition() {
    getNextToken(); // eat def.
    auto Proto = ParsePrototype();
    if (!Proto)
        return nullptr;

    if (auto E = ParseExpression())
        return std::make_unique<core::FunctionAST>(std::move(Proto), std::move(E));
    return nullptr;
}

static std::unique_ptr<core::FunctionAST> parser::ParseTopLevelExpr() {
    if (auto E = ParseExpression()) {
        // Make an anonymous proto.
        auto Proto = std::make_unique<core::PrototypeAST>("__anon_expr", std::vector<std::string>());

        return std::make_unique<core::FunctionAST>(std::move(Proto), std::move(E));
    }
    return nullptr;
}

static std::unique_ptr<core::PrototypeAST> parser::ParseExtern() {
    getNextToken(); // eat extern.
    return ParsePrototype();
}

//===----------------------------------------------------------------------===//
// Top-Level parsing
//===----------------------------------------------------------------------===//

static void HandleDefinition() {
    if (parser::ParseDefinition()) {
        fprintf(stderr, "Parsed a function definition.\n");
    } else {
        // Skip token for error recovery.
        parser::getNextToken();
    }
}

static void HandleExtern() {
    if (parser::ParseExtern()) {
        fprintf(stderr, "Parsed an extern\n");
    } else {
        // Skip token for error recovery.
        parser::getNextToken();
    }
}

static void HandleTopLevelExpression() {
    // Evaluate a top-level expression into an anonymous function.
    if (parser::ParseTopLevelExpr()) {
        fprintf(stderr, "Parsed a top-level expr\n");
    } else {
        // Skip token for error recovery.
        parser::getNextToken();
    }
}

static void parser::MainLoop() {
    while (true) {
        fprintf(stderr, "ready> ");
        switch (CurTok) {
            case lexer::tok_eof:
                return;
            case ';': // ignore top-level semicolons.
                getNextToken();
                break;
            case lexer::tok_def:
                HandleDefinition();
                break;
            case lexer::tok_extern:
                HandleExtern();
                break;
            default:
                HandleTopLevelExpression();
                break;
        }
    }
}