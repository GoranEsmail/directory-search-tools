# directory-search-tools

This repository contains a script for searching for text strings inside files.
There's a python version that is more fun to read and a C++ version that offers better performance.
Feel free to use it if you want to but there are much better search tools available like grep or ripgrep.

To run the python script I use:
`python3 search_dir /path/to/some/place "whatever I need to search for"`

I've currently only tested the C++ version in macos where I compile it with the folliwng command:
`clang++ dir_search.cpp -std=c++20 -m64 -Wall -Wextra -Wpedantic -Werror -fno-exceptions -O2 -o dir_search`
and then I run it with:
`./dir_search . "whatever I need to search for"`

The idea with the C++ code is to first use memchr to look for the first character in the string I search for and then memcmp to confirm that it is a match while the python file just opens a file and splits its lines before searching.
I find that the python version is more fun to customise but the main reason I had for writing the C++ version is that I found it easier to reason in bytes rather than characters for the type of searches I wanted to do.
