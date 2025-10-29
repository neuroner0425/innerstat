cmake_minimum_required(VERSION 3.16)
project(innerStat)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

# macOS toolchain hints (SDK & deployment target)
if(APPLE)
    # Prefer current Xcode SDK if available
    execute_process(COMMAND xcrun --sdk macosx --show-sdk-path OUTPUT_VARIABLE SDK_PATH OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    if(SDK_PATH)
        message(STATUS "Using macOS SDK: ${SDK_PATH}")
        set(CMAKE_OSX_SYSROOT "${SDK_PATH}" CACHE PATH "" FORCE)
    endif()
    if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "" FORCE)
    endif()
endif()

# Build options
option(BUILD_CLIENT "Build Client App" ON)
option(BUILD_AGENT "Build Agent App" ON)
option(BUILD_SL "Build SL App" ON)

# include folder
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party/wxborderlessframe/include
    ${CMAKE_SOURCE_DIR}/third_party/wxnosashsplitter/include
    /opt/wxWidgets/include
    /opt/wxWidgets/build-cocoa-debug/lib/wx/include/osx_cocoa-unicode-3.3
    /opt/wxWidgets/build-cocoa-debug
)


# wxWidgets configuration
set(wxWidgets_CONFIG_EXECUTABLE "/opt/wxWidgets/build-cocoa-debug/wx-config")
find_package(wxWidgets REQUIRED COMPONENTS core base adv aui xrc html xml net gl)
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
    # Avoid static linking on macOS with frameworks
    if(NOT APPLE)
        target_link_options(${target} PRIVATE -static)
    endif()
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
