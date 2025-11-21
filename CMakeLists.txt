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

# include folder (project headers only)
include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

# Helper for listing sources
function(glob_src out_var pattern)
    file(GLOB_RECURSE ${out_var} CONFIGURE_DEPENDS "${pattern}")
    set(${out_var} ${${out_var}} PARENT_SCOPE)
endfunction()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 1) Build agent first (no wx settings leak)
set(MOSQUITTO_INCLUDE_DIR /opt/homebrew/Cellar/mosquitto/2.0.22_1/include)
set(MOSQUITTO_LIB /opt/homebrew/lib/libmosquitto.dylib)
set(MOSQUITTOPP_LIB /opt/homebrew/lib/libmosquittopp.dylib)

if(BUILD_AGENT)
    glob_src(AGENT_SRC "${CMAKE_SOURCE_DIR}/src/agent/*.cpp")
    add_executable(agent_app ${AGENT_SRC})
    target_include_directories(agent_app PRIVATE ${PROJECT_INCLUDE_DIR} ${MOSQUITTO_INCLUDE_DIR})
    target_link_libraries(agent_app PRIVATE ${MOSQUITTO_LIB} ${MOSQUITTOPP_LIB})
    target_compile_definitions(agent_app PRIVATE USE_MQTT)
endif()

# 2) GUI apps (wxWidgets) — configure and add targets after agent
if(BUILD_CLIENT OR BUILD_SL)
    # Third-party includes for GUI only
    set(GUI_TP_INC
        ${CMAKE_SOURCE_DIR}/third_party/wxborderlessframe/include
        ${CMAKE_SOURCE_DIR}/third_party/wxnosashsplitter/include
        ${CMAKE_SOURCE_DIR}/third_party/wx_custum_titlebar_frame/include
    )

    set(wxWidgets_CONFIG_EXECUTABLE "/opt/wxWidgets/build-cocoa-debug/wx-config")
    find_package(wxWidgets REQUIRED COMPONENTS core base adv aui xrc html xml net gl)
    include(${wxWidgets_USE_FILE})

    add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/wxborderlessframe)
    add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/wx_custum_titlebar_frame)

    function(add_my_wx_app target src_glob)
        glob_src(SRC "${src_glob}")
        add_executable(${target} ${SRC})
        target_include_directories(${target} PRIVATE
            ${PROJECT_INCLUDE_DIR}
            ${GUI_TP_INC}
        )
        target_link_libraries(${target} PRIVATE wxBorderlessFrame wx_custom_titlebar_frame ${wxWidgets_LIBRARIES})
        if(NOT APPLE)
            target_link_options(${target} PRIVATE -static)
        endif()
    endfunction()

    if(BUILD_CLIENT)
        add_my_wx_app(client_app "${CMAKE_SOURCE_DIR}/src/client/*.cpp")
        target_compile_definitions(client_app PRIVATE USE_MQTT)
        target_include_directories(client_app PRIVATE ${MOSQUITTO_INCLUDE_DIR})
        target_link_libraries(client_app PRIVATE ${MOSQUITTO_LIB} ${MOSQUITTOPP_LIB})
    endif()

    if(BUILD_SL)
        add_my_wx_app(systemload_app "${CMAKE_SOURCE_DIR}/src/slapp/*.cpp")
    endif()
endif()
