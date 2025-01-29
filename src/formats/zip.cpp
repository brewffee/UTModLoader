#include <fstream>
#include <iostream>
#include <vector>
#include <zip.h>

#include "zip.h"
#include "umod.h"
#include "../commands/extract.h"
#include "../util/error.h"
#include "../util/str.h"

namespace fs = std::filesystem;

int extract_zip(const ModFile &mod, const fs::path &store_path) {
    const std::string prefix = gray("[ZIP] ") + magenta("[" + mod.name + "] ");
    std::cout << prefix << "Reading file " << underline(mod.path) << std::endl;

    // Attempt to open the archive in read-only mode
    zip_t *archive = zip_open(str(mod.path).c_str(), ZIP_RDONLY, nullptr);
    FAIL_IF(!archive, "Failed to open archive at " + mod.path);

    // Get the number of entries in the archive
    const zip_int64_t num_entries = zip_get_num_entries(archive, 0);
    FAIL_IF(num_entries == 0, "Archive file " + mod.path + " is empty!");
    FAIL_IF(num_entries == -1, "Failed to get number of entries in archive: " + mod.path);

    for (int i = 0; i < num_entries; ++i) {
        struct zip_stat st{};
        zip_stat_init(&st);

        // Get information about the current entry (filename and uncompressed size)
        if (zip_stat_index(archive, i, 0, &st) == 0) {
            std::cout << prefix << "Extracting " << green(st.name) << std::endl;

            // Look out for any impostors amongst ourselves
            bool is_impostor = false;
            if (std::string(st.name).contains(".umod")) {
                std::cout << prefix << "Found UMOD file " << green(st.name) << " in regular container" << std::endl;
                std::cout << prefix << "Cancelling current operation and extracting UMOD" << std::endl;
                is_impostor = true;
            }

            zip_file *file = zip_fopen_index(archive, i, 0);
            FAIL_IF(!file, "Failed to open file in archive: " + str(st.name));

            // Read the contents of the file and store them into our buffer
            std::vector<char> contents(st.size);
            const zip_uint64_t rstatus = zip_fread(file, contents.data(), st.size);
            FAIL_IF(rstatus != st.size, "Failed to read file " + str(st.name) + " from archive " + mod.path);
            FAIL_WITH(zip_fclose(file), "Failed to close file " + str(st.name) + " from archive " + mod.path);

            // Create the mod's directory if it doesn't exist yet
            const fs::path mod_path = store_path / mod.name;
            const fs::path file_path = mod_path / st.name;
            FAIL_EC(create_directories(file_path.parent_path(), EC), "Error creating paths for " + file_path);

            // Write the extracted contents to file
            std::ofstream file_out(file_path, std::ios::out | std::ios::binary);
            FAIL_IF(!file_out.is_open(), "Failed to open file for writing: " + file_path);

            file_out.write(contents.data(), static_cast<std::streamsize>(st.size));
            FAIL_IF(file_out.fail(), "Failed to write to file " + file_path);

            file_out.close();
            FAIL_IF(file_out.fail(), "Failed to close file " + file_path);

            [[unlikely]] if (is_impostor) {
                // Delete every file in the directory we created except for the UMOD file
                for (const auto &entry: fs::directory_iterator(mod_path)) {
                    if (entry.path().extension() != ".umod") {
                        std::cout << prefix << "Deleting outdated file " << gray(entry.path()) << std::endl;
                        FAIL_WITH(!fs::remove_all(entry.path()), "Failed to remove file " + entry.path());
                    }
                }

                // Pass the UMOD to the appropriate extraction function
                const ModFile umod = get_mod_file(mod_path / st.name);
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

    FAIL_WITH(zip_close(archive), "Failed to close archive " + mod.path); // tragic
    return 0; // :D
}
