#ifndef RPAL_TOKEN_H
#define RPAL_TOKEN_H

#include <fstream>
#include <iostream>
#include <sstream>

namespace rpal::token {

    typedef enum { Id, Int, Op, Str, Sp, Cmt, Pun, Key, End } Type;

    static const char* token_types[] = {"Identifier", "Integer", "Operator",    "String",
                                        "Delete",     "Comment", "Punctuation", "Keyword", "End"};
    std::ostream& operator<<(std::ostream& os, Type t);

    struct Token {
        Type type;
        const void* value;
        explicit Token(Type type);
        Token(Type type, const void* value);

        template <class T>
        T  get_value() const;

        friend std::ostream &operator<<(std::ostream &os, const Token &t);
        friend bool operator==(Token const &left, Token const &right);
        friend bool operator!=(Token const &left, Token const &right);
        friend bool operator==(Token const &left, Type const &right);
        friend bool operator!=(Token const &left, Type const &right);
    };

    // token source
    std::string read_source_code(std::string& filename);

    class TokenSource {
       private:
        std::string* source_str;
        int pointer;
        int lookup;
        int line_number;
        int column_number;

       public:
        explicit TokenSource(std::string& filename);

        char next();
        char read();
        std::string commit();
        void reset();

        Token* get_next_token();
    };

}  // namespace rpal::token

#endif
