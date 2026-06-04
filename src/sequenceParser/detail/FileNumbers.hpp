#ifndef _SEQUENCE_PARSER_FILE_NUMBERS_HPP_
#define _SEQUENCE_PARSER_FILE_NUMBERS_HPP_

#include <sequenceParser/common.hpp>

#include <unordered_map>
#include <set>
#include <vector>
#include <stdexcept>

namespace sequenceParser {
namespace detail {

/**
 * @brief Numbers inside a filename.
 * Each number can be a time inside a sequence.
 * Internal structures to detect sequence inside a directory
 */
class FileNumbers
{
  public:
    typedef FileNumbers This;
    typedef std::pair<Time, std::string> Pair;
    typedef std::vector<Pair> Vec;

  public:
    FileNumbers()
    {
        // we preverse reserve and take memory,
        // that realloc and takes time.
        _numbers.reserve(10);
    }

  public:
    void push_back(const std::string& s)
    {
        try
        {
            // Use std::stoll which handles the full range of ssize_t (= long/long long on 64-bit linux)
            const Time t = static_cast<Time>(std::stoll(s));
            _numbers.push_back(Pair(t, s));
        }
        catch (...)
        {
            // can't retrieve the number,
            // the number inside the string is probably
            // out of range for Time type - silently skip
        }
    }

    void clear() { _numbers.clear(); }

    const std::string& getString(const std::size_t& i) const { return _numbers[i].second; }

    static bool hasSign(const std::string& s) { return ((s[0] == '-') || (s[0] == '+')); }

    static std::size_t extractPadding(const std::string& str)
    {
        if (str.size() == 1)
            return 0;
        const bool withSign = hasSign(str);
        return str[withSign] == '0' ? str.size() - withSign : 0;
    }

    static std::size_t extractMaxPadding(const std::string& s) { return s.size() - hasSign(s); }

    std::size_t getMaxPadding(const std::size_t& i) const { return extractMaxPadding(_numbers[i].second); }

    std::size_t getFixedPadding(const std::size_t& i) const { return extractPadding(_numbers[i].second); }

    Time getTime(const std::size_t& i) const { return _numbers[i].first; }

    std::size_t size() const { return _numbers.size(); }

    struct SortByNumber
    {
        bool operator()(const FileNumbers& a, const FileNumbers& b) const;
    };
    struct SortByPadding
    {
        bool operator()(const FileNumbers& a, const FileNumbers& b) const;
    };
    struct SortByDigit
    {
        bool operator()(const FileNumbers& a, const FileNumbers& b) const;
    };

    bool operator<(const This& v) const
    {
        // by default sort by number
        return SortByNumber()(*this, v);
    }

    bool rangeEquals(const This& v, const size_t begin, const size_t end) const
    {
        for (std::size_t i = begin; i < end; ++i)
        {
            const Pair& me = this->_numbers[i];
            const Pair& other = v._numbers[i];

            // me.second.size() != other.second.size() ) // we don't check the padding...
            if (me.first != other.first)
                return false;
        }
        return true;
    }

    friend std::ostream& operator<<(std::ostream& os, const This& p);

  private:
    Vec _numbers;
};

}  // namespace detail
}  // namespace sequenceParser

#endif
