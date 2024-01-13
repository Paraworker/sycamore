cmake_minimum_required(VERSION 3.21)
project(sycamore)

set(CMAKE_CXX_STANDARD 23)

# main sources and headers
set(HEADER_DIRECTORY "include")
set(SOURCE_DIRECTORY "src")
set(PROTOCOL_DIRECTORY "protocols")

file(GLOB_RECURSE SOURCES_FILE "${SOURCE_DIRECTORY}/*.cpp")
file(GLOB_RECURSE HEADERS_FILE "${HEADER_DIRECTORY}/*.h")

add_executable(
        ${PROJECT_NAME}
        ${SOURCES_FILE}
        ${HEADERS_FILE}
)

find_package(PkgConfig REQUIRED)
pkg_search_module(WLR REQUIRED wlroots)
pkg_search_module(WS REQUIRED wayland-server)
pkg_search_module(XKBCOMMON REQUIRED xkbcommon)
pkg_search_module(LIBINPUT REQUIRED libinput)
pkg_search_module(FMT REQUIRED fmt)
pkg_search_module(SPDLOG REQUIRED spdlog)

set(CMAKE_CXX_FLAGS "-DWLR_USE_UNSTABLE")

include_directories(
        ${HEADER_DIRECTORY}
        ${WLR_INCLUDE_DIRS}
        ${PROTOCOL_DIRECTORY}
)

target_link_libraries(
        ${PROJECT_NAME}
        ${WLR_LINK_LIBRARIES}
        ${WS_LINK_LIBRARIES}
        ${XKBCOMMON_LINK_LIBRARIES}
        ${LIBINPUT_LINK_LIBRARIES}
        ${FMT_LINK_LIBRARIES}
        ${SPDLOG_LINK_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin)