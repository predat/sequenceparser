%include "common.i"

%include <std_vector.i>
%include <std_except.i>
%include <exception.i>

%{
#include "sequenceParser/Item.hpp"
#include "sequenceParser/filesystem.hpp"
%}

%template(ItemVector) ::std::vector<sequenceParser::Item>;

#ifdef SWIGPYTHON

%feature("docstring") sequenceParser::browseSequence "
Detect a file sequence from a filename pattern on the filesystem.

Args:
    outSequence (Sequence): Sequence object to populate (modified in-place).
    pattern (str): Absolute path with a frame-number placeholder, e.g.
        ``'/tmp/frame####.exr'`` or ``'/tmp/frame%04d.exr'``.
    accept (int): Bitmask of :data:`EPattern` flags (default:
        ``ePatternDefault``).

Returns:
    bool: True if a sequence was found and *outSequence* was populated.

Example::

    seq = sp.Sequence()
    if sp.browseSequence(seq, '/renders/frame####.exr'):
        print('Found', len(seq), 'frames')
";

%feature("docstring") sequenceParser::browse "
Browse a directory and return a list of filesystem items.

Detects files, folders, symbolic links, and numbered file sequences.

Args:
    directory (str or Item): Path to the directory to scan.
    detectOptions (int): Bitmask of :data:`EDetection` flags controlling
        how sequences are detected (default: ``eDetectionDefault``).
    filters (StringVector): Optional glob patterns to restrict results,
        e.g. ``['*.exr', '*.jpg']``.  Empty list means no filtering.

Returns:
    ItemVector: Items found in *directory* (unordered).

Raises:
    IOError: If *directory* does not exist or is not accessible.

Example::

    items = sp.browse('/path/to/renders')
    sequences = [i for i in items if i.getType() == sp.eTypeSequence]
    for seq_item in sequences:
        seq = seq_item.getSequence()
        print(seq.getFilenameWithStandardPattern(), len(seq), 'frames')
";

#endif

%include "filesystem.hpp"

namespace sequenceParser {
%ignore browse(
		const std::filesystem::path&,
		const EDetection detectOptions,
		const std::vector<std::string>& );
}

