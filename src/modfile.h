#ifndef MODFILE_H
#define MODFILE_H

#include <map>

enum ModType: uint8_t {
    UMOD,
    ZIP,
    RAR
};

struct ModFile {
    std::string name{};
    std::filesystem::path path{};
    ModType type{};
    bool ok = false;
};

inline const std::map<std::filesystem::path, ModType> valid_extensions = {
    { ".umod", UMOD },
    { ".zip",  ZIP  },
    { ".rar",  RAR  }
};

#endif // MODFILE_H
