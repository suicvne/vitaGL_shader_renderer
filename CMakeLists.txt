cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

#if(APPLE)
#set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for Mac OS X" FORCE)
#set(CMAKE_LIBRARY_ARCHITECTURE "arm64" CACHE STRING "Build architectures for Mac OS X" FORCE)
#endif()

option(Vita "Vita" OFF)

if(Vita)
  if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VITASDK})
      set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
    else()
      message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
    endif()
  endif()
endif()
unset(Vita CACHE)

project(VitaBobOmbs)

if(BUILD_VITA)
  if(DEFINED ENV{VITASDK})
    include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)
  endif()
endif()

set(VITA_APP_NAME "Vita Bob-Ombs")
set(VITA_TITLEID  "BOMB00420")
set(VITA_VERSION  "01.00")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-initializer-overrides")

if(BUILD_VITA)
  set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVITA -g -Wall -O0 -fno-lto -ftree-vectorize -Wno-missing-braces")
else()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -g -O0 -fno-lto -ftree-vectorize")  
endif()


# Resource Handling

file(GLOB_RECURSE resources "./*.gltf" "./*.glb" "./*.png" "./*.cgv" "./*.cgf" "res/*.png" "res/*.frag" "res/*.vert" "res/*.glsl")
set(data_VPKSHADOW "")
if(BUILD_VITA)
  foreach(resource ${resources})
    get_filename_component(filename ${resource} NAME)
    
    file(RELATIVE_PATH reldir ${PROJECT_SOURCE_DIR} ${resource})
    string(CONCAT data_VPKSHADOW "${data_VPKSHADOW}FILE;${reldir};${filename};")
  endforeach()
endif()

set(SRC_FILES
  src/main.c
  src/vgl3d/vgl3d_gl2es.c
  src/vgl3d/mesh/mesh.c
)

if(VITA)
  # PS Vita platform specifics.
  list(APPEND SRC_FILES
    src/vgl3d/input/tesla_input_vita.c
  )
else()
  # TODO: Move this whole statement down
  # So we can properly check for GLFW
  list(APPEND SRC_FILES
    src/vgl3d/input/tesla_input_glfw.c
    src/vgl3d/input/tesla_input_mouse_glfw.c
    src/vgl3d/input/tesla_input_gamepad_null.c
  )
  if(APPLE)
    enable_language(OBJC)
    set(CMAKE_OBJC_FLAGS "${CMAKE_OBJC_FLAGS} -fmodules -fno-objc-arc")

    list(APPEND 
      SRC_FILES 
      src/vgl3d/vgl3d_metal/mul/mul.m
      src/vgl3d/vgl3d_metal/vgl3d_metal_private.m
    )

    message("Sources: ${SRC_FILES}")
  endif()
endif()

if(APPLE)
  add_executable(${PROJECT_NAME}
    MACOSX_BUNDLE
    ${SRC_FILES}
  )
  # set_target_properties(${PROJECT_NAME} PROPERTIES
  #     BUNDLE True
  #     MACOSX_BUNDLE_GUI_IDENTIFIER xyz.mikesantiago.${PROJECT_NAME}
  #     MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
  #     MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
  #     MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
  # )
else()
  add_executable(${PROJECT_NAME}
    ${SRC_FILES}
  )
endif()

if(APPLE)
  # target_compile_definitions(${PROJECT_NAME} PUBLIC -DRENDERER_METAL)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
  message("Debug.")
  target_compile_definitions(${PROJECT_NAME} PUBLIC -DDEBUG_BUILD)
endif()

if(BUILD_VITA)
  target_link_libraries(${PROJECT_NAME}
    vitaGL
    debugnet
    SceCtrl_stub
    SceRtc_stub
    SceNetCtl_stub
    SceNet_stub
    SceLibKernel_stub
    ScePvf_stub
    mathneon
    SceAppMgr_stub
    SceAppUtil_stub
    ScePgf_stub
    png
    c
    SceCommonDialog_stub
    m
    z
    SceGxm_stub
    SceDisplay_stub
    SceSysmodule_stub
    SceTouch_stub
    vitashark
    SceShaccCg_stub
    SceShaccCgExt
    taihen_stub
    SceKernelDmacMgr_stub
  )
endif()

if(APPLE)
  message("--- LINKING LIBRARIES FOR MACOS!")

  list(APPEND CMAKE_PREFIX_PATH "/usr/local")
  
  find_library(GLFW glfw)
  find_library(LIBPNG png)
  find_library(CGLM cglm)

  find_package(GLEW REQUIRED)
  
  
  if(NOT GLFW)
    message(FATAL_ERROR "GLFW not found.")
  endif()

  if(NOT LIBPNG)
    message(FATAL_ERROR "libpng not found!")
  endif()

  if(NOT CGLM)
    message(FATAL_ERROR "cglm not found!")
  endif()


  if(GLEW_FOUND)
    message( "GLEW found! GLEW for you!")
  endif()

  target_include_directories(${PROJECT_NAME} PUBLIC ${GLEW_INCLUDE_DIRS})

  target_link_libraries(${PROJECT_NAME}
    "-framework CoreGraphics"
    "-framework MetalKit"
    "-framework Metal"
    "-framework Foundation"
    "-framework CoreVideo"
    "-framework OpenGL"
    "-framework IOKit"
    "-framework Cocoa"
    "-framework Carbon"
    z
    ${GLFW}
    GLEW::glew
  )
elseif(UNIX)
  message("LINUX!")
  
  find_library(GLFW glfw)
  find_library(LIBPNG png)
  find_library(CGLM cglm)
  find_package(GLEW REQUIRED)
  
  
  if(NOT GLFW)
    message(FATAL_ERROR "GLFW not found.")
  endif()

  if(NOT LIBPNG)
    message(FATAL_ERROR "libpng not found!")
  endif()

  if(NOT CGLM)
    message(FATAL_ERROR "cglm not found!")
  endif()


  if(GLEW_FOUND)
    message( "GLEW found! GLEW for you!")
  endif()
  
  target_compile_definitions(${PROJECT_NAME} PUBLIC PC_BUILD)
  target_include_directories(${PROJECT_NAME} PUBLIC ${GLEW_INCLUDE_DIRS})
  
  target_link_libraries(${PROJECT_NAME}
    z
    m
    ${GLFW}
    GLEW::glew
    GLU
    GL
  )
  
endif()

# in VitaSDK, you don't specify SceLibc heapsize with -h, you do it by
# setting `sceLibcHeapSize` in your code

if(BUILD_VITA)
  vita_create_self(eboot.bin ${PROJECT_NAME} UNSAFE)

#
  message("\n\nVita Create VPK with file args: ${data_VPKSHADOW}")
  vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} eboot.bin
    VERSION ${VITA_VERSION}
    NAME ${VITA_APP_NAME}
    ${data_VPKSHADOW}
  )
endif()
