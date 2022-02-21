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

add_source_groups(SRCS ${EXCLUDED_SRCS})

# Target
if(WIN32)
	add_executable(App WIN32 ${SRCS})
else()
	add_executable(App ${SRCS})
endif()

add_common_properties(App)

# Libraries
target_link_libraries(App PRIVATE Renderer)

# IDE specific
set_property(TARGET App PROPERTY FOLDER 3DPrimitives)


