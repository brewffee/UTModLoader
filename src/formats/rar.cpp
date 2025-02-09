// UnRAR only defines these if _UNIX is true for some reason yet unknown to me
#define CALLBACK
#define PASCAL
#define LONG long
#define HANDLE void *
#define LPARAM long
#define UINT unsigned int

#include <cstring>
#include <fstream>
#include <iostream>

#ifdef __linux__
    #include <unrar/dll.hpp>
#elifdef _WIN32
    #include <unrar.h>
#endif

#include "rar.h"
#include "umod.h"
#include "../commands/extract.h"
#include "../util/error.h"
#include "../util/str.h"

namespace fs = std::filesystem;

int extract_rar(const ModFile &mod, const fs::path &store_path) {
    const std::string prefix = gray("[RAR] ") + blue("["+mod.name +"] ");
    std::cout << prefix << "Reading file " << underline(mod.path) << std::endl;

    // Attempt to open the file
    std::ifstream archive_file(mod.path, std::ios::binary);
    FAIL_IF(!archive_file.is_open(), "Failed to open archive at " + mod.path);

    // Prepare to access the archive
    auto *arc_name = new char[str(mod.path).length() + 1];
    strcpy(arc_name, str(mod.path).c_str());

    std::vector<char> cmt_buf(1024);

    RAROpenArchiveDataEx ArchiveData{};
    memset(&ArchiveData, 0, sizeof(ArchiveData));
    ArchiveData.ArcName = arc_name;
    ArchiveData.CmtBuf = cmt_buf.data();
    ArchiveData.CmtBufSize = cmt_buf.size();
    ArchiveData.OpenMode = RAR_OM_EXTRACT;

    HANDLE handle = RAROpenArchiveEx(&ArchiveData);
    FAIL_IF(!handle, "Failed to open archive at " + mod.path);

    // Loop over the file entries
    RARHeaderData HeaderData{};
    while (RARReadHeader(handle, &HeaderData) == 0) {
        const auto* name = reinterpret_cast<const char*>(HeaderData.FileName);
        std::cout << prefix << "Extracting " << green(name) << std::endl;

        // Look out for any impostors amongst ourselves
        bool is_impostor = false;
        if (str(name).contains(".umod")) {
            std::cout << prefix << "Found UMOD file " << green(name) << " in regular container" << std::endl;
            std::cout << prefix << "Cancelling current operation and extracting UMOD" << std::endl;
            is_impostor = true;
        }

        // Create the mod's directory if it doesn't exist yet
        const fs::path mod_path = store_path / mod.name;
        const fs::path file_path = mod_path / name;
        FAIL_EC(create_directories(file_path.parent_path(), EC), "Error creating paths for " + file_path);

        auto *dest_path = new char[str(file_path).length() + 1];
        strcpy(dest_path, str(file_path).c_str());

        // Process the file contents
        const int result = RARProcessFile(handle, RAR_EXTRACT, dest_path, dest_path);
        FAIL_IF(result != ERAR_SUCCESS, "Failed to extract file: " + str(name));

        [[unlikely]] if (is_impostor) {
            // Delete every file in the directory we created except for the UMOD file
            for (const auto &entry: fs::directory_iterator(mod_path)) {
                if (entry.path().extension() != ".umod") {
                    std::cout << prefix << "Deleting outdated file " << gray(entry.path()) << std::endl;
                    FAIL_EC(fs::remove_all(entry.path(), EC), "Failed to remove file " + entry.path());
                }
            }

            // Pass the UMOD to the appropriate extraction function
            const ModFile umod = get_mod_file(mod_path / name);
            if (const int estatus = extract_umod(umod, store_path); estatus == 0) {
                // Delete the UMOD file
                std::cout << prefix << "Deleting UMOD file " << gray(umod.path) << std::endl;
                FAIL_EC(fs::remove(umod.path, EC), "Failed to remove file " + umod.path);

                // If the old directory is left empty (can occur when the UMOD has different case
                // or spelling than the original archive), delete it
                if (is_empty(mod_path)) {
                    std::cout << prefix << "Deleting outdated directory " << gray(mod_path) << std::endl;
                    FAIL_EC(!fs::remove(mod_path, EC), "Failed to remove directory " + mod_path);
                }
            } else {
                return estatus;
            }
        }
    }

    FAIL_WITH(RARCloseArchive(handle), "Failed to close archive " + mod.path);
    return 0;
}
