#ifndef UMOD_H
#define UMOD_H

#include <filesystem>
#include <vector>

#include "../modfile.h"

// Index values are signed integers stored in a compact format, occupying one to five bytes. In the first byte,
//  - the most significant bit (bit 7) specifies the sign of the integer value;
//  - the second-most significant bit (bit 6) is set if the value is continued in the next byte;
//  - and the six remaining bits (bits 5 to 0) are the six least significant bits of the resultant integer value.
//
// Each of the three following bytes (if applicable according to bit 6 of the first byte) contributes seven more
// bits to the final integer value (bits 6 to 0 of each byte), while its most significant bit (bit 7) is set if
// another byte must be read to continue the value. The fifth byte contributes full eight bits to the value. No more
// than five bytes are read for a compact index value.
struct Index {
    int value{};
    int prev_bit_len = 0;   // The amount of bits to shift this index by
    bool ok = false;        // Whether or not the the index is completely finished reading
};

/**
 * Reads a single byte, assuming it's part of an index integer
 *
 * @param c The byte to read
 * @param i Iterator, used to determine which byte is being read
 * @param index The index object to fill
 */
void read_index_byte(char c, int i, Index &index);

// The UMOD file "header" is 20 bytes long. The header is stored in the last 20 bytes of
// the file (hence the quotes around the term "header")
//
// Offset  Length  Type    Description
// 0       4       uint32  Magic number. Used to verify this file as a UMOD installer. Always 0x9FE3C5A3.
// 4       4       uint32  Byte offset of file directory in the UMOD file. (See below.)
// 8       4       uint32  Total byte size of the UMOD file.
// 12      4       uint32  UMOD file version.
// 16      4       uint32  CRC32 checksum over the file content.
struct UMODHeader {
    uint32_t magic_number;      // Magic number. Always 0x9FE3C5A3.
    uint32_t dir_offset;        // Byte offset of file directory
    uint32_t size;              // Total byte size of the UMOD file
    uint32_t version;           // UMOD file version
    uint32_t crc32;             // CRC32 checksum of the file
};
constexpr int32_t HEADER_SIZE = sizeof(UMODHeader);
constexpr uint32_t MAGIC_NUMBER = 0x9FE3C5A3;

// The file records are variable-length records, each describing one file in the UMOD installer.
// The file flags should be 0x3 for Manifest.ini and Manifest.int to prevent those files from being
// copied to the user's System directory, and set to 0x0 for all other files
struct UMODFileRecord {
    Index filename_length;      // Length in bytes of the filename
    std::string filename;       // The name of the file
    uint32_t file_offset = 0;   // Byte offset of the file
    uint32_t file_size = 0;     // Total byte size of the file
    uint32_t file_flags = 0;    // Installation flags
};

// The file directory describes the files stored in the first part of the UMOD file.
// Its byte offset in the UMOD file is given in the file "header"
struct UMODFileDirectory {
    std::vector<UMODFileRecord> records;
};

// Installation flags for UMOD files
enum UMODInstallationFlags: uint8_t {
    NO_COPY = 0x3, // Don't copy this file to the user's System directory
    NONE = 0x0  // Default
};

/**
 * Parses the header of a UMOD file.
 *
 * @param filename The path to the UMOD file.
 * @param header The header struct to be filled.
 *
 * @return 0 on success, 1 on failure
 */
int parse_umod_header(const std::filesystem::path &filename, UMODHeader &header);

/**
 * Parses the file directory of a UMOD file.
 *
 * @param filename The path to the UMOD file.
 * @param dir The file directory struct to be filled.
 * @param header The header of the UMOD file
 *
 * @return 0 on success, 1 on failure
 */
int parse_umod_file_directory(const std::filesystem::path &filename, UMODFileDirectory &dir, const UMODHeader &header);

/**
 * Extracts a single file from a UMOD file.
 *
 * @param mod The ModFile object containing the UMOD.
 * @param record The record of the file to extract.
 * @param store_path The path to store the file in.
 *
 * @return 0 on success, non-zero on failure
 */
int extract_umod_entry(const ModFile &mod, UMODFileRecord &record, const std::filesystem::path &store_path);

/**
 * Extracts the contents of a UMOD file to the specified store path.
 *
 * @param mod The ModFile object containing the UMOD.
 * @param store_path The path to store the files in.
 *
 * @return 0 on success, non-zero on failure.
 */
int extract_umod(const ModFile &mod, const std::filesystem::path &store_path);

#endif // UMOD_H
