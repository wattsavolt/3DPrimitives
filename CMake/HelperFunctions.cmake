function(add_common_properties target)
	get_target_property(target_type ${target} TYPE)

	if(MSVC)
		# Linker
		# The VS generator seems picky about how the linker flags are passed: we have to make sure
		# the options are quoted correctly and with append_string or random semicolons will be
		# inserted in the command line; and unrecognised options are only treated as warnings
		# and not errors so they won't be caught by CI. Make sure the options are separated by
		# spaces too.
		# For some reason this does not apply to the compiler options...

		set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS "/DYNAMICBASE /NOLOGO")

		set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS_DEBUG "/DEBUG")
		set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS_RELWITHDEBINFO "/DEBUG /LTCG:incremental /INCREMENTAL:NO /OPT:REF")
		set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS_MINSIZEREL "/DEBUG /LTCG /INCREMENTAL:NO /OPT:REF")
		set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS_RELEASE "/DEBUG /LTCG /INCREMENTAL:NO /OPT:REF")

		if(CLR_64BIT)
			set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS_RELWITHDEBINFO " /OPT:ICF")
			set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS_MINSIZEREL " /OPT:ICF")
			set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS_RELEASE " /OPT:ICF")
		endif()

		if (${target_type} STREQUAL "SHARED_LIBRARY" OR ${target_type} STREQUAL "MODULE_LIBRARY")
			set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " /DLL")
		endif()

		# Compiler
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS /GS- /W3 /WX- /MP /nologo /bigobj /wd4577 /wd4530)
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS -DWIN32 -D_WINDOWS)

		# Set character set of the project to unicode.
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS /UMBCS /D_UNICODE /DUNICODE)

		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:Debug>:/Od /RTC1 /MDd -DDEBUG>)

		if(CLR_64BIT) # Debug edit and continue for 64-bit
			set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:Debug>:/ZI>)
		else() # Normal debug for 32-bit
			set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:Debug>:/Zi>)
		endif()

		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:RelWithDebInfo>:/GL /Gy /Zi /O2 /Oi /MD -DDEBUG>)
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:MinSizeRel>:/GL /Gy /Zi /O2 /Oi /MD -DNDEBUG>)
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:Release>:/GL /Gy /Zi /O2 /Oi /MD -DNDEBUG>)

		# Global defines
		#add_definitions(-D_HAS_EXCEPTIONS=0)

	elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		# Note: Optionally add -ffunction-sections, -fdata-sections, but with linker option --gc-sections
		# TODO: Use link-time optimization -flto. Might require non-default linker.
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS -Wall -Wextra -Wno-unused-parameter -fpiC -fno-strict-aliasing -msse4.1)

		if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
			set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS -fno-ms-compatibility)

			if(APPLE)
				set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS -fobjc-arc $<$<COMPILE_LANGUAGE:CXX>:-std=c++1z>)
			endif()
		endif()

		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:Debug>:-ggdb -O0 -DDEBUG>)
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:RelWithDebInfo>:-ggdb -O2 -DDEBUG -Wno-unused-variable>)
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:MinSizeRel>:-ggdb -O2 -DNDEBUG -Wno-unused-variable>)
		set_property(TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS $<$<CONFIG:Release>:-ggdb -O2 -DNDEBUG -Wno-unused-variable>)

		if (${target_type} STREQUAL "EXECUTABLE")
			if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
				set_property(TARGET ${target} APPEND PROPERTY LINK_FLAGS_DEBUG -no-pie)
				set_property(TARGET ${target} APPEND PROPERTY LINK_FLAGS_RELWITHDEBINFO -no-pie)
				set_property(TARGET ${target} APPEND PROPERTY LINK_FLAGS_MINSIZEREL -no-pie)
				set_property(TARGET ${target} APPEND PROPERTY LINK_FLAGS_RELEASE -no-pie)
			endif()
		endif()
	else()
		# TODO_OTHER_COMPILERS_GO_HERE
	endif()

	if (${target_type} STREQUAL "SHARED_LIBRARY")
		set_property(TARGET ${target} PROPERTY VERSION ${CLR_FRAMEWORK_VERSION_MAJOR}.${CLR_FRAMEWORK_VERSION_MINOR}.${CLR_FRAMEWORK_VERSION_PATCH})
		set_property(TARGET ${target} PROPERTY SOVERSION ${CLR_FRAMEWORK_VERSION_MAJOR})
	endif()

	if(APPLE)
		set_property(TARGET ${target} PROPERTY INSTALL_RPATH "@loader_path;@loader_path/../lib;@loader_path/clr-${CLR_FRAMEWORK_VERSION_MAJOR}.${CLR_FRAMEWORK_VERSION_MINOR}.${CLR_FRAMEWORK_VERSION_PATCH}")
	else()
		if (${target_type} STREQUAL "EXECUTABLE")
			set_property(TARGET ${target} PROPERTY INSTALL_RPATH "\$ORIGIN/../lib:\$ORIGIN/../lib/clr-${CLR_FRAMEWORK_VERSION_MAJOR}.${CLR_FRAMEWORK_VERSION_MINOR}.${CLR_FRAMEWORK_VERSION_PATCH}")
		else()
			set_property(TARGET ${target} PROPERTY INSTALL_RPATH "\$ORIGIN:\$ORIGIN/clr-${CLR_FRAMEWORK_VERSION_MAJOR}.${CLR_FRAMEWORK_VERSION_MINOR}.${CLR_FRAMEWORK_VERSION_PATCH}")
		endif()
	endif()
endfunction()

# Returns all suitable files in the current and child directories.
function(add_source_groups srcsVarName srcsToExclude)
	file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h *.c *.cpp *.rc *.aps *.hlsl *.hlsli)
	
	foreach(f ${srcsToExclude})
		list(FILTER SRC_FILES EXCLUDE REGEX "^${f}")
	endforeach()

	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_FILES})

	set(${srcsVarName} ${SRC_FILES} PARENT_SCOPE)

	#message("srcs: ${SRC_FILES}")
endfunction()

# Sets the shader module for all the shader files in a project.
function(set_shader_config srcs)
	foreach(f ${srcs})   
		get_filename_component(EXTENSION ${f} LAST_EXT)
		if(${EXTENSION} MATCHES ".hlsl$")
			get_filename_component(FILE_NAME ${f} NAME_WE)
			set_property(SOURCE ${f} PROPERTY VS_SHADER_MODEL 5.0)
			# Default but maybe change later.
			set_property(SOURCE ${f} PROPERTY VS_SHADER_ENTRY_POINT main)
			if(${FILE_NAME} MATCHES "VS$")
				set_property(SOURCE ${f} PROPERTY VS_SHADER_TYPE Vertex)
			elseif(${FILE_NAME} MATCHES "PS$")
				set_property(SOURCE ${f} PROPERTY VS_SHADER_TYPE Pixel)
			elseif(${FILE_NAME} MATCHES "GS$")
				set_property(SOURCE ${f} PROPERTY VS_SHADER_TYPE Geometry)
			elseif(${FILE_NAME} MATCHES "HS$")
				set_property(SOURCE ${f} PROPERTY VS_SHADER_TYPE Hull)
			elseif(${FILE_NAME} MATCHES "DS$")
				set_property(SOURCE ${f} PROPERTY VS_SHADER_TYPE Domain)
			elseif(${FILE_NAME} MATCHES "CS$")
				set_property(SOURCE ${f} PROPERTY VS_SHADER_TYPE Compute)
			elseif(${FILE_NAME} MATCHES "MS$")
				set_property(SOURCE ${f} PROPERTY VS_SHADER_TYPE Mesh)
			elseif(${FILE_NAME} MATCHES "AS$")
				set_property(SOURCE ${f} PROPERTY VS_SHADER_TYPE Amplification)
			else()
				message(FATAL_ERROR "Shader file name is invalid!")
			endif()
		endif()
	endforeach()
endfunction()



