#ifndef _SEQUENCE_PARSER_FILE_STRINGS_HPP_
#define _SEQUENCE_PARSER_FILE_STRINGS_HPP_

#include <vector>
#include <iostream>

namespace sequenceParser {
namespace detail {

/**
 * @brief Unique identification for a sequence.
 * Internal structures to detect sequence inside a directory.
 */
class FileStrings
{
  public:
    typedef FileStrings This;
    typedef std::vector<std::string> Vec;

  public:
    Vec& getId() { return _id; }

    const Vec& getId() const { return _id; }

    void clear() { _id.clear(); }

    bool operator==(const This& v) const { return _id == v._id; }

    const std::string& operator[](const std::size_t i) const { return _id[i]; }

    std::size_t getHash() const;

    friend std::ostream& operator<<(std::ostream& os, const This& p);

  private:
    Vec _id;
};

struct SeqIdHash
{
    std::size_t operator()(const FileStrings& p) const { return p.getHash(); }
};

}  // namespace detail
}  // namespace sequenceParser

#endif
