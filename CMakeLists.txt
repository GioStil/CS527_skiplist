cmake_minimum_required(VERSION 3.17)
project(Skiplist LANGUAGES C CXX)

project(
  Skiplist
  VERSION 1.0.0
  LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_CLANG_TIDY "")
option(BUILD_SHARED_LIBS
       "Build shared libraries (.dll/.so) instead of static ones (.lib/.a)" OFF)

# Set a default build type if none was specified
set(default_build_type "Release")
if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
  set(default_build_type "Debug")
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(
    STATUS
      "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE
      "${default_build_type}"
      CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
                                               "MinSizeRel" "RelWithDebInfo")
endif()

include(GNUInstallDirs)
set(SOURCE_FILES src/skiplist.c)

add_library(${PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(
  ${PROJECT_NAME}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:include>
  PRIVATE src)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME skiplist
                                                 CLEAN_DIRECT_OUTPUT 1)

find_program(iwyu_path NAMES include-what-you-use iwyu iwyu-tool)
if(iwyu_path)
  set_property(TARGET ${PROJECT_NAME} PROPERTY C_INCLUDE_WHAT_YOU_USE
                                               ${iwyu_path})
else()
  message("Could not find the program include-what-you-use")
endif()

install(
  TARGETS ${PROJECT_NAME}
  EXPORT skiplist
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/ DESTINATION include)
install(EXPORT skiplist DESTINATION share/${PROJECT_NAME}/cmake)

export(TARGETS ${PROJECT_NAME} FILE ${PROJECT_NAME}.cmake)

if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)

  add_custom_target(
    uninstall COMMAND ${CMAKE_COMMAND} -P
                      ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()
