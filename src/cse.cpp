//
// Created by tlm on 10/30/21.
//

#include "cse.hpp"
namespace rpal::cse {

    using NT = rpal::parser::Type;

    // CSE
    void rec_pre_order(Cse* cse, AstNode* node) {
        cse->add_control(node);
        AstNode* left = node->get_left();
        while (left != nullptr) {
            rec_pre_order(cse, left);
            left = left->get_next();
        }
        AstNode* right = node->get_right();
        while (right != nullptr) {
            rec_pre_order(cse, right);
            right = right->get_next();
        }
    }

    void Cse::add_control(AstNode* node) { this->control.push(node); }

    AstNode* Cse::read_control() { return this->control.top(); }

    bool Cse::empty() { return this->control.empty(); }

    void Cse::move_to_stack() {
        this->stack.push(this->control.top());
        this->control.pop();
    }

    template <class T>
    T Cse::pop_stack_value(rpal::parser::Type type) {
        auto* node = this->stack.top();
        if (*node != type) throw std::runtime_error("expect integer");
        this->stack.pop();
        return node->template get_value<T>();
    }

    void Cse::add_stack(AstNode* node) { this->stack.push(node); }

    void Cse::set_env(const std::string& key, AstNode* value) {
        this->env[key] = value;
    }

    AstNode* Cse::get_env(std::string& key) {
        auto* value_node = this->env[key];
        if (value_node == nullptr) {
            if (this->parent == nullptr)
                return nullptr;
            else
                return this->parent->get_env(key);
        }
        return value_node;
    }

    Cse::Cse() {
        this->parent = nullptr;
    }

    Cse::Cse(Cse* parent) {
        this->parent = parent;
    }

    Cse* Cse::get_parent() {
        return this->parent;
    }

    AstNode* Cse::pop_stack() {
        auto* node = stack.top();
        stack.pop();
        return node;
    }

    void ast_to_cse(AstNode* ast, Cse* cse) {
        if (*ast == NT::Lambda) {
            Cse* lambda_cse = new Cse(cse);
            auto* lambda_node = new AstNode(NT::Lambda, lambda_cse);
            lambda_node->add_child(ast->get_left());
            cse->add_control(lambda_node);
            ast_to_cse(ast->get_right(), lambda_cse);
            delete ast;
            return;
        }

        cse->add_control(ast);
        AstNode* n;
        n = ast->get_left();
        while(n != nullptr) {
            ast_to_cse(n, cse);
            n = n->get_next();
        }

        n = ast->get_right();
        while(n != nullptr) {
            ast_to_cse(n, cse);
            n = n->get_next();
        }
    }

    template <class T>
    T cse_stack_resolve(Cse* cse, NT type) {
        if (type == NT::Int) {
            auto* st = cse->pop_stack();
            if (*st == NT::Id) {
                auto key = st->get_value<std::string>();
                st = cse->get_env(key);
                if (st == nullptr) throw std::runtime_error("undefined variable");
            }
            return st->get_value<int>();
        }
    }

    void execute(AstNode* ast) {

        // generate control stack
        Cse* cse = new Cse();
        ast_to_cse(ast, cse);

        while (true) {
            auto con = *cse->read_control();
             if (con == NT::Id || con == NT::Int || con == NT::Bool || con == NT::Str || con == NT::Lambda) {
                cse->move_to_stack();
            } else if (con == NT::Add) {
                int val1 = cse_stack_resolve<int>(cse, NT::Int);
                int val2 = cse_stack_resolve<int>(cse, NT::Int);
                cse->add_stack(new AstNode(NT::Int, new int(val1 + val2)));
            } else if (con == NT::Gamma) {
                auto con2 = *cse->pop_stack();
                if (con2 == NT::Lambda) {
                    auto key = con2.get_left()->get_value<std::string>();
                    auto* value = cse->pop_stack();
                    cse = (Cse*)con2.get_value();
                    cse->set_env(key, value);
                } else if (con2 == NT::Id) {
                    auto name = con2.get_value<std::string>();
                    if (name == "Print") {
                        std::cout << "print value" << std::endl;
                    }
                }
            }

            // move to parent cse
            if (cse->empty()) {
                if (cse->get_parent() == nullptr) break;
                cse = cse->get_parent();
            }
        }
    }
}  // namespace rpal::cse
