cmake_minimum_required(VERSION 3.0)

set(CMAKE_C_STANDARD 11)
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

if(BUILD_VITA)
  set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVITA -g -Wall -O0 -fno-lto -ftree-vectorize")
else()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -g -O0 -fno-lto -ftree-vectorize")  
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


# Resource Handling

file(GLOB_RECURSE resources "./*.png" "./*.cgv" "./*.cgf" "res/*.png" "res/*.frag" "res/*.vert" "res/*.glsl")
set(data_VPKSHADOW "")
if(BUILD_VITA)
  foreach(resource ${resources})
    get_filename_component(filename ${resource} NAME)
    
    file(RELATIVE_PATH reldir ${PROJECT_SOURCE_DIR} ${resource})
    string(CONCAT data_VPKSHADOW "${data_VPKSHADOW}FILE;${reldir};${filename};")
  endforeach()
endif()

#message("VPK DATA: ${data_VPKSHADOW}")

add_executable(${PROJECT_NAME}
  # src/main.c
  # src/vgl_renderer.c,
  src/version2/main.c
  src/version2/vgl3d_gl2es.c
)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  message("Debug.")
  target_compile_definitions(${PROJECT_NAME} PUBLIC -DDEBUG_BUILD)
endif()

if(BUILD_VITA)
  target_link_libraries(${PROJECT_NAME}
    vitaGL
    debugnet
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
