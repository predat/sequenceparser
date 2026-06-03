%include "common.i"

%include <std_vector.i>
%include <std_except.i>
%include <exception.i>

%{
#include "sequenceParser/Item.hpp"
#include "sequenceParser/filesystem.hpp"
%}

%template(ItemVector) ::std::vector<sequenceParser::Item>;

%include "filesystem.hpp"

namespace sequenceParser {
%ignore browse(
		const std::filesystem::path&,
		const EDetection detectOptions,
		const std::vector<std::string>& );
}

