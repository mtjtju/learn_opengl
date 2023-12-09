#---------------1.overall setting-----------------

#        -------a.version setting--------
cmake_minimum_required(VERSION 3.13)
project(ACCAT_opengl LANGUAGES C CXX)

#        -------b.language setting--------
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

#        -------c.compile setting--------
# wall: generate all warning
# wextra: print extra warning info 
add_compile_options(-Wall)

# pipe ?
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

#        -------d.compile type-------
# set(CMAKE_BUILD_TYPE Debug)
# set in cmd line: -DCMAKE_BUILD_TYPE=Debug(would be ingored by vs)

#        -------e.add global def-------
add_definitions(-GLOBAL_VAR)

#       -------f.add include dirs-------
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)



#---------------2.compile target files-----------------
#       -------a.compile exe-------
set(TARGET_NAME ACCAT_opengl)
add_executable(${TARGET_NAME})
file(GLOB_RECURSE H_FILES CONFIGURE_DEPENDS *.h)
aux_source_directory(source SRC_FILES)
message("${SRC_FILES}")
target_sources(${TARGET_NAME} PUBLIC ${SRC_FILES} ${H_FILES})
set_property(TARGET ${TARGET_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR})

#       -------b.link libs-------
find_library (GLFW3 glfw3.lib ${CMAKE_CURRENT_SOURCE_DIR}/libs)
target_link_libraries(${TARGET_NAME} ${GLFW3})

#---------------3.add custom command-----------------
set(SHADERS_DIR )
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD	
	COMMAND
	SET dstDir=$<TARGET_FILE_DIR:${TARGET_NAME}>\n
	# SET postfix=\n
	# IF /I $(Configuration)== Debug ( \n
		# SET postfix=d\n
	# ) \n
	SETLOCAL ENABLEDELAYEDEXPANSION \n
	echo copy shader
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shader %dstDir%/shader\n
)