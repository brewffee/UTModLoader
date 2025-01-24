#include "umod.h"
#include "../format.h"

#include <filesystem>
#include <fstream>
#include <iostream>

using namespace format;

bool parse_umod_header(const std::string& filename, UMODHeader& header) {
    std::ifstream file(filename, std::ios::binary);
    if (!file || !file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return false;
    }

    file.seekg(-20, std::ios::end);
    if (!file.good()) {
        std::cerr << "Error seeking to header in file: " << filename << std::endl;
        return false;
    }

    file.read(reinterpret_cast<char *>(&header), sizeof(UMODHeader));
    if (!file.good()) {
        std::cerr << "Error reading header from file: " << filename << std::endl;
        return false;
    }

    if (header.magic_number != 0x9FE3C5A3) {
        std::cerr << "Invalid UMOD file: incorrect magic number." << std::endl;
        return false;
    }

    return true;
}

int parse_umod_file_directory(const std::string &filename, UMODFileDirectory &dir, const UMODHeader &header) {
    std::ifstream file(filename, std::ios::binary);
    if (!file || !file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return false;
    }

    file.seekg(header.dir_offset);

    while (file.good() && file.tellg() < header.size - 20) {
        UMODFileRecord record;

        // The first null byte occurs after the length of the file name. We don't need to read it,
        // as the filename's length can be programmatically determined afterwards
        std::vector<char> entry_name_buffer;
        char c;
        while (file.get(c) && c != '\0') {
            entry_name_buffer.push_back(c);
        }
        entry_name_buffer.push_back('\0'); // Garbage data shows up otherwise
        auto entry_name = std::string(entry_name_buffer.data());

        // Trims unknown chars from the beginning of the filename (a consequence of not reading the length first)
        for (const auto& root_path: root_paths) {
            if (std::string::size_type pos = entry_name.find(root_path); pos != std::string::npos) {
                record.filename = entry_name.substr(pos);
                break;
            }
        }

        file.read(reinterpret_cast<char *>(&record.file_offset), sizeof(uint32_t));
        file.read(reinterpret_cast<char *>(&record.file_size), sizeof(uint32_t));
        file.read(reinterpret_cast<char *>(&record.file_flags), sizeof(uint32_t));

        dir.records.push_back(record);
    }

    file.close();

    return true;
}

int extract_umod_entry(const ModFile &mod, const UMODFileRecord &record, const fs::path &store_path) {
    std::ifstream file(mod.path, std::ios::binary);
    if (!file || !file.is_open()) {
        std::cerr << "Error opening file: " << mod.path << std::endl;
        return false;
    }

    if (file.is_open()) {
        char contents[record.file_size];
        file.seekg(record.file_offset);
        file.read(contents, record.file_size);
        file.close();

        std::string contents_str(contents, record.file_size);
        fs::path mod_path = store_path / mod.name;

        if (!exists(mod_path)) {
            create_directories(mod_path.parent_path());
        }

        // If we're on UNIX, \ needs to be translated to /
        fs::path file_path = mod_path / record.filename;
        #ifdef __linux__
            std::replace(file_path.string().begin(), file_path.string().end(), '\\', '/');
        #endif

        create_directories(file_path.parent_path());

        std::ofstream out(file_path, std::ios::out | std::ios::binary);
        out << contents_str;
        out.close();
    }

    file.close();

    return true;
}

void extract_umod(const ModFile &mod, const fs::path &store_path) {
    UMODHeader header{};
    if (parse_umod_header(mod.path, header)) {
        std::cout << underline(bold("UMOD Header Information for ")) << underline(bold(green(mod.name))) << std::endl;
        std::cout << "Magic Number: " << std::hex << header.magic_number << std::dec << std::endl;
        std::cout << "Directory Offset: " << header.dir_offset << std::endl;
        std::cout << "Total Size: " << header.size << std::endl;
        std::cout << "Version: " << header.version << std::endl;
        std::cout << "CRC32: " << header.crc32 << std::endl << std::endl;
    } // todo: error

    if (UMODFileDirectory dir{}; parse_umod_file_directory(mod.path, dir, header)) {
        std::cout << underline(bold("UMOD File Directory for ")) << underline(bold(green(mod.name))) << std::endl;

        for (const auto &[name, offset, size, flags]: dir.records) {
            std::cout << "- "
                << std::setw(40) << std::left << name
                << std::setw(50) << std::left <<
                " (offset: " + yellow(std::to_string(offset)) + ", size: "+ yellow(std::to_string(size)) + ")"
                << std::setw(32) << green(" 0x" + std::to_string(flags))
                << std::endl;
        }
        std::cout << std::endl;

        for (const auto &record: dir.records) {
            std::cout<< yellow("Extracting file ") << record.filename << std::endl;
            if (!extract_umod_entry(mod, record, store_path)) {
                std::cerr << "Error reading file: " << mod.path << std::endl;
            }
        }
        std::cout << std::endl;
    }
}
