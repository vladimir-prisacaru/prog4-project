cmake_minimum_required(VERSION 3.26)
project(Game)

# =====================================================================
# Executable name & subsystem
# =====================================================================
set(SOURCES
    src/Main.cpp
    src/FPSCounter.cpp
    src/Player.cpp
    src/TunnelComponent.cpp
    src/Enemy.cpp
 "src/AttackComponent.cpp" "src/GameManager.cpp" "src/PlayerUI.cpp")


if(EMSCRIPTEN)
  set(GAME_TARGET game_web)
  add_executable(${GAME_TARGET} ${SOURCES})
else()
  set(GAME_TARGET game)
  add_executable(${GAME_TARGET} WIN32 ${SOURCES})
endif()

# =====================================================================
# Include directories (game-private sources)
# =====================================================================
target_include_directories(${GAME_TARGET}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# =====================================================================
# Link against the Minigin engine (inherits all its PUBLIC deps)
# =====================================================================
target_link_libraries(${GAME_TARGET} PRIVATE minigin)

# =====================================================================
# Platform-specific extras
# =====================================================================

# Windows: xinput + DLL copying + Steam + VLD
if(WIN32)
  target_link_libraries(${GAME_TARGET} PRIVATE xinput)

  # Copy SDL runtime DLLs next to the executable
  foreach(dllTarget IN ITEMS SDL3::SDL3 SDL3_ttf::SDL3_ttf SDL3_mixer::SDL3_mixer)
    add_custom_command(TARGET ${GAME_TARGET} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:${dllTarget}>"
        "$<TARGET_FILE_DIR:${GAME_TARGET}>"
      VERBATIM
    )
  endforeach()

  if(Steamworks_FOUND)
    target_link_libraries(${GAME_TARGET} PRIVATE Steamworks::Steamworks)
    target_compile_definitions(${GAME_TARGET} PRIVATE USE_STEAMWORKS)
    add_custom_command(TARGET ${GAME_TARGET} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy "${Steamworks_RUNTIME_DLL}" "$<TARGET_FILE_DIR:${GAME_TARGET}>")
    add_custom_command(TARGET ${GAME_TARGET} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E echo "480" > "$<TARGET_FILE_DIR:${GAME_TARGET}>/steam_appid.txt"
      COMMENT "Generating steam_appid.txt"
    )
  endif()

  # Set game as the startup project in Visual Studio
  set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${GAME_TARGET})
  set_target_properties(${GAME_TARGET} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${GAME_TARGET}>"
  )

  add_custom_command(TARGET ${GAME_TARGET} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
      "${CMAKE_SOURCE_DIR}/Data"
      "$<TARGET_FILE_DIR:${GAME_TARGET}>/Data"
  )

# macOS: bundle + rpath + Steam
elseif(APPLE)
  set_target_properties(${GAME_TARGET} PROPERTIES
    MACOSX_BUNDLE TRUE
    INSTALL_RPATH "@executable_path/../Frameworks"
    BUILD_WITH_INSTALL_RPATH TRUE
  )
  add_custom_command(TARGET ${GAME_TARGET} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_BUNDLE_DIR:${GAME_TARGET}>/Contents/Frameworks"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:SDL3::SDL3-shared>"      "$<TARGET_BUNDLE_DIR:${GAME_TARGET}>/Contents/Frameworks/"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:SDL3_ttf::SDL3_ttf-shared>" "$<TARGET_BUNDLE_DIR:${GAME_TARGET}>/Contents/Frameworks/"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:SDL3_mixer::SDL3_mixer-shared>" "$<TARGET_BUNDLE_DIR:${GAME_TARGET}>/Contents/Frameworks/"
    COMMAND install_name_tool -change "@rpath/libSDL3_ttf.0.dylib" "@executable_path/../Frameworks/libSDL3_ttf.0.2.2.dylib" "$<TARGET_FILE:${GAME_TARGET}>"
    COMMAND install_name_tool -change "@rpath/libSDL3.0.dylib"     "@executable_path/../Frameworks/libSDL3.0.dylib"          "$<TARGET_FILE:${GAME_TARGET}>"
  )

  if(Steamworks_FOUND)
    target_link_libraries(${GAME_TARGET} PRIVATE Steamworks::Steamworks)
    target_compile_definitions(${GAME_TARGET} PRIVATE USE_STEAMWORKS)
    add_custom_command(TARGET ${GAME_TARGET} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${Steamworks_LIBRARIES}" "$<TARGET_BUNDLE_DIR:${GAME_TARGET}>/Contents/Frameworks/"
    )
    add_custom_command(TARGET ${GAME_TARGET} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E echo "480" > "$<TARGET_FILE_DIR:${GAME_TARGET}>/steam_appid.txt"
      COMMENT "Generating steam_appid.txt"
    )
  endif()

  add_custom_command(TARGET ${GAME_TARGET} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
      "${CMAKE_SOURCE_DIR}/Data"
      "$<TARGET_FILE_DIR:${GAME_TARGET}>/Data"
  )

# Emscripten: link flags, .html output, copy web shell + Data preload
elseif(EMSCRIPTEN)
  set(EMSCRIPTEN_COMPILE_FLAGS "-g")
  set(EMSCRIPTEN_LINK_FLAGS
    "-sUSE_FREETYPE=1 -sUSE_HARFBUZZ=1 -g -sALLOW_MEMORY_GROWTH --preload-file \"${CMAKE_SOURCE_DIR}/Data@/\""
  )

  set_target_properties(${GAME_TARGET} PROPERTIES
    COMPILE_FLAGS "${EMSCRIPTEN_COMPILE_FLAGS}"
    LINK_FLAGS    "${EMSCRIPTEN_LINK_FLAGS}"
    SUFFIX        ".html"
  )

  add_custom_command(TARGET ${GAME_TARGET} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
      "${CMAKE_SOURCE_DIR}/web"
      "$<TARGET_FILE_DIR:${GAME_TARGET}>/"
  )

# Linux / other Unix: Steam if available, copy Data
else()
  if(Steamworks_FOUND)
    target_link_libraries(${GAME_TARGET} PRIVATE Steamworks::Steamworks)
    target_compile_definitions(${GAME_TARGET} PRIVATE USE_STEAMWORKS)
    add_custom_command(TARGET ${GAME_TARGET} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${Steamworks_LIBRARIES}" "$<TARGET_FILE_DIR:${GAME_TARGET}>"
    )
    add_custom_command(TARGET ${GAME_TARGET} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E echo "480" > "$<TARGET_FILE_DIR:${GAME_TARGET}>/steam_appid.txt"
      COMMENT "Generating steam_appid.txt"
    )
  endif()

  add_custom_command(TARGET ${GAME_TARGET} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
      "${CMAKE_SOURCE_DIR}/Data"
      "$<TARGET_FILE_DIR:${GAME_TARGET}>/Data"
  )
endif()
