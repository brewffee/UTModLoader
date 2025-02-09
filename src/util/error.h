#ifndef ERROR_H
#define ERROR_H

// Returns EXIT_FAILURE if expr is true
#define FAIL_IF(expr, msg) if (expr) { std::clog << (msg) << std::endl; return EXIT_FAILURE; }

// Returns the value of expr if expr is true (expr should return an error code)
#define FAIL_WITH(expr, msg) if (const auto code = expr; code) { std::clog << (msg) << std::endl; return code; }

// Returns an std::error_code value and prints its message if expr is true
// Defines a local variable EC to pass as an argument
#define FAIL_EC(expr, msg) \
std::error_code EC{}; if (expr; EC) { std::clog << (msg) << "\n" << EC.message() << std::endl; return EC.value(); }

#endif // ERROR_H
