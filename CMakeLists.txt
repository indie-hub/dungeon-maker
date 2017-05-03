
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)

# See http://git.libssh.org/projects/libssh.git/tree/ for example of a good project configuration.

# Global vars

SET(CMAKE_PROJECT_NAME "DungeonMaker")

SET(EXECUTABLE_NAME "DungeonMaker")

SET(EXECUTABLE_VERSION_MAJOR "0")
SET(EXECUTABLE_VERSION_MINOR "1")
SET(EXECUTABLE_VERSION_PATCH "0")

SET(EXECUTABLE_VERSION "${EXECUTABLE_VERSION_MAJOR}.${EXECUTABLE_VERSION_MINOR}.${EXECUTABLE_VERSION_PATCH}")

# Additional CMake modules, specific for this project
SET(CMAKE_MODULE_PATH
  ${CMAKE_SOURCE_DIR}/CMake/Modules
  )

# Load those modules
INCLUDE(Define_CMakeDefaults)
INCLUDE(Define_PlatformDefaults)
INCLUDE(Define_CompilerFlags)
INCLUDE(Define_Nanogui)

# Project command is here, to set some values correctly.
PROJECT(${CMAKE_PROJECT_NAME})

# Nifty marco, which forbids in source build. (source: http://git.libssh.org/projects/libssh.git)
INCLUDE(Macro_EnsureOutOfSourceBuild)
MACRO_ENSURE_OUT_OF_SOURCE_BUILD("${PROJECT_NAME}  requires an out of source build. Please create a separate build directory and run 'cmake /path/to/${CMAKE_PROJECT_NAME} [options]' there.")

# Another macro used in this project to add sources efficiently.
INCLUDE(Macro_AddSources)

#INCLUDE ( Setup_OGRE )


#Add aditional components.


MESSAGE(STATUS "Current built type: ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "Current build type flags: ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}}")

MESSAGE(STATUS "Configuring application ${APPLICATION_NAME}")

MACRO_PREPARE_SOURCES(PROJECT_SOURCES)

ADD_SUBDIRECTORY( src )

#Line below outputs all included source.
MACRO_PRINT_SOURCES()

INCLUDE_DIRECTORIES(
  ext/nanogui/include
  ext/json/src
  ${NANOGUI_EXTRA_INCS}
)

ADD_EXECUTABLE(
    ${EXECUTABLE_NAME}
    ${PROJECT_SOURCES}
)

TARGET_LINK_LIBRARIES(
    ${EXECUTABLE_NAME}
    nanogui
    ${NANOGUI_EXTRA_LIBS}
)

#copy data into build folder
ADD_CUSTOM_COMMAND(TARGET ${EXECUTABLE_NAME} PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                 ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/data)
