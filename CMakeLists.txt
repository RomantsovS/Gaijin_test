cmake_minimum_required(VERSION 3.2)

project(Gaijin_test)

find_package(Boost COMPONENTS system REQUIRED)
find_package(Threads)
include_directories(${Boost_INCLUDE_DIR})

option(WITH_GTEST "Whether to build google test" ON)

if (MSVC)
    set(MY_COMLILE_OPTIONS /W4 /WX)
else ()
    set(MY_COMLILE_OPTIONS -Wall -Wextra -pedantic -Werror -Wshadow -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC)
endif()

add_library(server_lib SHARED Server.cpp Database.cpp Core.cpp ServerStat.cpp)

target_compile_options(server_lib PRIVATE ${MY_COMLILE_OPTIONS})
set_target_properties(server_lib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

add_executable(server ServerMain.cpp)
add_executable(client ClientMain.cpp Client.h)
target_link_libraries(server ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} server_lib)
target_link_libraries(client ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(server PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

set_target_properties(client PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(server PRIVATE "${CMAKE_BINARY_DIR}")
target_include_directories(client PRIVATE "${CMAKE_BINARY_DIR}")

if(WIN32)
    target_link_libraries(server_lib wsock32 ws2_32)
    target_link_libraries(client wsock32 ws2_32)
endif()

target_compile_options(server PRIVATE ${MY_COMLILE_OPTIONS})
target_compile_options(client PRIVATE ${MY_COMLILE_OPTIONS})

if(WITH_GTEST)
    include(FetchContent)
    FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/0affd45b62371c1de44064a5a5f8817242208442.zip
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(tests tests.cpp)

    set_target_properties(tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    )

    target_link_libraries(tests gtest_main server_lib)
    target_compile_options(tests PRIVATE ${MY_COMLILE_OPTIONS})

    include(GoogleTest)
    gtest_discover_tests(tests)
endif()