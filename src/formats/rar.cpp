#include <fstream>
#include <iostream>
#include <unarr.h>
#include <vector>

#include "rar.h"
#include "umod.h"
#include "../commands/extract.h"
#include "../util/error.h"
#include "../util/str.h"

namespace fs = std::filesystem;

int extract_rar(const ModFile &mod, const fs::path &store_path) {
    const std::string prefix = gray("[RAR] ") + blue("["+mod.name +"] ");
    std::cout << prefix << "Reading file " << underline(mod.path) << std::endl;

    // Attempt to open the archive
    ar_stream *st = ar_open_file(str(mod.path).c_str());
    FAIL_IF(!st, "Failed to open archive at " + mod.path);

    // Verify stream data and access archive
    ar_archive *ar = ar_open_rar_archive(st);
    FAIL_IF(!ar, "Could not read archive data for file " + mod.path);

    // Loop over the file entries
    while (ar_parse_entry(ar)) {
        size_t size = ar_entry_get_size(ar);
        const char *name = ar_entry_get_name(ar);
        FAIL_IF(name == nullptr, "Failed to read file name in archive: " + mod.path);

        // Read the current entry's data
        while (size > 0) {
            std::cout << prefix << "Extracting " << green(name) << std::endl;

            // Look out for any impostors amongst ourselves
            bool is_impostor = false;
            if (std::string(name).contains(".umod")) {
                std::cout << prefix << "Found UMOD file " << green(name) << " in regular container" << std::endl;
                std::cout << prefix << "Cancelling current operation and extracting UMOD" << std::endl;
                is_impostor = true;
            }

            // Read the contents of the file and store them into our buffer
            std::vector<char> contents(size);
            const bool ustatus = ar_entry_uncompress(ar, contents.data(), size);
            FAIL_IF(!ustatus, "Failed to decompress file: " + str(name));

            const fs::path mod_path = store_path / mod.name;
            const fs::path file_path = mod_path / name;
            FAIL_EC(create_directories(file_path.parent_path(), EC), "Error creating paths for " + file_path);

            // Write the extracted contents to file
            std::ofstream file_out(file_path, std::ios::out | std::ios::binary);
            FAIL_IF(!file_out.is_open(), "Failed to open file for writing: " + file_path);

            file_out.write(contents.data(), static_cast<std::streamsize>(size));
            FAIL_IF(file_out.fail(), "Failed to write to file " + file_path);

            file_out.close();
            FAIL_IF(file_out.fail(), "Failed to close file " + file_path);

            size = 0;

            if (is_impostor) {
                // Delete every file in the directory we created except for the UMOD file
                for (const auto &entry: fs::directory_iterator(mod_path)) {
                    if (entry.path().extension() != ".umod") {
                        std::cout << prefix << "Deleting outdated file " << gray(entry.path()) << std::endl;
                        FAIL_WITH(!fs::remove_all(entry.path()), "Failed to remove file " + entry.path());
                    }
                }

                // Pass the UMOD to the appropriate extraction function
                const ModFile umod = get_mod_file(mod_path / name);
                if (const int estatus = extract_umod(umod, store_path); estatus == 0) {
                    // Delete the UMOD file
                    std::cout << prefix << "Deleting UMOD file " << gray(umod.path) << std::endl;
                    FAIL_WITH(!fs::remove(umod.path), "Failed to remove file " + umod.path);

                    // If the old directory is left empty (can occur when the UMOD has different case
                    // or spelling than the original archive), delete it
                    if (is_empty(mod_path)) {
                        std::cout << prefix << "Deleting outdated directory " << gray(mod_path) << std::endl;
                        FAIL_WITH(!fs::remove(mod_path), "Failed to remove directory " + mod_path);
                    }
                } else {
                    return estatus;
                }
            }
        }
    }

    ar_close_archive(ar);
    ar_close(st);
    return 0;
}
