// multithreaded file search, compile with clang++ dir_search.cpp -std=c++20 -m64 -fno-exceptions -O2

#include "common.hpp"

// function that reads a file, manages memory and processes the file content
void process_file( program_args& args, std::filesystem::directory_entry dir_ent )
{
	std::string file_path = dir_ent.path().string();
	if ( !filter_valid_path_name( file_path ) ) { return; }

	std::error_code ec;
	const ui64 file_size = dir_ent.file_size( ec );
	if ( ec ) {
		std::lock_guard< std::mutex > lock( args.cout_lock );
		std::cout << "ERROR: could not get file size, message='" << ec.message() << "'." << std::endl;
		return;	
	}

	std::unique_ptr< char[] > file_buffer{ new ( std::align_val_t( ALIGNMENT ) ) char[ file_size ] };
	if ( !file_buffer.get() ) {
		std::lock_guard< std::mutex > lock( args.cout_lock );
		std::cout << "ERROR: memory alloc failed of size ("
			<< file_size << ") when reading file '" << file_path << "'." << std::endl;
		return;
	}

	std::ifstream ifs( file_path, std::ios::binary );
	if ( !ifs.is_open() ) {
		std::lock_guard< std::mutex > lock( args.cout_lock );
		std::cout << "ERROR: could not open '" << file_path << "'. as file." << std::endl;
		return;
	}

	if ( !ifs.read( file_buffer.get(), file_size ) ) {
		std::lock_guard< std::mutex > lock( args.cout_lock );
		std::cout << "ERROR: could not read data from file '" << file_size << "'." << std::endl;
		return;
	}
	ifs.close();
	search_file_content( args, file_path, file_buffer.get(), file_size );
}

int main( int argc, const char* argv[] )
{
	auto main_start_timestamp = std::chrono::high_resolution_clock::now();

	if ( argc < 3 ) {
		std::cout << "args: path search_string" << std::endl;
		exit( -1 );
	}
	program_args args;
	args.search_parameter_len = strnlen( argv[ 2 ], SEARCH_STR_MAX_LEN - 1 );
	memcpy( args.search_parameter, argv[ 2 ], args.search_parameter_len );
	args.search_parameter[ args.search_parameter_len ] = '\0';

	std::filesystem::recursive_directory_iterator 	dir_it{ std::filesystem::path{ argv[ 1 ] } };
	std::filesystem::recursive_directory_iterator	dir_it_pos{ std::filesystem::begin( dir_it ) };
	std::filesystem::recursive_directory_iterator	dir_it_end{ std::filesystem::end( dir_it ) };
	std::mutex 										dir_it_access;

	std::vector< std::thread > threads;
	for ( ui64 i = 0; i < std::thread::hardware_concurrency(); ++i ) {
		threads.emplace_back( std::thread{ [](
			program_args& args,
			std::mutex& dir_it_access,
			std::filesystem::recursive_directory_iterator& dir_it,
			const std::filesystem::recursive_directory_iterator& end )
		{
			dir_it_access.lock();
			while ( dir_it != end ) {
				std::filesystem::directory_entry dir_ent = *( dir_it++ );
				dir_it_access.unlock();
				process_file( args, dir_ent );
				dir_it_access.lock();
			}
			dir_it_access.unlock();
		}, std::ref( args ), std::ref( dir_it_access ), std::ref( dir_it_pos ), std::cref( dir_it_end ) } );
	}

	for ( std::thread& thread : threads ) { thread.join(); }

	auto duration = std::chrono::duration_cast< std::chrono::milliseconds >
		( std::chrono::high_resolution_clock::now() - main_start_timestamp ).count();
 	std::cout << "time passed: " << duration << "ms" << std::endl;

	return 0;
}

#include "user_functions.cpp"
