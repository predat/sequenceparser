import os
import time
import shutil
import getpass
import platform
import pytest

# This module asserts POSIX ownership (pwd/grp) and Unix permission bits, which
# have no equivalent on Windows. Skip the whole module on non-POSIX platforms
# before importing the POSIX-only pwd/grp modules.
if os.name != "posix":
    pytest.skip("ItemStat ownership/permission tests are POSIX-only",
                allow_module_level=True)

import pwd
import grp

from pySequenceParser import sequenceParser as seq
from . import createFile, createFolder, createSymLink, getSequencesFromPath


user_name = getpass.getuser()
try:
    grp_name = grp.getgrnam(getpass.getuser()).gr_name
except KeyError:
    gid = pwd.getpwuid(os.getuid()).pw_gid
    grp_name = grp.getgrgid(gid).gr_name


@pytest.fixture(scope="module")
def root_path(tmp_path_factory):
    path = tmp_path_factory.mktemp("stat")
    for f in ["plop.txt", "foo.001.png", "foo.002.png", "foo.003.png"]:
        createFile(str(path), f)
    createFolder(str(path), "dir1")
    createSymLink(str(path), "plop.txt", "plop_sym_link.txt")
    return str(path)


def checkCommonParameters(itemStat):
    assert itemStat.userName == user_name
    assert itemStat.groupName == grp_name
    assert itemStat.ownerCanRead is True
    assert itemStat.ownerCanWrite is True
    assert itemStat.ownerCanExecute is False
    assert itemStat.groupCanRead is True
    assert itemStat.groupCanWrite is True
    assert itemStat.groupCanExecute is False
    assert itemStat.otherCanRead is True
    assert itemStat.otherCanWrite is False
    assert itemStat.otherCanExecute is False


def checkUnsetItemStat(itemStat):
    assert itemStat.userName == "unknown"
    assert itemStat.groupName == "unknown"
    assert itemStat.deviceId == 0
    assert itemStat.inodeId == 0
    assert itemStat.userId == 0
    assert itemStat.groupId == 0
    assert itemStat.nbHardLinks == 0
    assert itemStat.fullNbHardLinks == 0
    assert itemStat.size == 0
    assert itemStat.minSize == 0
    assert itemStat.maxSize == 0
    assert itemStat.realSize == 0
    assert itemStat.sizeOnDisk == 0
    assert itemStat.accessTime == -1
    assert itemStat.modificationTime == -1
    assert itemStat.lastChangeTime == -1
    assert itemStat.ownerCanRead is False
    assert itemStat.ownerCanWrite is False
    assert itemStat.ownerCanExecute is False
    assert itemStat.groupCanRead is False
    assert itemStat.groupCanWrite is False
    assert itemStat.groupCanExecute is False
    assert itemStat.otherCanRead is False
    assert itemStat.otherCanWrite is False
    assert itemStat.otherCanExecute is False


def testFileStat(root_path):
    itemFile = seq.Item(seq.eTypeFile, os.path.join(root_path, "plop.txt"))
    itemStat = seq.ItemStat(itemFile)
    checkCommonParameters(itemStat)
    assert itemStat.nbHardLinks == 1
    assert itemStat.fullNbHardLinks == 1
    assert itemStat.size == itemStat.minSize
    assert itemStat.size == itemStat.maxSize
    assert itemStat.realSize == itemStat.size / itemStat.nbHardLinks
    assert itemStat.sizeOnDisk >= itemStat.size
    currentTime = round(time.time())
    assert itemStat.accessTime <= currentTime
    assert itemStat.modificationTime <= currentTime
    assert itemStat.lastChangeTime <= currentTime


def testFileDeleted(root_path):
    fileToDelete = "fileToDelete.txt"
    createFile(root_path, fileToDelete)
    items = seq.browse(root_path)
    itemStat = None
    for item in items:
        if item.getFilename() == fileToDelete:
            os.remove(os.path.join(root_path, fileToDelete))
            itemStat = seq.ItemStat(item)
            break
    checkUnsetItemStat(itemStat)


def testSymLinkStat(root_path):
    src = os.path.join(root_path, "plop_sym_link.txt")
    dst = os.path.join(root_path, "plop.txt")
    itemFile = seq.Item(seq.eTypeLink, src)
    itemStat = seq.ItemStat(itemFile)
    assert itemStat.userName == user_name
    assert itemStat.groupName == grp_name
    assert itemStat.nbHardLinks == 1
    assert itemStat.fullNbHardLinks == 1
    assert itemStat.size == len(dst)
    assert itemStat.size == itemStat.minSize
    assert itemStat.size == itemStat.maxSize
    assert itemStat.realSize == itemStat.size / itemStat.nbHardLinks
    assert itemStat.ownerCanRead is True
    assert itemStat.ownerCanWrite is True
    assert itemStat.ownerCanExecute is True
    assert itemStat.groupCanRead is True
    if platform.system() == "Linux":
        assert itemStat.groupCanWrite is True
        assert itemStat.otherCanRead is True
        assert itemStat.otherCanWrite is True
    assert itemStat.groupCanExecute is True
    assert itemStat.otherCanExecute is True


def testSymLinkDeleted(root_path):
    linkToDelete = "linkToDelete.txt"
    createSymLink(root_path, "plop.txt", linkToDelete)
    items = seq.browse(root_path)
    itemStat = None
    for item in items:
        if item.getFilename() == linkToDelete:
            os.remove(os.path.join(root_path, linkToDelete))
            itemStat = seq.ItemStat(item)
            break
    checkUnsetItemStat(itemStat)


def testFolderStat(root_path):
    itemFile = seq.Item(seq.eTypeFolder, os.path.join(root_path, "dir1"))
    itemStat = seq.ItemStat(itemFile)
    checkCommonParameters(itemStat)
    assert itemStat.nbHardLinks == 2
    assert itemStat.fullNbHardLinks == 2
    assert itemStat.size == itemStat.minSize
    assert itemStat.size == itemStat.maxSize
    assert itemStat.realSize == itemStat.size
    assert itemStat.sizeOnDisk >= 0


def testFolderDeleted(root_path):
    folderToDelete = "folderToDelete"
    createFolder(root_path, folderToDelete)
    items = seq.browse(root_path)
    itemStat = None
    for item in items:
        if item.getFilename() == folderToDelete:
            shutil.rmtree(os.path.join(root_path, folderToDelete))
            itemStat = seq.ItemStat(item)
            break
    checkUnsetItemStat(itemStat)


def testSequenceStat(root_path):
    itemSequence = getSequencesFromPath(root_path, seq.eDetectionDefault)[0]
    nbFilesInSequence = itemSequence.getSequence().getNbFiles()
    itemStat = seq.ItemStat(itemSequence)
    checkCommonParameters(itemStat)
    assert itemStat.nbHardLinks == 1
    assert itemStat.fullNbHardLinks == 3
    assert itemStat.size == itemStat.minSize * nbFilesInSequence
    assert itemStat.size == itemStat.maxSize * nbFilesInSequence
    assert itemStat.realSize == itemStat.size / itemStat.nbHardLinks
    assert itemStat.sizeOnDisk >= itemStat.size


def testSequenceDeleted(root_path):
    files_to_create = ["bar.001.jpg", "bar.002.jpg", "bar.003.jpg"]
    for f in files_to_create:
        createFile(root_path, f)
    items = seq.browse(root_path)
    itemStat = None
    for item in items:
        if item.getFilename() == "bar.###.jpg":
            for f in files_to_create:
                os.remove(os.path.join(root_path, f))
            itemStat = seq.ItemStat(item)
            break
    checkUnsetItemStat(itemStat)


def testUndefinedStat(root_path):
    itemFile = seq.Item(seq.eTypeUndefined, os.path.join(root_path, "plop.txt"))
    itemStat = seq.ItemStat(itemFile)
    checkUnsetItemStat(itemStat)
