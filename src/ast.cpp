//
// Created by tlm on 10/30/21.
//

#include "ast.hpp"

#define d_token(name, type, value)                                      \
    token::Token name() {                                               \
        const static token::Token t = {type, new std::string(value)};   \
        return t;                                                       \
    }

#define d_operator(name, value) d_token(name, token::Type::Op, value)

#define d_keyword(name, value) d_token(name, token::Type::Key, value)

#define d_keyword_same(name) d_token(name, token::Type::Key, #name)

namespace rpal::parser {
    // Node Type
    std::ostream& operator<<(std::ostream& os, const Type& nt) {
        static const char* node_types[] = {
            "let", "lambda",   "where", "gamma", "@",  "rec", "within", "tau", "aug", "->",   "and", "=",
            "ID",  "fcn_form", "()",    ",",     "+",  "-",   "*",      "/",   "neg", "**",   "or",  "&",
            "not", "gr",       "ge",    "ls",    "le", "eq",  "ne",     "INT", "STR", "BOOL", "nil", "unknown",
        };
        os << node_types[nt];
        return os;
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

    const void* AstNode::get_value() const {
        return this->value;
    }

    void AstNode::add_child(AstNode* node) {
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

    AstNode* AstNode::get_left() {
        return this->left;
    }

    AstNode* AstNode::get_right() {
        return this->right;
    }
    AstNode* AstNode::get_next() {
        return this->next;
    }
    bool AstNode::leave() {
        return (this->left == nullptr) && (this->right == nullptr);
    }

    Token* AstNode::get_token() {
        return this->token;
    }


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
            throw std::runtime_error("reach token end");
        }
        return this->tokens[pointer];
    }

    Token Parser::getToken(uint pass) {
        if (this->tokens.size() <= pointer + pass) {
            throw std::runtime_error("reach token end");
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
                node->add_child(parse_Vb());
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
            node->add_child(node_Ta);
            while (getToken() == symbols::comma()) {
                forward();
                if (getToken() != TT::Id) {
                    throw std::runtime_error("expect identifier after comma");
                }
                node->add_child(parse_Ta());
            }
            return node;
        }
        return node_Ta;
    }

    AstNode* Parser::parse_Ta() {
        // TODO : aug
        return parse_Tc();
    }

    AstNode* Parser::parse_Tc() {
        AstNode* node_B = parse_B();
        Token t = getToken();
        if (t == operators::ternary()) {
            forward();
            auto* node = new AstNode(&t, Type::Ternary);
            node->add_child(node_B);
            node->add_child(parse_Tc());
            if (getForward() != symbols::pipe()) {
                throw std::runtime_error("expect pipe symbol after expression");
            }
            node->add_child(parse_Tc());
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
            auto* node = new AstNode(&t, Type::And, node_Bs, node_Bt);
            return node;
        }
        return node_Bs;
    }

    AstNode* Parser::parse_Bs() {
        Token t = getToken();
        if (t == operators::bool_not()) {
            forward();
            return new AstNode(&t, Type::B_Not, parse_Bp());
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
        if (getToken() == operators::add()) {
            forward();
            return parse_A();
        }

        Token t = getToken();
        if (t == operators::sub()) {
            forward();
            AstNode* node_A = parse_A();
            return new AstNode(&t, Type::Neg, node_A);
        }

        AstNode* node_At = parse_At();
        t = getToken();
        if (t == operators::add()) {
            forward();
            AstNode* node_A = parse_A();
            auto* node = new AstNode(&t, Type::Add, node_At, node_A);
            return node;
        }
        t = getToken();
        if (t == operators::sub()) {
            forward();
            AstNode* node_A = parse_A();
            auto* node = new AstNode(&t, Type::Sub, node_At, node_A);
            return node;
        }

        return node_At;
    }

    AstNode* Parser::parse_At() {
        AstNode* node_Af = parse_Af();
        Token t = getToken();
        if (t == operators::mul()) {
            forward();
            AstNode* node_At = parse_At();
            auto* node = new AstNode(&t, Type::Mul, node_Af, node_At);
            return node;
        }

        t = getToken();
        if (t == operators::div()) {
            forward();
            AstNode* node_At = parse_At();
            auto* node = new AstNode(&t, Type::Div, node_Af, node_At);
            return node;
        }

        return node_Af;
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
            node->add_child(new AstNode(&t2, Type::Id, new std::string(t2.get_value<std::string>())));
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
            return new AstNode(&t2, Type::Str, new std::string(t2.get_value<std::string>()));
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
            node->add_child(node_Dr);
            while (getToken() == keywords::and_kw()) {
                forward();
                node->add_child(parse_Dr());
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
            return new AstNode(&t, Type::Rec, node_Db);
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
                node->add_child(new AstNode(&t, Type::Id, new std::string(t.get_value<std::string>())));
                node->add_child(parse_Vb());

                while (getToken() != operators::assign()) {
                    if (getToken() == TT::Id || getToken() == symbols::open_bracket()) {
                        node->add_child(parse_Vb());
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
                node_list->add_child(node_first);
                while (getToken() == symbols::comma()) {
                    forward();
                    Token next_t = getForward();
                    if (next_t == TT::Id) {
                        node_list->add_child(
                            new AstNode(&next_t, Type::Id, new std::string(next_t.get_value<std::string>())));
                    } else {
                        throw std::runtime_error("expect identifier after period");
                    }
                }
            } else {
                return node_first;
            }
        }
        throw std::runtime_error("expect identifier");
    }

    // generate and standardize
    AstNode* generate_ast(token::TokenSource &source) {
        auto parser = Parser(source);
        return parser.parse();
    }

    AstNode* standard_ast(AstNode* head) {
        if (*head == Type::Let) {
            if (*head->get_left() != Type::Assign) throw std::runtime_error("expect variable assignment");
            auto* X = standard_ast(head->get_left()->get_left());
            auto* E = standard_ast(head->get_left()->get_right());
            auto* P = standard_ast(head->get_right());
            auto* lambda = new AstNode(head->get_left()->get_token(), Type::Lambda, X, P);
            auto* gamma = new AstNode(head->get_token(), Type::Gamma, lambda, E);
            delete head->get_left();
            delete head;
            return gamma;
        } else if (*head == Type::Where) {
            if (*head->get_right() != Type::Assign) throw std::runtime_error("expect variable assignment");
            auto* X = standard_ast(head->get_right()->get_left());
            auto* E = standard_ast(head->get_right()->get_right());
            auto* P = standard_ast(head->get_left());
            auto* lambda = new AstNode(head->get_left()->get_token(), Type::Lambda, X, P);
            auto* gamma = new AstNode(head->get_token(), Type::Gamma, lambda, E);
            delete head->get_left();
            delete head;
            return gamma;
        }
        return head;
    }

    void print_ast(AstNode *node, uint level) {
        using namespace std;
        for (uint i = 0; i < level; i++) cout << '.';
        cout << *node << endl;

        if (node->leave()) return;

        AstNode * nl = node->get_left();
        while (true) {
            print_ast(nl, level + 1);
            if (nl->get_next() == nullptr) break;
            nl = nl->get_next();
        }

        AstNode *nr = node->get_right();
        while (true) {
            print_ast(nr, level + 1);
            if (nr->get_next() == nullptr) break;
            nr = nr->get_next();
        }
    }

    void print_ast(AstNode *head) {
        print_ast(head, 0);
    }

}  // namespace rpal::parser
