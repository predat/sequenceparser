#include "FileStrings.hpp"

#include <functional>
#include <ostream>

namespace sequenceParser {
namespace detail {

std::size_t FileStrings::getHash() const
{
    std::size_t seed = 0;
    // Equivalent to boost::hash_combine
    for (const Vec::value_type& i : _id)
    {
        seed ^= std::hash<std::string>{}(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>{}(1) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}

std::ostream& operator<<(std::ostream& os, const FileStrings& p)
{
    os << "[";
    for (const auto& s : p._id)
        os << s << ",";
    os << "]";
    return os;
}

}  // namespace detail
}  // namespace sequenceParser
