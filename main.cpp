// todo: create test mod files for each format

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "format.h"
#include "modfile.h"
#include "formats/rar.h"
#include "formats/umod.h"
#include "formats/zip.h"

namespace fs = std::filesystem;
using namespace format;

std::vector<ModFile> locate_mods(const fs::path &mods_path) {
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

int main() {
    // todo: should be invoked by the user instead of running automatically
    const auto mods = locate_mods("/home/kofy/db/CLionProjects/UTModLoader/mods");

    // todo: use local storage folder
    const fs::path store_path = fs::current_path().parent_path().string() + "/store";

    for (int i = 0; i < mods.size(); i++) {
        std::cout << "Extracting mod " << i + 1 << " of " << mods.size() << std::endl;

        switch (mods[i].type) {
            case UMOD:
                extract_umod(mods[i], store_path);
                break;
            case ZIP:
                extract_zip(mods[i], store_path);
                break;
            case RAR:
                extract_rar(mods[i], store_path);
                break;
            default:
                std::cerr << "Unknown mod type for mod " << mods[i].name << std::endl;
                break;
        }
    }

    // todo: sometimes ZIP or RAR files could contain a UMOD file hidden inside , check for this after extraction
    // todo: store mod information in local file
    // todo: redo all logging
    std::cout << blue("Done!") << std::endl;

    return 0;
}
