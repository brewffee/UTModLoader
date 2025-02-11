#ifndef MODDATA_H
#define MODDATA_H

#include <filesystem>
#include <string>
#include <vector>

/**
 * Generic structure for mod data
 */
struct ModData {
    std::string name;                           // Name is derived from the filename
    std::filesystem::path readme;               // Path to the mod's readme.txt or modname.txt
    std::vector<std::filesystem::path> files;   // Paths to each file owned by the mod
    bool is_umod = false;
};

/**
 * Specifies a required game or package for a mod. Currently, only UT and Bonus Pack 4 are supported,
 * but this could be expanded to support other Unreal titles in the future.
 */
struct Requirement {
    std::string game;                   // Name of the game
    std::string product;                // The product name as used in Manifest.ini
    std::vector<std::string> versions;  // List of compatible versions
};

/**
 * This mod requires Unreal Tournament 1999 versions 413, 436, or 469
 */
static const Requirement ut_requirement {
    .game = "Unreal Tournament",
    .product = "UnrealTournament",
    .versions = { "413", "436", "469" },
};

/**
 * This mod requires Unreal Tournament 1999 with Bonus Pack 4 installed
 */
static const Requirement utbp4_requirement {
    .game = "UT Bonuspack 4",
    .product = "UTBonusPack4",
    .versions = { "100" },
};

/**
 * Allows a mod to specify a value in a system INI file
 */
struct IniConfig {
    std::string system_file;    // The name of the system file to modify (e.g. UnrealTournament.ini)
    std::string key;            // The key to modify
    std::string value;          // The value to add
};

/**
 * Describes a file in a mod
 */
struct FileConfig {
    std::filesystem::path src;  // Path to the source file
    uint32_t size;  // todo: likely not necessary
    uint32_t flags;             // Installation flags (should use UMODInstallationFlags here)
};

/**
 * Describes a group of files in a mod for easy installation
 */
struct FileGroup {
    std::string name;               // Name of the group
    std::vector<FileConfig> files;  // Files in the group
    std::vector<IniConfig> inis;    // System INIs to modify
    bool is_setup_group = false;    // Whether or not this is a setup group
};

/**
 * Extended structure for representing UMOD data
 */
struct UMODData: ModData {
    std::string product;                    // The official name of the mod
    std::string version;                    // Its version
    std::vector<Requirement> requirements;  // Required games or packages
    std::vector<FileGroup> file_groups;     // Groups of files to install or modify
    std::string product_url;                // URL to the mod's website
    std::string developer;                  // The developer of the mod
    std::string developer_url;              // URL to the developer's website
    std::filesystem::path logo_path;        // Path to the mod's logo :0
    bool is_umod = true;
};

#endif // MODDATA_H
