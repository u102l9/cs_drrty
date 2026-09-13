cmake_minimum_required(VERSION 3.10)
project(fakeloc_loaders CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_library(log-lib log)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src)

foreach(loader fakeloc_init fakeloc_initzygote fakeloc_apphook)
    add_library(${loader} SHARED ${SRC_DIR}/root/lib${loader}.cpp)
    target_include_directories(${loader} PRIVATE ${SRC_DIR} ${SRC_DIR}/root)
    target_link_libraries(${loader} PRIVATE ${log-lib})
    set_target_properties(${loader} PROPERTIES
        OUTPUT_NAME "${loader}"
        POSITION_INDEPENDENT_CODE ON
    )
endforeach()