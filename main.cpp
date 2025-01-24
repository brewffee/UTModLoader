// todo: create test mod files for each format

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <pwd.h>
#include <string>
#include <unistd.h>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
    #include <stdlib.h>
    #define WINDOWS
#endif

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
            ModFile m = { file.path().stem(), file.path(), i -> second };
            mods.push_back(m);

            std::cout << "Located possible mod file: " << magenta(file.path().filename().string()) << std::endl;
        }
    }

    return mods;
}

int main(int argc, char* argv[]) {
    // todo: args
    // Usage: utmodloader [options] <command> <path>
    // Options:
    // -h, --help     Display this help message
    // -v, --version  Display version information
    // Commands:
    // extract        Extract mod files. Accepts a directory or a single file path

    // todo: should be invoked by the user instead of running automatically
    const auto mods = locate_mods(fs::current_path().parent_path() / "mods");

    char* user_dir;
    fs::path store_path;

    #ifdef __linux__
        if ((user_dir = getenv("HOME")) == nullptr) {
            user_dir = getpwuid(getuid()) -> pw_dir;
        }

        store_path = fs::path(user_dir) / ".local/share/UTModLoader/store";
    #elif WINDOWS
        // todo: i have no idea if this works yet i just read some random docs for this
        // todo: update there is definitely a better win32 function for this
        size_t len;
        if (errno_t err = _dupenv_s(&user_dir, &len, "APPDATA")) {
            std::cerr << "Failed to get APPDATA environment variable" << std::endl;
            return 1;
        }

        store_path = std::string(user_dir) + "\\UTModLoader\\store";
        free(user_dir);
    #endif

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
        }
    }

    // todo: sometimes ZIP or RAR files could contain a UMOD file hidden inside , check for this after extraction
    // todo: store mod information in local file
    // todo: redo all logging
    std::cout << blue("Done!") << std::endl;

    return 0;
}
