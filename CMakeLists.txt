# Works with 3.10.2
cmake_minimum_required(VERSION 3.10.2)

# Project name and a few useful settings.
# Other commands can pick up the results
project (AstromakerMount
        VERSION 1.0
        DESCRIPTION "Astromaker mount software."
        LANGUAGES CXX)

# add libraries source code directory
add_subdirectory(libs)
# add main source code directory
add_subdirectory(src)

add_compile_options( -pthread -Wall  -Wextra -pedantic -Werror )
