#ifndef PARSER_H
    #define PARSER_H

    #include<map>
    #include "lexer.h"
    #include "ast.h"

    namespace parser{
        // CurTok/getNextToken - Provide a simple token buffer. CurTok is the current token
        // the parser is looking at. getNextToken reads another token from the lexer and 
        // updates CurTok with its result.
        static int CurTok;
        static int getNextToken();

        // BinopPrecedence - This holds the precedence for each binary operator that is
        // defined.
        static std::map<char, int> BinopPrecedence;

        // getTokPrecedence - Get the precedence of the pending binary operator token.
        static int getTokPrecedence();

        /// LogError* - These are little helper functions for error handling.
        std::unique_ptr<core::ExprAST> LogError(const char *Str);
        std::unique_ptr<core::PrototypeAST> LogErrorP(const char *Str);

        /// expression
        ///   ::= primary binoprhs
        ///
        static std::unique_ptr<core::ExprAST> ParseExpression();

        /// numberexpr ::= number
        static std::unique_ptr<core::ExprAST> ParseNumberExpr();

        /// parenexpr ::= '(' expression ')'
        static std::unique_ptr<core::ExprAST> ParseParenExpr();

        /// identifierexpr
        ///   ::= identifier
        ///   ::= identifier '(' expression* ')'
        static std::unique_ptr<core::ExprAST> ParseIdentifierExpr();

        /// primary
        ///   ::= identifierexpr
        ///   ::= numberexpr
        ///   ::= parenexpr
        static std::unique_ptr<core::ExprAST> ParsePrimary();

        /// binoprhs
        ///   ::= ('+' primary)*
        static std::unique_ptr<core::ExprAST> ParseBinOpRHS(int ExprPrec, std::unique_ptr<core::ExprAST> LHS);
        /// prototype
        ///   ::= id '(' id* ')'
        static std::unique_ptr<core::PrototypeAST> ParsePrototype();

        /// definition ::= 'def' prototype expression
        static std::unique_ptr<core::FunctionAST> ParseDefinition();

        /// toplevelexpr ::= expression
        static std::unique_ptr<core::FunctionAST> ParseTopLevelExpr();

        /// external ::= 'extern' prototype
        static std::unique_ptr<core::PrototypeAST> ParseExtern();

        /// top ::= definition | external | expression | ';'
        static void MainLoop();
    }

#endif