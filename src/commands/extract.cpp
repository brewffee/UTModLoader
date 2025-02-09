#include "extract.h"
#include "../formats/rar.h"
#include "../formats/umod.h"
#include "../formats/zip.h"
#include "../util/error.h"
#include "../util/str.h"

#if _WIN32
    #include <shlobj.h>
#elif __linux__
    #include <pwd.h>
    #include <unistd.h>
#endif

#include <iostream>

namespace fs = std::filesystem;

ModFile get_mod_file(const fs::path &path) {
    if (const auto i = valid_extensions.find(path.extension()); i != valid_extensions.end()) {
        std::cout << "Located possible mod file: " << magenta(path.filename()) << std::endl;
        return ModFile { str(path.stem()), path, i -> second, true };
    }

    return ModFile{};
}

std::vector<ModFile> locate_mods(const fs::path &path) {
    std::cout << "Locating mods in " << path << std::endl;
    std::vector<ModFile> mods;

    if (is_directory(path)) {
        for (const auto &file: fs::directory_iterator(path)) {
            if (const ModFile m = get_mod_file(file); m.ok) {
                mods.push_back(m);
            }
        }
    } else if (is_regular_file(path)) {
        if (const ModFile m = get_mod_file(path); m.ok) {
            mods.push_back(m);
        } else {
            std::cout << "Invalid mod file " << magenta(path.filename()) << std::endl;
        }
    }

    return mods;
}

int extract_mods(const std::string &search_path) {
    // Verify the given path exists
    const fs::path mods_path = fs::weakly_canonical(search_path);
    FAIL_IF(!exists(mods_path), "Directory does not exist: " + search_path);

    const auto mods = locate_mods(mods_path);

    // This chunk of code will probably be moved elsewhere in the future
    fs::path store_path;
    #ifdef __linux__
        char* user_dir{};
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

    std::cout << "Found store path: " << store_path << std::endl;

    int error{};
    for (std::size_t i = 0; i < mods.size(); ++i) {
        std::cout << "Extracting " << mods[i].name << "... " <<
            gray("(" + str(i + 1) + " of " + str(mods.size()) + ")")
        << std::endl;

        switch (mods[i].type) {
            case UMOD: error = extract_umod(mods[i], store_path); break;
            case ZIP:  error = extract_zip(mods[i], store_path);  break;
            case RAR:  error = extract_rar(mods[i], store_path);  break;
        }
    }

    // todo: store mod information in local file
    if (error == 0) {
        std::cout << blue("Done!") << std::endl;
    } else {
        std::cout << blue("Extraction completed with errors ") << "(" << error << ")" << std::endl;
    }

    return error;
}
