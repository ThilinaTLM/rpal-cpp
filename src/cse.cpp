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

    void Cse::set_env(std::string& key, rpal::parser::AstNode* value) {
        this->env[key] = value;
        auto val = this->env[key];
    }

    AstNode* Cse::get_env(std::string& key) {
        if (this->env.find(key) == env.end()) {
            if (this->parent == nullptr)
                throw std::runtime_error("undefined variable");
            else
                return this->parent->get_env(key);
        }
        return this->env[key];
    }

    Cse::Cse() { this->parent = nullptr; }

    Cse::Cse(Cse* parent) { this->parent = parent; }

    Cse* Cse::get_parent() { return this->parent; }

    AstNode* Cse::pop_stack() {
        auto* node = stack.top();
        stack.pop();
        return node;
    }

    AstNode* Cse::pop_control() {
        auto* con = this->control.top();
        this->control.pop();
        return con;
    }

    AstNode* Cse::read_stack() {
        return this->stack.top();
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
        while (n != nullptr) {
            ast_to_cse(n, cse);
            n = n->get_next();
        }

        n = ast->get_right();
        while (n != nullptr) {
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
        throw std::runtime_error("unknown resolve type");
    }

    AstNode *cse_env_resolve(Cse *cse, NT type, AstNode* val_node) {
        if (*val_node == type) return val_node;
        if (*val_node == NT::Id) {
            auto key = val_node->get_value<std::string>();
            return cse->get_env(key);
        }
        throw std::runtime_error("invalid type");
    }

    namespace builtins {
        void Print(AstNode* value) {
            if (*value == NT::Int) {
                std::cout << value->get_value<int>() << std::endl;
            } else if (*value == NT::Str) {
                std::cout << value->get_value<std::string>() << std::endl;
            } else if (*value == NT::Bool) {
                std::cout << value->get_value<bool>() << std::endl;
            } else if (*value == NT::Nil) {
                std::cout << "nil" << std::endl;
            }
        }

        AstNode* Add(AstNode* first, AstNode* second) {
            if (*first != NT::Int || *second != NT::Int) throw std::runtime_error("cannot add non integers");
            int val1 = first->get_value<int>();
            int val2 = second->get_value<int>();
            return new AstNode(NT::Int, new int(val1 + val2));
        }

        AstNode* Sub(AstNode* first, AstNode* second) {
            if (*first != NT::Int || *second != NT::Int) throw std::runtime_error("cannot subtract non integers");
            int val1 = first->get_value<int>();
            int val2 = second->get_value<int>();
            return new AstNode(NT::Int, new int(val1 - val2));
        }
    }

    void execute(AstNode* ast) {
        // generate control stack
        Cse* cse = new Cse();
        ast_to_cse(ast, cse);

        while (true) {
            if (cse->empty()) {
                if (cse->get_parent() == nullptr) break;
                cse = cse->get_parent(); // move to parent cse
                continue;
            }
            auto con = *cse->read_control();

            if (con == NT::Id || con == NT::Int || con == NT::Bool || con == NT::Str || con == NT::Lambda) {
                // move to stack
                cse->move_to_stack();
            } else if (con == NT::Add || con == NT::Sub || con == NT::Mul || con == NT::Div || con == NT::Pow) {
                // arithmetic operations
                cse->pop_control();
                auto* first = cse_env_resolve(cse, NT::Int, cse->pop_stack());
                auto* second = cse_env_resolve(cse, NT::Int, cse->pop_stack());
                if (con == NT::Add) cse->add_stack(builtins::Add(first, second));
                if (con == NT::Sub) cse->add_stack(builtins::Sub(first, second));
            } else if (con == NT::Gamma) {
                cse->pop_control();
                auto* lambda = cse->pop_stack();
                auto* argument = cse->pop_stack();
                if (*lambda == NT::Lambda) {
                    auto key = lambda->get_left()->get_value<std::string>();
                    cse = (Cse*)lambda->get_value();
                    cse->set_env(key, argument);
                } else if (*lambda == NT::Id) {
                    auto name = lambda->get_value<std::string>();
                    if (name == "Print") {
                        builtins::Print(argument);
                    }
                }
            }
        }
    }
}  // namespace rpal::cse
