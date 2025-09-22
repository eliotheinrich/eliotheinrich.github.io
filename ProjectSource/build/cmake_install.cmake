# Install script for directory: /Users/eliotheinrich/Projects/eliotheinrich.github.io/ProjectSource

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/Users/eliotheinrich/Projects/eliotheinrich.github.io/ProjectSource/build")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/Users/eliotheinrich/Projects/eliotheinrich.github.io/assets/js/xymodel.js")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/Users/eliotheinrich/Projects/eliotheinrich.github.io/assets/js" TYPE EXECUTABLE FILES "/Users/eliotheinrich/Projects/eliotheinrich.github.io/ProjectSource/main/xymodel.js")
  if(EXISTS "$ENV{DESTDIR}/Users/eliotheinrich/Projects/eliotheinrich.github.io/assets/js/xymodel.js" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/Users/eliotheinrich/Projects/eliotheinrich.github.io/assets/js/xymodel.js")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/opt/homebrew/anaconda3/bin/strip" "$ENV{DESTDIR}/Users/eliotheinrich/Projects/eliotheinrich.github.io/assets/js/xymodel.js")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/Users/eliotheinrich/Projects/eliotheinrich.github.io/assets/js/dla.js")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/Users/eliotheinrich/Projects/eliotheinrich.github.io/assets/js" TYPE EXECUTABLE FILES "/Users/eliotheinrich/Projects/eliotheinrich.github.io/ProjectSource/main/dla.js")
  if(EXISTS "$ENV{DESTDIR}/Users/eliotheinrich/Projects/eliotheinrich.github.io/assets/js/dla.js" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/Users/eliotheinrich/Projects/eliotheinrich.github.io/assets/js/dla.js")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/opt/homebrew/anaconda3/bin/strip" "$ENV{DESTDIR}/Users/eliotheinrich/Projects/eliotheinrich.github.io/assets/js/dla.js")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/Users/eliotheinrich/Projects/eliotheinrich.github.io/assets/js/xymodel.wasm")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/Users/eliotheinrich/Projects/eliotheinrich.github.io/assets/js" TYPE FILE FILES "/Users/eliotheinrich/Projects/eliotheinrich.github.io/ProjectSource/main/xymodel.wasm")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/Users/eliotheinrich/Projects/eliotheinrich.github.io/assets/js/dla.wasm")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/Users/eliotheinrich/Projects/eliotheinrich.github.io/assets/js" TYPE FILE FILES "/Users/eliotheinrich/Projects/eliotheinrich.github.io/ProjectSource/main/dla.wasm")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/Users/eliotheinrich/Projects/eliotheinrich.github.io/ProjectSource/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
