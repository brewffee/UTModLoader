#ifndef MODFILE_H
#define MODFILE_H

#include <map>
#include <string>

enum ModType: uint8_t {
    UMOD,
    ZIP,
    RAR
};

struct ModFile {
    std::string name;
    std::filesystem::path path;
    ModType type;
    bool ok = false;
};

inline const std::map<std::string, ModType> valid_extensions = {
    { ".umod", UMOD },
    { ".zip",  ZIP  },
    { ".rar",  RAR  }
};

#endif // MODFILE_H
