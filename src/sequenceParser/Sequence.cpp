#include "Sequence.hpp"

#include "detail/FileNumbers.hpp"

#include <filesystem>
#include <numeric>
#include <regex>
#include <iterator>
#include <ostream>
#include <cassert>

namespace sequenceParser {

namespace fs = std::filesystem;

/// All regex to recognize a pattern
// common used pattern with # or @
static const std::regex regexPatternStandard("(.*?)"    // anything but without priority
                                             "\\[?"     // if pattern is myimage[####].jpg, don't capture []
                                             "(#+|@+)"  // we capture all # or @
                                             "\\]?"     // possible end of []
                                             "(.*?)"    // anything
);
// C style pattern
static const std::regex regexPatternCStyle("(.*?)"       // anything but without priority
                                           "\\[?"        // if pattern is myimage[%04d].jpg, don't capture []
                                           "%([0-9]*)d"  // we capture the padding value (eg. myimage%04d.jpg)
                                           "\\]?"        // possible end of []
                                           "(.*?)"       // anything
);
// image name
static const std::regex regexPatternFrame("(.*?"      // anything but without priority
                                          "[_\\.]?)"  // if multiple numbers, the number surround with . _ get priority
                                          "\\[?"      // if pattern is myimage[0001].jpg, don't capture []
                                          "([0-9]+)"  // one frame number, can only be positive ( 0012 )
                                          "\\]?"      // possible end of []
                                          "([_\\.]?"  // if multiple numbers, the number surround with . _ get priority
                                          ".*\\.?"    //
                                          ".*?)"      // anything
);

// image name with negative indexes
static const std::regex regexPatternFrameNeg("(.*?"               // anything but without priority
                                             "[_\\.]?)"           // surround with . _ get priority
                                             "\\[?"               // if pattern is myimage[0001].jpg, don't capture []
                                             "([\\-\\+]?[0-9]+)"  // one frame number, can be positive or negative
                                             "\\]?"               // possible end of []
                                             "([_\\.]?"           // surround with . _ get priority
                                             ".*\\.?"             //
                                             ".*?)"               // anything
);

/**
 * @brief Extract step from a sorted vector of time values.
 */
std::size_t extractStep(const std::vector<Time>& times)
{
    if (times.size() <= 1)
        return 1;
    std::size_t g = 0;
    for (auto a = times.begin(), b = std::next(a); b != times.end(); ++a, ++b)
        g = std::gcd(g, static_cast<std::size_t>(*b - *a));
    return g ? g : 1;
}

/**
 * @brief Extract step from a sorted vector of time values.
 */
std::size_t extractStep(const std::vector<detail::FileNumbers>::const_iterator& timesBegin,
                        const std::vector<detail::FileNumbers>::const_iterator& timesEnd,
                        const std::size_t i)
{
    if (std::distance(timesBegin, timesEnd) <= 1)
        return 1;
    std::size_t g = 0;
    for (auto a = timesBegin, b = std::next(timesBegin); b != timesEnd; ++a, ++b)
        g = std::gcd(g, static_cast<std::size_t>(b->getTime(i) - a->getTime(i)));
    return g ? g : 1;
}

std::size_t getFixedPaddingFromStringNumber(const std::string& timeStr)
{
    if (timeStr.size() > 1)
    {
        // if the number is signed, this charater does not count as padding.
        if (timeStr[0] == '-' || timeStr[0] == '+')
        {
            return timeStr.size() - 1;
        }
    }
    return timeStr.size();
}

/**
 * @brief extract the padding from a vector of frame numbers
 * @param[in] timesStr vector of frame numbers in string format
 */
std::size_t extractPadding(const std::vector<std::string>& timesStr)
{
    assert(timesStr.size() > 0);
    const std::size_t padding = getFixedPaddingFromStringNumber(timesStr.front());

    for (const std::string& s : timesStr)
    {
        if (padding != getFixedPaddingFromStringNumber(s))
        {
            return 0;
        }
    }
    return padding;
}

std::size_t extractPadding(const std::vector<detail::FileNumbers>::const_iterator& timesBegin,
                           const std::vector<detail::FileNumbers>::const_iterator& timesEnd,
                           const std::size_t i)
{
    assert(timesBegin != timesEnd);

    std::size_t nonZeroPadding = 0;
    for (auto s = timesBegin; s != timesEnd; ++s)
    {
        const std::size_t p = s->getFixedPadding(i);
        if (p == 0)
            continue;
        if (nonZeroPadding == 0)
            nonZeroPadding = p;
        else if (nonZeroPadding != p)
            return 0;  // @todo multi-padding: need to split into multiple sequences
    }
    return nonZeroPadding;
}

std::string Sequence::getFilenameAt(const Time time) const
{
    std::ostringstream o;
    if (time >= 0)
    {
        // "prefix.0001.jpg"
        o << _prefix << std::setw(_fixedPadding) << std::setfill(_fillCar) << time << _suffix;
    }
    else
    {
        // "prefix.-0001.jpg" (and not "prefix.000-1.jpg")
        o << _prefix << "-" << std::setw(_fixedPadding) << std::setfill(_fillCar) << -time << _suffix;
    }
    return o.str();
}

std::string Sequence::getCStylePattern() const
{
    if (getFixedPadding())
        return getPrefix() + "%0" + std::to_string(getFixedPadding()) + "d" + getSuffix();
    else
        return getPrefix() + "%d" + getSuffix();
}

Time Sequence::getNbFiles() const
{
    Time nbFiles = 0;
    for (const FrameRange& frameRange : _ranges)
    {
        nbFiles += frameRange.getNbFrames();
    }
    return nbFiles;
}

bool Sequence::isIn(const std::string& filename, Time& time, std::string& timeStr)
{
    time = 0;
    timeStr.clear();

    const std::size_t minLen = _prefix.size() + _suffix.size();
    if (filename.size() <= minLen)
        return false;

    // Compare prefix and suffix without allocating temporary strings
    if (filename.compare(0, _prefix.size(), _prefix) != 0)
        return false;
    if (filename.compare(filename.size() - _suffix.size(), _suffix.size(), _suffix) != 0)
        return false;

    // Parse the numeric part using a local variable — output params are only
    // written when we know the file belongs to the sequence.
    const std::size_t numStart = _prefix.size();
    const std::size_t numLen = filename.size() - minLen;
    const std::string numStr = filename.substr(numStart, numLen);
    Time t;
    try
    {
        t = static_cast<Time>(std::stoll(numStr));
    }
    catch (...)
    {
        return false;
    }

    // When no ranges are set yet (e.g. after initFromPattern), accept any matching file.
    // browseSequence relies on this to collect frame numbers before ranges are known.
    if (_ranges.empty())
    {
        time = t;
        timeStr = numStr;
        return true;
    }

    // Analytical membership test: O(nbRanges) instead of O(nbFrames).
    // Output params stay 0/"" (as initialised) when returning false.
    for (const FrameRange& r : _ranges)
    {
        if (t >= r.first && t <= r.last && (t - r.first) % r.step == 0)
        {
            time = t;
            timeStr = numStr;
            return true;
        }
    }
    return false;
}

EPattern Sequence::checkPattern(const std::string& pattern, const EDetection detectionOptions)
{
    if (regex_match(pattern.c_str(), regexPatternStandard))
    {
        return ePatternStandard;
    }
    else if (regex_match(pattern.c_str(), regexPatternCStyle))
    {
        return ePatternCStyle;
    }
    else if ((detectionOptions & eDetectionNegative) && regex_match(pattern.c_str(), regexPatternFrameNeg))
    {
        return ePatternFrameNeg;
    }
    else if (regex_match(pattern.c_str(), regexPatternFrame))
    {
        return ePatternFrame;
    }
    return ePatternNone;
}

/**
 * @brief This function creates a regex from the pattern,
 *        and init internal values.
 * @param[in] pattern
 * @param[in] accept
 */
bool Sequence::initFromPattern(const std::string& filePattern, const EPattern& accept)
{
    std::cmatch matches;
    if ((accept & ePatternStandard) && regex_match(filePattern.c_str(), matches, regexPatternStandard))
    {
        std::string paddingStr(matches[2].first, matches[2].second);
        _fixedPadding = paddingStr.size();
        _maxPadding = _fixedPadding;
    }
    else if ((accept & ePatternCStyle) && regex_match(filePattern.c_str(), matches, regexPatternCStyle))
    {
        std::string paddingStr(matches[2].first, matches[2].second);
        _fixedPadding = paddingStr.empty() ? 0 : static_cast<std::size_t>(std::stoull(paddingStr));
        _maxPadding = _fixedPadding;
    }
    else if ((accept & ePatternFrame) && regex_match(filePattern.c_str(), matches, regexPatternFrame))
    {
        std::string frame(matches[2].first, matches[2].second);
        _fixedPadding = frame.size();
        _maxPadding = _fixedPadding;
    }
    else if ((accept & ePatternFrameNeg) && regex_match(filePattern.c_str(), matches, regexPatternFrameNeg))
    {
        std::string frame(matches[2].first, matches[2].second);
        _fixedPadding = frame.size();
        _maxPadding = _fixedPadding;
    }
    else
    {
        // this is a file, not a sequence
        return false;
    }
    _prefix = std::string(matches[1].first, matches[1].second);
    _suffix = std::string(matches[3].first, matches[3].second);
    return true;
}

void Sequence::init(const std::string& prefix,
                    const std::size_t padding,
                    const std::size_t maxPadding,
                    const std::string& suffix,
                    const Time firstTime,
                    const Time lastTime,
                    const Time step)
{
    _prefix = prefix;
    _fixedPadding = padding;
    _maxPadding = maxPadding;
    _suffix = suffix;
    _ranges.clear();
    _ranges.push_back(FrameRange(firstTime, lastTime, step));
}

std::vector<std::string> Sequence::getFiles() const
{
    std::vector<std::string> allPaths;
    allPaths.reserve(getNbFiles());
    for (const FrameRange& range : _ranges)
        for (Time t = range.first; t <= range.last; t += range.step)
            allPaths.push_back(getFilenameAt(t));
    return allPaths;
}

std::vector<fs::path> Sequence::getAbsoluteFilesPath(fs::path const& parentPath) const
{
    std::vector<fs::path> allPaths;
    allPaths.reserve(getNbFiles());
    for (const FrameRange& range : _ranges)
        for (Time t = range.first; t <= range.last; t += range.step)
            allPaths.push_back(parentPath / getFilenameAt(t));
    return allPaths;
}

std::string Sequence::string() const
{
    std::ostringstream ss;
    ss << *this;
    return ss.str();
}

std::ostream& operator<<(std::ostream& os, const Sequence& sequence)
{
    os << sequence.getFilenameWithStandardPattern() << " [" << sequence.getFrameRanges() << "]";
    return os;
}

}  // namespace sequenceParser
