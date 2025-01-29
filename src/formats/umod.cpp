#include <bitset>
#include <fstream>
#include <iostream>

#include "umod.h"
#include "../util/error.h"
#include "../util/str.h"

namespace fs = std::filesystem;

void read_index_byte(const char c, const int i, Index &index) {
    if (index.ok) return; // Why are you running this function a second time?

    int sign = -1; // Positive default (we multiply by -1 at the end)
    constexpr int FIRST_BIT_LEN = 6; // On the first byte, 6 bits contribute to value
    index.prev_bit_len = FIRST_BIT_LEN;

    // First byte
    if (i == 0) {
        sign = c & 0x80; /* X0000000 */
        index.value = c & 0x3F; /* 00XXXXXX */

        if ((c & 0x40) == 0) { /* 0X000000 */
            if (sign) index.value *= -1;
            index.ok = true;
        }
    }

    // Final byte
    else if (i == 4) {
        index.value |= (c & 0x80) << index.prev_bit_len; /* X0000000 */

        index.value *= -sign;
        index.ok = true;
    }

    // Middle bytes
    else {
        constexpr int MIDDLE_BIT_LEN = 7;
        index.value |= (c & 0x7F) << index.prev_bit_len; /* 0XXXXXXX */
        index.prev_bit_len += MIDDLE_BIT_LEN;

        if ((c & 0x80) == 0) { /* X0000000 */
            index.value *= -sign;
            index.ok = true;
        }
    }
}

int parse_umod_header(const fs::path &filename, UMODHeader &header) {
    std::ifstream file(filename, std::ios::binary);
    FAIL_IF(!file || !file.is_open(), "Error opening file while reading header: " + filename);

    file.seekg(-HEADER_SIZE, std::ios::end);
    FAIL_IF(!file.good(), "Error seeking to header in file: " + filename);

    // Populate the header with the next 20 bytes of data
    file.read(reinterpret_cast<char*>(&header), HEADER_SIZE);
    FAIL_IF(!file.good(), "Error reading header from file: " + filename);

    FAIL_IF(header.magic_number != MAGIC_NUMBER, "Invalid magic number for UMOD file: " + filename);

    return 0; // :D
}

int parse_umod_file_directory(const fs::path &filename, UMODFileDirectory &dir, const UMODHeader &header) {
    std::ifstream file(filename, std::ios::binary);
    FAIL_IF(!file || !file.is_open(), "Error opening file while reading file directory: " + filename);

    file.seekg(header.dir_offset);
    FAIL_IF(!file.good(), "Error seeking to file directory in file: " + filename);

    int pass = 0;
    while (file.good() && file.tellg() < header.size - HEADER_SIZE) {
        UMODFileRecord record{};

        // Unreal uses a compact integer format to store both the amound of files in the module
        // and the length of each file name

        // Read the number of files in this UMOD
        // We don't really need to use the value afterwards, but it's useful to run this
        // so that way we can properly read the filename length that comes after
        if (pass == 0) {
            Index file_count{};
            for (int i = 0; i < 5; ++i) {
                char c{}; file.get(c);
                read_index_byte(c, i, file_count);
                if (file_count.ok) break;
            }
        }
        ++pass;

        // Get the length of the filename
        Index filename_length{};
        for (int i = 0; i < 5; ++i) {
            char c{}; file.get(c);
            read_index_byte(c, i, filename_length);
            if (filename_length.ok) break;
        }
        record.filename_length = filename_length;

        // Now that we know the filename's length, we can read exactly that amount
        // of bytes worry-free (hopefully)
        record.filename.resize(filename_length.value - 1);
        file.read(&record.filename[0], filename_length.value);
        FAIL_IF(file.fail(), "Read operation failed while reading file name in archive: " + filename); // Why

        // Read the file offset, size, and flags
        file.read(reinterpret_cast<char*>(&record.file_offset), sizeof(uint32_t));
        FAIL_IF(file.fail(), "Read operation failed while reading file offset in archive: " + filename);
        file.read(reinterpret_cast<char*>(&record.file_size), sizeof(uint32_t));
        FAIL_IF(file.fail(), "Read operation failed while reading file size in archive: " + filename);
        file.read(reinterpret_cast<char*>(&record.file_flags), sizeof(uint32_t));
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
        FAIL_IF(file.is_open(), "Failed to close UMOD: " + mod.path + " while extracting: " + record.filename);

        // If we're on linux, translate any \ characters to /
        #ifdef __linux__
            std::replace(record.filename.begin(), record.filename.end(), '\\', '/');
        #endif

        // Create the mod's directory if it doesn't exist yet
        const fs::path file_path = store_path / mod.name / record.filename;
        if (!exists(file_path.parent_path())) {
            FAIL_EC(create_directories(file_path.parent_path(), EC), "Error creating paths for " + file_path);
        }

        // Write the extracted contents to file
        std::ofstream file_out(file_path, std::ios::out | std::ios::binary);
        FAIL_IF(!file_out.is_open(), "Error opening file for writing: " + file_path);

        file_out.write(contents.data(), record.file_size);
        FAIL_IF(file_out.fail(), "Failed to write to file " + file_path);

        file_out.close();
        FAIL_IF(file_out.is_open(), "Failed to close file " + file_path);
    }

    file.close();
    FAIL_IF(file.is_open(), "Failed to close UMOD file while extracting: " + mod.path);
    return 0;
}

int extract_umod(const ModFile &mod, const fs::path &store_path) {
    const std::string prefix = gray("[UMOD] ") + yellow("["+mod.name+"] ");
    std::cout << prefix << "Reading file " << underline(mod.path) << std::endl;

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
            " (len: " + std::to_string(record.filename_length.value-1) +
            ", offset: " + std::to_string(record.file_offset) +
            ", size: " + std::to_string(record.file_size) +
            ", flags: 0x" + std::to_string(record.file_flags) +
        ")") << std::endl;

        const int estatus = extract_umod_entry(mod, record, store_path);
        FAIL_WITH(estatus,  "Error extracting entry " + record.filename + " from UMOD: " + mod.path);
    }

    return 0; // awesome !!!!!
}
