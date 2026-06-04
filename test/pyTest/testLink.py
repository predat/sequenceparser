import pytest

from pySequenceParser import sequenceParser as seq
from . import createFile, createFolder, createSymLink


@pytest.fixture(scope="module")
def root_path(tmp_path_factory):
    path = tmp_path_factory.mktemp("link")
    createFile(str(path), "plop.txt")
    createFolder(str(path), "dir1")
    createSymLink(str(path), "plop.txt", "plop_sym_link.txt")
    createSymLink(str(path), "dir1", "dir2")
    return str(path)


def testLinkType(root_path):
    items = seq.browse(root_path)
    for item in items:
        if item.getFilename() in ("plop_sym_link.txt", "dir2"):
            assert item.getType() == seq.eTypeLink
        elif item.getFilename() == "plop.txt":
            assert item.getType() == seq.eTypeFile
        elif item.getFilename() == "dir1":
            assert item.getType() == seq.eTypeFolder
