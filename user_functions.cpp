#include "common.hpp"

std::tuple< ui64, const char* > count_new_lines( const char* begin, const char* end )
{
	ui64		new_line_counter	= 0;
	ui64		offset				= 0;
	const ui64 	len					= end - begin;

	while ( offset < len ) {
		const char* new_line_pos = find_char( '\n', begin + offset, end );
		if ( new_line_pos ) {
			offset = ( new_line_pos - begin ) + 1;
			++new_line_counter;
		} else {
			break;
		}
	}
	return std::tuple< ui64, const char* >{ new_line_counter, begin + offset };	
}

void print_matches( program_args& args, const std::string& file_path, const char* buffer, const ui64 buffer_size, const std::vector< ui64 >& match_positions )
{
	if ( match_positions.empty() ) { return; }

	ui64 new_line_counter	= 0;
	ui64 last_offset		= 0;

	std::stringstream ss;
	
	for ( ui64 i = 0; i < match_positions.size(); ++i ) {
		auto [ new_lines, line_begin ] = count_new_lines( buffer + last_offset,  buffer + match_positions[ i ] );
		new_line_counter += new_lines;
		const char* line_end = find_char( '\n', line_begin, buffer + buffer_size );
		if ( !line_end ) { line_end = buffer + buffer_size; }

		ss << file_path << ':' << new_line_counter << ':'
			<< ( ( buffer + match_positions[ i ] ) - line_begin ) << ':'
			<< std::string( line_begin, line_end ) << '\n';

		last_offset = ( line_begin - buffer ) + 1;
	}

	std::lock_guard< std::mutex > lock( args.cout_lock );
	std::cout << ss.str();
}

void search_file_content( program_args& args, const std::string& file_path, char* buffer, const ui64 buffer_size )
{
	if ( args.search_parameter_len > buffer_size ) { return; }
	if ( args.case_sensitive ) {
		for ( ui64 i = 0; i < buffer_size; ++i ) {
			buffer[ i ] = std::tolower( buffer[ i ] );
		}
	}

	std::vector< ui64 >	match_positions;
	ui64 offset = 0;
	while ( offset < buffer_size ) {
		const char* match = find_str( args.search_parameter, args.search_parameter_len, buffer + offset, buffer + buffer_size );
		if ( match ) {
			const ui64 match_offset = match - buffer;
			match_positions.push_back( match_offset );
			offset = match_offset + 1;
		} else {
			break;
		}
	}

	print_matches( args, file_path, buffer, buffer_size, match_positions );
}

bool filter_valid_path_name( std::string& file_path, const std::vector< std::string >& extensions )
{
	for ( const std::string& ext : extensions ) {
		const ui64 path_size 	= file_path.size();
		const ui64 ext_size		= ext.size();
		if ( path_size >= ext_size ) {
			const ui64 offset = path_size - ext_size;
			if ( std::equal( ext.begin(), ext.end(), file_path.begin() + offset ) ) {
				return true;
			}
		}
	}
	return false;
}
