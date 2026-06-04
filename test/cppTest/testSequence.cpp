#include <catch2/catch_test_macros.hpp>

#include <sequenceParser/Sequence.hpp>

using namespace sequenceParser;

TEST_CASE("Sequence: parse a standard '#' pattern", "[Sequence]")
{
    Sequence s;
    REQUIRE(s.initFromPattern("foo.####.jpg", ePatternStandard));
    CHECK(s.getPrefix() == "foo.");
    CHECK(s.getSuffix() == ".jpg");
    CHECK(s.getFixedPadding() == 4);
    CHECK(s.getPatternCharacter() == '#');
    CHECK(s.getFilenameWithStandardPattern() == "foo.####.jpg");
    CHECK(s.getFilenameWithPrintfPattern() == "foo.%04d.jpg");
}

TEST_CASE("Sequence: parse a C-style '%04d' pattern", "[Sequence]")
{
    Sequence s;
    REQUIRE(s.initFromPattern("foo.%04d.jpg", ePatternCStyle));
    CHECK(s.getPrefix() == "foo.");
    CHECK(s.getSuffix() == ".jpg");
    CHECK(s.getFixedPadding() == 4);
}

TEST_CASE("Sequence: a C-style '%d' pattern has no fixed padding", "[Sequence]")
{
    Sequence s;
    REQUIRE(s.initFromPattern("foo.%d.jpg", ePatternCStyle));
    CHECK(s.getFixedPadding() == 0);
}

TEST_CASE("Sequence: a plain filename is not a sequence", "[Sequence]")
{
    Sequence s;
    CHECK_FALSE(s.initFromPattern("foo.jpg", ePatternDefault));
}

TEST_CASE("Sequence: getFilenameAt zero-pads the frame number", "[Sequence]")
{
    Sequence s;
    REQUIRE(s.initFromPattern("img.####.dpx", ePatternStandard));
    CHECK(s.getFilenameAt(1) == "img.0001.dpx");
    CHECK(s.getFilenameAt(1234) == "img.1234.dpx");
    CHECK(s.getFilenameAt(12345) == "img.12345.dpx");  // wider than the padding
    CHECK(s.getFilenameAt(-1) == "img.-0001.dpx");     // the sign precedes the padding
}

TEST_CASE("Sequence: range queries on a contiguous sequence", "[Sequence]")
{
    const Sequence s("img.####.dpx", {FrameRange(1, 10)}, ePatternStandard);
    REQUIRE(s.getFixedPadding() == 4);
    CHECK(s.getFirstTime() == 1);
    CHECK(s.getLastTime() == 10);
    CHECK(s.getNbFiles() == 10);
    CHECK(s.getDuration() == 10);
    CHECK_FALSE(s.hasMissingFile());
    CHECK(s.getNbMissingFiles() == 0);
    CHECK(s.getFirstFilename() == "img.0001.dpx");
    CHECK(s.getLastFilename() == "img.0010.dpx");
}

TEST_CASE("Sequence: a sequence with holes reports missing files", "[Sequence]")
{
    const Sequence s("img.####.dpx", {FrameRange(1, 3), FrameRange(5, 6)}, ePatternStandard);
    CHECK(s.getFirstTime() == 1);
    CHECK(s.getLastTime() == 6);
    CHECK(s.getNbFiles() == 5);  // 1, 2, 3, 5, 6
    CHECK(s.hasMissingFile());
    CHECK(s.getNbMissingFiles() == 1);  // frame 4
}

TEST_CASE("Sequence: equality and copy", "[Sequence]")
{
    const Sequence a("img.####.dpx", {FrameRange(1, 10)}, ePatternStandard);
    const Sequence b(a);
    CHECK(a == b);

    const Sequence c("img.####.dpx", {FrameRange(1, 9)}, ePatternStandard);
    CHECK(a != c);
}
