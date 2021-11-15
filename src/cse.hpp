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
        std::vector<AstNode*> original;
        std::stack<AstNode*> control;
        std::stack<AstNode*> stack;
        std::unordered_map<std::string, rpal::parser::AstNode*> env;
       public:
        explicit Cse();
        explicit Cse(Cse* parent);
        AstNode* get_env(std::string &key);
        AstNode* read_control();
        Cse* set_parent(Cse* p);
        Cse* get_parent();
        Cse* copy();
        bool empty();

        void add_original(AstNode* node);
        void reset();
        void set_env(std::string& key, rpal::parser::AstNode* value);
        void add_control(AstNode* node);
        void add_stack(AstNode* node);
        void move_to_stack();



        AstNode *pop_stack();
        AstNode* pop_control();
        AstNode* read_stack();
    };

    void execute(AstNode* ast);

}  // namespace rpal::cse

#endif  // RPAL_CSE_H
