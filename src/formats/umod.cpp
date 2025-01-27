#include <fstream>
#include <iostream>

#include "umod.h"
#include "../util.h"

namespace fs = std::filesystem;
using str = std::string;

int parse_umod_header(const std::string& filename, UMODHeader& header) {
    std::ifstream file(filename, std::ios::binary);
    FAIL_IF(!file || !file.is_open(), "Error opening file while reading header: " + filename);

    file.seekg(-HEADER_SIZE, std::ios::end);
    FAIL_IF(!file.good(), "Error seeking to header in file: " + filename);

    // Populate the header with the next 20 bytes of data
    file.read(reinterpret_cast<char *>(&header), HEADER_SIZE);
    FAIL_IF(!file.good(), "Error reading header from file: " + filename);

    FAIL_IF(header.magic_number != MAGIC_NUMBER, "Invalid magic number for UMOD file: " + filename);

    return 0; // :D
}

int parse_umod_file_directory(const std::string &filename, UMODFileDirectory &dir, const UMODHeader &header) {
    std::ifstream file(filename, std::ios::binary);
    FAIL_IF(!file || !file.is_open(), "Error opening file while reading file directory: " + filename);

    file.seekg(header.dir_offset);
    FAIL_IF(!file.good(), "Error seeking to file directory in file: " + filename);

    while (file.good() && file.tellg() < header.size - HEADER_SIZE) {
        UMODFileRecord record;

        // The first null byte occurs after the length of the file name. We don't need to read it,
        // as the filename's length can be programmatically determined afterwards.
        // This is definitely not best practice, but it isn't broken ((yet)), so we shan't fix it.
        std::vector<char> entry_name_buf;
        char c{};
        while (file.get(c) && c != '\0') {
            entry_name_buf.push_back(c);
        }
        FAIL_IF(entry_name_buf.empty(), "Failed to read file name in archive: " + filename); // NOOOOO
        FAIL_IF(file.fail(), "Read operation failed while reading file name in archive: " + filename);

        entry_name_buf.push_back('\0'); // Garbage data shows up otherwise
        auto entry_name = std::string(entry_name_buf.data());

        // Trims unknown chars from the beginning of the filename (a consequence of not reading the length first)
        bool found = false;
        for (const auto& root_path: root_paths) {
            if (const std::string::size_type pos = entry_name.find(root_path); pos != std::string::npos) {
                record.filename = entry_name.substr(pos);
                found = true;
                break;
            }
        }

        // Our method for determining the correct location for the file relies entirely upon
        // the file having a recognized path in its name. This should be extremely rare
        // if not nonexistent, but it is still possible.
        FAIL_IF(!found, "Failed to find root path in file name: " + entry_name);

        // Read the file offset, size, and flags
        file.read(reinterpret_cast<char *>(&record.file_offset), sizeof(uint32_t));
        FAIL_IF(file.fail(), "Read operation failed while reading file offset in archive: " + filename);
        file.read(reinterpret_cast<char *>(&record.file_size), sizeof(uint32_t));
        FAIL_IF(file.fail(), "Read operation failed while reading file size in archive: " + filename);
        file.read(reinterpret_cast<char *>(&record.file_flags), sizeof(uint32_t));
        FAIL_IF(file.fail(), "Read operation failed while reading file flags in archive: " + filename);

        dir.records.push_back(record);
    }

    file.close();
    FAIL_IF(file.is_open(), "Failed to close UMOD file while reading file directory: " + filename);
    return 0;
}

int extract_umod_entry(const ModFile &mod, UMODFileRecord &record, const fs::path &store_path) {
    std::ifstream file(mod.path, std::ios::binary);
    FAIL_IF(!file || !file.is_open(), "Error opening file while extracting: " + mod.path);

    if (file.is_open()) {
        // Read the contents of the file
        std::vector<char> contents(record.file_size);
        file.seekg(record.file_offset);
        FAIL_IF(file.fail(), "Error seeking to file offset for file " + record.filename + " in UMOD: " + mod.path);

        file.read(contents.data(), record.file_size);
        FAIL_IF(file.fail(), "Error reading file contents for file " + record.filename + " in from UMOD: " + mod.path);

        file.close();
        FAIL_IF(file.is_open(), "Failed to close UMOD file: " + mod.path + " while extracting: " + record.filename);

        // If we're on linux, translate any \ characters to /
        #ifdef __linux__
            std::replace(record.filename.begin(), record.filename.end(), '\\', '/');
        #endif

        // Create the mod's directory if it doesn't exist yet
        const fs::path file_path = store_path / mod.name / record.filename;
        if (!exists(file_path.parent_path())) {
            FAIL_EC(create_directories(file_path.parent_path(), EC), "Error creating paths for " + str(file_path));
        }

        // todo: there was a reason this was initially converted to an std::string, but until i test this
        //       again i really dont think this is necessary
        // Write the extracted contents to file
        const std::string contents_str(contents.data(), record.file_size);
        std::ofstream out(file_path, std::ios::out | std::ios::binary);
        FAIL_IF(!out.is_open(), "Error opening file for writing: " + file_path.string());

        out << contents_str;
        out.close();
        FAIL_IF(out.is_open(), "Failed to write file: " + file_path.string());
    }

    file.close();
    FAIL_IF(file.is_open(), "Failed to close UMOD file while extracting: " + mod.path);
    return 0;
}

int extract_umod(const ModFile &mod, const fs::path &store_path) {
    const std::string prefix = gray("[UMOD] ") + yellow("["+mod.name+"] ");
    std::cout << prefix << " Reading file " << underline(mod.path) << std::endl;

    // Attempt to parse the UMOD header
    UMODHeader header{};
    FAIL_IF(parse_umod_header(mod.path, header) != 0, "Failed to parse UMOD header: " + mod.path);

    std::cout << prefix << "Successfully parsed UMOD header" << std::endl;
    std::cout << prefix << "Magic Number: 0x" << std::hex << header.magic_number << std::dec << std::endl;
    std::cout << prefix << "Directory Offset: " << header.dir_offset << std::endl;
    std::cout << prefix << "Total Size: " << header.size << std::endl;
    std::cout << prefix << "UMOD Version: " << header.version << std::endl;
    std::cout << prefix << "CRC32 Hash: " << std::hex << header.crc32 << std::dec << std::endl;

    // Attempt to parse the file directory
    UMODFileDirectory dir{};
    FAIL_IF(parse_umod_file_directory(mod.path, dir, header) != 0, "Failed to parse UMOD file directory: " + mod.path);

    std::cout << prefix << "Successfully parsed UMOD file directory" << gray(
        " (" + std::to_string(dir.records.size()) + " entries)"
    ) << std::endl;

    // Extract each file
    for (UMODFileRecord &record: dir.records) {
        std::cout << prefix << "Extracting " << green(record.filename) << gray(
            " (offset: " + std::to_string(record.file_offset) +
            ", size: " + std::to_string(record.file_size) +
            ", flags: 0x" + std::to_string(record.file_flags) +
        ")") << std::endl;

        const int estatus = extract_umod_entry(mod, record, store_path);
        FAIL_WITH(estatus,  "Error extracting entry " + record.filename + " from UMOD: " + mod.path);
    }

    return 0; // awesome !!!!!
}
