#include "Item.hpp"

#include <cassert>
#include <filesystem>
#include <ostream>


namespace fs = std::filesystem;

namespace sequenceParser {


std::string Item::getAbsoluteFirstFilename() const
{
	if( getType() == eTypeSequence )
		return (_path.parent_path() / getSequence().getFirstFilename()).string();
	return getAbsoluteFilepath();
}


std::vector<Item> Item::explode() const
{
	std::vector<Item> outItems;

	if(_type != eTypeSequence){
		outItems.push_back(*this);
		return outItems;
	}

	const std::vector<fs::path>& seqFilesPath = getSequence().getAbsoluteFilesPath(_path.parent_path());
	for( const fs::path& filePath : seqFilesPath )
		outItems.push_back(Item(getTypeFromPath(filePath), filePath));
	return outItems;
}

std::string Item::getFirstFilename() const
{
	if( getType() == eTypeSequence )
		return getSequence().getFirstFilename();
	return getFilename();
}

EType getTypeFromPath( const fs::path& path )
{
	if( fs::is_symlink( path ) )
	{
		return eTypeLink;
	}
	if( fs::is_regular_file( path ) )
	{
		return eTypeFile;
	}
	if( fs::is_directory( path ) )
	{
		return eTypeFolder;
	}
	return eTypeUndefined;
}


EType getTypeFromPath( const std::string& pathStr )
{
	const fs::path path( pathStr );
	return getTypeFromPath(path);
}


std::ostream& operator<<( std::ostream& os, const Item& item )
{
	os << (item.getType() == eTypeSequence ? (item.getFolderPath() / item.getSequence().string()) : item.getPath());
	return os;
}


}
