from pySequenceParser import sequenceParser as seq
from . import createFile, getSequencesFromPath


def testSequenceWithPadding1(tmp_path):
    path = str(tmp_path)
    createFile(path, 'a1b2c001.j2c')
    createFile(path, 'a1b2c002.j2c')
    createFile(path, 'a1b2c003.j2c')
    createFile(path, 'a1b2c023.j2c')

    listSequence = getSequencesFromPath(path, seq.eDetectionDefault)
    assert len(listSequence) == 1
    assert listSequence[0].getSequence().getFixedPadding() == 3
    assert listSequence[0].getSequence().getMaxPadding() == 3


def testSequenceWithPadding2(tmp_path):
    path = str(tmp_path)
    createFile(path, 'a1b2c001.j2c')
    createFile(path, 'a1b2c002.j2c')
    createFile(path, 'a1b2c003.j2c')
    createFile(path, 'a1b2c809.j2c')
    createFile(path, 'a1b2c999.j2c')

    listSequence = getSequencesFromPath(path, seq.eDetectionDefault)
    assert len(listSequence) == 1
    assert listSequence[0].getSequence().getFixedPadding() == 3
    assert listSequence[0].getSequence().getMaxPadding() == 3


def testSequenceWithoutPadding(tmp_path):
    path = str(tmp_path)
    createFile(path, 'a1b2c99.j2c')
    createFile(path, 'a1b2c102.j2c')
    createFile(path, 'a1b2c1234.j2c')
    createFile(path, 'a1b2c12345.j2c')
    createFile(path, 'a1b2c123456.j2c')

    listSequence = getSequencesFromPath(path, seq.eDetectionDefault)
    assert len(listSequence) == 1
    sequence = listSequence[0].getSequence()
    assert sequence.getFixedPadding() == 0
    assert sequence.getMaxPadding() == 2
    assert sequence.getFirstTime() == 99
    assert sequence.getLastTime() == 123456
    assert sequence.getNbFiles() == 5
    assert sequence.hasMissingFile() is True
    assert sequence.getFrameRanges().size() == 5


def testDoublePaddingForTheSameSequence(tmp_path):
    path = str(tmp_path)
    createFile(path, 'a1b2c00099.j2c')
    createFile(path, 'a1b2c00102.j2c')
    createFile(path, 'a1b2c1234.j2c')
    createFile(path, 'a1b2c12345.j2c')
    createFile(path, 'a1b2c123456.j2c')

    listSequence = getSequencesFromPath(path, seq.eDetectionDefault)
    assert len(listSequence) == 2
