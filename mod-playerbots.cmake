# This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
# information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
# or (at your option) any later version.

#
# Optional integration-test build for mod-playerbots.
# Included by modules/CMakeLists.txt after add_library(modules ...).
#
option(PLAYERBOTS_INTEGRATION_TESTS "Compile mod-playerbots integration test scenarios" OFF)

if(PLAYERBOTS_INTEGRATION_TESTS)
  target_compile_definitions(modules PRIVATE PLAYERBOTS_INTEGRATION_TESTS)
  message(STATUS "mod-playerbots: integration tests ENABLED")

  # Copy the whole integration-tests/ helper folder next to the worldserver binary
  # (copy scheme from the config-merger tool in src/server/apps/CMakeLists.txt).
  if(WIN32 AND "${CMAKE_MAKE_PROGRAM}" MATCHES "MSBuild")
    foreach(cfg IN ITEMS Debug Release RelWithDebInfo MinSizeRel)
      file(COPY "${CMAKE_CURRENT_LIST_DIR}/apps/integration-tests" DESTINATION "${CMAKE_BINARY_DIR}/bin/${cfg}")
    endforeach()
  else()
    file(COPY "${CMAKE_CURRENT_LIST_DIR}/apps/integration-tests" DESTINATION "${CMAKE_BINARY_DIR}/bin")
  endif()
endif()
