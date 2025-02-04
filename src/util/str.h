#ifndef UTIL_H
#define UTIL_H

/**
 * Adds concatenation support for paths on Windows
 */
inline std::string operator+(const std::string &lhs, const std::filesystem::path &rhs) {
 return lhs + rhs.string();
}

inline std::string operator+(const std::filesystem::path &lhs, const std::string &rhs) {
 return lhs.string() + rhs;
}

/**
 * Utility functions to convert various types into a string
 */

inline std::string str(const char* a) { return std::string{a}; }
inline std::string str(const std::filesystem::path &p) { return p.string(); }

template <typename T>
std::string str(const T &i) { return std::to_string(i); }

// Console formatting functions
#define FMT_CODE(name, code) \
inline std::string name(const std::string &s) { return std::format("\033[{}m{}\033[0m", code, s); } \
inline std::string name(const char* a) { return std::format("\033[{}m{}\033[0m", code, a); } \
inline std::string name(const std::filesystem::path &p) { return std::format("\033[{}m{}\033[0m", code, str(p)); }

FMT_CODE(green, 32);
FMT_CODE(yellow, 33);
FMT_CODE(blue, 34);
FMT_CODE(magenta, 35);
FMT_CODE(gray, 90);
FMT_CODE(bold, 1)
FMT_CODE(underline, 4);

#endif // UTIL_H
