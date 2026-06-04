import pytest

from pySequenceParser import sequenceParser as seq


@pytest.fixture(scope="module")
def root_path(tmp_path_factory):
    path = tmp_path_factory.mktemp("browse")
    for f in ["plop.txt", "foo.001.png", "foo.002.png", "foo.003.png", "foo.006.png",
              "a.1", "a.2", "4", "5", "6", "11"]:
        (path / f).touch()
    for d in ["dir1", "dir2", "dir3", "dir_d", "dir_f"]:
        (path / d).mkdir()
    return str(path)


def testBrowse(root_path):
    items = seq.browse(root_path)
    for item in items:
        print("item:", item)
        print("item:", item.getFolder())
        print("item:", item.getFilename())
        print("item:", item.getType())
        if item.getType() == seq.eTypeSequence:
            sequence = item.getSequence()
            print("item sequence:", sequence.getFirstFilename())
            print("item sequence:", sequence.getFirstTime(), sequence.getLastTime(),
                  sequence.getDuration(), sequence.getFilenameWithStandardPattern())
            for f in sequence.getFramesIterable():
                print("file:", sequence.getFilenameAt(f))
