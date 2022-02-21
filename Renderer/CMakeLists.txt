set(EXCLUDED_SRCS 
	"Win32"
	"Unix"
	"Linux"
	"MacOS")

if(WIN32)
	list(REMOVE_ITEM EXCLUDED_SRCS "Win32")
endif()

if(UNIX)
	list(REMOVE_ITEM EXCLUDED_SRCS "Unix")

	if(LINUX)
		list(REMOVE_ITEM EXCLUDED_SRCS "Linux")
	elseif(APPLE)
		list(REMOVE_ITEM EXCLUDED_SRCS "MacOS")
	endif()
endif()

# List arguments need to be quoted. Otherwise the first entry in the list will be passed only.'
add_source_groups(SRCS "${EXCLUDED_SRCS}")
set_shader_config("${SRCS}")

# Target
add_library(Renderer STATIC ${SRCS})

add_common_properties(Renderer)

# Includes
target_include_directories(Renderer PUBLIC ${3DP_ROOT_DIR}/Renderer)

# IDE specific
set_property(TARGET Renderer PROPERTY FOLDER 3DPrimitives)



