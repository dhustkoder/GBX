cmake_minimum_required(VERSION 3.1)
project(gbx)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Wshadow \
	-Wstrict-aliasing -fno-exceptions -fno-rtti")

set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -Werror -fstrict-aliasing \
	-march=native -fomit-frame-pointer -ffunction-sections \
	-fdata-sections -fno-unwind-tables -fno-asynchronous-unwind-tables")

set(CMAKE_CXX_FLAGS_DEBUG "-D_DEBUG -DDEBUG -O1 -g3")

option(ASAN OFF)
option(USAN OFF)
option(MSAN OFF)

option(ENABLE_LTO OFF)
option(ASM_OUTPUT OFF)

if(NOT CMAKE_BUILD_TYPE)
	message(STATUS "No build type selected, defaulted to Release")
	set(CMAKE_BUILD_TYPE "Release")
endif()


# "Release" full optimization , no debug info.
if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
	set(LINK_FLAGS "-nodefaultlibs -s -Wl,--gc-sections")
	if(ENABLE_LTO)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
		set(LINK_FLAGS "${LINK_FLAGS} -flto") 
	endif()

	set(CMAKE_EXE_LINKER_FLAGS "${LINK_FLAGS}")
	set(CMAKE_SHARED_LINKER_FLAGS "${LINK_FLAGS}")
	set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "${LINK_FLAGS}")
	set(CMAKE_STATIC_LIBRARY_LINK_CXX_FLAGS "${LINK_FLAGS}")


# "Debug" full debug information, no optimization, asserts enabled
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	if (ASAN)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
	endif()

	if (USAN)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
	endif()
endif()

#directories
set(GBX_SRC_DIR "./src")

#include utix
include_directories("./Dependencies/Utix/Utix/include/")
#include project
include_directories("${GBX_SRC_DIR}")

#files
file(GLOB GBX_SRC "${GBX_SRC_DIR}/*.cpp" "${GBX_SRC_DIR}/SDL2/*.cpp")

# compile
add_executable(${PROJECT_NAME} ${GBX_SRC})

# link libraries
target_link_libraries(${PROJECT_NAME} -lc -lSDL2)


if (ASM_OUTPUT)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -S")
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAG "-save-temps")
endif()



