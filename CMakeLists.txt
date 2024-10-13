# CMakeList.txt : CMake project for AutoDifferentiation, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.15...3.30)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("AutoDifferentiation" LANGUAGES CXX)

# Add source to this project's executable.
file(GLOB SOURCES "src/*.cpp")
add_executable (AutoDifferentiation ${SOURCES})

set_target_properties(AutoDifferentiation PROPERTIES
	CXX_STANDARD 23
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS NO)


# TODO: Add tests and install targets if needed.
