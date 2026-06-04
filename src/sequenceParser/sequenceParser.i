
%module(moduleimport="from . import _sequenceParser") sequenceParser

%pythonbegin %{
"""sequenceParser — detect and browse file sequences in the filesystem.

Wraps the C++ sequenceParser library. Scans directories and identifies
numbered file sequences (e.g. ``frame.0001.exr``, ``frame.0002.exr``) as
unified :class:`Sequence` objects alongside plain files, folders and links.

Example::

    from PySequenceParser import sequenceParser as sp

    for item in sp.browse('/path/to/renders'):
        if item.getType() == sp.eTypeSequence:
            seq = item.getSequence()
            print(seq.getFilenameWithStandardPattern(), len(seq), 'frames')

**EType** constants (item kind):
    ``eTypeUndefined``, ``eTypeFile``, ``eTypeFolder``,
    ``eTypeSequence``, ``eTypeLink``, ``eTypeAll``.

**EDetection** flags (combinable with ``|``):
    ``eDetectionNone``, ``eDetectionNegative``,
    ``eDetectionSequenceNeedAtLeastTwoFiles``,
    ``eDetectionSingleFileSeqUseFirstNumber``,
    ``eDetectionIgnoreDotFile``, ``eDetectionSequenceFromFilename``,
    ``eDetectionSequenceWithoutHoles``, ``eDetectionDefault``.

**EPattern** flags (filename pattern styles):
    ``ePatternNone``, ``ePatternStandard``, ``ePatternCStyle``,
    ``ePatternFrame``, ``ePatternFrameNeg``,
    ``ePatternDefault``, ``ePatternAll``.
"""
%}

%include "common.i"

%include "FrameRange.i"
%include "Sequence.i"
%include "Item.i"
%include "ItemStat.i"

%include "detector.i"
%include "filesystem.i"
