cmake_minimum_required(VERSION 3.16)
project(AssaultCubeCheat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Cheat DLL (32-bit) ──────────────────────────────────────────────
add_library(ACCheat SHARED
    cheat/dllmain.cpp
    cheat/hooks.cpp
    cheat/render.cpp
    cheat/features.cpp
    cheat/menu.cpp
)
target_include_directories(ACCheat PRIVATE cheat)
target_link_libraries(ACCheat PRIVATE opengl32 gdi32)
set_target_properties(ACCheat PROPERTIES OUTPUT_NAME "ACCheat")

# ── Injector EXE (32-bit) ───────────────────────────────────────────
add_executable(Injector injector/main.cpp)
target_link_libraries(Injector PRIVATE)
set_target_properties(Injector PROPERTIES OUTPUT_NAME "Injector")
