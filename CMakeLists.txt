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
# set(wxWidgets_ROOT_DIR "{wxWidgets installation path}")
# set(wxWidgets_LIB_DIR "{wxWidgets installation path}")/lib/gcc_lib")
set(wxWidgets_CONFIGURATION mswud)

find_package(wxWidgets REQUIRED COMPONENTS core base adv aui xrc html xml net gl png jpeg zlib regex)
include(${wxWidgets_USE_FILE})

# Client build
if(BUILD_CLIENT)
    file(GLOB_RECURSE CLIENT_SRC CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/client/*.cpp")
    add_executable(client_app ${CLIENT_SRC})
    target_include_directories(client_app PRIVATE ${PROJECT_INCLUDE_DIR})
    target_link_libraries(client_app ${wxWidgets_LIBRARIES})
    target_link_options(client_app PRIVATE -static)
endif()

# Agent build
if(BUILD_AGENT)
    file(GLOB_RECURSE AGENT_SRC CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/agent/*.cpp")
    add_executable(agent_app ${AGENT_SRC})
    target_include_directories(agent_app PRIVATE ${PROJECT_INCLUDE_DIR})
    target_link_libraries(agent_app ${wxWidgets_LIBRARIES})
    target_link_options(agent_app PRIVATE -static)
endif()

# SL build
if(BUILD_SL)
    file(GLOB_RECURSE SL_SRC CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/slapp/*.cpp")
    add_executable(systemload_app ${SL_SRC})
    target_include_directories(systemload_app PRIVATE ${PROJECT_INCLUDE_DIR})
    target_link_libraries(systemload_app ${wxWidgets_LIBRARIES})
    target_link_options(systemload_app PRIVATE -static)
endif()