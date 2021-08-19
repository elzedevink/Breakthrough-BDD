include_directories(.)

include(CheckIncludeFiles)
check_include_files("gperftools/profiler.h" HAVE_PROFILER)

add_executable(breakthrough breakthrough.cpp)
target_link_libraries(breakthrough sylvan m pthread)
set_property(TARGET breakthrough PROPERTY C_STANDARD 11)
