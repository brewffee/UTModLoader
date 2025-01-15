#include "umod.h"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

bool readUMODHeader(const std::string& filename, UMODHeader& header) {
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

int readUMODFileDirectory(const std::string &filename, UMODFileDirectory &dir, const UMODHeader &header) {
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

int readUMODFileContents(const std::string &filename, const UMODFileRecord &record, const std::string &mod_name) {
    std::ifstream file(filename, std::ios::binary);
    if (!file || !file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return false;
    }

    if (file.is_open()) {
        // Convert the contents to a string
        char contents[record.file_size];
        file.seekg(record.file_offset);
        file.read(contents, record.file_size);
        file.close();

        std::string contents_str(contents, record.file_size);

        // todo: dont hardcode path
        std::string store_path = "/home/kofy/db/CLionProjects/Unreal/store/" + mod_name;
        if (!std::filesystem::exists(store_path)) {
            std::filesystem::path path(store_path);
            create_directories(path.parent_path());
        }

        // If we're on UNIX, \ needs to be translated to /
        std::string file_path = store_path + "/" + record.filename;
        if constexpr (__linux__ || __linux) { // idk the diff between the two yet
            std::replace(file_path.begin(), file_path.end(), '\\', '/');
        }

        std::filesystem::path path(file_path);
        create_directories(path.parent_path());

        std::ofstream out(file_path, std::ios::out | std::ios::binary);
        out << contents_str;
        out.close();
    }

    file.close();

    return true;
}
