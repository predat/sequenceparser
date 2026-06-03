#include "analyze.hpp"

#include "FileNumbers.hpp"
#include "FileStrings.hpp"

#include <regex>
#include <unordered_map>
#include <set>
#include <cassert>
#include <iterator>
#include <filesystem>


namespace sequenceParser {

using detail::FileNumbers;
using detail::FileStrings;
namespace fs = std::filesystem;

bool detectDirectoryInResearch( std::string& researchPath, std::vector<std::string>& filters, std::string& filename )
{
	if( fs::exists( researchPath ) )
	{
		if( !fs::is_directory( researchPath ) )
		{
			// the researchPath is an existing file, we search into the parent directory with filtering these filename
			// warning: can find a sequence based on a filename
			fs::path tmpPath( researchPath );
			filename = researchPath;

			if( tmpPath.has_parent_path() )
				researchPath = tmpPath.parent_path().string();
			else
				researchPath = ".";
		}
		// else
		// the researchPath is a directory, we search into the directory without filtering
	}
	else
	{
		fs::path tmpPath( researchPath );
		if( !tmpPath.has_parent_path() )
		{
			filters.push_back( researchPath );
			researchPath = ".";
			return true;
		}
		fs::path parentPath( tmpPath.parent_path() );
		if( !fs::exists( parentPath ) )
		{
			// researchPath and it parent don't exists, could not find file/sequence/folder
			return false;
		}
		// the researchPath is not a directory, but it parent is a directory
		// we search in this parent directory, with the filtering pattern
		filters.push_back( tmpPath.filename().string() );
		researchPath = parentPath.string();
	}
	return true;
}


Sequence privateBuildSequence(
		const Sequence& defaultSeq,
		const FileStrings& stringParts,
		const std::vector<FileNumbers>::const_iterator& numberPartsBegin,
		const std::vector<FileNumbers>::const_iterator& numberPartsEnd,
		const std::size_t index,
		const std::size_t padding,
		const std::size_t maxPadding
	)
{
	const std::size_t len = numberPartsBegin->size();
	Sequence sequence( defaultSeq );

	// fill information in the sequence...
	for( std::size_t i = 0; i < index; ++i )
	{
		sequence._prefix += stringParts[i];
		sequence._prefix += numberPartsBegin->getString( i );
	}
	sequence._prefix += stringParts[index];
	for( std::size_t i = index + 1; i < len; ++i )
	{
		sequence._suffix += stringParts[i];
		sequence._suffix += numberPartsBegin->getString( i );
	}
	sequence._suffix += stringParts[len];

	std::vector<FileNumbers>::const_iterator numberPartsLast = numberPartsEnd;
	--numberPartsLast;

	// standard case, one sequence detected
	std::vector<Time> times;
	times.reserve(std::distance( numberPartsBegin, numberPartsEnd ));
	for( std::vector<FileNumbers>::const_iterator it = numberPartsBegin; it != numberPartsEnd; ++it )
	{
		times.push_back(it->getTime(index));
	}
	sequence._ranges = extractFrameRanges(times);
	sequence._fixedPadding = padding;
	sequence._maxPadding = maxPadding;

	return sequence;
}

/**
 *
 * @param result
 * @param numberPartsBegin
 * @param numberPartsEnd
 * @param index
 */
void privateBuildSequencesAccordingToPadding(
	std::vector<Sequence>& result,
	const Sequence& defaultSeq,
	const FileStrings& stringParts,
	const std::vector<FileNumbers>::iterator& numberPartsBegin,
	const std::vector<FileNumbers>::iterator numberPartsEnd,
	const int index )
{
	std::set<std::size_t> paddings;
	std::set<std::size_t> ambiguousMaxPaddings;
	for( std::vector<FileNumbers>::const_iterator it = numberPartsBegin;
	     it != numberPartsEnd;
		 ++it )
	{
		const std::size_t padding  = it->getFixedPadding(index);
		const std::size_t maxPadding = it->getMaxPadding(index);

		paddings.insert( padding );

		if( padding == 0 )
		{
			ambiguousMaxPaddings.insert( maxPadding );
		}
	}

	if( paddings.size() == 1 )
	{
		// standard case: only one padding used in the sequence!
		const std::size_t padding = *paddings.begin();
		const std::size_t maxPadding = ( padding == 0 ? *ambiguousMaxPaddings.begin() : padding );
		// simple sort
		std::sort( numberPartsBegin, numberPartsEnd, FileNumbers::SortByNumber() );
		result.push_back( privateBuildSequence( defaultSeq, stringParts, numberPartsBegin, numberPartsEnd, index, padding, maxPadding ) );
		return;
	}

	bool onlyConsiderPadding = false;
	if( paddings.find( 0 ) == paddings.end() )
	{
		// No element without padding.
		// All parts are prefixed by 0, only strict padding,
		// so we can sort by padding without ambiguity
		onlyConsiderPadding = true;
	}
	else
	{
		onlyConsiderPadding = false;
		for( const std::size_t maxPadding : ambiguousMaxPaddings )
		{
			if( paddings.find( maxPadding ) == paddings.end() )
			{
				// if one digits from ambiguous digits doesn't correspond to
				// a padding... we keep the whole sequence without padding.
				onlyConsiderPadding = true;
				break;
			}
		}
	}

	if( onlyConsiderPadding )
	{
		// sort by padding
		std::sort( numberPartsBegin, numberPartsEnd, FileNumbers::SortByPadding() );
		// split when the padding changed
		std::vector<FileNumbers>::const_iterator first = numberPartsBegin;
		for( std::vector<FileNumbers>::const_iterator it = std::next(first); it != numberPartsEnd; ++it )
		{
			if( first->getFixedPadding(index) != it->getFixedPadding(index) )
			{
				const std::size_t p = first->getFixedPadding(index);
				result.push_back( privateBuildSequence( defaultSeq, stringParts, first, it, index, p, first->getMaxPadding(index) ) );
				first = it;
			}
		}
		const std::size_t p = first->getFixedPadding(index);
		result.push_back( privateBuildSequence( defaultSeq, stringParts, first, numberPartsEnd, index, p, first->getMaxPadding(index) ) );
		return;
	}
	else
	{
		// sort by digits
		std::sort( numberPartsBegin, numberPartsEnd, FileNumbers::SortByDigit() );
		// split when the number of digits changed
		std::vector<FileNumbers>::const_iterator first = numberPartsBegin;
		for( std::vector<FileNumbers>::const_iterator it = std::next(numberPartsBegin); it != numberPartsEnd; ++it )
		{
			if( first->getMaxPadding(index) != it->getMaxPadding(index) )
			{
				const std::size_t p = std::prev(it)->getFixedPadding(index);
				const std::size_t pStart = first->getFixedPadding(index);
				result.push_back( privateBuildSequence( defaultSeq, stringParts, first, it, index, pStart, first->getMaxPadding(index) ) );
				first = it;
			}
		}
		const std::size_t p = std::prev(numberPartsEnd)->getFixedPadding(index);
		const std::size_t pStart = first->getFixedPadding(index);
		result.push_back( privateBuildSequence( defaultSeq, stringParts, first, numberPartsEnd, index, pStart, first->getFixedPadding(index) ) );
		return;
	}
}


bool getVaryingNumber( std::ssize_t& index, const FileNumbers& a, const FileNumbers& b )
{
	assert( a.size() == b.size() );
	bool foundOne = false;
	for( std::size_t i = 0; i < a.size(); ++i )
	{
		if( a.getString(i) != b.getString(i) )
		{
			if( foundOne )
			{
				index = -1;
				return false; // more than one element founded
			}
			foundOne = true;
			index = i;
		}
	}
	if( !foundOne )
		index = -1;
	return foundOne; // we found one varying index
}

std::vector<Sequence> buildSequences( const fs::path& directory, const FileStrings& stringParts, std::vector<FileNumbers>& numberParts, const EDetection detectOptions )
{
	Sequence defaultSeq;

	assert( numberParts.size() > 0 );
	// assert all FileNumbers have the same size...
	assert( numberParts.front().size() == numberParts.back().size() );
	const std::size_t len = numberParts.front().size();
	std::vector<Sequence> result;
	
	if( numberParts.size() == 1 )
	{
		std::size_t index = (detectOptions & eDetectionSingleFileSeqUseFirstNumber) ? 0 : len-1;
		privateBuildSequencesAccordingToPadding( result, defaultSeq, stringParts, numberParts.begin(), numberParts.end(), index );
		return result;
	}
	
	// detect which part is the sequence number
	// for the moment, accept only one sequence
	// but we can easily support multi-sequences
	std::vector<std::size_t> allIndex; // vector of indices (with 0 < index < len) with value changes
	for( std::size_t i = 0; i < len; ++i )
	{
		const std::string t = numberParts.front().getString( i );

		for( const FileNumbers& sn : numberParts )
		{
			if( sn.getString( i ) != t )
			{
				allIndex.push_back( i );
				break;
			}
		}
	}
	
	if( allIndex.size() == 1 )
	{
		// if it's a simple sequence, but may be mix multiple paddings
		privateBuildSequencesAccordingToPadding( result, defaultSeq, stringParts, numberParts.begin(), numberParts.end(), allIndex.front() );
		return result;
	}
	
	// it's a multi-sequence
	std::sort( numberParts.begin(), numberParts.end(), FileNumbers::SortByPadding() );

	std::vector<FileNumbers>::iterator first = numberParts.begin();
	std::vector<FileNumbers>::iterator it = std::next(first);
	std::vector<FileNumbers>::iterator itEnd = numberParts.end();
	std::ssize_t previousIndex = -1;
	std::ssize_t index = -1;
	bool split = false;
	
	for( ; it != itEnd; ++it )
	{
		if( getVaryingNumber( index, *first, *it ) )
		{
			if( previousIndex != -1 && // we previously have a sequence and
			    index != previousIndex ) // the index is not the same than previous: split!
			{
				split = true;
			}
		}
		else
		{
			// more than one varying number: split in all cases!
			split = true;
			// if no sequence before... the file is alone...
			// Set the number as the last number.
			if( previousIndex == -1 )
				previousIndex = first->size() - 1;
		}
		if( split )
		{
			privateBuildSequencesAccordingToPadding( result, defaultSeq, stringParts, first, it, previousIndex );
			split = false;
			index = -1;
			first = it;
		}
		previousIndex = index;
	}
	if( previousIndex == -1 )
		previousIndex = first->size() - 1;
	privateBuildSequencesAccordingToPadding( result, defaultSeq, stringParts, first, it, previousIndex );
	
	return result;
}

std::size_t decomposeFilename( const std::string& filename, FileStrings& stringParts, FileNumbers& numberParts, const EDetection& options )
{
	static const std::size_t max = std::numeric_limits<Time>::digits10;
	std::string regexStr;
	if( options & eDetectionNegative )
	{
		regexStr = "[\\+\\-]?\\d{1," + std::to_string( max ) + "}";
	}
	else
	{
		regexStr = "\\d{1," + std::to_string( max ) + "}";
	}
	const std::regex re( regexStr );
	// Iterate over matches and non-matches to interleave string/number parts
	auto it  = std::sregex_iterator( filename.begin(), filename.end(), re );
	auto end = std::sregex_iterator();

	std::size_t lastEnd = 0;
	for( ; it != end; ++it )
	{
		const std::smatch& m = *it;
		// The string part before this number match
		stringParts.getId().push_back( filename.substr( lastEnd, m.position() - lastEnd ) );
		// The number part
		numberParts.push_back( m.str() );
		lastEnd = m.position() + m.length();
	}
	// Trailing string part (may be empty)
	stringParts.getId().push_back( filename.substr( lastEnd ) );

	// Remove the trailing empty string only if we ended on a number (original code behaviour)
	// The original code pushed a trailing "" when stringParts.size() == numberParts.size(),
	// but with our approach we always push a trailing string (possibly ""), so the invariant
	// stringParts.size() == numberParts.size() + 1 is always satisfied here.
	// Nothing to do.

	return numberParts.size();
}

}
