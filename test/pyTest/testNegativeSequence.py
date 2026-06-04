from pySequenceParser import sequenceParser as seq
from . import createFile, getSequencesFromPath


def testNoNegativeValues(tmp_path):
    path = str(tmp_path)
    createFile(path, 'a1b-2c1.j2c')
    createFile(path, 'a1b-2c2.j2c')
    createFile(path, 'a1b-2c3.j2c')
    createFile(path, 'a1b+3c6.j2c')
    createFile(path, 'a1b+3c2.j2c')
    createFile(path, 'a1b+3c0.j2c')
    createFile(path, 'a1b+3c3.j2c')
    createFile(path, 'a1b+3c9.j2c')
    createFile(path, 'a1b9c6.j2c')
    createFile(path, 'a1b9c2.j2c')

    listSequence = getSequencesFromPath(path, seq.eDetectionDefault)
    assert len(listSequence) == 3

    listSequence.sort(key=lambda x: x.getSequence().getNbFiles())

    assert listSequence[0].getSequence().getNbFiles() == 2
    assert listSequence[0].getSequence().getFirstTime() == 2
    assert listSequence[0].getSequence().getLastTime() == 6

    assert listSequence[1].getSequence().getNbFiles() == 3
    assert listSequence[1].getSequence().getFirstTime() == 1
    assert listSequence[1].getSequence().getLastTime() == 3

    assert listSequence[2].getSequence().getNbFiles() == 5
    assert listSequence[2].getSequence().getFirstTime() == 0
    assert listSequence[2].getSequence().getLastTime() == 9


def testNegativeSequence(tmp_path):
    path = str(tmp_path)
    createFile(path, 'a1b2c-3.j2c')
    createFile(path, 'a1b2c-2.j2c')
    createFile(path, 'a1b2c-1.j2c')
    createFile(path, 'a1b2c0.j2c')
    createFile(path, 'a1b2c1.j2c')
    createFile(path, 'a1b2c2.j2c')
    createFile(path, 'a1b2c3.j2c')

    listSequence = getSequencesFromPath(path, seq.eDetectionNegative)
    assert len(listSequence) == 1
    sequence = listSequence[0].getSequence()
    assert sequence.getFirstTime() == -3
    assert sequence.getLastTime() == 3
    assert sequence.getNbFiles() == 7
    assert sequence.hasMissingFile() is False
    assert sequence.getFrameRanges()[-1].step == 1


def testSignedSequencePlus(tmp_path):
    path = str(tmp_path)
    createFile(path, 'a1b2c-3.j2c')
    createFile(path, 'a1b2c-2.j2c')
    createFile(path, 'a1b2c-1.j2c')
    createFile(path, 'a1b2c+0.j2c')
    createFile(path, 'a1b2c+1.j2c')
    createFile(path, 'a1b2c+2.j2c')
    createFile(path, 'a1b2c+3.j2c')

    listSequence = getSequencesFromPath(path, seq.eDetectionNegative)
    assert len(listSequence) == 1
    sequence = listSequence[0].getSequence()
    assert sequence.getFirstTime() == -3
    assert sequence.getLastTime() == 3
    assert sequence.getNbFiles() == 7
    assert sequence.hasMissingFile() is False
    assert sequence.getFrameRanges()[-1].step == 1


def testAmbiguousSignedSequencePlus(tmp_path):
    path = str(tmp_path)
    createFile(path, 'a1b2c-3.j2c')
    createFile(path, 'a1b2c-2.j2c')
    createFile(path, 'a1b2c-1.j2c')
    createFile(path, 'a1b2c+0.j2c')
    createFile(path, 'a1b2c+1.j2c')
    createFile(path, 'a1b2c+2.j2c')
    createFile(path, 'a1b2c+3.j2c')
    createFile(path, 'a1b2c2.j2c')
    createFile(path, 'a1b2c8.j2c')

    listSequence = getSequencesFromPath(path, seq.eDetectionNegative)
    assert len(listSequence) == 1
    sequence = listSequence[0].getSequence()
    assert sequence.getFirstTime() == -3
    assert sequence.getLastTime() == 8
    assert sequence.getNbFiles() == 9
    assert sequence.hasMissingFile() is True
    assert sequence.getFrameRanges()[-1].step == 1


def testAmbiguousNegativeZeroMinus(tmp_path):
    path = str(tmp_path)
    createFile(path, 'a1b2c-3.j2c')
    createFile(path, 'a1b2c-2.j2c')
    createFile(path, 'a1b2c-1.j2c')
    createFile(path, 'a1b2c-0.j2c')

    listSequence = getSequencesFromPath(
        path, seq.eDetectionNegative | seq.eDetectionSequenceNeedAtLeastTwoFiles)
    assert len(listSequence) == 1
