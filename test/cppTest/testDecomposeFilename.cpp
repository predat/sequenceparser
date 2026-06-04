#include <catch2/catch_test_macros.hpp>

#include <sequenceParser/common.hpp>
#include <sequenceParser/detail/analyze.hpp>
#include <sequenceParser/detail/FileStrings.hpp>
#include <sequenceParser/detail/FileNumbers.hpp>

using namespace sequenceParser;
using namespace sequenceParser::detail;

TEST_CASE("decomposeFilename: splits alternating string and number parts", "[decompose]")
{
    FileStrings stringParts;
    FileNumbers numberParts;
    const std::size_t nbNumbers = decomposeFilename("aa1b22cccc3", stringParts, numberParts, eDetectionDefault);

    CHECK(nbNumbers == 3);

    REQUIRE(numberParts.size() == 3);
    CHECK(numberParts.getTime(0) == 1);
    CHECK(numberParts.getTime(1) == 22);
    CHECK(numberParts.getTime(2) == 3);

    // String parts wrap around the numbers; because the name ends with a
    // digit there is a trailing empty string part, hence 4 (not 3) entries.
    REQUIRE(stringParts.getId().size() == 4);
    CHECK(stringParts[0] == "aa");
    CHECK(stringParts[1] == "b");
    CHECK(stringParts[2] == "cccc");
    CHECK(stringParts[3] == "");
}

TEST_CASE("decomposeFilename: a typical frame filename", "[decompose]")
{
    FileStrings stringParts;
    FileNumbers numberParts;
    const std::size_t nbNumbers = decomposeFilename("frame.0042.exr", stringParts, numberParts, eDetectionDefault);

    CHECK(nbNumbers == 1);
    REQUIRE(numberParts.size() == 1);
    CHECK(numberParts.getTime(0) == 42);
    CHECK(numberParts.getString(0) == "0042");
    CHECK(numberParts.getFixedPadding(0) == 4);  // leading zero => fixed padding of 4
}

TEST_CASE("decomposeFilename: a name with no number yields no number part", "[decompose]")
{
    FileStrings stringParts;
    FileNumbers numberParts;
    const std::size_t nbNumbers = decomposeFilename("readme.txt", stringParts, numberParts, eDetectionDefault);

    CHECK(nbNumbers == 0);
    CHECK(numberParts.size() == 0);
}
