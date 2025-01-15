#ifndef COLORS_H
#define COLORS_H

#include <string>

// could be better idc
namespace format {
    inline std::string black(const std::string& text) { return "\033[30m" + text + "\033[0m"; }
    inline std::string red(const std::string& text) { return "\033[31m" + text + "\033[0m"; }
    inline std::string green(const std::string& text) { return "\033[32m" + text + "\033[0m"; }
    inline std::string yellow(const std::string& text) { return "\033[33m" + text + "\033[0m"; }
    inline std::string blue(const std::string& text) { return "\033[34m" + text + "\033[0m"; }
    inline std::string magenta(const std::string &text) { return "\033[35m" + text + "\033[0m"; }
    inline std::string cyan(const std::string &text) { return "\033[36m" + text + "\033[0m"; }
    inline std::string white(const std::string &text) { return "\033[37m" + text + "\033[0m"; }
    inline std::string gray(const std::string &text) { return "\033[90m" + text + "\033[0m"; }

    inline std::string bold(const std::string &text) { return "\033[1m" + text + "\033[0m"; }
    inline std::string underline(const std::string &text) { return "\033[4m" + text + "\033[0m"; }
    inline std::string blink(const std::string &text) { return "\033[5m" + text + "\033[0m"; }
    inline std::string reverse(const std::string &text) { return "\033[7m" + text + "\033[0m"; }

    inline std::string reset(const std::string &text) { return "\033[0m" + text + "\033[0m"; }

    inline std::string parens(const std::string &text) { return "(" + text + ")"; }
    inline std::string quotes(const std::string &text) { return "\"" + text + "\""; }
    inline std::string brackets(const std::string &text) { return "[" + text + "]"; }
    inline std::string braces(const std::string &text) { return "{" + text + "}"; }
    inline std::string angle_brackets(const std::string &text) { return "<" + text + ">"; }
}

#endif // COLORS_H
