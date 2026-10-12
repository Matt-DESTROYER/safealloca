cmake_minimum_required(VERSION 3.10)

project(safealloca C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

file(GLOB_RECURSE SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	set(IS_ROOT_PROJECT TRUE)
else()
	set(IS_ROOT_PROJECT FALSE)
endif()

if(IS_ROOT_PROJECT)
	file(GLOB installed_packages LIST_DIRECTORIES true
		"${CMAKE_CURRENT_SOURCE_DIR}/.libraries/*"
	)

	set(PACKAGE_TARGETS "")

	foreach(package_dir ${installed_packages})
		if(IS_DIRECTORY "${package_dir}")
			add_subdirectory("${package_dir}")

			get_filename_component(target_name "${package_dir}" NAME)
			list(APPEND PACKAGE_TARGETS ${target_name})
		endif()
	endforeach()

	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c")
		add_executable(${PROJECT_NAME} ${SRC})
	else()
		add_library(${PROJECT_NAME} STATIC ${SRC})
	endif()

	set(CUTE_TARGET ${PROJECT_NAME})

	if(PACKAGE_TARGETS)
		target_link_libraries(${CUTE_TARGET} PRIVATE ${PACKAGE_TARGETS})
	endif()
else()
	get_filename_component(CUTE_TARGET "${CMAKE_CURRENT_SOURCE_DIR}" NAME)

	add_library(${CUTE_TARGET} STATIC ${SRC})

	target_include_directories(${CUTE_TARGET} PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/src"
	)
endif()

if(MSVC)
	target_compile_options(${CUTE_TARGET} PRIVATE
		/W4
		/WX
		$<$<CONFIG:Release>:/O2>
		$<$<CONFIG:Debug>:/ZI;/Od>
	)
else()
	target_compile_options(${CUTE_TARGET} PRIVATE
		-Wall
		-Werror
		$<$<CONFIG:Release>:-O2>
		$<$<CONFIG:Debug>:-g;-O0>
	)
endif()

if(IS_ROOT_PROJECT)
	INSTALL(TARGETS ${PROJECT_NAME}
		RUNTIME DESTINATION bin
	)
endif()
