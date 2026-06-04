from pySequenceParser import sequenceParser as seq
from . import createFile, getSequencesFromPath


def testSimpleMultiSequence(tmp_path):
    path = str(tmp_path)
    createFile(path, 'a1b2c1.j2c')
    createFile(path, 'a1b2c2.j2c')
    createFile(path, 'a1b2c3.j2c')
    createFile(path, 'a1b3c6.j2c')
    createFile(path, 'a1b3c2.j2c')
    createFile(path, 'a1b3c3.j2c')
    createFile(path, 'a1b9c6.j2c')
    createFile(path, 'a1b9c2.j2c')

    listSequence = getSequencesFromPath(path, seq.eDetectionDefault)
    assert len(listSequence) == 3


def testSimpleMultiSequenceMultiLevel(tmp_path):
    path = str(tmp_path)
    createFile(path, 'a1b2c1.j2c')
    createFile(path, 'a1b2c2.j2c')
    createFile(path, 'a1b2c3.j2c')
    createFile(path, 'a1b3c4.j2c')
    createFile(path, 'a1b4c4.j2c')
    createFile(path, 'a1b5c4.j2c')
    createFile(path, 'a1b6c4.j2c')

    listSequence = getSequencesFromPath(path, seq.eDetectionDefault)
    assert len(listSequence) == 2
    assert listSequence[0].getSequence().getNbFiles() == 3
    assert listSequence[1].getSequence().getNbFiles() == 4


def testMultiSequenceMultiLevelMultiPadding(tmp_path):
    path = str(tmp_path)
    createFile(path, 'a1b2c1.j2c')
    createFile(path, 'a1b2c3.j2c')
    createFile(path, 'a1b002c2.j2c')
    createFile(path, 'a1b3c4.j2c')
    createFile(path, 'a1b4c4.j2c')
    createFile(path, 'a1b5c4.j2c')
    createFile(path, 'a1b6c4.j2c')

    listSequence = getSequencesFromPath(path, seq.eDetectionDefault)
    assert len(listSequence) == 2


def testMultiSequenceMultiLevelMultiPadding2(tmp_path):
    path = str(tmp_path)
    createFile(path, 'a1b2c1.j2c')
    createFile(path, 'a1b2c3.j2c')
    createFile(path, 'a1b002c2.j2c')
    createFile(path, 'a1b002c3.j2c')
    createFile(path, 'a1b3c4.j2c')
    createFile(path, 'a1b4c4.j2c')
    createFile(path, 'a1b5c4.j2c')
    createFile(path, 'a1b6c4.j2c')

    listSequence = getSequencesFromPath(path, seq.eDetectionDefault)
    assert len(listSequence) == 3


def testMultiSequenceMultiLevelMultiPaddingWithNegValues(tmp_path):
    path = str(tmp_path)
    createFile(path, 'a1b2c1.j2c')
    createFile(path, 'a1b2c3.j2c')
    createFile(path, 'a1b3c4.j2c')
    createFile(path, 'a1b5c4.j2c')
    createFile(path, 'a1b6c4.j2c')
    createFile(path, 'a1b002c2.j2c')
    createFile(path, 'a1b002c-3.j2c')
    createFile(path, 'a1b4c-4.j2c')
    createFile(path, 'a1b5c-4.j2c')
    createFile(path, 'a1b-6c-4.j2c')

    listSequence = getSequencesFromPath(
        path, seq.eDetectionNegative | seq.eDetectionSequenceNeedAtLeastTwoFiles)
    assert len(listSequence) == 4


def testMultiSequenceMinusSeparatorAmbiguityWithNegValues(tmp_path):
    path = str(tmp_path)
    createFile(path, 'a1b2-1.j2c')
    createFile(path, 'a1b2-3.j2c')
    createFile(path, 'a1b2+5.j2c')
    createFile(path, 'a1b2+10.j2c')
    createFile(path, 'a1b24.j2c')

    listSequence = getSequencesFromPath(
        path, seq.eDetectionNegative | seq.eDetectionSequenceNeedAtLeastTwoFiles)
    assert len(listSequence) == 1
    assert listSequence[0].getSequence().getFirstTime() == -3
    assert listSequence[0].getSequence().getLastTime() == 10
    assert listSequence[0].getSequence().getFrameRanges().size() == 4
    assert listSequence[0].getSequence().getNbFiles() == 4


def testMultiSequenceSingleFile(tmp_path):
    path = str(tmp_path)
    createFile(path, 'a1b24.jpg')

    listSequence = getSequencesFromPath(path, seq.eDetectionNegative)
    assert len(listSequence) == 1
    assert listSequence[0].getSequence().getFirstTime() == 24
    assert listSequence[0].getSequence().getLastTime() == 24
    assert listSequence[0].getSequence().getNbFiles() == 1

    listSequence = getSequencesFromPath(
        path, seq.eDetectionNegative | seq.eDetectionSingleFileSeqUseFirstNumber)
    assert len(listSequence) == 1
    assert listSequence[0].getSequence().getFirstTime() == 1
    assert listSequence[0].getSequence().getLastTime() == 1
    assert listSequence[0].getSequence().getNbFiles() == 1
