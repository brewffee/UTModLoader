// todo: create test mod files for each format

#include <bitset>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "format.h"
#include "formats/rar.h"
#include "formats/umod.h"
#include "formats/zip.h"

namespace fs = std::filesystem;
using namespace format;

enum ModType {
    UMOD,
    ZIP,
    RAR
};

class ModFile {
    public:
        std::string name;
        std::string path;
        mutable ModType type;
};

std::map<std::string, ModType> valid_extensions = {
    { ".umod", UMOD },
    { ".zip",  ZIP  },
    { ".rar",  RAR  }
};

std::vector<ModFile> locate_mods() {
    // todo: accept path as argument
    const fs::path mods_path("/home/kofy/db/CLionProjects/UTModLoader/mods");
    std::cout << "Locating mods in " << mods_path << std::endl;

    // Filter out non-mod files
    std::vector<ModFile> mods;
    for (const auto &file: fs::directory_iterator(mods_path)) {
        auto ext = file.path().extension().string();
        if (auto i = valid_extensions.find(ext); i != valid_extensions.end()) {
            ModFile m = { file.path().stem().string(), file.path().string(), i -> second };
            mods.push_back(m);

            // extract files first before assuming they are mod files
            std::cout << "Located possible mod file: " << magenta(file.path().filename().string()) << std::endl;
        }
    }

    return mods;
}

// todo: move to umod.cpp
void extract_umod(const ModFile &mod) {
    UMODHeader header{};
    if (readUMODHeader(mod.path, header)) {
        std::cout << underline(bold("UMOD Header Information for ")) << underline(bold(green(mod.name))) << std::endl;
        std::cout << "Magic Number: " << std::hex << header.magic_number << std::dec << std::endl;
        std::cout << "Directory Offset: " << header.dir_offset << std::endl;
        std::cout << "Total Size: " << header.size << std::endl;
        std::cout << "Version: " << header.version << std::endl;
        std::cout << "CRC32: " << header.crc32 << std::endl << std::endl;
    }

    if (UMODFileDirectory dir{}; readUMODFileDirectory(mod.path, dir, header)) {
        std::cout << underline(bold("UMOD File Directory for ")) << underline(bold(green(mod.name))) << std::endl;

        for (const auto &[name, offset, size, flags]: dir.records) {
            std::cout << "- "
                << std::setw(40) << std::left << name
                << std::setw(50) << std::left <<
                " (offset: " + yellow(std::to_string(offset)) + ", size: "+ yellow(std::to_string(size)) + ")"
                << std::setw(32) << green(" 0x" + std::to_string(flags))
                << std::endl;
        }
        std::cout << std::endl;

        for (const auto &record: dir.records) {
            std::cout<< yellow("Extracting file ") << record.filename << std::endl;
            if (!extractUMODFile(mod.path, record, mod.name)) {
                std::cerr << "Error reading file: " << mod.path << std::endl;
            }
        }
        std::cout << std::endl;

    }
}

int main() {
    const auto mods = locate_mods(); // todo: should be invoked by the user instead of running automatically
    // todo: determine store path somewhere here instead of hardcoding

    for (int i = 0; i < mods.size(); i++) {
        std::cout << "Extracting mod " << i + 1 << " of " << mods.size() << std::endl;

        switch (mods[i].type) {
            case UMOD:
                extract_umod(mods[i]);
                break;
            case ZIP:
                // todo: pass ModFile instance instead of accessing properties directly
                extract_zip(mods[i].path, mods[i].name);
                break;
            case RAR:
                extract_rar(mods[i].path, mods[i].name);
                break;
            default:
                std::cerr << "Unknown mod type for mod " << mods[i].name << std::endl;
                break;
        }
    }

    // todo: sometimes ZIP or RAR files could contain a UMOD file hidden inside , check for this after extraction
    // todo: store mod information in local file
    std::cout << blue("Done!") << std::endl;



    return 0;
}
