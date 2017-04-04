# - Try to find MODULECONNECTOR
# Once done this will define
# MODULECONNECTOR_FOUND - System has MODULECONNECTOR
# MODULECONNECTOR_INCLUDE_DIRS - The MODULECONNECTOR include directories
# MODULECONNECTOR_LIBRARIES - The libraries needed to use MODULECONNECTOR
# MODULECONNECTOR_DEFINITIONS - Compiler switches required for using MODULECONNECTOR

if(APPLE)

  set(MODULECONNECTOR_LIBNAME "libModuleConnector64.dylib")

elseif(UNIX)

  if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm") 

    set(MODULECONNECTOR_LIBNAME "libModuleConnector64.so")

  else()

    message(FATAL_ERROR "Novelda Module Connector is not set up for linux on an intel pc yet")

  endif()

else() 

  message(FATAL_ERROR "Novelda Module Connector is not set up for windows yet")

endif()

find_path ( MODULECONNECTOR_INCLUDE_DIR ModuleConnector.hpp PATHS "${CMAKE_SOURCE_DIR}/thirdparty/novelda/include")
find_library ( MODULECONNECTOR_LIBRARY NAMES ${MODULECONNECTOR_LIBNAME} PATHS "${CMAKE_SOURCE_DIR}/thirdparty/novelda/lib/osx" "${CMAKE_SOURCE_DIR}/thirdparty/novelda/lib/rpi")

set ( MODULECONNECTOR_LIBRARIES ${MODULECONNECTOR_LIBRARY} )
set ( MODULECONNECTOR_INCLUDE_DIRS ${MODULECONNECTOR_INCLUDE_DIR} )

include ( FindPackageHandleStandardArgs )
# handle the QUIETLY and REQUIRED arguments and set MODULECONNECTOR_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args ( MODULECONNECTOR DEFAULT_MSG MODULECONNECTOR_LIBRARY MODULECONNECTOR_INCLUDE_DIR )
                                                                                        
