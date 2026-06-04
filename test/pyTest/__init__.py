import os

from pySequenceParser import sequenceParser as seq


def createFile(path, filename, mode=0o664):
    ff = os.path.join(path, filename)
    open(ff, 'w').close()
    os.chmod(ff, mode)


def createFolder(path, folderName, mode=0o664):
    dd = os.path.join(path, folderName)
    os.mkdir(dd)
    os.chmod(dd, mode)


def createSymLink(path, target, symLinkName):
    src = os.path.join(path, target)
    dst = os.path.join(path, symLinkName)
    os.symlink(src, dst)


def getSequencesFromPath(path, detectionOptions, filters=[]):
    listSequence = []
    items = seq.browse(path, detectionOptions, filters)
    for item in items:
        if item.getType() == seq.eTypeSequence:
            listSequence.append(item)
    return listSequence
