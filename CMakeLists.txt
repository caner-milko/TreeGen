cmake_minimum_required(VERSION 3.21)

project(TreeGeneration LANGUAGES C CXX)

# ---- Global settings -------------------------------------------------------

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Put all build artefacts in one place so the executable sits next to its
# copied Assets/ folder.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# ---- Third-party dependencies ---------------------------------------------

include(FetchContent)

# GLFW (windowing / input)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE
)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)

# GLM (math)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(glfw glm)

find_package(OpenGL REQUIRED)

# ---- Vendored dependencies -------------------------------------------------

set(DEPS_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/deps/include")
set(IMGUI_DIR "${DEPS_INCLUDE}/imgui")

if(NOT EXISTS "${IMGUI_DIR}/imgui.cpp")
    message(FATAL_ERROR
        "ImGui sources not found at ${IMGUI_DIR}. "
        "Did you forget to run 'git submodule update --init --recursive'?")
endif()

# glad (OpenGL loader)
add_library(glad STATIC "${DEPS_INCLUDE}/glad/glad.c")
target_include_directories(glad PUBLIC "${DEPS_INCLUDE}")
target_link_libraries(glad PUBLIC ${CMAKE_DL_LIBS})

# Dear ImGui (built from the submodule, GLFW + OpenGL3 backends)
add_library(imgui STATIC
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)
target_include_directories(imgui PUBLIC
    "${IMGUI_DIR}"
    "${IMGUI_DIR}/backends"
)
target_link_libraries(imgui PUBLIC glfw)

# ---- TreeGeneration executable --------------------------------------------

file(GLOB_RECURSE TREEGEN_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(TreeGeneration
    ${TREEGEN_SOURCES}
    "${DEPS_INCLUDE}/header_impl.cpp"   # stb_image + tiny_obj_loader implementations
)

target_include_directories(TreeGeneration PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/generation"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/graphics"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/opengl"
    "${DEPS_INCLUDE}"
)

target_link_libraries(TreeGeneration PRIVATE
    glad
    imgui
    glfw
    glm::glm
    OpenGL::GL
)

# The code uses GLM's GTX extensions (e.g. gtx/component_wise), which GLM 1.x
# guards behind this opt-in macro.
target_compile_definitions(TreeGeneration PRIVATE GLM_ENABLE_EXPERIMENTAL)

# ---- Assets ----------------------------------------------------------------
# The application loads assets via the relative path "./Assets/...", so make
# sure a copy lives next to the executable and that the debugger starts there.

add_custom_command(TARGET TreeGeneration POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Assets"
        "$<TARGET_FILE_DIR:TreeGeneration>/Assets"
    COMMENT "Copying Assets next to the executable"
)

set_target_properties(TreeGeneration PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:TreeGeneration>"
)
