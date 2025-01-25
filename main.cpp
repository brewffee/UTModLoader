// todo: create test mod files for each format

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <unistd.h>
#include <vector>

#if _WIN32
    #include <shlobj.h>
#elif __linux__
    #include <pwd.h>
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
            ModFile m = { file.path().stem().string(), file.path().string(), i -> second };
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

    fs::path store_path;

    #ifdef __linux__
        char* user_dir;
        if ((user_dir = getenv("HOME")) == nullptr) {
            user_dir = getpwuid(getuid()) -> pw_dir;
        }

        if (user_dir == nullptr) {
            std::cerr << "Failed to get user directory" << std::endl;
            return 1;
        }

        store_path = fs::path(user_dir) / ".local/share/UTModLoader/store";
    #elifdef _WIN32
        PWSTR udir;
        if (SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &udir) != S_OK) {
            std::cerr << "Failed to get user directory" << std::endl;
            CoTaskMemFree(udir);
            return 1;
        }

        store_path = fs::path(udir) / "UTModLoader\\store";
        CoTaskMemFree(udir);
    #endif

    std::cout << "Store path: " << store_path << std::endl;

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
