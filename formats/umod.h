#ifndef UMOD_H
#define UMOD_H

#include <cstdint>
#include <string>
#include <vector>

// Test for reading UMOD files, a custom file format by Unreal Engine

// The UMOD file "header" is 20 bytes long. The header is stored in the last 20 bytes of
// the file (hence the quotes around the term "header")
//
// Offset 	Length 	Type 	        Description
// 0 	    4 	    unsigned int 	Magic number. Used to verify this file as a UMOD installer. Always 0x9FE3C5A3.
// 4 	    4 	    unsigned int 	Byte offset of file directory in the UMOD file. (See below.)
// 8 	    4 	    unsigned int 	Total byte size of the UMOD file.
// 12 	    4 	    unsigned int 	UMOD file version.
// 16 	    4 	    unsigned int 	CRC32 checksum over the file content.
struct UMODHeader {
    uint32_t magic_number;      // Magic number. Always 0x9FE3C5A3.
    uint32_t dir_offset;        // Byte offset of file directory
    uint32_t size;              // Total byte size of the UMOD file
    uint32_t version;           // UMOD file version
    uint32_t crc32;             // CRC32 checksum of the file
};

// Parses the header of a UMOD file
bool readUMODHeader(const std::string& filename, UMODHeader& header);

// The file directory describes the files stored in the first part of the UMOD file.
// Its byte offset in the UMOD file is given in the file "header" (see above).
//
// The directory consists of an index-type file count (the index data type is described below),
// followed by variable-size records, each describing one file in the UMOD installer.
//
// Offset 	Length 	    Type 	        Description
// 0 	    variable 	index 	        Length of file name (including trailing null byte).
// ?        variable 	char 	        File name, with trailing null byte.
// ?        4 	        unsigned int 	Byte offset of file in UMOD file.
// ?        4 	        unsigned int 	Byte length of file.
// ?        4 	        unsigned int 	Bit field describing file flags. (See below.)
//
// The file flags should be 0x03 for Manifest.ini and Manifest.int to prevent those files from being
// copied to the user's System directory, and set to 0x00 for all other files
struct UMODFileRecord {
    std::string filename;
    uint32_t file_offset = 0;
    uint32_t file_size = 0;
    uint32_t file_flags = 0;
};

struct UMODFileDirectory {
    std::vector<UMODFileRecord> records;
};

namespace UMODFileFlags {
    enum UMODInstallationFlag {
        NoCopy = 0x3, // Don't copy this file to the user's System directory
    };
}

// List of root paths in the UMOD file
inline std::vector<std::string> root_paths = {
    "System", "System64", "SystemLocalized", "Help",
    "Manual", "Maps", "Music", "Patches", "Sounds",
    "Textures", "Web"
};

// Reads and prints the file directory. Note that there are no definite offsets as the number
// and the length of the content of the files are variable
int readUMODFileDirectory(const std::string &filename, UMODFileDirectory &dict, const UMODHeader &header);

// Reads and extracts the contents of a file
int readUMODFileContents(const std::string &filename, const UMODFileRecord &record, const std::string &mod_name);

#endif // UMOD_H
