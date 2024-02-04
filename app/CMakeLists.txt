cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(GCToolKit)
include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "GC ToolKit")
set(VITA_TITLEID  "GCTK10000")

set(VITA_VERSION  "01.00")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS}")
set(VITA_MAKE_FSELF_FLAGS "${VITA_MAKE_FSELF_FLAGS}")

include_directories(
)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(${PROJECT_NAME}
	aes.c
	sha256.c
	sha1.c
	ctrl.c
	draw.c
	device.c
	crypto.c
	kernel.c
	io.c
	gameinfo.c
	sfo.c
	main.c
	menu.c
	ime.c
	net.c
	bgm.cpp
)

target_link_libraries(${PROJECT_NAME}
	vita2d
	f00dbridge_stub
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

  FILE ../kern/build/kplugin.skprx kplugin.skprx
  FILE ../kern/psp2spl/psp2spl.skprx psp2spl.skprx
  
  FILE res/sce_sys/icon0.png sce_sys/icon0.png
  FILE res/sce_sys/pic0.png sce_sys/pic0.png
  FILE res/sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
  FILE res/sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
  
  FILE res/background.png res/background.png
  FILE res/insertgc.png res/insertgc.png
  FILE res/menuoverlay.png res/menuoverlay.png
  FILE res/bgm.ogg res/bgm.ogg  
)
