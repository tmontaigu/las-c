cmake_minimum_required(VERSION 3.22)
project(las_c)

set(CMAKE_C_STANDARD 17)

#if (MSVC)
#    add_compile_options(/fsanitize=address /0d)
#    add_link_options(/fsanitize=address)
#else()
#    add_compile_options(-fsanitize=address)
#    add_link_options(-fsanitize=address)
#endif()


option(WITH_DEBUG_ASSERTIONS "Enable debug assertions" ON)
option(WITH_LAZRS "Build with lazrs to support LAZ" OFF)
option(BUIlD_SHARED_LIBS "Build libraries as shared" OFF)

add_library(las_c)
set_property(TARGET las_c PROPERTY C_STANDARD 17)
target_include_directories(las_c PUBLIC include/)
target_include_directories(las_c PRIVATE src/private)

if (WITH_DEBUG_ASSERTIONS)
    target_compile_definitions(las_c PRIVATE -DLAS_DEBUG_ASSERTIONS=1)
else()
    target_compile_definitions(las_c PRIVATE -DLAS_DEBUG_ASSERTIONS=0)
endif()

if (DEFINED CMAKE_C_BYTE_ORDER AND CMAKE_C_BYTE_ORDER)
    message(STATUS "Byte order: `${CMAKE_C_BYTE_ORDER}`")
    if (CMAKE_C_BYTE_ORDER STREQUAL "LITTLE_ENDIAN")
    else()
        message(FATAL_ERROR "Only little endian is supported")
    endif()
else()
    message(FATAL_ERROR "Failed to get endianess")
endif()

if (WITH_LAZRS)
    add_subdirectory(laz-rs-c)
    target_link_libraries(las_c PRIVATE laz-rs-c)
    target_compile_definitions(las_c PRIVATE -DWITH_LAZRS)
endif ()

include(cmake/CompilerWarnings.cmake)
set_project_warnings(las_c)

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(bin)
enable_testing()
add_subdirectory(tests)

#target_compile_options(las_c PRIVATE '-Wconversion')
