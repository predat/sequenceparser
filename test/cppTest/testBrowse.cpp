#include <catch2/catch_test_macros.hpp>

#include <sequenceParser/filesystem.hpp>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>

using namespace sequenceParser;
namespace fs = std::filesystem;

namespace {

/// A unique temporary directory, created on construction and removed on
/// destruction. Cross-platform (no getpid / mkdtemp).
struct TempDir
{
    fs::path path;

    TempDir()
    {
        static std::atomic<unsigned> counter{0};
        const auto stamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        path = fs::temp_directory_path() / ("sp_cpptest_" + std::to_string(stamp) + "_" + std::to_string(counter++));
        fs::create_directories(path);
    }

    ~TempDir()
    {
        std::error_code ec;
        fs::remove_all(path, ec);
    }

    TempDir(const TempDir&) = delete;
    TempDir& operator=(const TempDir&) = delete;

    void touch(const std::string& name) const { std::ofstream(path / name).put('x'); }

    void touchFrame(const char* fmt, int frame) const
    {
        char buf[64];
        std::snprintf(buf, sizeof(buf), fmt, frame);
        touch(buf);
    }
};

/// Return the single sequence among browsed items, or nullptr if not exactly one.
const Sequence* singleSequence(const std::vector<Item>& items)
{
    const Sequence* found = nullptr;
    int count = 0;
    for (const Item& item : items)
    {
        if (item.getType() == eTypeSequence)
        {
            ++count;
            found = &item.getSequence();
        }
    }
    return count == 1 ? found : nullptr;
}

}  // namespace

TEST_CASE("browse: detects a contiguous sequence", "[browse]")
{
    TempDir dir;
    for (int i = 1; i <= 5; ++i)
        dir.touchFrame("frame.%04d.exr", i);

    const std::vector<Item> items = browse(dir.path);
    const Sequence* seq = singleSequence(items);
    REQUIRE(seq != nullptr);

    CHECK(seq->getPrefix() == "frame.");
    CHECK(seq->getSuffix() == ".exr");
    CHECK(seq->getFixedPadding() == 4);
    CHECK(seq->getFirstTime() == 1);
    CHECK(seq->getLastTime() == 5);
    CHECK(seq->getNbFiles() == 5);
    CHECK_FALSE(seq->hasMissingFile());
}

TEST_CASE("browse: detects holes in a sequence", "[browse]")
{
    TempDir dir;
    for (int i : {1, 2, 3, 5, 6})
        dir.touchFrame("frame.%04d.exr", i);

    const std::vector<Item> items = browse(dir.path);
    const Sequence* seq = singleSequence(items);
    REQUIRE(seq != nullptr);

    CHECK(seq->getFirstTime() == 1);
    CHECK(seq->getLastTime() == 6);
    CHECK(seq->getNbFiles() == 5);
    CHECK(seq->hasMissingFile());
    CHECK(seq->getNbMissingFiles() == 1);  // frame 4
}

TEST_CASE("browse: a lone file is not promoted to a sequence by default", "[browse]")
{
    TempDir dir;
    dir.touch("single.0001.exr");
    dir.touch("readme.txt");

    const std::vector<Item> items = browse(dir.path);

    int nbFiles = 0;
    int nbSequences = 0;
    for (const Item& item : items)
    {
        if (item.getType() == eTypeFile)
            ++nbFiles;
        if (item.getType() == eTypeSequence)
            ++nbSequences;
    }
    CHECK(nbSequences == 0);  // eDetectionDefault needs at least two files
    CHECK(nbFiles == 2);
}

TEST_CASE("browseSequence: builds a sequence from a pattern on disk", "[browseSequence]")
{
    TempDir dir;
    for (int i = 10; i <= 12; ++i)
        dir.touchFrame("plate.%03d.png", i);

    Sequence seq;
    const std::string pattern = (dir.path / "plate.###.png").string();
    REQUIRE(browseSequence(seq, pattern));

    CHECK(seq.getFirstTime() == 10);
    CHECK(seq.getLastTime() == 12);
    CHECK(seq.getNbFiles() == 3);
    CHECK(seq.getFixedPadding() == 3);
}
