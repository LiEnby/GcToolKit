cmake_minimum_required(VERSION 3.10)
project(gc_backup_network)

IF(CMAKE_SYSTEM_NAME MATCHES "^MINGW")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc -Wl,-Bstatic -O3")
ELSE()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
ENDIF()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions")

link_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(${PROJECT_NAME}
	main.c
)

IF(CMAKE_SYSTEM_NAME MATCHES "^MINGW")
target_link_libraries(${PROJECT_NAME}
	pthread
	ws2_32
)
ELSE()
target_link_libraries(${PROJECT_NAME}
	pthread
)
ENDIF()
