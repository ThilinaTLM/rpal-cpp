#include "utils.hpp"

namespace rpal::options {
    void show_usage() {
        Println("USAGE:");
        Println("\t rpal [-h, --help] [-t, --tokens] [-a, --ast] <filename>");
        exit(0);
    }

    void incorrect_arguments() {
        Println("Incorrect arguments! use -h or --help to get more information.");
        exit(1);
    }

    params::params(int argc, char** argv) {
        for (int i = 1; i < argc; i++) {
            std::string option(argv[i]);
            if (option.at(0) == '-') {
                if (option == "-h" || option == "--help") {
                    this->help = true;
                } else if (option == "-a" || option == "--ast") {
                    this->ast = true;
                } else if (option == "-s" || option == "--st") {
                    this->st = true;
                } else if (option == "-t" || option == "--tokens") {
                    this->tokens = true;
                } else {
                    rpal::options::incorrect_arguments();
                }
            } else {
                if (!this->filename.empty()) rpal::options::incorrect_arguments();
                this->filename = option;
            }
        }
    }
}  // namespace rpal::options
