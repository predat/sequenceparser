#ifndef _SEQUENCE_PARSER_COMMON_DEFINITIONS_HPP
#define _SEQUENCE_PARSER_COMMON_DEFINITIONS_HPP

#define SEQUENCEPARSER_VERSION_MAJOR 2
#define SEQUENCEPARSER_VERSION_MINOR 2
#define SEQUENCEPARSER_VERSION_MICRO 0

#include <string>
#include <cstddef>

#ifdef _MSC_VER
#include <BaseTsd.h>
#else
#include <unistd.h>
#endif

// Compatibility problems
namespace std {
#ifdef _MSC_VER
typedef SSIZE_T ssize_t;
#else
typedef ::ssize_t ssize_t;
#endif
}

namespace sequenceParser {

#ifndef SWIG
typedef ::std::ssize_t Time;

#define SEQUENCEPARSER_ENUM_BITWISE_OPERATORS(ENUM_TYPE) \
\
inline ENUM_TYPE operator~(const ENUM_TYPE& a ) \
{ \
	return (ENUM_TYPE)(~int(a)); \
} \
\
inline ENUM_TYPE operator&=( ENUM_TYPE& a, const ENUM_TYPE& b ) \
{ \
	return a = (ENUM_TYPE)(int(b) & int(a)); \
} \
\
inline ENUM_TYPE operator&( const ENUM_TYPE& a, const ENUM_TYPE& b ) \
{ \
	return (ENUM_TYPE)(int(b) & int(a)); \
} \
\
inline ENUM_TYPE operator|=( ENUM_TYPE& a, const ENUM_TYPE& b ) \
{ \
	return a = (ENUM_TYPE)(int(b) | int(a) ); \
} \
\
inline ENUM_TYPE operator|( const ENUM_TYPE& a, const ENUM_TYPE& b ) \
{ \
	return (ENUM_TYPE)(int(b) | int(a)); \
}
#endif

enum EType
{
	eTypeUndefined = 0,
	eTypeFolder = 1,
	eTypeFile = 2,
	eTypeSequence = 4,
	eTypeLink = 8,
	eTypeAll = (eTypeFolder | eTypeFile | eTypeSequence | eTypeLink)
};

enum EDetection
{
	eDetectionNone = 0,
	/// detect negative numbers (instead of detecting "-" as a non-digit character)
	eDetectionNegative = 1,
	/// A file alone with a number in the filename could be considered as a simple
	/// file or a sequence of 1 item.
	eDetectionSequenceNeedAtLeastTwoFiles = 2,
	/// By default, single file sequence use the latest number of the filename.
	/// You could use this option to use the first number instead.
	eDetectionSingleFileSeqUseFirstNumber = 4,
	eDetectionIgnoreDotFile = 8,
	/// detect from an existing filename of the sequence
	eDetectionSequenceFromFilename = 16,
	/// detect sequences without holes
	eDetectionSequenceWithoutHoles = 32,
	eDetectionDefaultWithDotFile = (eDetectionSequenceNeedAtLeastTwoFiles | eDetectionSequenceFromFilename),
	eDetectionDefault = (eDetectionSequenceNeedAtLeastTwoFiles | eDetectionIgnoreDotFile | eDetectionSequenceFromFilename)
};

#ifndef SWIG
SEQUENCEPARSER_ENUM_BITWISE_OPERATORS(EType)
SEQUENCEPARSER_ENUM_BITWISE_OPERATORS(EDetection)
#endif

#ifdef SWIGJAVA
/**
 * @brief Convert a UTF-8 string to Latin-1 (ISO-8859-1).
 *
 * JNI delivers strings via GetStringUTFChars in Modified UTF-8.
 * Filesystem paths on legacy systems may be in Latin-1, so we
 * re-encode before passing them to std::filesystem.
 *
 * Latin-1 codepoints U+0000–U+00FF map to UTF-8 as:
 *   U+0000–U+007F → 0xxxxxxx          (1 byte, identical)
 *   U+0080–U+00FF → 110000xx 10xxxxxx (2 bytes)
 * Any codepoint outside U+00FF is replaced with '?'.
 */

inline std::string utf8_to_latin1( const std::string& utf8 )
{
	std::string latin1;
	latin1.reserve( utf8.size() );
	for( std::size_t i = 0; i < utf8.size(); )
	{
		const unsigned char c = static_cast<unsigned char>( utf8[i] );
		if( c < 0x80 )
		{
			// U+0000–U+007F: single byte, identical in Latin-1
			latin1 += static_cast<char>( c );
			++i;
		}
		else if( (c & 0xE0) == 0xC0 && i + 1 < utf8.size() )
		{
			// U+0080–U+07FF: two-byte sequence
			const unsigned char c2 = static_cast<unsigned char>( utf8[i + 1] );
			if( (c2 & 0xC0) == 0x80 )
			{
				const unsigned int codepoint = ((c & 0x1F) << 6) | (c2 & 0x3F);
				// Keep only Latin-1 range (U+0000–U+00FF)
				latin1 += ( codepoint <= 0xFF )
				           ? static_cast<char>( codepoint )
				           : '?';
				i += 2;
			}
			else
			{
				latin1 += '?';
				++i;
			}
		}
		else
		{
			// Three/four-byte sequence: codepoint > U+07FF, outside Latin-1.
			// Skip the full sequence to stay in sync.
			if( (c & 0xF0) == 0xE0 )      i += 3;
			else if( (c & 0xF8) == 0xF0 ) i += 4;
			else                           ++i;
			latin1 += '?';
		}
	}
	return latin1;
#endif

}


#endif
