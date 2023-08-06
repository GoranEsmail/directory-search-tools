#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>
#include <mutex>
#include <tuple>
#include <vector>
#include <string>
#include <new>
#include <cctype>
#include <iostream>
#include <sstream>

// typedefs:
using ui32		= unsigned int;
using ui64		= unsigned long long;
using uint		= ui32;

static_assert( sizeof( ui32 ) == 4, "invalid byte size for type." );
static_assert( sizeof( ui64 ) == 8, "invalid byte size for type." );

// global constants
const ui64	ALIGNMENT				= std::size_t{ 64 };
const ui64	SEARCH_STR_MAX_LEN		= 4096;

struct program_args
{
	std::mutex cout_lock;

	bool case_sensitive			= false;
	ui64 search_parameter_len	= 0;
	alignas( ALIGNMENT ) char search_parameter[ SEARCH_STR_MAX_LEN ];
};

std::vector< std::string > default_extensions
{
	".txt",		".csv",		".json",	".yaml",	".xml",
	".h",		".hh",		".hpp",		".hxx",
	".c",		".cc",		".cpp",		".cxx",
	".cs",		".py",
};

bool filter_valid_path_name( std::string& file_path, const std::vector< std::string >& extensions = default_extensions );
void search_file_content( program_args& args, const std::string& file_path, char* buffer, const ui64 buffer_size );

// string helper functions:

inline const char* find_char( char c, const char* begin, const char* end )
{
	return reinterpret_cast< const char* >( memchr( begin, c, end - begin ) );
}

const char* find_str( const char* str, const ui64 str_len, const char* begin, const char* end )
{
	const ui64 range_len = ( end - begin );
	if ( str_len > range_len ) { return nullptr; }
	ui64 search_len	= range_len - ( str_len - 1 );
	ui64 offset = 0;
	while ( offset < search_len ) { // TODO: try match with item at the end of a file
		const char* str_match = find_char( str[ 0 ], begin + offset, begin + search_len );
		if ( str_match ) {
			if ( memcmp( str_match, str, str_len ) ) {
				offset = ( str_match - begin ) + 1;
			} else {
				return str_match;
			}
		} else {
			return nullptr;
		}
	}
	return nullptr;
}
