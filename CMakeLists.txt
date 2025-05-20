cmake_minimum_required(VERSION 3.11)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(GcToolKit)
include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "GC ToolKit")
set(VITA_TITLEID  "GCTK10000")
set(VITA_VERSION  "01.05")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS}")
set(VITA_MAKE_FSELF_FLAGS "${VITA_MAKE_FSELF_FLAGS}")

add_subdirectory(kern)

include_directories(
	kern
)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/kern
)

add_executable(${PROJECT_NAME}
	app/aes.c
	app/aes_cmac.c
	app/sha256.c
	app/sha1.c
	app/ctrl.c
	app/draw.c
	app/device.c
	app/crypto.c
	app/kernel.c
	app/io.c
	app/gameinfo.c
	app/sfo.c
	app/main.c
	app/menu.c
	app/ime.c
	app/net.c
	app/gc_ident.c
	app/bgm.cpp
)

add_dependencies(GcToolKit GcKernKit-stubs)
add_dependencies(GcToolKit GcKernKit.skprx-self)

target_link_libraries(${PROJECT_NAME}
	GcKernKit_stub
	vita2d
	taihen_stub

	SceSysmodule_stub
	SceNet_stub
	SceNetCtl_stub_weak
	SceDisplay_stub
	SceGxm_stub
	ScePgf_stub
	ScePvf_stub
	SceCommonDialog_stub
	SceCtrl_stub
	SceVshBridge_stub
	SceAppMgr_stub
	SceAppUtil_stub
	SceIofilemgr_stub
	ScePower_stub
	SceRegistryMgr_stub
	SceShellSvc_stub
	SceAppUtil_stub
	SceIme_stub
	SceAudio_stub

	soloud
	pthread
	stdc++
	freetype
	png
	jpeg
	m
	z
)

vita_create_self(eboot.bin ${PROJECT_NAME} UNSAFE)

vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} eboot.bin
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}

  FILE ${CMAKE_CURRENT_BINARY_DIR}/kern/GcKernKit.skprx GcKernKit.skprx
  
  FILE app/res/sce_sys/icon0.png sce_sys/icon0.png
  FILE app/res/sce_sys/pic0.png sce_sys/pic0.png
  FILE app/res/sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
  FILE app/res/sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
  
  FILE app/res/background.png res/background.png
  FILE app/res/insertgc.png res/insertgc.png
  FILE app/res/menuoverlay.png res/menuoverlay.png
  FILE app/res/bgm.ogg res/bgm.ogg  
)
