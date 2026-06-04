from pySequenceParser import sequenceParser as seq
from . import createFile, getSequencesFromPath


def testNothing(tmp_path):
    listSequence = getSequencesFromPath(str(tmp_path), seq.eDetectionDefault)
    assert len(listSequence) == 0


def testSingleFilename(tmp_path):
    path = str(tmp_path)
    createFile(path, "a1b2.j2c")

    listSequence = getSequencesFromPath(path, seq.eDetectionDefault)
    assert len(listSequence) == 0


def testSingleSequence(tmp_path):
    path = str(tmp_path)
    createFile(path, "a1b2.j2c")
    createFile(path, "a1b3.j2c")
    createFile(path, "a1b4.j2c")

    listSequence = getSequencesFromPath(path, seq.eDetectionDefault)
    assert len(listSequence) == 1


def testFilenameInSequence(tmp_path):
    path = str(tmp_path)
    createFile(path, "a1b2.j2c")
    createFile(path, "a1b3.j2c")
    createFile(path, "a1b4.j2c")

    listSequence = getSequencesFromPath(path, seq.eDetectionDefault)
    sequence = listSequence[0].getSequence()

    assert sequence.isIn("") == [False, 0, '']
    assert sequence.isIn("toto") == [False, 0, '']
    assert sequence.isIn("a1c2.j2c") == [False, 0, '']
    assert sequence.isIn("a1b2.jpg") == [False, 0, '']
    assert sequence.isIn("a1b10.j2c") == [False, 0, '']

    assert sequence.isIn("a1b2.j2c") == [True, 2, '2']


def testSingleSequenceMultiRanges(tmp_path):
    path = str(tmp_path)
    createFile(path, "a1b2.j2c")
    createFile(path, "a1b3.j2c")
    createFile(path, "a1b4.j2c")
    createFile(path, "a1b14.j2c")
    createFile(path, "a1b15.j2c")
    createFile(path, "a1b16.j2c")
    createFile(path, "a1b20.j2c")
    createFile(path, "a1b22.j2c")
    createFile(path, "a1b24.j2c")

    listSequence = getSequencesFromPath(path, seq.eDetectionDefault)
    assert len(listSequence) == 1

    sequence = listSequence[0].getSequence()
    assert sequence.getFrameRanges().size() == 3

    times = [2, 3, 4, 14, 15, 16, 20, 22, 24]
    for frame, expected in zip(sequence.getFramesIterable(), times):
        assert frame == expected
