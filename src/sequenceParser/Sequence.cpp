#include "Sequence.hpp"

#include "detail/FileNumbers.hpp"

#include <filesystem>
#include <regex>
#include <unordered_map>
#include <set>
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

template<typename T>
inline T greatestCommonDivisor(T a, T b)
{
    T r;
    if (b == 0)
        return 0;
    while ((r = a % b) != 0)
    {
        a = b;
        b = r;
    }
    return b;
}

/**
 * @brief Find the biggest common step from a set of all steps.
 */
std::size_t greatestCommonDivisor(const std::set<std::size_t>& steps)
{
    if (steps.size() == 1)
    {
        return *steps.begin();
    }
    std::set<std::size_t> allSteps;
    for (std::set<std::size_t>::const_iterator itA = steps.begin(), itB = ++steps.begin(), itEnd = steps.end(); itB != itEnd; ++itA, ++itB)
    {
        allSteps.insert(greatestCommonDivisor(*itB, *itA));
    }
    return greatestCommonDivisor(allSteps);
}

/**
 * @brief Extract step from a sorted vector of time values.
 */
std::size_t extractStep(const std::vector<Time>& times)
{
    if (times.size() <= 1)
    {
        return 1;
    }
    std::set<std::size_t> allSteps;
    for (std::vector<Time>::const_iterator itA = times.begin(), itB = ++times.begin(), itEnd = times.end(); itB != itEnd; ++itA, ++itB)
    {
        allSteps.insert(*itB - *itA);
    }
    return greatestCommonDivisor(allSteps);
}

/**
 * @brief Extract step from a sorted vector of time values.
 */
std::size_t extractStep(const std::vector<detail::FileNumbers>::const_iterator& timesBegin,
                        const std::vector<detail::FileNumbers>::const_iterator& timesEnd,
                        const std::size_t i)
{
    if (std::distance(timesBegin, timesEnd) <= 1)
    {
        return 1;
    }
    std::set<std::size_t> allSteps;
    for (std::vector<detail::FileNumbers>::const_iterator itA = timesBegin, itB = std::next(timesBegin), itEnd = timesEnd; itB != itEnd; ++itA, ++itB)
    {
        allSteps.insert(itB->getTime(i) - itA->getTime(i));
    }
    return greatestCommonDivisor(allSteps);
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

    std::set<std::size_t> padding;
    std::set<std::size_t> maxPadding;

    for (std::vector<detail::FileNumbers>::const_iterator s = timesBegin; s != timesEnd; ++s)
    {
        padding.insert(s->getFixedPadding(i));
        maxPadding.insert(s->getMaxPadding(i));
    }

    std::set<std::size_t> pad = padding;
    pad.erase(0);

    if (pad.size() == 0)
    {
        return 0;
    }
    else if (pad.size() == 1)
    {
        return *pad.begin();
    }

    // @todo multi-padding !
    // need to split into multiple sequences !
    return 0;
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
        o << _prefix << "-" << std::setw(_fixedPadding) << std::setfill(_fillCar) << std::abs((int)time) << _suffix;
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
    // initialize the output arguments
    time = 0;
    timeStr = "";

    const std::size_t min = _prefix.size() + _suffix.size();

    // different size of the filename
    if (filename.size() <= min)
        return false;

    // different prefix or suffix
    if (filename.substr(0, _prefix.size()) != _prefix || filename.substr(filename.size() - _suffix.size(), _suffix.size()) != _suffix)
        return false;

    // not an existing time
    std::istringstream iss(filename.substr(_prefix.size(), filename.size() - _prefix.size() - _suffix.size()));
    size_t expectedTime;
    iss >> expectedTime;
    bool timeIsIn = false;
    for (Time t : getFramesIterable())
    {
        if (expectedTime == t)
        {
            timeIsIn = true;
            break;
        }
    }
    if (!timeIsIn)
        return false;

    try
    {
        timeStr = filename.substr(_prefix.size(), filename.size() - _suffix.size() - _prefix.size());
        time = static_cast<Time>(std::stoll(timeStr));
    }
    catch (...)
    {
        return false;
    }
    return true;
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
    for (const FrameRange& range : _ranges)
    {
        for (Time t = range.first; t <= range.last; t += range.step)
            allPaths.push_back(getFilenameAt(t));
    }

    return allPaths;
}

std::vector<fs::path> Sequence::getAbsoluteFilesPath(fs::path const& parentPath) const
{
    std::vector<fs::path> allPaths;
    for (const FrameRange& range : _ranges)
    {
        for (Time t = range.first; t <= range.last; t += range.step)
            allPaths.push_back(parentPath / getFilenameAt(t));
    }

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
