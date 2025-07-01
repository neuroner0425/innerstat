cmake_minimum_required(VERSION 3.16)
project(ineerStat)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

# Build options
option(BUILD_CLIENT "Build Client App" ON)
option(BUILD_AGENT "Build Agent App" ON)
option(BUILD_SL "Build SL App" ON)

# include folder
include_directories(${CMAKE_SOURCE_DIR}/include)

# wxWidgets configuration
set(wxWidgets_CONFIGURATION mswud)
find_package(wxWidgets REQUIRED COMPONENTS core base adv aui xrc html xml net gl png jpeg zlib regex)
include(${wxWidgets_USE_FILE})

add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/wxborderlessframe)

# 공통 빌드 함수
function(add_my_app target src_glob)
    file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${src_glob}")
    add_executable(${target} ${SRC})
    target_include_directories(${target} PRIVATE
        ${PROJECT_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/third_party/wxborderlessframe/include
    )
    target_link_libraries(${target} PRIVATE wxBorderlessFrame ${wxWidgets_LIBRARIES})
    target_link_options(${target} PRIVATE -static)
endfunction()

if(BUILD_CLIENT)
    add_my_app(client_app "${CMAKE_SOURCE_DIR}/src/client/*.cpp")
endif()

if(BUILD_AGENT)
    add_my_app(agent_app "${CMAKE_SOURCE_DIR}/src/agent/*.cpp")
endif()

if(BUILD_SL)
    add_my_app(systemload_app "${CMAKE_SOURCE_DIR}/src/slapp/*.cpp")
endif()
