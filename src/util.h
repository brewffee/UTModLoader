#ifndef UTIL_H
#define UTIL_H

// Returns EXIT_FAILURE if expr is true
#define FAIL_IF(expr, msg) if (expr) { std::cerr << msg << std::endl; return EXIT_FAILURE; }

// Returns the value of expr if expr is true (expr should return an error code)
#define FAIL_WITH(expr, msg) if (const auto code = expr; code) { std::cerr << msg << std::endl; return code; }

// Returns an std::error_code value and prints its message if expr is true
// Defines a local variable EC to pass as an argument
#define FAIL_EC(expr, msg) \
std::error_code EC; if (expr; EC) { std::cerr << msg <<"\n" << EC.message() << std::endl; return EC.value(); }

/**
 * Checks if a item exists in a container
 *
 * @param c The container to check
 * @param t The item to search for
 *
 * @return true if the item exists, false otherwise
 */
template<class C, typename T>
static bool find_in(C &c, T t) {
    return std::find(std::begin(c), std::end(c), t) != std::end(c);
}

// Console formatting functions
inline std::string red(const std::string& text) { return "\033[31m" + text + "\033[0m"; }
inline std::string green(const std::string& text) { return "\033[32m" + text + "\033[0m"; }
inline std::string yellow(const std::string& text) { return "\033[33m" + text + "\033[0m"; }
inline std::string blue(const std::string& text) { return "\033[34m" + text + "\033[0m"; }
inline std::string magenta(const std::string &text) { return "\033[35m" + text + "\033[0m"; }
inline std::string cyan(const std::string &text) { return "\033[36m" + text + "\033[0m"; }
inline std::string gray(const std::string &text) { return "\033[90m" + text + "\033[0m"; }
inline std::string bold(const std::string &text) { return "\033[1m" + text + "\033[0m"; }
inline std::string underline(const std::string &text) { return "\033[4m" + text + "\033[0m"; }

#endif // UTIL_H
