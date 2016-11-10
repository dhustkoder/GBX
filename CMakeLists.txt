cmake_minimum_required(VERSION 3.1)
project(gbx)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Wshadow \
	-Wstrict-aliasing -fno-exceptions -fno-rtti")

set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -Werror -fstrict-aliasing \
	-march=native -fomit-frame-pointer -ffunction-sections \
	-fdata-sections -fno-unwind-tables -fno-asynchronous-unwind-tables")

set(CMAKE_CXX_FLAGS_DEBUG "-D_DEBUG -DDEBUG -O1 -g3")

# build options
set(TARGET_PLATFORM "Linux" CACHE STRING "Specifies your target platform for build config")
option(ASAN OFF)
option(USAN OFF)
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

# gbx src directory
set(GBX_SRC_DIR "./src")
# include src directory
include_directories("${GBX_SRC_DIR}")

# specific platform builds
if (${TARGET_PLATFORM} STREQUAL "Linux")
	set(GBX_PLATFORM_SRC_FILES "${GBX_SRC_DIR}/SDL2/*.cpp")
	set(GBX_LINK_LIBRARIES "-lc -lSDL2")
else()
	message(FATAL_ERROR "Add your platform build configuration")
endif()


add_executable(${PROJECT_NAME} "${GBX_SRC_DIR}/*.cpp" ${GBX_PLATFORM_SRC_FILES})
target_link_libraries(${PROJECT_NAME} ${GBX_LINK_LIBRARIES})

if (ASM_OUTPUT)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -S")
	set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAG "-save-temps")
endif()



