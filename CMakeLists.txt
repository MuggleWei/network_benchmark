cmake_minimum_required (VERSION 3.16.3)
project(network_benchmark)

################################
# general config
################################

# print compiler
message("-- use c compiler ${CMAKE_C_COMPILER}")
message("-- use c++ compiler ${CMAKE_CXX_COMPILER}")

# set compile parameter
if (NOT CMAKE_CROSSCOMPILING)
	if (${CMAKE_C_COMPILER_ID} STREQUAL GNU)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
	elseif (${CMAKE_C_COMPILER_ID} MATCHES Clang)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
	elseif (${CMAKE_C_COMPILER_ID} STREQUAL MSVC)
		add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
		add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
	endif()
endif()

# set standard and print features
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

message("-- c compiler support features: ")
foreach(feature ${CMAKE_C_COMPILE_FEATURES})
	message("support feature: ${feature}")
endforeach()

# set output directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# for LSP
if (NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# set use folder in vs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################
# download deps
################################

include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

message("# fetch mugglec")
FetchContent_Declare(
	mugglec
	GIT_REPOSITORY https://github.com/MuggleWei/mugglec.git
	GIT_TAG v1.8.1
	GIT_SHALLOW TRUE
	)
FetchContent_MakeAvailable(mugglec)

message("# fetch haclog")
FetchContent_Declare(
	haclog
	GIT_REPOSITORY https://github.com/MuggleWei/haclog.git
	GIT_TAG v0.2.0
	GIT_SHALLOW TRUE
	)
FetchContent_MakeAvailable(haclog)

################################
# install config
################################
include(GNUInstallDirs)
set(CMAKE_INSTALL_LIBDIR lib)

################################
# library
################################

# option
if (BUILD_SHARED_LIBS)
	set(NET_BENCH_USE_DLL ON)
else()
	set(NET_BENCH_USE_DLL OFF)
endif()
option(BUILD_SANITIZER "Build sanitizer" OFF)

message("--------------------------------")
message("# CMAKE_C_STANDARD: ${CMAKE_C_STANDARD}")
message("# c compiler: ${CMAKE_C_COMPILER}")
message("# c compiler flags: ${CMAKE_C_FLAGS}")
message("# CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
message("# CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message("# BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}")
message("# NET_BENCH_USE_DLL: ${NET_BENCH_USE_DLL}")
message("# BUILD_SANITIZER: ${BUILD_SANITIZER}")
message("--------------------------------")

# configure
set(src_dir ${CMAKE_CURRENT_LIST_DIR}/src)
set(gen_dir ${CMAKE_BINARY_DIR}/gen)
configure_file(
	"${src_dir}/network_benchmark/config.h.in"
	"${gen_dir}/network_benchmark/config.h")

# lib
file(GLOB src_h ${src_dir}/network_benchmark/*.h)
file(GLOB src_c ${src_dir}/network_benchmark/*.c)
add_library(netbench ${src_h} ${src_c})
target_include_directories(netbench PUBLIC ${src_dir} ${gen_dir})
target_link_libraries(netbench PUBLIC mugglec haclog)
add_dependencies(netbench mugglec haclog)
target_compile_definitions(netbench PRIVATE NET_BENCH_EXPORTS)
set_target_properties(netbench PROPERTIES
	LINKER_LANGUAGE C
	DEBUG_POSTFIX d)

# install
if (APPLE)
	set_target_properties(netbench
		PROPERTIES
		INSTALL_RPATH "@executable_path/../lib")
else ()
	set_target_properties(netbench
		PROPERTIES
		INSTALL_RPATH "\$ORIGIN/../lib")
endif()

install(TARGETS netbench
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
	ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")

################################
# apps
################################

function(add_app name folder)
	file(GLOB tmp_h ${folder}/*.h)
	file(GLOB tmp_c ${folder}/*.c)

	if (WIN32)
		add_executable(${name} ${tmp_h} ${tmp_c})
		set_target_properties(${name}
			PROPERTIES
			FOLDER "app"
			VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
		)
	else()
		add_executable(${name} ${tmp_c})
	endif(WIN32)

	add_dependencies(${name} netbench)
	target_link_libraries(${name} PUBLIC netbench)

	if (BUILD_SANITIZER)
		if ((${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU") OR
			(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
			target_compile_options(${name} PUBLIC
				-fsanitize=undefined
				-fsanitize=address
				-fsanitize=leak)
			target_link_options(${name} PUBLIC
				-fsanitize=undefined
				-fsanitize=address
				-fsanitize=leak)
		endif()
	endif()

	install(TARGETS ${name}
		RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
		ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
		LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
	if (APPLE)
		set_target_properties(${name}
			PROPERTIES
			INSTALL_RPATH "@executable_path/../lib"
		)
	else ()
		set_target_properties(${name}
			PROPERTIES
			INSTALL_RPATH "\$ORIGIN/../lib"
		)
	endif()
endfunction()

set(app_root_dir ${CMAKE_CURRENT_LIST_DIR}/app)
FILE(GLOB subdirs RELATIVE ${app_root_dir} ${app_root_dir}/*)
FOREACH(subdir ${subdirs})
	IF(IS_DIRECTORY ${app_root_dir}/${subdir})
		add_app(${subdir} ${app_root_dir}/${subdir})
	ENDIF()
ENDFOREACH()
