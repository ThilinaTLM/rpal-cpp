//
// Created by tlm on 10/30/21.
//

#ifndef RPAL_CSE_H
#define RPAL_CSE_H

#include <stack>
#include <string>
#include <unordered_map>

#include "ast.hpp"

namespace rpal::cse {

    using AstNode = rpal::parser::AstNode;

    class Cse {
       private:
        Cse* parent;
        std::stack<AstNode*> control;
        std::stack<AstNode*> stack;
        std::unordered_map<std::string, AstNode*> env;
       public:
        explicit Cse();
        explicit Cse(Cse* parent);
        AstNode *get_env(std::string &key);
        AstNode* read_control();
        Cse* get_parent();
        bool empty();

        void set_env(const std::string& key, AstNode* value);
        void add_control(AstNode* node);
        void add_stack(AstNode* node);
        void move_to_stack();

        template <class T>
        T pop_stack_value(rpal::parser::Type type);
        AstNode *pop_stack();
    };

    void execute(AstNode* ast);

}  // namespace rpal::cse

#endif  // RPAL_CSE_H
