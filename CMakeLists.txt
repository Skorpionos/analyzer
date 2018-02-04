message(STATUS "start running cmake...")

cmake_minimum_required(VERSION 3.9)
project(analyzer)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
        main.cpp
        dump.h
        dump.cpp
        file_operations.h
        file_operations.cpp
        utilities.h
        utilities.cpp
        read_options.cpp
        read_options.h
        finder.h
        finder.cpp
        Types.h
        /usr/include/boost/program_options/options_description.hpp)
        add_executable(analyzer ${SOURCE_FILES})


find_package(Boost COMPONENTS #[[system filesystem]] program_options REQUIRED)

if(Boost_FOUND)
#    message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
#    message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
#    message(STATUS "Boost_VERSION: ${Boost_VERSION}")
#    message(STATUS "Boost_PROGRAM_OPTIONS: ${Boost_PROGRAM_OPTIONS}")
    include_directories(${Boost_INCLUDE_DIRS})

    target_link_libraries(analyzer ${Boost_LIBRARIES})

endif()