cmake_minimum_required(VERSION 3.16)
project(ineerStat)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

# include folder (필요하면 사용)
include_directories(${CMAKE_SOURCE_DIR}/include)

# Client build
if(BUILD_CLIENT)
    file(GLOB_RECURSE CLIENT_SRC CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/client/*.cpp")
    add_executable(client_app ${CLIENT_SRC})
    # 윈도우즈 GUI용 라이브러리만 링크
    target_link_libraries(client_app PRIVATE gdi32 user32 kernel32 comctl32)
    target_link_options(client_app PRIVATE -static)
    target_compile_definitions(client_app PRIVATE UNICODE _UNICODE)
endif()
