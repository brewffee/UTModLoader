#include <bitset>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "format.h"
#include "formats/umod.h"
#include "formats/zip.h"

namespace fs = std::filesystem;
using namespace format;

// Use mapping of file extension to mod type later
enum ModType {
    UNKNOWN,
    UMOD,
    ZIP
};

class ModFile {
    public:
        std::string name;
        std::string path;
        mutable ModType type;
};

std::vector<ModFile> locate_mods() {
    const std::string mods_path = "/home/kofy/db/CLionProjects/Unreal/mods"; // yea

    // Mods can either be ZIP files or UMOD files, filter other files
    // todo: i forgot about RAR UUUUUUUUUUGHHHGHGHGHGHHHJHHKJHFKJSHKjhjkdfksjkl;
    std::vector<ModFile> mods;
    for (const auto &file: fs::directory_iterator(mods_path)) {
        if (file.path().extension() == ".umod") {
            ModFile m = { file.path().stem().string(), file.path().string(), UMOD };
            mods.push_back(m);
        } else if (file.path().extension() == ".zip") {
            ModFile m = { file.path().stem().string(), file.path().string(), ZIP };
            mods.push_back(m);
        }
    }

    std::cout << "Located " << mods.size() << " mods!" << std::endl;
    std::cout << "====================================================================================" << std::endl;
    for (const auto &[name, path, type]: mods) {
        std::cout
        << std::setw(15) << std::left << yellow(type == UMOD ? "UMOD" : "ZIP")
        << std::setw(36) << std::left << green(bold(name))
        << gray(parens(path))
        << std::endl;
    }
    std::cout << std::endl;

    return mods;
}

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
            if (!readUMODFileContents(mod.path, record, mod.name)) {
                std::cerr << "Error reading file: " << mod.path << std::endl;
            }
        }
        std::cout << std::endl;

    }
}

int main() {
    const auto mods = locate_mods();

    for (int i = 0; i < mods.size(); i++) {
        std::cout << "Extracting mod " << i + 1 << " of " << mods.size() << std::endl;

        switch (mods[i].type) {
            case UMOD:
                extract_umod(mods[i]);
                break;
            case ZIP:
                extract_archive(mods[i].path, mods[i].name);
                break;
            default:
                std::cerr << "Unknown mod type for mod " << mods[i].name << std::endl;
                break;
        }
    }

    std::cout << blue("Done!") << std::endl;

    return 0;
}
