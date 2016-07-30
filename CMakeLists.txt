cmake_minimum_required(VERSION 2.8.8)
project(gbx)

set(CMAKE_CXX_FLAGS "-std=c++11 -pedantic -pedantic-errors -Wall -Wextra -Werror -fno-exceptions -fno-rtti")

option(ADDRESS_SANITIZER OFF)
option(UNDEFINED_SANITIZER OFF)


if(NOT CMAKE_BUILD_TYPE)
	message(STATUS "No build type selected! default to Debug")
	set(CMAKE_BUILD_TYPE "Debug")
endif()



# "Release" full optimization , no debug info.
if(${CMAKE_BUILD_TYPE} STREQUAL "Release")

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG -O3 -flto -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-unwind-tables -fno-asynchronous-unwind-tables")

	set(LINK_FLAGS "-nodefaultlibs -flto -s -Wl,--gc-sections")
	set(CMAKE_EXE_LINKER_FLAGS "${LINK_FLAGS}")
	set(CMAKE_SHARED_LINKER_FLAGS "${LINK_FLAGS}")
	set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "${LINK_FLAGS}")
	set(CMAKE_STATIC_LIBRARY_LINK_CXX_FLAGS "${LINK_FLAGS}")



# "Debug" full debug information, no optimization, asserts enabled
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Debug")

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DEBUG -O0 -g3")


	if(ADDRESS_SANIZIER)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
	endif()

	if(UNDEFINED_SANITIZER)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
	endif()

endif()

#directories
set(GBX_SRC_DIR "./Source")

#include utix
include_directories("./Dependencies/Utix/Utix/include/")

#files
file(GLOB_RECURSE GBX_SRC "${GBX_SRC_DIR}/*.cpp")

# compile
add_executable(${PROJECT_NAME} ${GBX_SRC})

# link libraries
target_link_libraries(${PROJECT_NAME} -lc)
