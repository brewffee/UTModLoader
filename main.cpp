// todo: create test mod files for each format

#include <cstring>
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

#include "config.h"
#include "format.h"
#include "macros.h"
#include "modfile.h"
#include "formats/rar.h"
#include "formats/umod.h"
#include "formats/zip.h"

namespace fs = std::filesystem;
using str = std::string;

std::vector<ModFile> locate_mods(const fs::path &mods_path) {
    std::cout << "Locating mods in " << mods_path << std::endl;

    // Filter out non-mod files
    std::vector<ModFile> mods;
    for (const auto &file: fs::directory_iterator(mods_path)) {
        auto ext = file.path().extension().string();
        if (auto i = valid_extensions.find(ext); i != valid_extensions.end()) {
            const ModFile m = { file.path().stem().string(), file.path().string(), i -> second };
            mods.push_back(m);

            std::cout << "Located possible mod file: " << magenta(file.path().filename().string()) << std::endl;
        }
    }

    return mods;
}

int extract_mods(const std::string &search_path) {
    // Verify the given path exists
    const fs::path mods_path = fs::canonical(search_path);
    FAIL_IF(!exists(mods_path), "Directory does not exist: " + search_path);

    const auto mods = locate_mods(mods_path);
    fs::path store_path;

    #ifdef __linux__
        char* user_dir = nullptr;
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
        std::cout << "Extracting " << mods[i].name << gray(
            "... (" + std::to_string(i + 1) + " of " + std::to_string(mods.size()) + ")"
        ) << std::endl;

        // todo: use returned value
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

    // todo: store mod information in local file
    std::cout << blue("Done!") << std::endl;

    return 0;
}

int main(const int argc, char* argv[]) {
    const std::string usage_str = "Usage: utmodloader [options] <command> <path>\n"
        "Options:\n"
        "\t-h, --help       Display this help message\n"
        "\t-v, --version    Display version information\n"
        "Commands:\n"
        "\textract          Extract mod files. Accepts a directory or a single file path";

    if (argc < 2) {
        std::cout << usage_str << std::endl;
        return 1;
    }

    const std::vector<std::string> args(argv, argv + argc);

    // todo: there's a better way to do this
    for (int i = 1; i < args.size(); i++) {
        const std::string &arg = args[i];

        if (arg == "-h" || arg == "--help") {
            std::cout << usage_str << std::endl;
            return 0;
        }

        if (arg == "-v" || arg == "--version") {
            std::cout << "UTModLoader v" << PROJECT_VERSION << std::endl;
            return 0;
        }

        // Commands
        if (arg == "extract") {
            if (i + 1 < args.size()) {
                extract_mods(args[i + 1]);
                break;
            }

            std::cerr << "No path specified" << std::endl;
            return 1;
        }
    }

    return EXIT_SUCCESS;
}
