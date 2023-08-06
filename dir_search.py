#encoding: utf-8

# The following code can be changed to change the behavior of your search:

search_path = "."
search_str = ""

exts = [ ".txt", ".csv", ".json", ".yaml", ".xml", ".h", ".hh",	 ".hpp", ".hxx", ".c", ".cc", ".cpp", ".cxx", ".cs", ".py" ]
def path_filter( file_path ):
	global exts
	lower_path = file_path.lower();
	for ext in exts:
		if lower_path.endswith( ext ):
			return True
	return False

# return -1 to filter out a line, otherwise return pos
def line_filter( line, search_param ):
	return str( line ).find( search_param )

""" IMPLEMENTATION DETAILS ARE FOUND BELOW THIS LINE """
import time, os, sys, multiprocessing

def search_in_directory( start_path ):
	all_files = []
	for root, dirs, files in os.walk( start_path ):
		for f in files:
			path_candidate = os.path.join( root, f )
			if path_filter( path_candidate ):
				all_files += [ path_candidate ]
	return all_files

class LineData:
	def __init__( self, line_number, column, text ):
		self.num = line_number
		self.col = column
		self.text = text

def search_in_file( file_path, search_str ):
	line_counter = 0
	with open( file_path, "r" ) as f:
		try:
			lines = []
			for num, l in enumerate( f ):
				line = l.rstrip()
				col = line_filter( line, search_str )
				if col != -1:
					lines.append( LineData( num, col, line ) )
			return lines
		except:
			return []

class WrappedFunc:
	def __init__( self, func, *extra_params ):
		self.func = func
		self.extra_params = extra_params
		
	def __call__( self, param ):
		return self.func( param, *self.extra_params )

def dir_search( search_path, num_threads = multiprocessing.cpu_count() ):
	file_paths 		= search_in_directory( search_path )
	fn = WrappedFunc( search_in_file, search_str )
	lines_in_files 	= multiprocessing.Pool( num_threads ).map( fn, file_paths )
	for i in range( len( lines_in_files ) ):
		file_path = file_paths[ i ]
		for line in lines_in_files[ i ]:
			print( "{}:{}:{}:{}".format( file_path, line.num, line.col, line.text ) )

if __name__ == "__main__":
	begin_benchmarking = time.perf_counter()
	if len( sys.argv ) > 1:
		search_path = sys.argv[ 1 ]
	if len( sys.argv ) > 2:
		search_str = sys.argv[ 2 ]
	if len( search_str ):
		dir_search( search_path )
		benchmark_result = time.perf_counter() - begin_benchmarking
		print( "time passed: " + str( round( benchmark_result * 1000 ) ) + "ms." )
