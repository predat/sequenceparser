%include "common.i"

%{
#include "sequenceParser/ItemStat.hpp"
%}

#ifdef SWIGPYTHON

%feature("docstring") sequenceParser::ItemStat "
Filesystem statistics for an :class:`Item` (size, permissions, timestamps).

Wraps the POSIX ``stat(2)`` syscall.  For sequences, aggregated values
are computed across all files in the sequence.

Args:
    item (Item): The item to stat.
    approximative (bool): When True (default), use faster approximations for
        sequences (e.g. extrapolate size from a subset of files).

Attributes:
    deviceId (int): Device ID containing the file.
    inodeId (int): Inode number.
    nbHardLinks (float): Number of hard links (average for sequences).
    fullNbHardLinks (int): Total hard-link count across the whole sequence.
    userId (int): Numeric user ID of the owner.
    groupId (int): Numeric group ID of the owner.
    userName (str): Owner user name.
    groupName (str): Owner group name.
    size (int): Total size in bytes.
    minSize (int): Smallest file size in the sequence.
    maxSize (int): Largest file size in the sequence.
    realSize (int): Size accounting for hard-links.
    sizeOnDisk (int): Disk usage accounting for hard-links.
    accessTime (int): Last access time (Unix timestamp).
    modificationTime (int): Last modification time (Unix timestamp).
    lastChangeTime (int): Last status-change time (Unix timestamp).
    ownerCanRead (bool): Owner read permission.
    ownerCanWrite (bool): Owner write permission.
    ownerCanExecute (bool): Owner execute permission.
    groupCanRead (bool): Group read permission.
    groupCanWrite (bool): Group write permission.
    groupCanExecute (bool): Group execute permission.
    otherCanRead (bool): World read permission.
    otherCanWrite (bool): World write permission.
    otherCanExecute (bool): World execute permission.

Example::

    stat = sp.ItemStat(item)
    print(stat.size, 'bytes, owner:', stat.userName)
";

%feature("docstring") sequenceParser::ItemStat::ItemStat "
Collect filesystem statistics for *item*.

Args:
    item (Item): Filesystem element to stat.
    approximative (bool): Use faster approximations for sequences
        (default True).
";

#endif

%include "ItemStat.hpp"

//%extend sequenceParser::ItemStat
//{
//	%pythoncode
//	{
//		def __str__(self):
//			return self.string()
//	}
//}
