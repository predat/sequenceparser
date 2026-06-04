#ifndef _SEQUENCE_PARSER_ITEM_HPP_
#define _SEQUENCE_PARSER_ITEM_HPP_

#include "common.hpp"
#include "Sequence.hpp"

#include <filesystem>

namespace sequenceParser {

/**
 * @brief Represents an element in the filesystem: file, folder or link.
 * It could contain a Sequence.
 * @see getSequence
 */
class Item
{
  public:
    Item()
      : _type(eTypeFile)
    {}

    Item(const EType type, const std::string& filepath)
      : _type(type)
#ifdef SWIGJAVA
        ,
        _path(utf8_to_latin1(filepath))
#else
        ,
        _path(filepath)
#endif
    {}

#ifndef SWIG
    Item(const EType type, const std::filesystem::path& filepath)
      : _type(type),
        _path(filepath)
    {
        assert(type != eTypeSequence);
    }

    Item(const Sequence& sequence, const std::filesystem::path& folder)
      : _type(eTypeSequence),
        _path(folder),
        _sequence(sequence)
    {
        _path /= sequence.getFilenameWithStandardPattern();
    }
#endif

    Item(const Sequence& sequence, const std::string& folder)
      : _type(eTypeSequence),
        _path(folder),
        _sequence(sequence)
    {
        _path /= sequence.getFilenameWithStandardPattern();
    }

    [[nodiscard]] EType getType() const { return _type; }

    [[nodiscard]] std::string getAbsoluteFilepath() const { return _path.string(); }
    [[nodiscard]] std::string getFilename() const { return _path.filename().string(); }
    [[nodiscard]] std::string getFolder() const { return getFolderPath().string(); }

    /**
     * @brief Usefull for sequences items: explode sequence
     */
    [[nodiscard]] std::vector<Item> explode() const;

    [[nodiscard]] const Sequence& getSequence() const { return _sequence; }

#ifndef SWIG
    [[nodiscard]] const std::filesystem::path& getPath() const { return _path; }
    [[nodiscard]] const std::filesystem::path getFolderPath() const { return _path.parent_path(); }
#endif

    [[nodiscard]] std::string getAbsoluteFirstFilename() const;
    [[nodiscard]] std::string getFirstFilename() const;

    bool operator<(const Item& other) const { return _path < other._path; }

    [[nodiscard]] std::string string() const { return getType() == eTypeSequence ? getSequence().string() : _path.string(); }

  private:
    EType _type;

    std::filesystem::path _path;

    Sequence _sequence;
};

#ifndef SWIG
[[nodiscard]] EType getTypeFromPath(const std::filesystem::path& path);
#endif
/**
 * @warning The methods checks if the given path corresponds to a link, a file or a folder (in the filesystem).
 * Else the method returns unknown type (it never returns a sequence).
 * @see getTypeFromPath
 */
[[nodiscard]] EType getTypeFromPath(const std::string& pathStr);

#ifndef SWIG
std::ostream& operator<<(std::ostream& os, const Item& item);
#endif

}  // namespace sequenceParser

#endif
