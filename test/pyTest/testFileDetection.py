import pytest

from pySequenceParser import sequenceParser as seq
from . import createFile


@pytest.fixture(scope="module")
def paths(tmp_path_factory):
    """Create the full directory tree used by all file-detection tests.

    Hierarchy mirrors the original test setup:
      root/
        sub/          ← one sub-directory at the top level
          film/
            strict/
            no_strict/
          trash/
            plop.txt
            dpx/
              img.dpx
              negative/
    """
    root = tmp_path_factory.mktemp("filedetection")
    sub = root / "sub"
    sub.mkdir()

    film = sub / "film"
    film.mkdir()

    strict = film / "strict"
    strict.mkdir()
    for i in range(100):
        createFile(str(strict), f'img.{i:04d}.dpx')
        createFile(str(strict), f'.img.{i:04d}.dpx')
        createFile(str(strict), f'img.{i:04d}.jpg')
        createFile(str(strict), f'imgBroken.{i:04d}.jpg')

    no_strict = film / "no_strict"
    no_strict.mkdir()
    for i in range(100):
        createFile(str(no_strict), f'img.{i:04d}.dpx')
        createFile(str(no_strict), f'.img.{i:04d}.dpx')
        createFile(str(no_strict), f'img.{i:04d}.jpg')
        createFile(str(no_strict), f'imgBroken.{i:04d}.jpg')

    trash = sub / "trash"
    trash.mkdir()
    createFile(str(trash), 'plop.txt')

    dpx = trash / "dpx"
    dpx.mkdir()
    createFile(str(dpx), 'img.dpx')

    negative = dpx / "negative"
    negative.mkdir()
    for i in range(100):
        createFile(str(negative), f'img.-{i:04d}.dpx')
        createFile(str(negative), f'seqTest.-{i:04d}.dpx')

    return {
        "root": str(root),
        "sub": str(sub),
        "film": str(film),
        "strict": str(strict),
        "no_strict": str(no_strict),
        "trash": str(trash),
        "dpx": str(dpx),
        "negative": str(negative),
    }


def checkItemsInDirectory(path, detectionOptions, nbFolders, nbFiles, nbSequences, nbFileObjects, filters=[]):
    listFileObject = []
    listFolder = []
    listFile = []
    listSequence = []

    items = seq.browse(path, detectionOptions, filters)
    for item in items:
        itemType = item.getType()
        if itemType | seq.eTypeAll:
            listFileObject.append(item)
        if itemType == seq.eTypeFolder:
            listFolder.append(item)
        if itemType == seq.eTypeFile:
            listFile.append(item)
        if itemType == seq.eTypeSequence:
            listSequence.append(item)

    assert len(listFileObject) == nbFileObjects
    assert len(listFolder) == nbFolders
    assert len(listFile) == nbFiles
    assert len(listSequence) == nbSequences


def checkFirstSequence(path, detectionOptions, minValue, maxValue, filters):
    listSequence = []

    items = seq.browse(path, detectionOptions, filters)
    for item in items:
        if item.getType() == seq.eTypeSequence:
            listSequence.append(item)

    assert len(listSequence) > 0

    sequence = listSequence[0].getSequence()
    assert sequence.getFirstTime() == minValue
    assert sequence.getLastTime() == maxValue


def testFolder(paths):
    checkItemsInDirectory(paths["root"], seq.eDetectionDefault, 1, 0, 0, 1)
    checkItemsInDirectory(paths["sub"], seq.eDetectionDefault, 2, 0, 0, 2)
    checkItemsInDirectory(paths["trash"], seq.eDetectionDefault, 1, 1, 0, 2)
    checkItemsInDirectory(paths["dpx"], seq.eDetectionDefault, 1, 1, 0, 2)
    checkItemsInDirectory(paths["negative"], seq.eDetectionDefault, 0, 0, 2, 2)


def testNegativeSequences(paths):
    negative = paths["negative"]
    checkItemsInDirectory(negative, seq.eDetectionDefault, 0, 0, 0, 0, ['img.####.dpx'])
    checkItemsInDirectory(negative, seq.eDetectionDefault, 0, 0, 0, 0, ['img.#####.dpx'])
    checkItemsInDirectory(negative, seq.eDetectionDefault, 0, 0, 1, 1, ['img.-####.dpx'])
    checkFirstSequence(negative, seq.eDetectionDefault, 0, 99, ['img.-####.dpx'])
    checkItemsInDirectory(negative, seq.eDetectionDefault, 0, 0, 0, 0, ['seqTest.@.dpx'])
    checkItemsInDirectory(negative, seq.eDetectionDefault, 0, 0, 1, 1, ['seqTest.-@.dpx'])
    checkFirstSequence(negative, seq.eDetectionDefault, 0, 99, ['seqTest.-@.dpx'])

    checkItemsInDirectory(negative, seq.eDetectionNegative | seq.eDetectionSequenceNeedAtLeastTwoFiles, 0, 0, 2, 2)

    checkItemsInDirectory(negative, seq.eDetectionNegative, 0, 0, 1, 1, ['seqTest.@.dpx'])
    checkFirstSequence(negative, seq.eDetectionNegative, -99, -0, ['seqTest.@.dpx'])

    checkItemsInDirectory(negative, seq.eDetectionNegative, 0, 0, 1, 1, ['seqTest.-@.dpx'])
    checkFirstSequence(negative, seq.eDetectionNegative, -99, -0, ['seqTest.-@.dpx'])

    checkItemsInDirectory(negative, seq.eDetectionNegative, 0, 0, 0, 0, ['img.#### .dpx'])
    checkItemsInDirectory(negative, seq.eDetectionNegative, 0, 0, 1, 1, ['img.####.dpx'])
    checkItemsInDirectory(negative, seq.eDetectionNegative, 0, 0, 0, 0, ['img.#####.dpx'])
    checkItemsInDirectory(negative, seq.eDetectionNegative, 0, 0, 1, 1, ['img.-####.dpx'])
    checkFirstSequence(negative, seq.eDetectionNegative, -99, 0, ['img.-####.dpx'])


def testStrictPadding(paths):
    strict = paths["strict"]
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 3, 3)
    checkItemsInDirectory(strict, seq.eDetectionNone, 0, 0, 4, 4)

    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 1, 1, ['img.####.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 1, 1, ['*.0050.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 1, 1, ['*0050.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 1, 1, ['???.0050.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 1, 1, ['????0050.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 1, 1, ['*.####.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 1, 1, ['*.@.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 1, 1, ['???.####.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 1, 1, ['???.@.dpx'])

    checkItemsInDirectory(strict, seq.eDetectionSequenceNeedAtLeastTwoFiles | seq.eDetectionIgnoreDotFile, 0, 1, 0, 1, ['*.0050.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionSequenceNeedAtLeastTwoFiles | seq.eDetectionIgnoreDotFile, 0, 1, 0, 1, ['*0050.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionSequenceNeedAtLeastTwoFiles, 0, 1, 0, 1, ['???.0050.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionSequenceNeedAtLeastTwoFiles, 0, 1, 0, 1, ['????0050.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionSequenceNeedAtLeastTwoFiles | seq.eDetectionIgnoreDotFile, 0, 0, 1, 1, ['*.####.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionSequenceNeedAtLeastTwoFiles | seq.eDetectionIgnoreDotFile, 0, 0, 1, 1, ['*.@.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionSequenceNeedAtLeastTwoFiles, 0, 0, 1, 1, ['???.####.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionSequenceNeedAtLeastTwoFiles, 0, 0, 1, 1, ['???.@.dpx'])

    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 0, 0, ['img.50.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionNone, 0, 0, 0, 0, ['img.50.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 1, 1, ['img.0500.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionNone, 0, 0, 0, 0, ['img.0500.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 0, 0, ['img.000050.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionNone, 0, 0, 0, 0, ['img.000050.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 1, 1, ['img.####.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionNone, 0, 0, 1, 1, ['img.####.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 0, 0, ['img.###.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionNone, 0, 0, 0, 0, ['img.###.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 0, 0, ['img.#####.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionNone, 0, 0, 0, 0, ['img.#####.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 1, 1, ['img.%04d.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionNone, 0, 0, 1, 1, ['img.%04d.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 0, 0, ['img.%03d.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionNone, 0, 0, 0, 0, ['img.%03d.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 0, 0, ['img.%05d.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionNone, 0, 0, 0, 0, ['img.%05d.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 1, 1, ['img.@.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionNone, 0, 0, 1, 1, ['img.@.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 0, 0, ['.img.0050.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionNone, 0, 0, 1, 1, ['.img.0050.dpx'])

    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 0, 0, ['.img.50.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionNone, 0, 0, 0, 0, ['.img.50.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 0, 0, ['.img.0500.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionNone, 0, 0, 0, 0, ['.img.0500.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 0, 0, ['.img.000050.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionNone, 0, 0, 0, 0, ['.img.000050.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 0, 0, ['.img.####.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionNone, 0, 0, 1, 1, ['.img.####.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 0, 0, ['.img.###.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionNone, 0, 0, 0, 0, ['.img.###.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 0, 0, ['.img.#####.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionNone, 0, 0, 0, 0, ['.img.#####.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 0, 0, ['.img.%04d.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionNone, 0, 0, 1, 1, ['.img.%04d.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 0, 0, ['.img.%03d.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionNone, 0, 0, 0, 0, ['.img.%03d.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 0, 0, ['.img.%05d.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionNone, 0, 0, 0, 0, ['.img.%05d.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionDefault, 0, 0, 0, 0, ['.img.@.dpx'])
    checkItemsInDirectory(strict, seq.eDetectionNone, 0, 0, 1, 1, ['.img.@.dpx'])


def testNoStrictPadding(paths):
    no_strict = paths["no_strict"]
    checkItemsInDirectory(no_strict, seq.eDetectionDefault, 0, 0, 3, 3)
    checkItemsInDirectory(no_strict, seq.eDetectionNone, 0, 0, 4, 4)

    checkItemsInDirectory(no_strict, seq.eDetectionDefault, 0, 0, 1, 1, ['img.0050.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionNone, 0, 0, 1, 1, ['img.0050.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionDefault, 0, 0, 0, 0, ['img.50.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionNone, 0, 0, 0, 0, ['img.50.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionDefault, 0, 0, 1, 1, ['img.0050.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionNone, 0, 0, 1, 1, ['img.0050.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionDefault, 0, 0, 1, 1, ['img.####.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionNone, 0, 0, 1, 1, ['img.####.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionDefault, 0, 0, 0, 0, ['img.###.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionNone, 0, 0, 0, 0, ['img.###.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionDefault, 0, 0, 0, 0, ['img.#####.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionNone, 0, 0, 0, 0, ['img.#####.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionDefault, 0, 0, 1, 1, ['img.%04d.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionNone, 0, 0, 1, 1, ['img.%04d.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionDefault, 0, 0, 0, 0, ['img.%05d.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionNone, 0, 0, 0, 0, ['img.%05d.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionDefault, 0, 0, 0, 0, ['img.%03d.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionNone, 0, 0, 0, 0, ['img.%03d.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionDefault, 0, 0, 1, 1, ['img.@.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionNone, 0, 0, 1, 1, ['img.@.dpx'])

    checkItemsInDirectory(no_strict, seq.eDetectionDefault, 0, 0, 0, 0, ['.img.0050.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionNone, 0, 0, 1, 1, ['.img.0050.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionDefault, 0, 0, 0, 0, ['.img.50.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionNone, 0, 0, 0, 0, ['.img.50.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionDefault, 0, 0, 0, 0, ['.img.0500.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionNone, 0, 0, 0, 0, ['.img.0500.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionDefault, 0, 0, 0, 0, ['.img.0050.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionNone, 0, 0, 1, 1, ['.img.0050.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionDefault, 0, 0, 0, 0, ['.img.####.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionNone, 0, 0, 1, 1, ['.img.####.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionDefault, 0, 0, 0, 0, ['.img.###.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionNone, 0, 0, 0, 0, ['.img.###.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionDefault, 0, 0, 0, 0, ['.img.#####.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionNone, 0, 0, 0, 0, ['.img.#####.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionDefault, 0, 0, 0, 0, ['.img.%04d.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionNone, 0, 0, 1, 1, ['.img.%04d.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionDefault, 0, 0, 0, 0, ['.img.%05d.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionNone, 0, 0, 0, 0, ['.img.%05d.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionDefault, 0, 0, 0, 0, ['.img.%03d.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionNone, 0, 0, 0, 0, ['.img.%03d.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionDefault, 0, 0, 0, 0, ['.img.@.dpx'])
    checkItemsInDirectory(no_strict, seq.eDetectionNone, 0, 0, 1, 1, ['.img.@.dpx'])
