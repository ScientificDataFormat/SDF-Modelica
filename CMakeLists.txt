cmake_minimum_required (VERSION 3.2)

set (CMAKE_CXX_STANDARD 14)

project (ModelicaSDF)

if (WIN32)
  set(MODELICA_PLATFORM win)
elseif (APPLE)
  set(MODELICA_PLATFORM darwin)
else ()
  set(MODELICA_PLATFORM linux)
endif ()

if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64")
  set (MODELICA_PLATFORM aarch64-${MODELICA_PLATFORM})
elseif ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
  set (MODELICA_PLATFORM ${MODELICA_PLATFORM}64)
else ()
  set (MODELICA_PLATFORM ${MODELICA_PLATFORM}32)
endif ()

message("MODELICA_PLATFORM: " ${MODELICA_PLATFORM})

set(HDF5_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/hdf5-${MODELICA_PLATFORM}/install"  CACHE STRING "HDF5 directory")
set(MATIO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/matio-${MODELICA_PLATFORM}/install" CACHE STRING "MatIO directory")

if (MSVC)
  # link statically against the Visual C runtime
  set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   /MT")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
endif ()

add_library(ModelicaSDF SHARED
  C/include/ModelicaSDFFunctions.h
  C/src/ModelicaSDFFunctions.c
  C/src/dsres.cpp
)

if (MSVC)
  target_include_directories(ModelicaSDF PUBLIC
    "${HDF5_DIR}/include"
    "${MATIO_DIR}/include"
    C/include
  )

  target_link_libraries(ModelicaSDF
    "${HDF5_DIR}/lib/libhdf5.lib"
    "${HDF5_DIR}/lib/libhdf5_hl.lib"
    "${MATIO_DIR}/lib/libmatio.lib"
  )
else ()
  target_include_directories(ModelicaSDF PUBLIC
    "${HDF5_DIR}/include"
    "${MATIO_DIR}/include"
  	C/include
  )

  # the order of the libhdf5* libraries is important, so we don't get undefined symbols
  target_link_libraries(ModelicaSDF
    "${HDF5_DIR}/lib/libhdf5_hl.a"
    "${HDF5_DIR}/lib/libhdf5.a"
    "${MATIO_DIR}/lib/libmatio.a"
  )

if (NOT APPLE)
  # exclude symbols from dependencies to avoid linking issues on the target systems
  set (CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} "-Wl,--exclude-libs,ALL")
endif ()

endif ()

add_custom_command(TARGET ModelicaSDF POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
  "$<TARGET_FILE:ModelicaSDF>"
  "${CMAKE_CURRENT_SOURCE_DIR}/SDF/Resources/Library/${MODELICA_PLATFORM}/"
)

add_executable(ModelicaSDF_Test
  C/tests/ModelicaSDF_test.cpp
  C/tests/catch_amalgamated.hpp
  C/tests/catch_amalgamated.cpp
)

target_include_directories(ModelicaSDF_Test PUBLIC
	C/include
)

if (UNIX)
  target_link_libraries(ModelicaSDF_Test dl)
endif ()

target_compile_definitions(ModelicaSDF_Test PRIVATE
  TESTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/C/tests/"
  SHARED_LIBRARY_PATH="$<TARGET_FILE:ModelicaSDF>"
)

set_property(TARGET ModelicaSDF_Test PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/C/tests")

enable_testing()

add_test(NAME ModelicaSDF_Test COMMAND $<TARGET_FILE:ModelicaSDF_Test> WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/C/tests")
