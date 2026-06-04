%include "common.i"

%{
#include "sequenceParser/common.hpp"
#include "sequenceParser/Sequence.hpp"
%}

#ifdef SWIGPYTHON

%feature("docstring") sequenceParser::Sequence "
A numbered file sequence (prefix + zero-padded frame numbers + suffix).

Has no parent-folder notion; the folder is carried by the wrapping
:class:`Item`.  Supports ``len()``, ``in`` (filename membership test),
and boolean evaluation (``True`` when non-empty).

Example::

    >>> seq = item.getSequence()
    >>> print(seq.getFilenameWithStandardPattern())  # e.g. 'frame.####.exr'
    >>> print(len(seq), 'files,', seq.getFirstTime(), '-', seq.getLastTime())
";

%feature("docstring") sequenceParser::Sequence::Sequence "
Construct a Sequence.

Three forms are available:

``Sequence()``
    Empty sequence.

``Sequence(prefix, padding, maxPadding, suffix, firstTime, lastTime[, step=1])``
    Build a sequence from its components (no filesystem access).

    Args:
        prefix (str): Filename prefix (e.g. ``'frame.'``).
        padding (int): Fixed padding width (0 = variable).
        maxPadding (int): Maximum padding width observed.
        suffix (str): Filename suffix (e.g. ``'.exr'``).
        firstTime (int): First frame number.
        lastTime (int): Last frame number.
        step (int): Step between consecutive frames (default 1).

``Sequence(pattern, frameRanges[, accept=ePatternDefault])``
    Build a sequence from a pattern string and explicit frame ranges.

    Args:
        pattern (str): Filename pattern (e.g. ``'frame.####.exr'``).
        frameRanges (FrameRangeVector): Frame ranges to assign.
        accept (int): Bitmask of ``EPattern`` flags (default
            ``ePatternDefault``).

Note:
    Use :func:`browseSequence` to detect a sequence from the filesystem.
";

%feature("docstring") sequenceParser::Sequence::getFiles "
Return a list of every filename in the sequence.

Returns:
    StringVector: All filenames (without directory path), in frame order.
";

%feature("docstring") sequenceParser::Sequence::getFilenameAt "
Return the filename for a specific frame number.

Args:
    time (int): Frame number. No bounds-checking is performed.

Returns:
    str: Filename at the given frame number.
";

%feature("docstring") sequenceParser::Sequence::getFirstFilename "
Return the filename of the first frame.

Returns:
    str: Filename for :attr:`getFirstTime`.
";

%feature("docstring") sequenceParser::Sequence::getLastFilename "
Return the filename of the last frame.

Returns:
    str: Filename for :attr:`getLastTime`.
";

%feature("docstring") sequenceParser::Sequence::getFilenameWithStandardPattern "
Return the sequence pattern using the standard ``####`` notation.

The number of ``#`` characters reflects the padding width.

Returns:
    str: Pattern string, e.g. ``'frame.####.exr'``.
";

%feature("docstring") sequenceParser::Sequence::getFilenameWithPrintfPattern "
Return the sequence pattern using printf / ffmpeg notation.

Returns:
    str: Pattern string, e.g. ``'frame.%04d.exr'``.
";

%feature("docstring") sequenceParser::Sequence::getCStylePattern "
Return the sequence pattern using C-style notation.

Returns:
    str: Pattern string, e.g. ``'frame.%04d.exr'`` (same as printf style).
";

%feature("docstring") sequenceParser::Sequence::getGlobalRange "
Return the (first, last) frame numbers as a pair.

Returns:
    TimePair: ``(first_frame, last_frame)``.  The range may contain holes.
";

%feature("docstring") sequenceParser::Sequence::getFirstTime "
Return the first frame number in the sequence.

Returns:
    int: Earliest frame number.
";

%feature("docstring") sequenceParser::Sequence::getLastTime "
Return the last frame number in the sequence.

Returns:
    int: Latest frame number.
";

%feature("docstring") sequenceParser::Sequence::getDuration "
Return the total span from first to last frame (may include holes).

Returns:
    int: ``last - first + 1``.  Use :meth:`getNbFiles` for the actual count.
";

%feature("docstring") sequenceParser::Sequence::getNbFiles "
Return the number of files actually present in the sequence.

Returns:
    int: File count (excludes holes).  Also available as ``len(sequence)``.
";

%feature("docstring") sequenceParser::Sequence::getFixedPadding "
Return the fixed padding width, or 0 if padding varies.

Returns:
    int: Padding width (number of digits, with leading zeros enforced),
    or 0 when padding is variable or unknown.
";

%feature("docstring") sequenceParser::Sequence::getMaxPadding "
Return the maximum padding width used across all frame numbers.

For fixed-padding sequences this equals :meth:`getFixedPadding`.
For variable or unknown padding this is the maximum observed width.

Returns:
    int: Maximum padding width.
";

%feature("docstring") sequenceParser::Sequence::hasMissingFile "
Return True if the sequence contains at least one hole (missing frame).

Returns:
    bool: True when ``getDuration() > getNbFiles()``.
";

%feature("docstring") sequenceParser::Sequence::getNbMissingFiles "
Return the number of missing frames (holes) in the sequence.

Returns:
    int: ``getDuration() - getNbFiles()``.
";

%feature("docstring") sequenceParser::Sequence::getIdentification "
Return the filename without the frame-number part.

Example: ``'sequence-.jpg'`` instead of ``'sequence-####.jpg'``.

Returns:
    str: Identification string (prefix + suffix, no frame placeholder).
";

%feature("docstring") sequenceParser::Sequence::getPrefix "
Return the filename prefix (characters before the frame number).

Returns:
    str: Prefix, e.g. ``'frame.'`` for ``'frame.####.exr'``.
";

%feature("docstring") sequenceParser::Sequence::getSuffix "
Return the filename suffix (characters after the frame number).

Returns:
    str: Suffix, e.g. ``'.exr'`` for ``'frame.####.exr'``.
";

%feature("docstring") sequenceParser::Sequence::isIn "
Check whether a filename belongs to this sequence and return its frame number.

Args:
    filename (str): Filename to look up (just the filename, not the full path).

Returns:
    tuple[bool, int, str]: A 3-tuple ``(found, frame_number, frame_string)``.
    *found* is True when the file is part of the sequence.
";

%feature("docstring") sequenceParser::Sequence::getFrameRanges "
Return the underlying list of FrameRange objects.

Returns:
    FrameRangeVector: Mutable list of contiguous sub-ranges that make up
    the sequence (e.g. two ranges when there is a hole).
";

%feature("docstring") sequenceParser::Sequence::getFramesIterable "
Return an iterable view over the frame numbers in the sequence.

With no arguments yields every frame number in order.
With *first* and *last* yields only frames within that window.

Args:
    first (int, optional): Lower bound of the window (inclusive).
    last  (int, optional): Upper bound of the window (inclusive).

Returns:
    FrameRangesView or FrameRangesSubView: Iterable of ``int`` frame numbers.

Example::

    for frame in seq.getFramesIterable():
        print(seq.getFilenameAt(frame))
";

%feature("docstring") sequenceParser::Sequence::initFromPattern "
Initialise this Sequence from a filename pattern (no filesystem access).

Args:
    pattern (str): Pattern string (e.g. ``'frame.####.exr'`` or
        ``'frame.%04d.exr'``).
    accept (int): Bitmask of :data:`EPattern` flags accepted during parsing.

Returns:
    bool: True if the pattern was recognised and the sequence initialised.

Warning:
    Range and directory information are NOT available after this call;
    use :func:`browseSequence` to also populate the frame ranges.
";

%feature("docstring") sequenceParser::Sequence::clear "
Reset the sequence to an empty state.
";

#endif

namespace std {
// Allow vector of object with no default constructor
%ignore vector< sequenceParser::FrameRange >::vector(size_type);
%ignore vector< sequenceParser::FrameRange >::resize;

// Ignore the default constructor
%ignore pair::pair();

// Create instantiations of a template classes
%template(FrameRangeVector) vector< sequenceParser::FrameRange >;
%template(TimePair) pair< sequenceParser::Time, sequenceParser::Time >;

// Custom typemaps: apply a special type handling rule to a type
%apply sequenceParser::Time& OUTPUT { sequenceParser::Time & timeOut };
%apply string& OUTPUT { string & timeStrOut };
}

#ifdef SWIGJAVA

%rename (toString) sequenceParser::Sequence::string;
%rename (equals) sequenceParser::Sequence::operator==;
%ignore sequenceParser::Sequence::operator!=;
%ignore sequenceParser::Sequence::operator<;

#elif SWIGPYTHON

%rename(__str__) sequenceParser::Sequence::string;
%rename(__eq__) sequenceParser::Sequence::operator==;
%rename(__ne__) sequenceParser::Sequence::operator!=;
%rename(__lt__) sequenceParser::Sequence::operator<;

#endif

%include "Sequence.hpp"

// Remove all typemaps
%clear sequenceParser::Time & timeOut;
%clear std::string & timeStrOut;

#ifdef SWIGPYTHON
%extend sequenceParser::Sequence {
    %pythoncode {
        def __len__(self):
            """Return the number of files in the sequence."""
            return self.getNbFiles()

        def __bool__(self):
            """Return True if the sequence contains at least one file."""
            return self.getNbFiles() > 0

        def __contains__(self, filename):
            """Return True if *filename* belongs to this sequence.

            Args:
                filename (str): Filename to test (just the basename, not a
                    full path).

            Returns:
                bool: True when *filename* matches this sequence pattern and
                its frame number falls within the sequence ranges.
            """
            found, _, _ = self.isIn(str(filename))
            return found
    }
}
#endif
