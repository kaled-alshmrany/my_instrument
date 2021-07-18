#cmake -LAH
#sudo apt  install cmake-curses-gui
#ccmake .
#cmake -S . -B ./build -DClang_DIR=/home/kaled/sdb1/FuSeBMC/clang11  -DLLVM_DIR=/home/kaled/sdb1/FuSeBMC/clang11

cmake_minimum_required(VERSION 3.0)
project (FuSeBMC_instrument)
message("Welcome to ${PROJECT_NAME}....")

# Used by try_compile
set (CMAKE_POSITION_INDEPENDENT_CODE ON)
set (DEFAULT_BUILD_TYPE "Release")
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g -o0 -DMYDEBUG")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -os")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti  -Wno-unsequenced -Wno-unused-parameter ") # NoTE -DMYDEBUG
#set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
#set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
#set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libstdc++ -static-libgcc -lm -l:libm.a")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libstdc++ -static-libgcc -lm -l:libm.a")

set (Clang_DIR "${LLVM_DIR}" CACHE STRING "Clang Directory (if not set, this will be set to the LLVM_DIR")






if(NOT (("${LLVM_DIR}" STREQUAL "LLVM_DIR-NOTFOUND") OR ("${LLVM_DIR}" STREQUAL "")))

	# Note: LLVM_DIR will be changed after 'find_package'
	if(EXISTS ${LLVM_DIR}/bin/clang++)
		set(CMAKE_CXX_COMPILER ${LLVM_DIR}/bin/clang++)
	else()
		message("Compiler: '${LLVM_DIR}/bin/clang++' not found.")
	endif()

	find_package(LLVM REQUIRED CONFIG
	  PATHS ${LLVM_DIR}
	  NO_DEFAULT_PATH
	)

	find_package(Clang REQUIRED CONFIG
	  PATHS ${Clang_DIR}
	  NO_DEFAULT_PATH
	)

	
else()
  find_package(LLVM REQUIRED CONFIG)
  find_package(Clang REQUIRED CONFIG)
endif()

if (${LLVM_VERSION_MAJOR} EQUAL 11)
  message(STATUS "LLVM version: ${LLVM_VERSION}")
else()
  message(FATAL_ERROR "Could not find LLVM/Clang 11.0 at all: please specify with -DLLVM_DIR")
endif()
# Note: it is not tested with Systems LLVM




message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "LLVM_INCLUDE_DIRS=${LLVM_INCLUDE_DIRS}")
#message(STATUS "LLVM_DEFINITIONS=${LLVM_DEFINITIONS}")
#message(STATUS "LLVM_AVAILABLE_LIBS:${LLVM_AVAILABLE_LIBS}")

#include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${LLVM_INCLUDE_DIRS})

#add_subdirectory(${CMAKE_SOURCE_DIR}/src)
file(GLOB_RECURSE fusebmc_SRC "${CMAKE_SOURCE_DIR}/src/*.cpp" )



#add_definitions(${LLVM_DEFINITIONS})

add_executable(${PROJECT_NAME} ${fusebmc_SRC})

target_include_directories(${PROJECT_NAME} 
    PRIVATE ${CMAKE_SOURCE_DIR}/include
    PRIVATE ${LLVM_INCLUDE_DIRS}
)



add_custom_command(TARGET ${PROJECT_NAME}
					PRE_BUILD 
					COMMAND ${CMAKE_COMMAND} -E remove -f "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}")

add_custom_command(TARGET ${PROJECT_NAME}
					POST_BUILD 
					COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_SOURCE_DIR})




#message(${fusebmc_SRC})
#add_subdirectory(src)
#llvm_map_components_to_libnames(llvm_libs support core irreader)
#message("llvm_libs: ${llvm_libs}")
# m pthread z
target_link_libraries(${PROJECT_NAME} ncurses  clangFrontend clangParse clangSema clangEdit clangAnalysis clangAST  clangLex clangBasic clangDriver  clangSerialization clangRewrite)
target_link_libraries(${PROJECT_NAME} LLVMCore LLVMRemarks LLVMOption LLVMProfileData LLVMMCParser LLVMMC LLVMBitReader LLVMBinaryFormat LLVMSupport LLVMBitstreamReader LLVMFrontendOpenMP LLVMDemangle)


install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME})
install(FILES ${CMAKE_SOURCE_DIR}/myincludes.txt DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/external_include DESTINATION ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME})
