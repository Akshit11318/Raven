cmake_minimum_required(VERSION 3.22)

project(Raven)

# CMAKE functions----------------------------------------------------------------
function(get_all_files_in_directory directory result)
    file(GLOB_RECURSE files "${directory}/*")
    set(${result} ${files} PARENT_SCOPE)
endfunction()
# -------------------------------------------------------------------------------


# General Settings--------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
# Set compile options as private to not warn about msquic header files
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -fsanitize=undefined")
  set(QUIC_ENABLE_LOGGING "ON" CACHE INTERNAL "QUIC_ENABLE_LOGGING")
  set(QUIC_LOGGING_TYPE "stdout" CACHE INTERNAL "stdout")
endif()

# -------------------------------------------------------------------------------

# Setting MSQUIC related variables
set(QUIC_STATIC_LINK_CRT "ON" CACHE INTERNAL "QUIC_STATIC_LINK_CRT")
add_subdirectory(submodules/msquic)
set(MSQUIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/submodules/msquic/src/inc)
set(MSQUIC_LINK_LIBRARY ${CMAKE_BINARY_DIR}/submodules/msquic/bin/${CMAKE_BUILD_TYPE}/libmsquic.so)


# Setting Raven variables
set(RAVEN_SRC_DIR "raven/src")
set(RAVEN_INCLUDE_DIR "raven/includes")

set(MOODY_CAMEL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/submodules/moody_camel")

add_definitions(-DRAVEN_CERT_FILE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/server.cert")
add_definitions(-DRAVEN_KEY_FILE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/server.key")
add_definitions(-DDUMMY_PAYLOAD_FILE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/dummy_payload.txt")
add_definitions(-DDATA_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/data/")


set(RAVEN_SRC "")
get_all_files_in_directory(${RAVEN_SRC_DIR} RAVEN_SRC)

# Building Raven shared library linked with msquic -----------------------------
add_library(raven STATIC ${RAVEN_SRC})
# target_compile_options(raven PRIVATE -Wall -Wextra -Werror -Wpedantic)
# MSQUIC include directories should not leak to users of Raven
# They should be able to use their own msquic library includes
target_include_directories(raven PUBLIC ${RAVEN_INCLUDE_DIR})
target_include_directories(raven PUBLIC ${MSQUIC_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} ${MOODY_CAMEL_INCLUDE_DIR})
target_link_libraries(raven PUBLIC ${MSQUIC_LINK_LIBRARY} ${Boost_LIBRARIES})
# -------------------------------------------------------------------------------

# Add playground server
add_executable(server server.cpp)
# target_compile_options(server PRIVATE -Wall -Wextra -Werror -Wpedantic)
target_include_directories(server PUBLIC ${RAVEN_INCLUDE_DIR})
target_include_directories(server PUBLIC ${MSQUIC_INCLUDE_DIR} ${MOODY_CAMEL_INCLUDE_DIR})
target_link_libraries(server PUBLIC raven)

# Add playground client
add_executable(client client.cpp)
# target_compile_options(client PRIVATE -Wall -Wextra -Werror -Wpedantic)
target_include_directories(client PUBLIC ${RAVEN_INCLUDE_DIR})
target_include_directories(client PUBLIC ${MSQUIC_INCLUDE_DIR} ${MOODY_CAMEL_INCLUDE_DIR})
target_link_libraries(client PUBLIC raven)
# -------------------------------------------------------------------------------

add_subdirectory(tests)

add_subdirectory(examples) #---adding a new examples directory
