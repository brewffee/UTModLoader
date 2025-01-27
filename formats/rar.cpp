#include <fstream>
#include <iostream>
#include <unarr.h>
#include <vector>

#include "rar.h"
#include "../format.h"
#include "../util.h"

// todo: check if this rar is secretly just a container for a umod file

namespace fs = std::filesystem;
using str = std::string;

int extract_rar(const ModFile &mod, const fs::path &store_path) {
    const std::string prefix = gray("[RAR] ") + blue("["+mod.name +"]");
    std::cout << prefix << " Reading file " << underline(mod.path) << std::endl;

    // Attempt to open the archive
    ar_stream *st = ar_open_file(mod.path.c_str());
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
            std::cout << prefix << " Extracting " << green(name) << std::endl;

            // Read the contents of the file and store them into our buffer
            std::vector<char> contents(size);
            const bool ustatus = ar_entry_uncompress(ar, contents.data(), size);
            FAIL_IF(!ustatus, "Failed to decompress file: " + str(name));

            const fs::path file_path = store_path / mod.name / name;
            FAIL_EC(create_directories(file_path.parent_path(), EC), "Error creating paths for " + file_path.string());

            // Write the extracted contents to file
            std::ofstream file_out(file_path, std::ios::out | std::ios::binary);
            FAIL_IF(!file_out.is_open(), "Failed to open file for writing: " + file_path.string());

            file_out.write(contents.data(), static_cast<std::streamsize>(size));
            FAIL_IF(file_out.fail(), "Failed to write to file " + file_path.string());

            file_out.close();
            FAIL_IF(file_out.fail(), "Failed to close file " + file_path.string());

            size = 0;
        }
    }

    ar_close_archive(ar);
    ar_close(st);
    return 0;
}
