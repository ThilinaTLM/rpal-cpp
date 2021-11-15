
#include <iostream>
#include "token.hpp"
#include "utils.hpp"
#include "ast.hpp"
#include "cse.hpp"

int main(int argc, char* argv[]) {
    // parse command line arguments
    rpal::options::params opts(argc, argv);

    // process command line arguments
    if (!opts.help && opts.filename.empty()) rpal::options::incorrect_arguments();
    if (opts.help) {
        rpal::options::show_usage();

    } else if (opts.tokens) {
        auto tc = rpal::token::TokenSource(opts.filename);
        rpal::token::Token* t;
        while ((t = tc.get_next_token()) != nullptr) {
            std::cout << *t << std::endl;
        }
    } else if (opts.ast) {
        auto ts = rpal::token::TokenSource(opts.filename);
        auto* ast = rpal::parser::generate_ast(ts);
        rpal::parser::print_ast(ast);
    } else if (opts.st) {
        auto ts = rpal::token::TokenSource(opts.filename);
        auto* ast = rpal::parser::generate_ast(ts);
        ast = rpal::parser::standard_ast(ast);
        rpal::parser::print_ast(ast);

    } else {
        auto ts = rpal::token::TokenSource(opts.filename);
        auto* ast = rpal::parser::generate_ast(ts);
        ast = rpal::parser::standard_ast(ast);
        rpal::cse::execute(ast);

    }
    return 0;
}
