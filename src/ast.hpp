//
// Created by tlm on 10/30/21.
//

#ifndef RPAL_AST_H
#define RPAL_AST_H

#include <iostream>
#include <vector>
#include "token.hpp"

namespace rpal::parser {
    using Token = token::Token;
    using TT = token::Type;

    typedef enum {
        Let,
        Lambda,
        Where,
        Gamma,
        At,
        Rec,
        Within,
        Tau,
        Aug,
        Ternary,
        And,

        Assign,
        Id,
        Fn,

        NoArg,
        ArgList,

        Add,
        Sub,
        Mul,
        Div,
        Neg,
        Pow,

        B_Or,
        B_Not,
        B_And,
        B_Gr,
        B_Ge,
        B_Ls,
        B_Le,
        B_Eq,
        B_Ne,

        Int,
        Str,
        Bool,
        Nil,
        Unknown,
        Dummy,
    } Type;
    std::ostream& operator<<(std::ostream& os, const Type& n);

    class AstNode {
       private:
        Type type;
        AstNode* left{};
        AstNode* right{};
        AstNode* next{};
        const void* value;
        rpal::token::Token* token{};

       public:
        explicit AstNode(Type type);
        AstNode(Token* token, Type type);
        AstNode(Type type, const void* value);
        AstNode(Token* token, Type type, const void* value);
        AstNode(Type type, AstNode* left, AstNode* right);
        AstNode(Token* token, Type type, AstNode* left, AstNode* right);
        ~AstNode();

        friend std::ostream& operator<<(std::ostream& os, const AstNode& n);
        friend bool operator==(AstNode& left, AstNode& right);
        friend bool operator==(AstNode& left, Type type);
        friend bool operator!=(AstNode& left, Type type);

        template <class T>
        T get_value() const;
        [[nodiscard]] const void* get_value() const;

        void add_child_left(AstNode* node);
        void add_child_right(AstNode* node);
        void set_left_child(AstNode* node);
        void set_right_child(AstNode* node);
        void clear_relations();
        void clear_next();

        AstNode* get_left();
        AstNode* get_right();
        AstNode* get_next();
        Token* get_token();
        Type get_type();
        bool leave();
    };

    // parser
    class Parser {
       private:
        std::vector<Token> tokens;
        uint pointer;

       public:
        Parser() = delete;
        explicit Parser(token::TokenSource& source);
        AstNode* parse();

       private:
        Token getToken();
        Token getToken(uint pass);
        Token getForward();

        uint forward();

        AstNode* parse_E();

        AstNode* parse_D();

        AstNode* parse_Da();

        AstNode* parse_Dr();

        AstNode* parse_Db();

        AstNode* parse_Ew();

        AstNode* parse_T();

        AstNode* parse_Ta();
        AstNode* parse_Ta_(AstNode* left);

        AstNode* parse_Tc();

        AstNode* parse_B();

        AstNode* parse_Bt();

        AstNode* parse_Bs();

        AstNode* parse_Bp();

        AstNode* parse_A();
        AstNode* parse_A_(AstNode* left);

        AstNode* parse_At();
        AstNode* parse_At_(AstNode* left);

        AstNode* parse_Af();

        AstNode* parse_Ap();

        AstNode* parse_R();

        AstNode* parse_Rn();

        AstNode* parse_Vl();

        AstNode* parse_Vb();
    };

    AstNode* generate_ast(token::TokenSource &source);
    AstNode* standard_ast(AstNode* head);
    void print_ast(AstNode* head);

}  // namespace rpal::parser

#endif  // RPAL_AST_H
