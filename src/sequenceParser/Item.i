%include "common.i"

%{
#include "sequenceParser/Item.hpp"
%}

#ifdef SWIGPYTHON

%feature("docstring") sequenceParser::Item "
A filesystem element: file, folder, symbolic link, or file sequence.

An Item is the central object returned by :func:`browse`.  For sequence
items, the :class:`Sequence` descriptor is accessible via
:meth:`getSequence`.

Example::

    for item in sp.browse('/path/to/dir'):
        t = item.getType()
        if t == sp.eTypeSequence:
            print('Sequence:', item.getSequence().getFilenameWithStandardPattern())
        elif t == sp.eTypeFile:
            print('File:', item.getFilename())
        elif t == sp.eTypeFolder:
            print('Folder:', item.getFolder())
";

%feature("docstring") sequenceParser::Item::getType "
Return the type of this item.

Returns:
    int: One of ``eTypeUndefined``, ``eTypeFile``, ``eTypeFolder``,
    ``eTypeSequence``, ``eTypeLink``.
";

%feature("docstring") sequenceParser::Item::getAbsoluteFilepath "
Return the absolute path of this item.

For sequences the path contains the standard ``####`` pattern instead of a
frame number.

Returns:
    str: Absolute path string.
";

%feature("docstring") sequenceParser::Item::getFilename "
Return the filename (basename) of this item.

Returns:
    str: Filename without directory component.
";

%feature("docstring") sequenceParser::Item::getFolder "
Return the directory path containing this item.

Returns:
    str: Parent directory path.
";

%feature("docstring") sequenceParser::Item::explode "
Expand a sequence item into individual single-file Items.

Returns:
    list[Item]: One :class:`Item` per frame in the sequence.  Returns a
    single-element list for non-sequence items.
";

%feature("docstring") sequenceParser::Item::getSequence "
Return the :class:`Sequence` descriptor for a sequence item.

Returns:
    Sequence: The underlying Sequence object.

Warning:
    Only valid when :meth:`getType` == ``eTypeSequence``.
";

%feature("docstring") sequenceParser::Item::getAbsoluteFirstFilename "
Return the absolute path of the first file in the sequence.

Returns:
    str: Absolute path of the first frame (or the item path for non-sequences).
";

%feature("docstring") sequenceParser::Item::getFirstFilename "
Return the filename (basename) of the first file in the sequence.

Returns:
    str: Basename of the first frame.
";

%feature("docstring") sequenceParser::getTypeFromPath "
Detect the filesystem type of a path (file, folder, or link).

Args:
    pathStr (str): Path to inspect.

Returns:
    int: One of ``eTypeFile``, ``eTypeFolder``, ``eTypeLink``, or
    ``eTypeUndefined`` if the path does not exist or is unrecognised.
    Never returns ``eTypeSequence``.
";

#endif

#ifdef SWIGJAVA

%rename (toString) sequenceParser::Item::string;
%ignore sequenceParser::Item::operator<;

#elif SWIGPYTHON

%rename(__str__) sequenceParser::Item::string;
%rename(__lt__) sequenceParser::Item::operator<;

#endif

%include "Item.hpp"
