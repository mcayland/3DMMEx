include(FindPackageHandleStandardArgs)
# This is technically bad form because we aren't "finding" anything, however if
# there is ever any hope to eventually port 3DMM to "not Windows", there needs
# to be a shim of some kind for slotting in other possible releases of BRender.
#
# This file can be expanded *properly* at a later date.

# TODO: Keep in mind the NO_<...>_PATH stuff should be removed if 3DMMForever
# starts to support external versions of BRender

foreach (name IN ITEMS BRFMMXR BRFWMXR BRZBMXR)
  foreach (cfg IN ITEMS DEBUG)
    set(variable ${CMAKE_FIND_PACKAGE_NAME}_${name}_${cfg}_LIBRARY)
    set(suffix "s")
    if (${cfg} STREQUAL "DEBUG")
      set(suffix "d")
    endif()
    if (NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
      find_library(${variable}
        NAMES ${name}
        PATHS "${PROJECT_SOURCE_DIR}/elib/brender/x64-3dmm/win${suffix}"
        NO_DEFAULT_PATH
        NO_PACKAGE_ROOT_PATH
        NO_CMAKE_PATH
        NO_CMAKE_ENVIRONMENT_PATH
        NO_SYSTEM_ENVIRONMENT_PATH
        NO_CMAKE_SYSTEM_PATH
        NO_CMAKE_FIND_ROOT_PATH)
    else()
      find_library(${variable}
        NAMES ${name}
        PATHS "${PROJECT_SOURCE_DIR}/elib/brender/x86-3dmm/win${suffix}"
        NO_DEFAULT_PATH
        NO_PACKAGE_ROOT_PATH
        NO_CMAKE_PATH
        NO_CMAKE_ENVIRONMENT_PATH
        NO_SYSTEM_ENVIRONMENT_PATH
        NO_CMAKE_SYSTEM_PATH
        NO_CMAKE_FIND_ROOT_PATH)
    endif()
    list(APPEND ${CMAKE_FIND_PACKAGE_NAME}_LIBRARIES ${variable})
  endforeach()
endforeach()

find_package_handle_standard_args(${CMAKE_FIND_PACKAGE_NAME}
  REQUIRED_VARS ${${CMAKE_FIND_PACKAGE_NAME}_LIBRARIES})

if (${CMAKE_FIND_PACKAGE_NAME}_FOUND AND NOT TARGET BRender::Libraries)
  add_library(BRender::Libraries INTERFACE IMPORTED)
  target_include_directories(BRender::Libraries
    INTERFACE
    "${PROJECT_SOURCE_DIR}/elib/brender/x64-3dmm/inc"
  )
  target_compile_definitions(
      BRender::Libraries
      INTERFACE
      XBRENDER_ORIGINAL
  )
  foreach (library IN ITEMS BRFMMXR BRFWMXR BRZBMXR)
    add_library(BRender::${library} STATIC IMPORTED)
    set_target_properties(BRender::${library}
      PROPERTIES
        IMPORTED_LOCATION_DEBUG ${${CMAKE_FIND_PACKAGE_NAME}_${library}_DEBUG_LIBRARY})
      target_link_libraries(BRender::Libraries INTERFACE BRender::${library})
    mark_as_advanced(
      ${CMAKE_FIND_PACKAGE_NAME}_${library}_RELEASE_LIBRARY
      ${CMAKE_FIND_PACKAGE_NAME}_${library}_DEBUG_LIBRARY)
  endforeach()

  # Link legacy stdio functions on Visual Studio 2015 and later
  if("${MSVC_VERSION}" GREATER_EQUAL "1900")
    target_link_libraries(BRender::BRFWMXR
      INTERFACE
      legacy_stdio_definitions
    )
  endif()

  # Precompiled BRender libraries do not support SafeSEH
  target_link_options(BRender::BRFWMXR INTERFACE
      $<$<LINK_LANG_AND_ID:CXX,MSVC>:/SAFESEH:NO>
  )
endif()
