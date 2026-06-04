%include "common.i"

%{
#include "sequenceParser/FrameRange.hpp"
%}

#ifdef SWIGPYTHON

%feature("docstring") sequenceParser::FrameRange "
A contiguous range of frame numbers with an optional step.

Attributes:
    first (int): First frame number (inclusive).
    last (int): Last frame number (inclusive).
    step (int): Step between consecutive frames (>= 1).

Example::

    >>> r = FrameRange(1, 10)
    >>> str(r)
    '1-10'
    >>> r = FrameRange(1, 10, 2)
    >>> str(r)
    '1-10x2'
    >>> r.getNbFrames()
    5
";

%feature("docstring") sequenceParser::FrameRange::FrameRange "
Construct a FrameRange.

Args:
    first (int): First frame number.
    last (int): Last frame number (inclusive, default equals *first*).
    step (int): Step between consecutive frames (default 1, must be >= 1).
";

%feature("docstring") sequenceParser::FrameRange::atIndex "
Return the frame number at a given zero-based index within the range.

Args:
    index (int): Zero-based position (0 → first frame, 1 → first+step, …).

Returns:
    int: Frame number at *index*.
";

%feature("docstring") sequenceParser::FrameRange::getNbFrames "
Return the total number of frames covered by this range.

Returns:
    int: ``(last - first) // step + 1``.
";

%feature("docstring") sequenceParser::FrameRangesView "
A read-only iterable view over a list of FrameRange objects.

Supports iteration (yields frame numbers in order) and ``str()``.
";

%feature("docstring") sequenceParser::FrameRangesSubView "
A read-only iterable view over a sub-set of FrameRange objects clipped to
[firstTime, lastTime].

Supports iteration (yields frame numbers in order) and ``str()``.
";

#endif

#ifdef SWIGJAVA

%rename(toString) sequenceParser::FrameRange::string;
%rename(equals) sequenceParser::FrameRange::operator==;

%ignore sequenceParser::FrameRangesConstIterator::operator++;
%ignore sequenceParser::FrameRangesConstIterator::operator--;
%rename(equals) sequenceParser::FrameRangesConstIterator::operator==;
%ignore sequenceParser::FrameRangesConstIterator::operator!=;

#elif SWIGPYTHON

%rename(__str__) sequenceParser::FrameRange::string;
%rename(__eq__) sequenceParser::FrameRange::operator==;

%rename(__next__) sequenceParser::FrameRangesConstIterator::operator++;
%rename(__previous__) sequenceParser::FrameRangesConstIterator::operator--;
%rename(__eq__) sequenceParser::FrameRangesConstIterator::operator==;
%rename(__ne__) sequenceParser::FrameRangesConstIterator::operator!=;

#endif

%include "FrameRange.hpp"

#ifdef SWIGPYTHON

%pythoncode
{
    class PyFrameRangesConstIterator:
        """Internal iterator over a sequence of frame numbers.

        Produced by iterating a :class:`FrameRangesView` or
        :class:`FrameRangesSubView`.  Each value is an ``int`` frame number.
        """
        def __init__(self, it, itEnd):
            self.it = it
            self.itEnd = itEnd
        def __iter__(self):
            return self
        def __next__(self):
            self.it = self.it.next()
            if self.it == self.itEnd:
                raise StopIteration
            return self.it.value()
        def next(self):
            return self.__next__()
}

%extend sequenceParser::FrameRangesView
{
    %pythoncode
	{
        def __iter__(self):
            return PyFrameRangesConstIterator(self.begin().previous(), self.end())
        def __str__(self):
            return self.string()
    }
}

%extend sequenceParser::FrameRangesSubView
{
    %pythoncode
	{
        def __iter__(self):
            return PyFrameRangesConstIterator(self.begin().previous(), self.end())
        def __str__(self):
            return self.string()
    }
}

#endif
