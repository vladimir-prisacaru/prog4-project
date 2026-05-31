cmake_minimum_required(VERSION 3.26)
project(MiniginLib)

# --------------------
# --- Source files ---
# --------------------
set(MINIGIN_SOURCES
  src/Font.cpp
  src/GameObject.cpp
  src/InputManager.cpp
  src/Minigin.cpp
  src/Renderer.cpp
  src/ResourceManager.cpp
  src/Scene.cpp
  src/SceneManager.cpp
  src/Texture2D.cpp
  src/TextComponent.cpp
  src/TextureComponent.cpp
  src/Gamepad.cpp
  src/EventManager.cpp
  src/ServiceLocator.cpp
  src/SoundSystemSDL.cpp

  # Dear ImGui (compiled into the engine library)
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp

  # ImPlot (compiled into the engine library)
  ${implot_SOURCE_DIR}/implot.cpp
  ${implot_SOURCE_DIR}/implot_items.cpp
)

# Suppress warnings on third-party sources we don't control
set(THIRD_PARTY_SOURCES
  ${IMGUI_SOURCE_DIR}/imgui.cpp
  ${IMGUI_SOURCE_DIR}/imgui_draw.cpp
  ${IMGUI_SOURCE_DIR}/imgui_tables.cpp
  ${IMGUI_SOURCE_DIR}/imgui_widgets.cpp
  ${IMGUI_SOURCE_DIR}/imgui_demo.cpp
  ${IMGUI_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
  ${IMGUI_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
  ${IMPLOT_SOURCE_DIR}/implot.cpp
  ${IMPLOT_SOURCE_DIR}/implot_items.cpp
)

if(MSVC)
  set_source_files_properties(${THIRD_PARTY_SOURCES} PROPERTIES COMPILE_FLAGS "/W0")
else()
  set_source_files_properties(${THIRD_PARTY_SOURCES} PROPERTIES COMPILE_FLAGS "-w")
endif()

add_library(minigin STATIC ${MINIGIN_SOURCES})

# =====================================================================
# Include directories
# PUBLIC  → propagated to anything that links minigin
# PRIVATE → only needed to compile the library itself
# =====================================================================
target_include_directories(minigin
  PUBLIC
    # Engine public API headers
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    # ImGui & ImPlot headers (consumers need these to call ImGui:: / ImPlot::)
    ${IMGUI_SOURCE_DIR}
    ${IMGUI_SOURCE_DIR}/backends
    ${IMPLOT_SOURCE_DIR}
    # VLD (no-op if not found)
    ${VLD_INCLUDE_DIR}
  PRIVATE
    # Private implementation details
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# =====================================================================
# Compile features & warnings
# =====================================================================
target_compile_features(minigin PUBLIC cxx_std_20)

if(MSVC)
  target_compile_options(minigin PRIVATE /W4 /WX)
else()
  target_compile_options(minigin PRIVATE -Wall -Wextra -Werror)
endif()

# =====================================================================
# Link dependencies
# PUBLIC so that Game (and any other consumer) inherits them
# =====================================================================
target_link_libraries(minigin
  PUBLIC
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    SDL3_mixer::SDL3_mixer
    glm::glm
    tinyxml2::tinyxml2
    ${VLD_LIBRARIES}
)