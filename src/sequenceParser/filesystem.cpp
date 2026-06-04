#include "filesystem.hpp"

#include "utils.hpp"

#include "detail/analyze.hpp"
#include "detail/FileNumbers.hpp"
#include "detail/FileStrings.hpp"

#include <regex>
#include <unordered_map>
#include <set>
#include <filesystem>

namespace sequenceParser {

using detail::FileNumbers;
using detail::FileStrings;
using detail::SeqIdHash;
namespace fs = std::filesystem;

fs::path getDirectoryFromPath(const fs::path& p)
{
    // if it's not a directory, use the parent directory of the file
    fs::path directory = p.parent_path();
    if (directory.empty())  // relative path
    {
        directory = fs::current_path();
    }
    return directory;
}

bool browseSequence(Sequence& outSequence, const std::string& pattern, const EPattern accept)
{
    outSequence.clear();
    fs::path directory = getDirectoryFromPath(pattern);

    if (!outSequence.initFromPattern(fs::path(pattern).filename().string(), accept))
        return false;  // not recognized as a pattern, maybe a still file

    if (!fs::exists(directory))
        return false;  // an empty sequence

    std::vector<std::string> allTimesStr;
    std::vector<Time> allTimes;

    for (const auto& entry : fs::directory_iterator(directory))
    {
        Time time;
        std::string timeStr;

        // if the file is inside the sequence
        if (outSequence.isIn(entry.path().filename().string(), time, timeStr))
        {
            // create a big vector of all times in our sequence
            allTimesStr.push_back(timeStr);
            allTimes.push_back(time);
        }
    }
    if (allTimes.size() < 2)
    {
        if (allTimes.size() == 1)
        {
            outSequence._ranges.push_back(FrameRange(allTimes.front()));
        }
        return true;  // an empty sequence
    }
    std::sort(allTimes.begin(), allTimes.end());
    outSequence._ranges = extractFrameRanges(allTimes);
    return true;  // a real file sequence
}

bool isConsideredAsSingleFile(const Sequence& s, const EDetection detectOptions)
{
    return (detectOptions & eDetectionSequenceNeedAtLeastTwoFiles) && (s.getNbFiles() == 1);
}

std::vector<Item> browse(const fs::path& dir, const EDetection detectOptions, const std::vector<std::string>& filters)
{
    std::vector<Item> output;
    std::string tmpDir(dir.string());
    std::vector<std::string> tmpFilters(filters);
    std::string filename;

    if (!detectDirectoryInResearch(tmpDir, tmpFilters, filename))
        return output;

    const std::vector<std::regex> reFilters = convertFilterToRegex(tmpFilters, detectOptions);

    // variables for sequence detection
    typedef std::unordered_map<FileStrings, std::vector<FileNumbers>, SeqIdHash> SeqIdMap;
    fs::path directory(dir);
    SeqIdMap sequences;
    FileStrings tmpStringParts;  // an object uniquely identify a sequence
    FileNumbers tmpNumberParts;  // the vector of numbers inside one filename

    // for all files in the directory
    for (const auto& entry : fs::directory_iterator(directory))
    {
        // clear previous infos
        tmpStringParts.clear();
        tmpNumberParts.clear();  // (clear but don't realloc the vector inside)

        if (!filepathRespectsAllFilters(entry.path(), reFilters, filename, detectOptions))
            continue;

        // if at least one number detected
        if (decomposeFilename(entry.path().filename().string(), tmpStringParts, tmpNumberParts, detectOptions))
        {
            const SeqIdMap::iterator it(sequences.find(tmpStringParts));
            if (it != sequences.end())  // is already in map
            {
                // append the vector of numbers
                sequences.at(tmpStringParts).push_back(tmpNumberParts);
            }
            else
            {
                // create an entry in the map
                std::vector<FileNumbers> li;
                li.push_back(tmpNumberParts);
                sequences.insert(SeqIdMap::value_type(tmpStringParts, li));
            }
        }
        else
        {
            output.push_back(Item(getTypeFromPath(entry.path()), entry.path()));
        }
    }

    // add sequences in the output vector
    for (SeqIdMap::value_type& p : sequences)
    {
        const std::vector<Sequence> ss = buildSequences(directory, p.first, p.second, detectOptions);

        for (const Sequence& s : ss)
        {
            if (fs::is_directory(directory / s.getFirstFilename()))
            {
                // It's a sequence of directories, so it's not a sequence.
                for (Time t : s.getFramesIterable())
                {
                    fs::path folderPath = directory / s.getFilenameAt(t);
                    output.push_back(Item(getTypeFromPath(folderPath), folderPath));
                }
            }
            else
            {
                // if it's a sequence of 1 file, it could be considered as a sequence or as a single file
                if (isConsideredAsSingleFile(s, detectOptions))
                {
                    output.push_back(Item(getTypeFromPath(directory / s.getFirstFilename()), directory / s.getFirstFilename()));
                }
                else
                {
                    // if it's a sequence with holes, it could be split in several sequences depending on the detect options
                    if ((detectOptions & eDetectionSequenceWithoutHoles) && (s.getFrameRanges().size() > 1))
                    {
                        for (FrameRange f : s.getFrameRanges())
                        {
                            const Sequence sequenceWithoutHoles(
                              s.getPrefix(), s.getFixedPadding(), s.getMaxPadding(), s.getSuffix(), f.first, f.last, f.step);
                            if (isConsideredAsSingleFile(sequenceWithoutHoles, detectOptions))
                            {
                                output.push_back(Item(getTypeFromPath(directory / sequenceWithoutHoles.getFirstFilename()),
                                                      directory / sequenceWithoutHoles.getFirstFilename()));
                            }
                            else
                            {
                                output.push_back(Item(Sequence(directory, sequenceWithoutHoles), directory));
                            }
                        }
                    }
                    else
                    {
                        output.push_back(Item(Sequence(directory, s), directory));
                    }
                }
            }
        }
    }
    return output;
}

}  // namespace sequenceParser
