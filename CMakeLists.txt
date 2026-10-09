cmake_minimum_required(VERSION 3.20)
project(BlessiMart VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# vcpkg provides these via CMAKE_TOOLCHAIN_FILE
find_package(Drogon CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)   # vcpkg package name
find_package(OpenSSL REQUIRED)
find_package(Catch2 3 REQUIRED)

# Collect all src .cpp files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/**/*.cpp")

add_executable(blessimart ${SOURCES})

target_include_directories(blessimart PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(blessimart PRIVATE
    Drogon::Drogon
    unofficial::sqlite3::sqlite3   # vcpkg target
    OpenSSL::SSL
    OpenSSL::Crypto
)

# MSVC flags
if(MSVC)
    target_compile_options(blessimart PRIVATE /utf-8 /W3)
endif()

# Pass schema path as compile definition (absolute path at build time)
target_compile_definitions(blessimart PRIVATE
    BLESSIMART_SCHEMA_PATH="${CMAKE_SOURCE_DIR}/schema.sql"
)

# Copy runtime assets to build directory
add_custom_target(CopyAssets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/config  ${CMAKE_BINARY_DIR}/config
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/frontend ${CMAKE_BINARY_DIR}/frontend
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/schema.sql ${CMAKE_BINARY_DIR}/schema.sql
    COMMENT "Copying config, frontend, and schema to build directory"
)
add_dependencies(blessimart CopyAssets)

enable_testing()
add_subdirectory(tests)
