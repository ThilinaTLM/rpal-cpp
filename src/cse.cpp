//
// Created by tlm on 10/30/21.
//

#include "cse.hpp"

#include <cmath>

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

    void Cse::add_original(AstNode* node) { this->original.push_back(node); }

    void Cse::reset() {
        while (!this->control.empty()) this->control.pop();
        while (!this->stack.empty()) this->stack.pop();
        for (auto ctrl : this->original) {
            this->add_control(ctrl);
        }
    }

    void Cse::add_stack(AstNode* node) { this->stack.push(node); }

    void Cse::set_env(std::string& key, rpal::parser::AstNode* value) { this->env[key] = value; }

    AstNode* Cse::get_env(std::string& key) {
        if (this->env.find(key) == env.end()) {
            if (this->parent == nullptr)
                throw std::runtime_error("undefined variable " + key);
            else
                return this->parent->get_env(key);
        }
        return this->env[key];
    }

    Cse::Cse() { this->parent = nullptr; }

    Cse::Cse(Cse* parent) { this->parent = parent; }

    Cse* Cse::set_parent(Cse* p) {
        this->parent = p;
        return this;
    }

    Cse* Cse::get_parent() { return this->parent; }

    Cse* Cse::copy() {
        auto* cse = new Cse(this->parent);
        for (auto* n : this->original) {
            cse->add_original(n);
        }

        for (const auto& entry : this->env) {
            cse->set_env(const_cast<std::string&>(entry.first), entry.second);
        }
        return cse;
    }

    AstNode* Cse::pop_stack() {
        if (stack.empty()) {
            return nullptr;
        }
        auto* node = stack.top();
        stack.pop();
        return node;
    }

    AstNode* Cse::pop_control() {
        auto* con = this->control.top();
        this->control.pop();
        return con;
    }

    AstNode* Cse::read_stack() { return this->stack.top(); }

    void ast_to_cse(AstNode* ast, Cse* cse) {
        if (*ast == NT::Lambda) {
            Cse* lambda_cse = new Cse(cse);
            auto* lambda_node = new AstNode(NT::Lambda, lambda_cse);
            lambda_node->add_child_left(ast->get_left());
            cse->add_original(lambda_node);
            ast_to_cse(ast->get_right(), lambda_cse);
            delete ast;
            return;
        }

        cse->add_original(ast);
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

    void cse_set_primitive_env(Cse* cse, std::string* kv) { cse->set_env(*kv, new AstNode(NT::Id, kv)); }

    AstNode* cse_env_resolve(Cse* cse, AstNode* val_node) {
        if (*val_node == NT::Id) {
            auto key = val_node->get_value<std::string>();
            return cse->get_env(key);
        }
        return val_node;
    }

    AstNode* cse_env_resolve(Cse* cse, NT type, AstNode* val_node) {
        if (*val_node == type) return val_node;
        if (*val_node == NT::Id) {
            auto key = val_node->get_value<std::string>();
            return cse->get_env(key);
        }
        throw std::runtime_error("invalid type");
    }

    namespace prebuilt {
        AstNode* node_true() {
            static auto* node = new AstNode(NT::Bool, new bool(true));
            return node;
        }

        AstNode* node_false() {
            static auto* node = new AstNode(NT::Bool, new bool(false));
            return node;
        }
    }  // namespace prebuilt

    namespace builtins {

        void _print(AstNode* value) {
            if (*value == NT::Int) {
                std::cout << value->get_value<int>();
            } else if (*value == NT::Str) {
                std::cout << value->get_value<std::string>();
            } else if (*value == NT::Bool) {
                if (value->get_value<bool>()) {
                    std::cout << "true";
                } else {
                    std::cout << "false";
                }
            } else if (*value == NT::Nil) {
                std::cout << "nil";
            } else if (*value == NT::Dummy) {
                std::cout << "";
            }
        }

        void Print(AstNode* value) {
             if (*value == NT::ArgList) {
                auto* v = value->get_left();
                _print(v);
                v = v->get_next();
                while (v != nullptr) {
                    std::cout << ", ";
                    _print(v);
                    v = v->get_next();
                }
                std::cout << std::endl;
            } else {
                _print(value);
                std::cout << std::endl;
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

        AstNode* Mul(AstNode* first, AstNode* second) {
            if (*first != NT::Int || *second != NT::Int) throw std::runtime_error("cannot multiply non integers");
            int val1 = first->get_value<int>();
            int val2 = second->get_value<int>();
            return new AstNode(NT::Int, new int(val1 * val2));
        }

        AstNode* Div(AstNode* first, AstNode* second) {
            if (*first != NT::Int || *second != NT::Int) throw std::runtime_error("cannot divide non integers");
            int val1 = first->get_value<int>();
            int val2 = second->get_value<int>();
            return new AstNode(NT::Int, new int(val1 / val2));
        }

        AstNode* Pow(AstNode* first, AstNode* second) {
            if (*first != NT::Int || *second != NT::Int) throw std::runtime_error("cannot raise power non integers");
            int val1 = first->get_value<int>();
            int val2 = second->get_value<int>();
            return new AstNode(NT::Int, new int((int)std::pow(val1, val2)));
        }

        AstNode* Neg(AstNode* first) {
            if (*first != NT::Int) throw std::runtime_error("cannot apply neg to non integers");
            int val1 = first->get_value<int>();
            return new AstNode(NT::Int, new int(-val1));
        }

        AstNode* B_Gr(AstNode* first, AstNode* second) {
            if (*first != NT::Int || *second != NT::Int) throw std::runtime_error("cannot compare non integers");
            int val1 = first->get_value<int>();
            int val2 = second->get_value<int>();
            return new AstNode(NT::Bool, new bool(val1 > val2));
        }

        AstNode* B_Ge(AstNode* first, AstNode* second) {
            if (*first != NT::Int || *second != NT::Int) throw std::runtime_error("cannot compare non integers");
            int val1 = first->get_value<int>();
            int val2 = second->get_value<int>();
            return new AstNode(NT::Bool, new bool(val1 >= val2));
        }

        AstNode* B_Ls(AstNode* first, AstNode* second) {
            if (*first != NT::Int || *second != NT::Int) throw std::runtime_error("cannot compare non integers");
            int val1 = first->get_value<int>();
            int val2 = second->get_value<int>();
            return new AstNode(NT::Bool, new bool(val1 < val2));
        }

        AstNode* B_Le(AstNode* first, AstNode* second) {
            if (*first != NT::Int || *second != NT::Int) throw std::runtime_error("cannot compare non integers");
            int val1 = first->get_value<int>();
            int val2 = second->get_value<int>();
            return new AstNode(NT::Bool, new bool(val1 <= val2));
        }

        AstNode* B_Or(AstNode* first, AstNode* second) {
            if (*first != NT::Bool || *second != NT::Bool) throw std::runtime_error("cannot bool or non bool");
            int val1 = first->get_value<bool>();
            int val2 = second->get_value<bool>();
            return new AstNode(NT::Bool, new bool(val1 or val2));
        }

        AstNode* B_And(AstNode* first, AstNode* second) {
            if (*first != NT::Bool || *second != NT::Bool) throw std::runtime_error("cannot bool and non bool");
            int val1 = first->get_value<bool>();
            int val2 = second->get_value<bool>();
            return new AstNode(NT::Bool, new bool(val1 and val2));
        }

        AstNode* B_Eq(AstNode* first, AstNode* second) {
            if (first->get_type() != second->get_type()) return prebuilt::node_false();
            if (first->get_value() == second->get_value()) return prebuilt::node_true();
            bool res;
            if (*first == NT::Int)
                res = ((first->get_value<int>()) == (second->get_value<int>()));
            else if (*first == NT::Str)
                res = ((first->get_value<std::string>()) == (second->get_value<std::string>()));
            else if (*first == NT::Bool)
                res = ((first->get_value<bool>()) == (second->get_value<bool>()));
            else if (*first == NT::Nil || *first == NT::Dummy)
                res = true;
            else
                throw std::runtime_error("incompatible operator 'eq'");
            return (res) ? prebuilt::node_true() : prebuilt::node_false();
        }

        AstNode* B_Ne(AstNode* first, AstNode* second) {
            auto* eq_res = B_Eq(first, second);
            return (eq_res == prebuilt::node_true()) ? prebuilt::node_false() : prebuilt::node_true();
        }

        AstNode* B_Not(AstNode* first) {
            if (*first != NT::Bool) throw std::runtime_error("cannot apply not to non bool");
            int val1 = first->get_value<bool>();
            return new AstNode(NT::Bool, new bool(not val1));
        }

        AstNode* Order(AstNode* pNode) {
            if (*pNode != NT::ArgList) throw std::runtime_error("Order function cannot be work with non-tuples");
            int count = 0;
            auto *arg = pNode->get_left();
            while (arg != nullptr) {
                ++count;
                arg = arg->get_next();
            }
            return new AstNode(NT::Int, new int(count));
        }

        AstNode* Null(AstNode* tuple) {
            auto eq = *tuple == NT::Nil;
            return (eq) ? prebuilt::node_true() : prebuilt::node_false();
        }

        AstNode* IsDataType(AstNode* node, NT type) {
            auto eq = *node == type;
            return (eq) ? prebuilt::node_true() : prebuilt::node_false();
        }

        AstNode* Stem(AstNode* node) {
            if (*node != NT::Str) throw std::runtime_error("Cannot apply stem to non-strings");
            auto val = node->get_value<std::string>();
            return new AstNode(NT::Str, new std::string(val.substr(0, 1)));
        }

        AstNode* Stern(AstNode* node) {
            if (*node != NT::Str) throw std::runtime_error("Cannot apply stem to non-strings");
            auto val = node->get_value<std::string>();
            return new AstNode(NT::Str, new std::string(val.substr(1, val.length() - 1)));
        }

        AstNode* Conc(AstNode* args) {
            auto* s1 = args->get_left();
            if (*s1 != NT::Str) throw std::runtime_error("Cannot apply stem to non-strings; use brackets with Conc");
            auto* s2 = s1->get_next();
            if (*s2 != NT::Str) throw std::runtime_error("Cannot apply stem to non-strings; use brackets with Conc");
            auto val1 = s1->get_value<std::string>();
            auto val2 = s2->get_value<std::string>();
            return new AstNode(NT::Str, new std::string(val1 + val2));
        }

    }  // namespace builtins

    void execute(AstNode* ast) {
        // Create Primitive CSE
        Cse* cse = new Cse();

        // Set PE values
        cse_set_primitive_env(cse, new std::string("Print"));
        cse_set_primitive_env(cse, new std::string("Cond"));
        cse_set_primitive_env(cse, new std::string("Ystar"));
        cse_set_primitive_env(cse, new std::string("Order"));
        cse_set_primitive_env(cse, new std::string("Null"));

        cse_set_primitive_env(cse, new std::string("Isinteger"));
        cse_set_primitive_env(cse, new std::string("Istruthvalue"));
        cse_set_primitive_env(cse, new std::string("Isstring"));
        cse_set_primitive_env(cse, new std::string("Istuple"));
        cse_set_primitive_env(cse, new std::string("Isfunction"));
        cse_set_primitive_env(cse, new std::string("Isdummy"));

        cse_set_primitive_env(cse, new std::string("Stem"));
        cse_set_primitive_env(cse, new std::string("Stern"));
        cse_set_primitive_env(cse, new std::string("Conc"));

        // Fill control set structure
        ast_to_cse(ast, cse);

        // Move control set to control stack
        cse->reset();

        while (true) {
            if (cse->empty()) {
                auto* parent_cse = cse->get_parent();
                if (parent_cse == nullptr) break;
                auto* ret = cse->pop_stack();
                if (ret != nullptr) {
                    if (*ret == NT::Id) ret = cse_env_resolve(cse, ret);
                    parent_cse->add_stack(ret);
                };
                cse = parent_cse;
                continue;
            }
            auto con = *cse->read_control();

            if (con == NT::Id || con == NT::Int || con == NT::Bool || con == NT::Str || con == NT::Lambda ||
                con == NT::Nil || con == NT::Dummy || con == NT::NoArg) {
                cse->move_to_stack();

            } else if (con == NT::Add || con == NT::Sub || con == NT::Mul || con == NT::Div || con == NT::Pow) {
                // arithmetic binary operations
                cse->pop_control();
                auto* first = cse_env_resolve(cse, NT::Int, cse->pop_stack());
                auto* second = cse_env_resolve(cse, NT::Int, cse->pop_stack());
                if (con == NT::Add) cse->add_stack(builtins::Add(first, second));
                if (con == NT::Sub) cse->add_stack(builtins::Sub(first, second));
                if (con == NT::Mul) cse->add_stack(builtins::Mul(first, second));
                if (con == NT::Div) cse->add_stack(builtins::Div(first, second));
                if (con == NT::Pow) cse->add_stack(builtins::Pow(first, second));

            } else if (con == NT::Neg) {
                    // arithmetic binary operations
                    cse->pop_control();
                    auto* first = cse_env_resolve(cse, NT::Int, cse->pop_stack());
                    if (con == NT::Neg) cse->add_stack(builtins::Neg(first));

            } else if (con == NT::B_Or || con == NT::B_And) {
                // boolean binary operations
                cse->pop_control();
                auto* first = cse_env_resolve(cse, NT::Bool, cse->pop_stack());
                auto* second = cse_env_resolve(cse, NT::Bool, cse->pop_stack());
                if (con == NT::B_Or) cse->add_stack(builtins::B_Or(first, second));
                if (con == NT::B_And) cse->add_stack(builtins::B_And(first, second));

            } else if (con == NT::B_Gr || con == NT::B_Ge || con == NT::B_Ls || con == NT::B_Le || con == NT::B_Eq ||
                       con == NT::B_Ne) {
                // comparison operations
                cse->pop_control();
                auto* first = cse_env_resolve(cse, cse->pop_stack());
                auto* second = cse_env_resolve(cse, cse->pop_stack());
                if (con == NT::B_Eq) cse->add_stack(builtins::B_Eq(first, second));
                if (con == NT::B_Ne) cse->add_stack(builtins::B_Ne(first, second));

                if (con == NT::B_Gr) cse->add_stack(builtins::B_Gr(first, second));
                if (con == NT::B_Ge) cse->add_stack(builtins::B_Ge(first, second));
                if (con == NT::B_Ls) cse->add_stack(builtins::B_Ls(first, second));
                if (con == NT::B_Le) cse->add_stack(builtins::B_Le(first, second));
            } else if (con == NT::B_Not) {
                cse->pop_control();
                auto* first = cse_env_resolve(cse, cse->pop_stack());
                if (con == NT::B_Not) cse->add_stack(builtins::B_Not(first));

            } else if (con == NT::Gamma) {
                cse->pop_control();

                auto* lambda = cse->pop_stack();
                auto* argument = cse->pop_stack();

                // resolve lambda and argument from environment
                if (*lambda == NT::Id) lambda = cse_env_resolve(cse, lambda);
                if (*argument == NT::Id) argument = cse_env_resolve(cse, argument);

                if (*lambda == NT::Lambda) {
                    if (*lambda->get_left() == NT::NoArg) {
                        cse = ((Cse*)lambda->get_value())->copy()->set_parent(cse);

                    } else if (*lambda->get_left() == NT::ArgList) {
                        if (*argument != NT::ArgList) throw std::runtime_error("not enough arguments for function call");
                        cse = ((Cse*)lambda->get_value())->copy()->set_parent(cse);
                        auto* arg_name = lambda->get_left()->get_left();
                        auto* arg_value = argument->get_left();
                        while (arg_name != nullptr) {
                            auto key = arg_name->get_value<std::string>();
                            if (arg_value == nullptr) throw std::runtime_error("not enough arguments for function call");
                            cse->set_env(key, arg_value);
                            arg_name = arg_name->get_next();
                            arg_value = arg_value->get_next();
                        }
                        if (arg_value != nullptr) throw std::runtime_error("more arguments provided function call");

                    } else {
                        auto key = lambda->get_left()->get_value<std::string>();
                        cse = ((Cse*)lambda->get_value())->copy()->set_parent(cse);
                        cse->set_env(key, argument);
                    }
                    cse->reset();
                }
                else if (*lambda == NT::Id) {
                    auto name = lambda->get_value<std::string>();
                    if (name == "Print") {
                        builtins::Print(argument);
                    } else if (name == "Cond") {
                        if (*argument != NT::Bool) throw std::runtime_error("invalid condition");
                        cse->pop_control();
                        cse->pop_control();  // pop two gammas
                        auto* lambda1 = cse->pop_stack();
                        auto* lambda2 = cse->pop_stack();
                        if (argument->get_value<bool>()) {
                            cse->add_stack(lambda1);
                        } else {
                            cse->add_stack(lambda2);
                        }
                    } else if (name == "Ystar") {
                        auto fn_name = argument->get_left()->get_value<std::string>();
                        auto arg_cse = *(Cse*)argument->get_value();
                        arg_cse.reset();
                        auto* arg_lambda = arg_cse.pop_control();
                        ((Cse*)arg_lambda->get_value())->set_env(*new std::string(fn_name), arg_lambda);
                        cse->add_stack(arg_lambda);

                    }
                    else if (name == "Order") cse->add_stack(builtins::Order(argument));
                    else if (name == "Null") cse->add_stack(builtins::Null(argument));
                    else if (name == "Isinteger") cse->add_stack(builtins::IsDataType(argument, NT::Int));
                    else if (name == "Istruthvalue") cse->add_stack(builtins::IsDataType(argument, NT::Bool));
                    else if (name == "Isstring") cse->add_stack(builtins::IsDataType(argument, NT::Str));
                    else if (name == "Istuple") cse->add_stack(builtins::IsDataType(argument, NT::ArgList));
                    else if (name == "Isfunction") cse->add_stack(builtins::IsDataType(argument, NT::Lambda));
                    else if (name == "Isdummy") cse->add_stack(builtins::IsDataType(argument, NT::Dummy));

                    else if (name == "Stem") cse->add_stack(builtins::Stem(argument));
                    else if (name == "Stern") cse->add_stack(builtins::Stern(argument));
                    else if (name == "Conc") cse->add_stack(builtins::Conc(argument));
                }
                else if (*lambda == NT::ArgList) {
                    if (*argument != NT::Int) throw std::runtime_error("tuple index must be an integer");
                    int index = argument->get_value<int>();
                    if (index < 1) throw std::runtime_error("tuple index out of range");
                    auto* value = lambda->get_left();
                    if (value == nullptr) throw std::runtime_error("tuple index out of range");
                    while(index-- > 1) {
                        value = value->get_next();
                        if (value == nullptr) throw std::runtime_error("tuple index out of range");
                    }
                    cse->add_stack(value);
                }
            } else if (con == NT::Aug) {
                cse->pop_control();
                auto* arg_list = cse_env_resolve(cse, cse->pop_stack());
                auto* second = cse_env_resolve(cse, cse->pop_stack());
                if (*arg_list == NT::Nil) {
                    arg_list = new AstNode(NT::ArgList);
                }
                if (*arg_list != NT::ArgList)throw std::runtime_error("Cannot use aug with non tuples");
                second->clear_next();
                arg_list->add_child_left(second);
                cse->add_stack(arg_list);
            }
        }
    }
}  // namespace rpal::cse
