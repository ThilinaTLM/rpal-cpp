//
// Created by tlm on 10/30/21.
//

#include "ast.hpp"

#define d_token(name, type, value)                                    \
    token::Token name() {                                             \
        const static token::Token t = {type, new std::string(value)}; \
        return t;                                                     \
    }

#define d_operator(name, value) d_token(name, token::Type::Op, value)

#define d_keyword(name, value) d_token(name, token::Type::Key, value)

#define d_keyword_same(name) d_token(name, token::Type::Key, #name)

namespace rpal::parser {

    // Node Type
    std::ostream& operator<<(std::ostream& os, const Type& nt) {
        static const char* node_types[] = {"let", "lambda", "where", "gamma",   "@",    "rec",      "within", "tau",
                                           "aug", "->",     "and",   "=",       "ID",   "fcn_form", "()",     ",",
                                           "+",   "-",      "*",     "/",       "neg",  "**",       "or",     "not",
                                           "&",   "gr",     "ge",    "ls",      "le",   "eq",       "ne",     "INT",
                                           "STR", "BOOL",   "nil",   "unknown", "dummy"};
        os << node_types[nt];
        return os;
    }

    AstNode::AstNode(Type type) {
        this->type = type;
        this->token = nullptr;
        this->left = nullptr;
        this->right = nullptr;
        this->next = nullptr;
        this->value = nullptr;
    }

    // Ast Node constructors
    AstNode::AstNode(Token* token, Type type) {
        this->type = type;
        this->token = token;
        this->left = nullptr;
        this->right = nullptr;
        this->next = nullptr;
        this->value = nullptr;
    }

    AstNode::AstNode(Token* token, Type type, const void* value) {
        this->type = type;
        this->token = token;
        this->value = value;
        this->left = nullptr;
        this->right = nullptr;
        this->next = nullptr;
    }

    AstNode::AstNode(Type type, AstNode* left, AstNode* right) {
        this->type = type;
        this->token = nullptr;
        this->value = nullptr;
        this->left = left;
        this->right = right;
        this->next = nullptr;
    }

    AstNode::AstNode(Token* token, Type type, AstNode* left, AstNode* right) {
        this->type = type;
        this->token = token;
        this->value = nullptr;
        this->left = left;
        this->right = right;
        this->next = nullptr;
    }

    AstNode::AstNode(Type type, const void* value) {
        this->type = type;
        this->value = value;
    }

    AstNode::~AstNode() {
        //        if (this->value != nullptr) {
        //            if (this->type == Type::Int) {
        //                delete (int*)this->value;
        //            } else if (this->type == Type::Bool) {
        //                delete (bool*)this->value;
        //            } else {
        //                delete (std::string*)this->value;
        //            }
        //        }
    }

    // AstNode overloads
    std::ostream& operator<<(std::ostream& os, const AstNode& n) {
        if (n.type == Type::Id || n.type == Type::Str) {
            os << "<" << n.type << ":" << n.get_value<std::string>() << ">";
            return os;
        }
        if (n.type == Type::Bool) {
            os << "<" << n.type << ":" << n.get_value<bool>() << ">";
            return os;
        }
        if (n.type == Type::Int) {
            os << "<" << n.type << ":" << n.get_value<int>() << ">";
            return os;
        }
        os << n.type;
        return os;
    }

    bool operator==(AstNode& left, AstNode& right) {
        if (left.type == Type::Id || left.type == Type::Str) {
            return (left.type == right.type) && (left.get_value<std::string>() == right.get_value<std::string>());
        }
        if (left.type == Type::Bool) {
            return (left.type == right.type) && (left.get_value<bool>() == right.get_value<bool>());
        }
        if (left.type == Type::Int) {
            return (left.type == right.type) && (left.get_value<int>() == right.get_value<int>());
        }
        return left.type == right.type;
    }

    bool operator==(AstNode& left, Type type) { return left.type == type; }

    template <class T>
    T AstNode::get_value() const {
        return *(T*)this->value;
    }

    const void* AstNode::get_value() const { return this->value; }

    void AstNode::add_child_left(AstNode* node) {
        if (this->left == nullptr) {
            this->left = node;
            return;
        }
        auto* last = this->left;
        while (last->next != nullptr) last = last->next;
        last->next = node;
    }

    void AstNode::add_child_right(AstNode* node) {
        if (this->right == nullptr) {
            this->right = node;
            return;
        }
        auto* last = this->right;
        while (last->next != nullptr) last = last->next;
        last->next = node;
    }

    void AstNode::set_left_child(AstNode* node) { this->left = node; }

    void AstNode::set_right_child(AstNode* node) { this->right = node; }

    void AstNode::clear_relations() {
        this->left = nullptr;
        this->right = nullptr;
        this->next = nullptr;
    }

    void AstNode::clear_next() { this->next = nullptr; }

    AstNode* AstNode::get_left() { return this->left; }

    AstNode* AstNode::get_right() { return this->right; }
    AstNode* AstNode::get_next() { return this->next; }
    bool AstNode::leave() { return (this->left == nullptr) && (this->right == nullptr); }

    Token* AstNode::get_token() { return this->token; }
    bool operator!=(AstNode& left, Type type) { return !(left == type); }

    Type AstNode::get_type() { return this->type; }

    // parser pre-made tokens
    namespace keywords {
        d_keyword_same(let)

            d_keyword_same(in)

                d_keyword_same(fn)

                    d_keyword_same(where)

                        d_keyword_same(aug)

                            d_keyword_same(within)

                                d_keyword(and_kw, "and")

                                    d_keyword_same(rec)

                                        d_keyword(v_true, "true")

                                            d_keyword(v_false, "false")

                                                d_keyword(v_nil, "nil")

                                                    d_keyword(v_dummy, "dummy")
    }

    namespace operators {

        d_operator(ternary, "->")

            d_operator(assign, "=")

                d_operator(add, "+")

                    d_operator(sub, "-")

                        d_operator(mul, "*")

                            d_operator(div, "/")

                                d_operator(at, "@")

                                    d_operator(pow, "**")

                                        d_operator(bool_gr, ">")

                                            d_operator(bool_ge, ">=")

                                                d_operator(bool_ls, "<")

                                                    d_operator(bool_le, "<=")

                                                        d_operator(bool_or, "or")

                                                            d_operator(bool_not, "not")

                                                                d_operator(bool_gr_kw, "gr")

                                                                    d_operator(bool_ge_kw, "ge")

                                                                        d_operator(bool_ls_kw, "ls")

                                                                            d_operator(bool_le_kw, "le")

                                                                                d_operator(bool_eq, "eq")

                                                                                    d_operator(bool_ne, "ne")

                                                                                        d_operator(bool_and, "&")
    }

    namespace symbols {
        d_token(dot, token::Type::Op, ".")

            d_token(pipe, token::Type::Op, "|")

                d_token(open_bracket, token::Type::Pun, "(")

                    d_token(close_bracket, token::Type::Pun, ")")

                        d_token(comma, token::Type::Pun, ",")

                            d_token(semicolon, token::Type::Pun, ";")
    }

    // parser
    Parser::Parser(token::TokenSource& source) {
        Token* t;
        while ((t = source.get_next_token()) != nullptr) this->tokens.push_back(*t);
        this->pointer = 0;
    }

    Token Parser::getToken() {
        if (this->tokens.size() <= pointer) {
            return *(new Token(TT::End));
        }
        return this->tokens[pointer];
    }

    Token Parser::getToken(uint pass) {
        if (this->tokens.size() <= pointer + pass) {
            return *(new Token(TT::End));
        }
        return this->tokens[pointer + pass];
    }

    uint Parser::forward() {
        if (this->tokens.size() - 1 > pointer) {
            return ++this->pointer;
        }
        return this->pointer;
    }

    Token Parser::getForward() {
        if (this->tokens.size() <= pointer) {
            throw std::runtime_error("reach token end");
        }
        return this->tokens[pointer++];
    }

    AstNode* Parser::parse() { return parse_E(); }

    // --- grammar
    AstNode* Parser::parse_E() {
        Token t = getToken();
        if (t == keywords::let()) {
            forward();
            AstNode* node_D = parse_D();  // parse until 'in'
            if (getToken() != keywords::in()) {
                throw std::runtime_error("expect keyword in");
            }
            forward();
            AstNode* node_E = parse_E();
            return new AstNode(&t, Type::Let, node_D, node_E);
        }

        if (t == keywords::fn()) {
            forward();
            auto* node = new AstNode(&t, Type::Lambda);

            do {
                node->add_child_left(parse_Vb());
            } while (getToken() == TT::Id || getToken() == symbols::open_bracket());

            if (getToken() != symbols::dot()) {
                throw std::runtime_error("expect . after identifiers");
            }
            forward();
            node->add_child_right(parse_E());
            return node;
        }

        return parse_Ew();
    }

    AstNode* Parser::parse_Ew() {
        AstNode* node_T = parse_T();
        Token t = getToken();
        if (getToken() == keywords::where()) {
            forward();
            AstNode* node_Dr = parse_Dr();
            return new AstNode(&t, Type::Where, node_T, node_Dr);
        }
        return node_T;
    }

    AstNode* Parser::parse_T() {
        AstNode* node_Ta = parse_Ta();
        Token t = getToken();
        if (getToken() == symbols::comma()) {
            auto* node = new AstNode(&t, Type::Tau);
            node->add_child_left(node_Ta);
            while (getToken() == symbols::comma()) {
                forward();
                t = getToken();
                if (t != TT::Int && t != TT::Str && t != TT::Id) {
                    throw std::runtime_error("expect value after comma");
                }
                node->add_child_left(parse_Ta());
            }
            return node;
        }
        return node_Ta;
    }

    AstNode* Parser::parse_Ta() {
        auto* node_Tc = parse_Tc();
        return parse_Ta_(node_Tc);
    }

    AstNode* Parser::parse_Ta_(AstNode* left) {
        Token t = getToken();
        if (t == keywords::aug()) {
            forward();
            auto* node_Tc = parse_Tc();
            auto* node_aug = new AstNode(&t, Type::Aug, left, node_Tc);
            return parse_Ta_(node_aug);
        }
        return left;
    }

    AstNode* Parser::parse_Tc() {
        AstNode* node_B = parse_B();
        Token t = getToken();
        if (t == operators::ternary()) {
            forward();
            auto* node = new AstNode(&t, Type::Ternary);
            node->add_child_left(node_B);
            node->add_child_right(parse_Tc());
            if (getForward() != symbols::pipe()) {
                throw std::runtime_error("expect pipe symbol after expression");
            }
            node->add_child_right(parse_Tc());
            return node;
        }
        return node_B;
    }

    AstNode* Parser::parse_B() {
        AstNode* node_Bt = parse_Bt();
        Token t = getToken();
        if (t == operators::bool_or()) {
            forward();
            auto* node_B = parse_B();
            auto* node = new AstNode(&t, Type::B_Or, node_Bt, node_B);
            return node;
        }
        return node_Bt;
    }

    AstNode* Parser::parse_Bt() {
        AstNode* node_Bs = parse_Bs();
        Token t = getToken();
        if (t == operators::bool_and()) {
            forward();
            AstNode* node_Bt = parse_Bt();
            auto* node = new AstNode(&t, Type::B_And, node_Bs, node_Bt);
            return node;
        }
        return node_Bs;
    }

    AstNode* Parser::parse_Bs() {
        Token t = getToken();
        if (t == operators::bool_not()) {
            forward();
            auto* node_Bp = parse_Bp();
            return new AstNode(&t, Type::B_Not, node_Bp, nullptr);
        }
        return parse_Bp();
    }

    AstNode* Parser::parse_Bp() {
        AstNode* node_A = parse_A();
        Token t = getToken();
        if (t == operators::bool_gr() || t == operators::bool_gr_kw()) {
            forward();
            AstNode* node_A2 = parse_A();
            return new AstNode(&t, Type::B_Gr, node_A, node_A2);
        }
        if (t == operators::bool_ge() || t == operators::bool_ge_kw()) {
            forward();
            AstNode* node_A2 = parse_A();
            return new AstNode(&t, Type::B_Ge, node_A, node_A2);
        }
        if (t == operators::bool_ls() || t == operators::bool_ls_kw()) {
            forward();
            AstNode* node_A2 = parse_A();
            return new AstNode(&t, Type::B_Ls, node_A, node_A2);
        }
        if (t == operators::bool_le() || t == operators::bool_le_kw()) {
            forward();
            AstNode* node_A2 = parse_A();
            return new AstNode(&t, Type::B_Le, node_A, node_A2);
        }
        if (t == operators::bool_eq()) {
            forward();
            AstNode* node_A2 = parse_A();
            return new AstNode(&t, Type::B_Eq, node_A, node_A2);
        }
        if (t == operators::bool_ne()) {
            forward();
            AstNode* node_A2 = parse_A();
            return new AstNode(&t, Type::B_Ne, node_A, node_A2);
        }
        return node_A;
    }

    AstNode* Parser::parse_A() {
        Token t = getToken();
        if (t == operators::add()) {
            forward();
            return parse_A();
        }

        if (t == operators::sub()) {
            forward();
            auto* node_At = parse_At();
            auto* neg = new AstNode(&t, Type::Neg);
            neg->add_child_left(node_At);
            return parse_A_(neg);
        }

        auto* node_At = parse_At();
        return parse_A_(node_At);
    }

    AstNode* Parser::parse_A_(AstNode* left) {
        Token t = getToken();
        if (t == operators::add()) {
            forward();
            auto* node_At = parse_At();
            auto* node_add = new AstNode(&t, Type::Add, left, node_At);
            return parse_A_(node_add);
        }

        if (t == operators::sub()) {
            forward();
            auto* node_At = parse_At();
            auto* node_sub = new AstNode(&t, Type::Sub, left, node_At);
            return parse_A_(node_sub);
        }

        return left;
    }

    AstNode* Parser::parse_At() {
        AstNode* node_Af = parse_Af();
        return parse_At_(node_Af);
    }

    AstNode* Parser::parse_At_(AstNode* left) {
        Token t = getToken();
        if (t == operators::mul()) {
            forward();
            AstNode* node_Af = parse_Af();
            auto* node_At = new AstNode(&t, Type::Mul, left, node_Af);
            return parse_At_(node_At);
        }

        if (t == operators::div()) {
            forward();
            AstNode* node_Af = parse_Af();
            auto* node_At = new AstNode(&t, Type::Div, left, node_Af);
            return parse_At_(node_At);
        }

        return left;
    }

    AstNode* Parser::parse_Af() {
        AstNode* node_Ap = parse_Ap();
        Token t = getToken();
        if (t == operators::pow()) {
            forward();
            AstNode* node_Af = parse_Af();
            return new AstNode(&t, Type::Pow, node_Ap, node_Af);
        }
        return node_Ap;
    }

    AstNode* Parser::parse_Ap() {
        AstNode* node_R = parse_R();
        Token t = getToken();
        if (t == operators::at()) {
            forward();
            if (getToken() != TT::Id) {
                throw std::runtime_error("expect identifier after the @");
            }
            auto* node = new AstNode(&t, Type::At);
            Token t2 = getForward();
            node->add_child_left(new AstNode(&t2, Type::Id, new std::string(t2.get_value<std::string>())));
            node->add_child_right(parse_R());
            return node;
        }
        return node_R;
    }

    AstNode* Parser::parse_R() {
        AstNode* node_Rn = parse_Rn();
        Token next = getToken();
        if (next == symbols::open_bracket() || next == TT::Id || next == TT::Int || next == TT::Str ||
            next == keywords::v_true() || next == keywords::v_false() || next == keywords::v_nil()) {
            AstNode* node_Rn2 = parse_Rn();
            return new AstNode(&next, Type::Gamma, node_Rn, node_Rn2);
        }
        return node_Rn;
    }

    AstNode* Parser::parse_Rn() {
        Token t = getToken();
        if (t == symbols::open_bracket()) {
            forward();
            AstNode* node_E = parse_E();
            if (getToken() != symbols::close_bracket()) {
                throw std::runtime_error("expect close bracket");
            }
            forward();
            return node_E;
        }

        if (t == TT::Id) {
            Token t2 = getForward();
            return new AstNode(&t2, Type::Id, new std::string(t2.get_value<std::string>()));
        }
        if (t == TT::Str) {
            Token t2 = getForward();
            auto value = t2.get_value<std::string>();
            return new AstNode(&t2, Type::Str, new std::string(value.substr(1, value.length() - 2)));
        }
        if (t == TT::Int) {
            Token t2 = getForward();
            return new AstNode(&t2, Type::Int, new int(t2.get_value<int>()));
        }
        if (t == keywords::v_true()) {
            Token t2 = getForward();
            return new AstNode(&t2, Type::Bool, new bool(true));
        }
        if (t == keywords::v_false()) {
            Token t2 = getForward();
            return new AstNode(&t2, Type::Bool, new bool(false));
        }
        if (t == keywords::v_nil()) {
            Token t2 = getForward();
            return new AstNode(&t2, Type::Nil);
        }
        if (t == keywords::v_dummy()) {
            Token t2 = getForward();
            return new AstNode(&t2, Type::Dummy);
        }

        throw std::runtime_error("unknown token");
    }

    AstNode* Parser::parse_D() {
        AstNode* node_Da = parse_Da();
        Token t = getToken();
        if (t == keywords::within()) {
            forward();
            AstNode* node_D = parse_D();
            return new AstNode(&t, Type::Within, node_Da, node_D);
        }
        return node_Da;
    }

    AstNode* Parser::parse_Da() {
        AstNode* node_Dr = parse_Dr();
        Token t = getToken();
        if (t == keywords::and_kw()) {
            auto* node = new AstNode(&t, Type::And);
            node->add_child_left(node_Dr);
            while (getToken() == keywords::and_kw()) {
                forward();
                node->add_child_left(parse_Dr());
            }
            return node;
        }
        return node_Dr;
    }

    AstNode* Parser::parse_Dr() {
        Token t = getToken();
        if (t == keywords::rec()) {
            forward();
            AstNode* node_Db = parse_Db();
            return new AstNode(&t, Type::Rec, node_Db, nullptr);
        }
        return parse_Db();
    }

    AstNode* Parser::parse_Db() {
        if (getToken() == symbols::open_bracket()) {
            forward();
            AstNode* node = parse_D();
            if (getForward() != symbols::close_bracket()) {
                throw std::runtime_error("expect closing bracket");
            }
            return node;
        }

        Token t = getToken();
        if (t == TT::Id) {
            Token t2 = getToken(1);
            if (t2 == TT::Id || t2 == symbols::open_bracket()) {
                // function form
                auto* node = new AstNode(&t, Type::Fn);
                t = getForward();
                node->add_child_left(new AstNode(&t, Type::Id, new std::string(t.get_value<std::string>())));
                node->add_child_left(parse_Vb());

                while (getToken() != operators::assign()) {
                    if (getToken() == TT::Id || getToken() == symbols::open_bracket()) {
                        node->add_child_left(parse_Vb());
                    } else {
                        throw std::runtime_error("expect = symbol");
                    }
                }
                forward();
                node->add_child_right(parse_E());
                return node;
            }

            AstNode* node_Vl = parse_Vl();
            if (getForward() != operators::assign()) {
                throw std::runtime_error("expect = operator");
            }
            return new AstNode(&t, Type::Assign, node_Vl, parse_E());
        }
        throw std::runtime_error("invalid syntax");
    }

    AstNode* Parser::parse_Vb() {
        Token t = getToken();
        if (t == TT::Id) {
            t = getForward();
            return new AstNode(&t, Type::Id, new std::string(t.get_value<std::string>()));
        }
        if (t == symbols::open_bracket()) {
            forward();
            if (getToken() == symbols::close_bracket()) {
                forward();
                return new AstNode(&t, Type::NoArg);
            }
            AstNode* node = parse_Vl();
            if (getToken() != symbols::close_bracket()) {
                throw std::runtime_error("expect close bracket");
            }
            forward();
            return node;
        }
        return nullptr;
    }

    AstNode* Parser::parse_Vl() {
        Token t = getToken();
        if (t == TT::Id) {
            t = getForward();
            auto* node_first = new AstNode(&t, Type::Id, new std::string(t.get_value<std::string>()));
            Token t2 = getToken();
            if (t2 == symbols::comma()) {
                auto* node_list = new AstNode(&t2, Type::ArgList);
                node_list->add_child_left(node_first);
                while (getToken() == symbols::comma()) {
                    forward();
                    Token next_t = getForward();
                    if (next_t == TT::Id) {
                        node_list->add_child_left(
                            new AstNode(&next_t, Type::Id, new std::string(next_t.get_value<std::string>())));
                    } else {
                        throw std::runtime_error("expect identifier after period");
                    }
                }
                return node_list;
            } else {
                return node_first;
            }
        }
        throw std::runtime_error("expect identifier");
    }

    // generate and standardize
    AstNode* generate_ast(token::TokenSource& source) {
        auto parser = Parser(source);
        return parser.parse();
    }

    AstNode* standard_ast(AstNode* head) {
        if (*head == Type::Let) {
            auto* assign = standard_ast(head->get_left());
            if (*assign != Type::Assign) throw std::runtime_error("expect assignment operator");
            auto* X = standard_ast(assign->get_left());
            auto* E = standard_ast(assign->get_right());
            auto* P = standard_ast(head->get_right());
            auto* lambda = new AstNode(head->get_left()->get_token(), Type::Lambda, X, P);
            auto* gamma = new AstNode(head->get_token(), Type::Gamma, lambda, E);
            delete assign;
            delete head;
            return gamma;

        } else if (*head == Type::Where) {
            auto* assign = standard_ast(head->get_right());
            if (*assign != Type::Assign) throw std::runtime_error("expect variable assignment");

            auto* X = standard_ast(assign->get_left());
            auto* E = standard_ast(assign->get_right());
            auto* P = standard_ast(head->get_left());

            auto* lambda = new AstNode(head->get_left()->get_token(), Type::Lambda, X, P);
            auto* gamma = new AstNode(head->get_token(), Type::Gamma, lambda, E);
            delete assign;
            delete head;
            return gamma;

        } else if (*head == Type::Fn) {
            auto* P = head->get_left();
            auto* Vs = P->get_next();
            P->clear_next();
            auto* E = standard_ast(head->get_right());

            auto* lambda = new AstNode(Type::Lambda, Vs, E);
            auto* assign = new AstNode(head->get_token(), Type::Assign, P, lambda);
            delete head;
            return assign;

        } else if (*head == Type::Rec) {
            static auto* Ystar = new AstNode(Type::Id, new std::string("Ystar"));
            auto* assign = standard_ast(head->get_left());
            if (*assign != Type::Assign) throw std::runtime_error("expect assignment operator");

            auto* name = assign->get_left();
            auto* E = standard_ast(assign->get_right());

            auto* lambda = new AstNode(Type::Lambda, name, E);
            auto* gamma = new AstNode(Type::Gamma, Ystar, lambda);
            return new AstNode(Type::Assign, name, gamma);

        }  else if (*head == Type::Tau) {
            static auto* nil = new AstNode(Type::Nil);
            auto* val = standard_ast(head->get_left());
            auto* node_aug = new AstNode(head->get_token(), Type::Aug, nil, val);
            auto* p_val = val;
            val = val->get_next();
            p_val->clear_next();
            while (val != nullptr) {
                val = standard_ast(val);
                node_aug = new AstNode(head->get_token(), Type::Aug, node_aug, val);
                p_val = val;
                val = val->get_next();
                p_val->clear_next();
            }
            return node_aug;

        } else if (*head == Type::Gamma) {
            auto* std_left = standard_ast(head->get_left());
            auto* std_right = standard_ast(head->get_right());
            head->set_left_child(std_left);
            head->set_right_child(std_right);
            return head;

        } else if (*head == Type::Ternary) {
            static auto* cond_node = new AstNode(Type::Id, new std::string("Cond"));
            static auto* no_arg = new AstNode(Type::NoArg);
            static auto* nil_node = new AstNode(Type::Nil);

            auto* B = standard_ast(head->get_left());
            auto* E1 = new AstNode(Type::Lambda, no_arg, standard_ast(head->get_right()));
            auto* E2 = new AstNode(Type::Lambda, no_arg, standard_ast(head->get_right()->get_next()));
            head->get_right()->clear_next();
            auto* g1 = new AstNode(Type::Gamma, cond_node, B);
            auto* g2 = new AstNode(Type::Gamma, g1, E1);
            auto* g3 = new AstNode(Type::Gamma, g2, E2);
            return new AstNode(Type::Gamma, g3, nil_node);

        } else if (*head == Type::Within) {
            auto* left_assign = standard_ast(head->get_left());
            auto* right_assign = standard_ast(head->get_right());
            if (*left_assign != Type::Assign) throw std::runtime_error("expect assignment left of within");
            if (*right_assign != Type::Assign) throw std::runtime_error("expect assignment right of within");
            auto* X1 = left_assign->get_left();
            auto* E1 = left_assign->get_right();
            auto* X2 = right_assign->get_left();
            auto* E2 = right_assign->get_right();
            auto* lambda = new AstNode(Type::Lambda, X1, E2);
            auto* gamma = new AstNode(Type::Gamma, lambda, E1);
            return new AstNode(Type::Assign, X2, gamma);

        } else if (
            *head == Type::Add || *head == Type::Sub || *head == Type::Mul || *head == Type::Div || *head == Type::Pow ||
            *head == Type::B_Eq || *head == Type::B_Ne || *head == Type::B_Ls || *head == Type::B_Le ||
            *head == Type::B_Gr || *head == Type::B_Ge || *head == Type::B_Or ||  *head == Type::B_And
                   ) {
            auto* L = standard_ast(head->get_left());
            auto* R = standard_ast(head->get_right());
            head->set_left_child(L);
            head->set_right_child(R);
        } else if (*head == Type::Gamma || *head == Type::Lambda) {
            auto* L = standard_ast(head->get_left());
            auto* R = standard_ast(head->get_right());
            head->set_left_child(L);
            head->set_right_child(R);
        } else if (*head == Type::Neg || *head == Type::B_Not) {
            auto* L = standard_ast(head->get_left());
            head->set_left_child(L);
        }
        return head;
    }

    void print_ast(AstNode* node, uint level) {
        using namespace std;
        for (uint i = 0; i < level; i++) cout << '.';
        cout << *node << endl;

        if (node->leave()) return;

        AstNode* nl = node->get_left();
        while (nl != nullptr) {
            print_ast(nl, level + 1);
            if (nl->get_next() == nullptr) break;
            nl = nl->get_next();
        }

        AstNode* nr = node->get_right();
        while (nr != nullptr) {
            print_ast(nr, level + 1);
            if (nr->get_next() == nullptr) break;
            nr = nr->get_next();
        }
    }

    void print_ast(AstNode* head) { print_ast(head, 0); }

}  // namespace rpal::parser
