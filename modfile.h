#ifndef MODFILE_H
#define MODFILE_H

#include <map>
#include <string>

enum ModType {
    UMOD,
    ZIP,
    RAR
};

struct ModFile {
    std::string name;
    std::string path;
    ModType type;
};

inline std::map<std::string, ModType> valid_extensions = {
    { ".umod", UMOD },
    { ".zip",  ZIP  },
    { ".rar",  RAR  }
};

#endif // MODFILE_H
