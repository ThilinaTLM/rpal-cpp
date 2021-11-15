#include "token.hpp"

#define CharMatch(c, m) (((c)) == (m))
#define OpenBracket(c) CharMatch(c, '(')
#define CloseBracket(c) CharMatch(c, ')')
#define Semicolon(c) CharMatch(c, ';')
#define Period(c) CharMatch(c, ',')
#define Eol(c) CharMatch(c, '\n')
#define Tab(c) CharMatch(c, '\t')
#define Eof(c) CharMatch(c, 0)
#define Underscore(c) CharMatch(c, '_')
#define Space(c) CharMatch(c, ' ')
#define FSlash(c) CharMatch(c, '/')
#define Quote(c) CharMatch(c, '"')
#define Letter(c) (('a' <= ((c)) && ((c)) <= 'z') || ('A' <= ((c)) && ((c)) <= 'Z'))
#define Digit(c) ('0' <= ((c)) && ((c)) <= '9')

bool Symbol(const char c) {
    static std::string operators = R"([+\-*<>&.@/:=~|$!#%^_\[\]{}'?])";
    return operators.find(c) != std::string::npos;
}

namespace rpal::token {

    std::ostream& operator<<(std::ostream& os, Type t) {
        os << token_types[t];
        return os;
    }

    Token::Token(Type type) {
        this->type = type;
        this->value = nullptr;
    }

    Token::Token(Type type, const void* value) {
        this->type = type;
        this->value = value;
    }

    template <class T>
    T Token::get_value() const {
        return *(T*) this->value;
    }

    std::ostream &operator<<(std::ostream &os, const Token &t) {
        if (t.type == Type::Int) {
            os << *(int*)t.value;
        } else {
            os << *(std::string*)t.value;
        }
        return os;
    }

    bool operator==(const Token& left, const Token& right) {
        if (left.type == Type::Int) {
            return (left.type == right.type) && (left.get_value<int>() == right.get_value<int>());
        }
        return (left.type == right.type) && (left.get_value<std::string>() == right.get_value<std::string>());
    }

    bool operator!=(const Token& left, const Token& right) {
        return !(left == right);
    }

    bool operator==(const Token& left, const Type& right) {
        return left.type == right;
    }

    bool operator!=(const Token& left, const Type& right) {
        return left.type != right;
    }

    // lexers
    namespace lexer {
        [[maybe_unused]] bool Identifier(TokenSource& src) {
            src.reset();
            if (Letter(src.read())) {
                char c;
                do c = src.next();
                while (Letter(c) || Digit(c) || Underscore(c));
                return true;
            }
            return false;
        }

        [[maybe_unused]] bool Integer(TokenSource& src) {
            src.reset();
            char c = src.read();
            if (Digit(c)) {
                do c = src.next();
                while (Digit(c));
                return true;
            }
            return false;
        }

        [[maybe_unused]] bool Operator(TokenSource& src) {
            src.reset();
            char c1 = src.next();
            if (Symbol(c1)) {
                char c2 = src.next();
                if (c1 == '-' && Symbol(c2) && (c2 == '>')) {
                    src.next();
                    return true;
                }
                if (c1 == '>' && Symbol(c2) && (c2 == '=')) {
                    src.next();
                    return true;
                }
                if (c1 == '<' && Symbol(c2) && (c2 == '=')) {
                    src.next();
                    return true;
                }
                if (c1 == '*' && Symbol(c2) && (c2 == '*')) {
                    src.next();
                    return true;
                }
                return true;
            }
            return false;
        }

        [[maybe_unused]] bool String(TokenSource& src) {
            src.reset();
            char c = src.next();
            if (Quote(c)) {
                do {
                    c = src.next();
                    if (Eol(c) || Eof(c)) {
                        throw std::runtime_error("unclosed string literal");
                    }
                } while (!Quote(c));
                src.next();
                return true;
            }

            return false;
        }

        [[maybe_unused]] bool Comment(TokenSource& src) {
            src.reset();
            char c = src.next();
            if (FSlash(c) && FSlash(src.next())) {
                do c = src.next();
                while (!Eol(c) && !Eof(c));
                if (Eol(c)) src.next();
                return true;
            }
            return false;
        }

        [[maybe_unused]] bool Delete(TokenSource& src) {
            src.reset();
            char c = src.next();
            if (Space(c) || Eol(c) || Tab(c)) {
                do {
                    c = src.next();
                }
                while(Space(c) || Eol(c) || Tab(c)) ;
                return true;
            }
            return false;
        }

        [[maybe_unused]] bool Punctuation(TokenSource& src) {
            src.reset();
            char c = src.next();
            if (OpenBracket(c) || CloseBracket(c) || Semicolon(c) || Period(c)) {
                src.next();
                return true;
            }
            return false;
        }
    }  // namespace lexer

    // extra processors
    bool is_keyword(const std::string& keyword) {
        static std::string keywords[] = {
            "let", "in", "fn", "where", "aug", "within", "rec", "true", "false", "nil", "dummy",
        };
        static uint size = 11;
        for (uint i = 0; i < size; i++) {
            if (keywords[i] == keyword) {
                return true;
            }
        }
        return false;
    }

    bool is_operator(const std::string& op) {
        static std::string operators[]{
            "=",  "+",  "-",   "*",  "/",  "@",  "**", ">",  ">=", "<",
            "<=", "or", "not", "gr", "ge", "ls", "le", "eq", "ne", "and",
        };
        static uint size = 20;
        for (uint i = 0; i < size; i++) {
            if (operators[i] == op) {
                return true;
            }
        }
        return false;
    }

    // token source
    std::string read_source_code(std::string& filename) {
        std::stringstream source;
        std::ifstream file((const char*)filename.data());
        source << file.rdbuf();
        return source.str();
    }

    TokenSource::TokenSource(std::string& filename) {
        this->source_str = new std::string(read_source_code(filename));
        this->pointer = 0;
        this->lookup = 0;
        this->line_number = 1;
        this->column_number = 1;
    }

    char TokenSource::next() {
        if (pointer + lookup >= source_str->size()) {
            ++lookup;
            return 0;
        }
        return source_str->at(pointer + lookup++);
    }
    char TokenSource::read() {
        if (pointer + lookup >= source_str->size()) {
            return 0;
        }
        return source_str->at(pointer + lookup);
    }
    std::string TokenSource::commit() {
        auto value = source_str->substr(pointer, lookup - 1);
        this->pointer += lookup - 1;
        lookup = 0;
        return value;
    }
    void TokenSource::reset() { this->lookup = 0; }

    Token* TokenSource::get_next_token() {
        this->reset();
        if (Eof(read())) return nullptr;
        if (lexer::Comment(*this) || lexer::Delete(*this)) {
            this->commit();
            return get_next_token();
        } else if (lexer::Identifier(*this)) {
            const auto* value = new std::string(commit());
            if (is_keyword(*value)) {
                return new Token(Type::Key, value);
            } else if (is_operator(*value)) {
                return new Token(Type::Op, value);
            } else {
                return new Token(Type::Id, value);
            }
        } else if (lexer::Integer(*this)) {
            const auto* value = new int(std::stoi(commit()));
            return new Token(Type::Int, value);
        } else if (lexer::Operator(*this)) {
            const auto* value = new std::string(commit());
            return new Token(Type::Op, value);
        } else if (lexer::String(*this)) {
            const auto* value = new std::string(commit());
            return new Token(Type::Str, value);
        } else if (lexer::Punctuation(*this)) {
            const auto* value = new std::string(commit());
            return new Token(Type::Pun, value);
        }
        throw std::runtime_error("invalid token");
    }

}  // namespace rpal::token
