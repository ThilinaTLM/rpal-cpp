#ifndef __RPAL_UTILS_H__
#define __RPAL_UTILS_H__
#include <iostream>
#include <string>

// Macros
#define Println(stuff) std::cout << stuff << std::endl;

namespace rpal::options {
    struct params {
        bool help = false;
        bool ast = false;
        bool st = false;
        bool tokens = false;
        std::string filename;

        params(int argc, char* argv[]);
    };

    void show_usage();
    void incorrect_arguments();
}  // namespace rpal::options


#endif
