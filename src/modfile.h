#ifndef MODFILE_H
#define MODFILE_H

#include <map>
#include <string>

enum ModType: uint8_t {
    UMOD,
    ZIP,
    RAR
};

// todo: modfile.path can probably survive being an fs::path obj
struct ModFile {
    std::string name;
    std::string path;
    ModType type;
    bool ok = false;
};

inline const std::map<std::string, ModType> valid_extensions = {
    { ".umod", UMOD },
    { ".zip",  ZIP  },
    { ".rar",  RAR  }
};

#endif // MODFILE_H
