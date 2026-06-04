#ifndef _SEQUENCE_PARSER_UTILS_HPP_
#define _SEQUENCE_PARSER_UTILS_HPP_

#include "common.hpp"

#include <filesystem>
#include <regex>
#include <vector>

namespace sequenceParser {

/**
 * @brief Convert a user filter into a regex.
 * A user filter looks like: "foo###.jpg", "foo@.tiff" or "foo%04d.jpg".
 */
std::regex convertFilterToRegex(const std::string& filter, const EDetection detectOptions);

/**
 * @brief Convert user filters into regexes.
 * @see convertFilterToRegex
 */
std::vector<std::regex> convertFilterToRegex(const std::vector<std::string>& filters, const EDetection detectOptions);

/**
 * Detect if the filename is filtered by one of the filter
 *
 * @param[in] filename filename need to be check if it filtered
 * @param[in] filters vector of filters
 * @param[in] detectOptions enable research options (Cf. EDetection in commonDefinitions.hpp )
 *
 * @return return true if the filename is filtered by filter(s)
 */
bool filenameRespectsFilters(const std::string& filename, const std::vector<std::regex>& filters);

bool filepathRespectsAllFilters(const std::filesystem::path& inputPath,
                                const std::vector<std::regex>& filters,
                                const std::string& filename,
                                const EDetection detectOptions);

}  // namespace sequenceParser

#endif
