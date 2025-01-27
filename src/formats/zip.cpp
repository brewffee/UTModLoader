#include <fstream>
#include <iostream>
#include <vector>
#include <zip.h>

#include "zip.h"
#include "../util.h"

// todo: check if this zip is secretly just a container for a umod file

namespace fs = std::filesystem;
using str = std::string;

int extract_zip(const ModFile &mod, const fs::path &store_path) {
    const std::string prefix = gray("[ZIP] ") + magenta("["+mod.name+"] ");
    std::cout << prefix << " Reading file " << underline(mod.path) << std::endl;

    // Attempt to open the archive in read-only mode
    zip_t *archive = zip_open(mod.path.c_str(), ZIP_RDONLY, nullptr);
    FAIL_IF(!archive, "Failed to open archive at " + mod.path);

    // Get the number of entries in the archive
    const zip_int64_t num_entries = zip_get_num_entries(archive, 0);
    FAIL_IF(num_entries == 0, "Archive file " + mod.path + " is empty!");
    FAIL_IF(num_entries == -1, "Failed to get number of entries in archive: " + mod.path);

    for (int i = 0; i < num_entries; i++) {
        struct zip_stat st{};
        zip_stat_init(&st);

        // Get information about the current entry (filename and uncompressed size)
        if (zip_stat_index(archive, i, 0, &st) == 0) {
            std::cout << prefix << " Extracting " << green(st.name) << std::endl;

            zip_file *file = zip_fopen_index(archive, i, 0);
            FAIL_IF(!file, "Failed to open file in archive: " + str(st.name));

            // Read the contents of the file and store them into our buffer
            std::vector<char> contents(st.size);
            const zip_int64_t rstatus = zip_fread(file, contents.data(), st.size);
            FAIL_IF(rstatus != st.size, "Failed to read file " + str(st.name) + " from archive " + mod.path);
            FAIL_WITH(zip_fclose(file), "Failed to close file " + str(st.name) + " from archive " + mod.path);

            // Create the mod's directory if it doesn't exist yet
            const fs::path file_path = store_path / mod.name / st.name;
            FAIL_EC(create_directories(file_path.parent_path(), EC), "Error creating paths for " + file_path.string());

            // Write the extracted contents to file
            std::ofstream file_out(file_path, std::ios::out | std::ios::binary);
            FAIL_IF(!file_out.is_open(), "Failed to open file for writing: " + file_path.string());

            file_out.write(contents.data(), static_cast<std::streamsize>(st.size));
            FAIL_IF(file_out.fail(), "Failed to write to file " + file_path.string());

            file_out.close();
            FAIL_IF(file_out.fail(), "Failed to close file " + file_path.string());
        }
    }

    FAIL_WITH(zip_close(archive), "Failed to close archive " + mod.path); // tragic
    return 0; // :D
}
