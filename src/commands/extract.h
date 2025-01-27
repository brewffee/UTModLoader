#ifndef EXTRACT_H
#define EXTRACT_H

#include <filesystem>
#include <string>
#include <vector>

#include "../modfile.h"

/**
 * Checks if the given path is a valid mod file.
 * If the file is not valid, returns an empty ModFile object.
 *
 * @param path The path to the file to extract
 *
 * @return A ModFile object.
 */
ModFile get_mod_file(const std::filesystem::path &path);

/**
 * Finds mod files in the given path. Accepts a directory or a single file path
 *
 * @param path The path to search for mod files
 *
 * @return A vector of OK ModFile objects.
 */
std::vector<ModFile> locate_mods(const std::filesystem::path &path);

/**
 * Extracts mod files from the given path. Accepts a directory or a single file path
 *
 * @param search_path The path to search for mod files
 *
 * @return 0 on success, non-zero on failure
 */
int extract_mods(const std::string &search_path);

#endif // EXTRACT_H
