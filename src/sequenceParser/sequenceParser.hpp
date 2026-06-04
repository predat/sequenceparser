#ifndef _SEQUENCE_PARSER_HPP_
#define _SEQUENCE_PARSER_HPP_

/**
 * @file sequenceParser.hpp
 * @brief Convenience umbrella header: include this single file to get the
 *        whole public sequenceParser C++ API.
 *
 * @code
 * #include <sequenceParser/sequenceParser.hpp>
 *
 * for (const sequenceParser::Item& item : sequenceParser::browse("/path"))
 *     if (item.getType() == sequenceParser::eTypeSequence)
 *         std::cout << item.getSequence().getFilenameWithStandardPattern() << '\n';
 * @endcode
 *
 * You still link against libsequenceParser. The detail/ headers and utils.hpp
 * are implementation details and are intentionally not part of this surface.
 */

#include "common.hpp"      // types (Time), enums (EType, EDetection)
#include "FrameRange.hpp"  // FrameRange and frame iteration views
#include "Sequence.hpp"    // Sequence, EPattern
#include "Item.hpp"        // Item, getTypeFromPath
#include "ItemStat.hpp"    // ItemStat (filesystem metadata)
#include "filesystem.hpp"  // browse / browseSequence — the entry points
#include "detector.hpp"

#endif
