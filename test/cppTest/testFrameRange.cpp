#include <catch2/catch_test_macros.hpp>

#include <sequenceParser/FrameRange.hpp>

using namespace sequenceParser;

TEST_CASE("FrameRange: a single frame", "[FrameRange]")
{
    const FrameRange r(5);
    CHECK(r.first == 5);
    CHECK(r.last == 5);
    CHECK(r.step == 1);
    CHECK(r.getNbFrames() == 1);
    CHECK(r.atIndex(0) == 5);
    CHECK(r.string() == "5");
}

TEST_CASE("FrameRange: a contiguous range", "[FrameRange]")
{
    const FrameRange r(1, 10);
    CHECK(r.getNbFrames() == 10);
    CHECK(r.atIndex(0) == 1);
    CHECK(r.atIndex(9) == 10);
    CHECK(r.string() == "1-10");
}

TEST_CASE("FrameRange: a strided range", "[FrameRange]")
{
    const FrameRange r(1, 10, 2);  // 1, 3, 5, 7, 9
    CHECK(r.getNbFrames() == 5);
    CHECK(r.atIndex(0) == 1);
    CHECK(r.atIndex(4) == 9);
    CHECK(r.string() == "1-10x2");
}

TEST_CASE("FrameRange: negative frame numbers", "[FrameRange]")
{
    const FrameRange r(-3, 3);
    CHECK(r.getNbFrames() == 7);
    CHECK(r.atIndex(0) == -3);
    CHECK(r.atIndex(6) == 3);
}

TEST_CASE("FrameRange: equality compares first, last and step", "[FrameRange]")
{
    CHECK(FrameRange(1, 10, 2) == FrameRange(1, 10, 2));
    CHECK_FALSE(FrameRange(1, 10) == FrameRange(1, 10, 2));
    CHECK_FALSE(FrameRange(1, 10) == FrameRange(1, 11));
}
