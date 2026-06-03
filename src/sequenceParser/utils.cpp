#include "detector.hpp"
#include "Sequence.hpp"

#include "detail/analyze.hpp"
#include "detail/FileNumbers.hpp"
#include "detail/FileStrings.hpp"

#include <regex>
#include <unordered_map>
#include <set>
#include <filesystem>


namespace sequenceParser {

using detail::FileNumbers;
using detail::FileStrings;
namespace fs = std::filesystem;

std::regex convertFilterToRegex( const std::string& filter, const EDetection detectOptions )
{
	std::string filterToRegex = filter;
	
	// match to pattern like: %04d
	std::cmatch match;
	std::regex expression( "(.*[%])([0-9]{2})([d].*)" );
	if( std::regex_match( filterToRegex.c_str(), match, expression ) )
	{
		std::string matched = match[1].second;
		matched.erase( 2 , matched.size()-2); // keep only numbers
		const int patternWidth = std::stoi( matched );
		std::string replacing( patternWidth, '#' );
		filterToRegex = std::regex_replace( filterToRegex, std::regex( "\\%\\d{1,2}d" ), replacing );
	}

	// for detect sequence based on a single file
	if( ( detectOptions & eDetectionSequenceFromFilename ) )
		filterToRegex = std::regex_replace( filterToRegex, std::regex( "\\d" ), "[0-9]" );

	filterToRegex = std::regex_replace( filterToRegex, std::regex( "\\*" ), "(.*)" );
	filterToRegex = std::regex_replace( filterToRegex, std::regex( "\\?" ), "(.)" );
	if( detectOptions & eDetectionNegative )
	{
		filterToRegex = std::regex_replace( filterToRegex, std::regex( "\\@" ), "[\\-\\+]?[0-9]+" ); // one @ correspond to one or more digits
		filterToRegex = std::regex_replace( filterToRegex, std::regex( "\\#" ), "[\\-\\+]?[0-9]" ); // each # in pattern correspond to a digit
	}
	else
	{
		filterToRegex = std::regex_replace( filterToRegex, std::regex( "\\@" ), "[0-9]+" ); // one @ correspond to one or more digits
		filterToRegex = std::regex_replace( filterToRegex, std::regex( "\\#" ), "[0-9]" ); // each # in pattern correspond to a digit
	}
	return std::regex( filterToRegex );
}

std::vector<std::regex> convertFilterToRegex( const std::vector<std::string>& filters, const EDetection detectOptions )
{
	std::vector<std::regex> res;
	for( const std::string& filter : filters )
	{
		res.push_back( convertFilterToRegex( filter, detectOptions ) );
	}
	return res;
}

bool filenameRespectsFilters( const std::string& filename, const std::vector<std::regex>& filters )
{
	// If there is no filter, it means that it respects filters...
	if( filters.size() == 0 )
		return true;

	for( const std::regex& filter : filters )
	{
		if( std::regex_match( filename, filter ) )
		{
			return true;
		}
	}
	return false;
}

bool filepathRespectsAllFilters( const fs::path& inputFilepath, const std::vector<std::regex>& filters, const std::string& filterFilename, const EDetection detectOptions )
{
	const std::string inputFilename = inputFilepath.filename().string();
	if( inputFilename.empty() )
		return false; // no sense...

	// hidden files
	if( ( detectOptions & eDetectionIgnoreDotFile ) && ( inputFilename[0] == '.' ) )
		return false;

	// filtering of entries with filters strings
	if( ! filenameRespectsFilters( inputFilename, filters ) )
		return false;

	if( filterFilename.empty() )
		return true;

	return filterFilename == inputFilepath.string();
}

}
