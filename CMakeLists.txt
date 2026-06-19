cmake_minimum_required(VERSION 3.24)

project(Velvet LANGUAGES C CXX)

option(USE_HIP "Build with HIP for AMD GPUs" OFF)

# GPU language setup
if(USE_HIP)
  enable_language(HIP)
  if(NOT DEFINED CMAKE_HIP_ARCHITECTURES OR CMAKE_HIP_ARCHITECTURES STREQUAL "")
    set(CMAKE_HIP_ARCHITECTURES "gfx90a")
  endif()
  message(STATUS "Building with HIP, architectures: ${CMAKE_HIP_ARCHITECTURES}")
else()
  enable_language(CUDA)
  message(STATUS "Building with CUDA")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages (all provided via vcpkg)
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

# HIP-specific dependencies
if(USE_HIP)
  find_package(hipcub REQUIRED)
  find_package(rocthrust REQUIRED)
endif()

# Host/GPU sources. Upstream keeps these as .cpp (built by MSVC) plus the two
# .cu files (built by nvcc) in Velvet.vcxproj; that layout is preserved here.
# The .cpp files include device code through Common.cuh, so the HIP build below
# compiles them as HIP via LANGUAGE without renaming, leaving the Visual Studio
# CUDA build (which references the .cpp names) intact.
set(CPP_SOURCES
  Velvet/Actor.cpp
  Velvet/Component.cpp
  Velvet/GUI.cpp
  Velvet/GameInstance.cpp
  Velvet/Helper.cpp
  Velvet/Input.cpp
  Velvet/MeshRenderer.cpp
  Velvet/Timer.cpp
  Velvet/VtEngine.cpp
  Velvet/stb_image.cpp
  Velvet/main.cpp
)
set(CU_SOURCES
  Velvet/VtClothSolverGPU.cu
  Velvet/SpatialHashGPU.cu
)
set(SOURCES ${CPP_SOURCES} ${CU_SOURCES})

# Create executable
add_executable(Velvet ${SOURCES})

if(USE_HIP)
  # Compile every source as HIP. A mixed CXX/HIP target would apply the HIP
  # arch flags to the .cpp sources as plain C++ and fail, so mark them all HIP.
  set_source_files_properties(${SOURCES} PROPERTIES LANGUAGE HIP)
  target_compile_definitions(Velvet PRIVATE USE_HIP)
  target_link_libraries(Velvet PRIVATE hip::hipcub roc::rocthrust)
  set_target_properties(Velvet PROPERTIES
    HIP_ARCHITECTURES "${CMAKE_HIP_ARCHITECTURES}"
  )
endif()
# For the non-HIP (CUDA) build, language auto-detection drives the split:
# .cpp -> CXX, .cu -> CUDA, matching upstream's Visual Studio project.

# Include directories
target_include_directories(Velvet PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/Velvet
  ${CMAKE_CURRENT_SOURCE_DIR}/Velvet/External
  ${CMAKE_CURRENT_SOURCE_DIR}/Velvet/External/cuda
)

# Link libraries
target_link_libraries(Velvet PRIVATE
  OpenGL::GL
  glfw
  glad::glad
  glm::glm
  fmt::fmt
  assimp::assimp
  imgui::imgui
)

# Set output directory
set_target_properties(Velvet PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Copy assets to build directory
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Velvet/Assets")
  add_custom_command(TARGET Velvet POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/Velvet/Assets"
    "$<TARGET_FILE_DIR:Velvet>/Assets"
  )
endif()
