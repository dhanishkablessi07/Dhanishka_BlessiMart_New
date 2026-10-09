cmake_minimum_required(VERSION 3.20)

find_package(Catch2 3 REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

set(TEST_SOURCES
    test_crypto.cpp
    test_db.cpp
    test_helpers.cpp
)

add_executable(blessimart_tests ${TEST_SOURCES})

target_include_directories(blessimart_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(blessimart_tests PRIVATE
    Catch2::Catch2WithMain
    unofficial::sqlite3::sqlite3   # vcpkg target
    OpenSSL::Crypto
    Drogon::Drogon
)

target_compile_definitions(blessimart_tests PRIVATE
    BLESSIMART_SCHEMA_PATH="${CMAKE_SOURCE_DIR}/schema.sql"
)

if(MSVC)
    target_compile_options(blessimart_tests PRIVATE /utf-8)
endif()

include(CTest)
include(Catch)
catch_discover_tests(blessimart_tests)
