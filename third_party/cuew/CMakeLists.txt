set(SRC
	src/cuew.c
)

set(SRC_HEADERS
	include/cuew.h
)

include_directories(
	.
	include
)

add_library(extern_cuew ${SRC} ${SRC_HEADERS})
install_external_library(extern_cuew)

if(UNIX)
	target_link_libraries(extern_cuew dl)
endif()
