###################################################################################################
# CMakeLists.txt - 20140821
# Original generated gencmake.pl, on 2012/07/31 16:43:33
# Lots of things to know about...
# MFC app - Need to add -D_AFXDLL -D_WIN32_WINNT=0x0501, and set(CMAKE_MFC_FLAG 2) 2=DLL 1=static
# __declspec(dllexport|dllimport) - Originally switched by VER_INFO_EXPORTS, but have NOT 
# found a way add_definitions(-DVER_INFO_EXPORTS) just for library. Seems remove_definitions(-DVER_INFO_EXPORTS)
# REMOVES it for BOTH - Could separate the makes, but found cmake added 'Versinfo_EXPORTS' for library, so used this
# The DeclSpec.h does have a -DVER_INFO_STATIC for building as a static, and it seems no problem leaving
# the DllMain() module in the compile...
# precompile headers: not found a way to instruct cmake to 'understand' this, so removed stdafx.cpp|h
# RC/RES compile - seems cmake 'know' what to do with the '*.rc' files, but not for MinGW build.
# UNICODE - Seems need to add_definitions( -DUNICODE -D_UNICODE ) for this to be enabled - default is _MSCS (multi-byte)
#
#
# The cmake build-me.bat
# @setlocal
# @set TMPSRC=..
# @set TMPOPTS=
# cmake %TMPSRC% %TMPOPTS%
# @if ERRORLEVEL 1 goto CMERR1
# cmake --build . --config Debug
# @if ERRORLEVEL 1 goto CMERR2
# cmake --build . --config Release
# @if ERRORLEVEL 1 goto CMERR3
# @echo Look like a successful build
# @echo Proceed to install to C:\MDOS?
# @pause
# cmake --build . --config Release --target INSTALL
# @goto END
# :CERR1
# :CERR2
# :CERR3
# @echo build error
# :END
######################################################################################################
cmake_minimum_required( VERSION 2.6 )
include (CheckIncludeFile)

project( dump4 )

# The version number - is through Dump4Vers.h

# special C:/MDOS install
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "C:/MDOS" CACHE PATH "dump4 install prefix" FORCE)
else()
  set(CMAKE_INSTALL_PREFIX "C:/MDOS" CACHE PATH "dump4 install prefix" FORCE)
endif()

# Allow developer to select is Dynamic or static library built
set( LIB_TYPE STATIC )  # set default static
option( BUILD_SHARED_LIB "Build Shared Library"                                OFF )
# EXPERIMENTAL - Turn ON to combine library into EXE - above shared library MUST be OFF
#option( BUILD_AS_ONE "Build as one. Combine library into EXE" OFF )
option( ADD_BEYE_ELF     "TODO: Set ON to add beye ELF parsing support"         ON )

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(IS_64BIT_BUILD 1)
endif ()

# Do NOT need all this for a pure Windows only build
#if(CMAKE_COMPILER_IS_GNUCXX)
#    set( WARNING_FLAGS -Wall )
#endif(CMAKE_COMPILER_IS_GNUCXX)

#if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang") 
#   set( WARNING_FLAGS "-Wall -Wno-overloaded-virtual" )
#endif() 

if(WIN32 AND MSVC)
    # turn off various warnings - none needed in this compile
    #set(WARNING_FLAGS "${WARNING_FLAGS} /wd4996")
    # foreach(warning 4244 4251 4267 4275 4290 4786 4305)
    if (IS_64BIT_BUILD)
        # C4267: '=' : conversion from 'size_t' to 'DWORD', possible loss of data
        # C4244: 'function' : conversion from '__int64' to 'DWORD', possible loss of data
        # C4018: '<' : signed/unsigned mismatch
        foreach(warning 4018 4267 4244)
            set(WARNING_FLAGS "${WARNING_FLAGS} /wd${warning}")
        endforeach()
    endif ()
    ##
    set( MSVC_FLAGS "-DNOMINMAX -D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -D__CRT_NONSTDC_NO_WARNINGS" )
    # if (${MSVC_VERSION} EQUAL 1600)
    #    set( MSVC_LD_FLAGS "/FORCE:MULTIPLE" )
    # endif (${MSVC_VERSION} EQUAL 1600)
    #set( NOMINMAX 1 )
    #if(MINGW)
        # need to specifically handle rc files, like
        # resource compilation for mingw
        #ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_rc.o
        #                   COMMAND windres.exe -I${CMAKE_CURRENT_SOURCE_DIR}
        #                                       -i${CMAKE_CURRENT_SOURCE_DIR}/test.rc
        #                                       -o ${CMAKE_CURRENT_BINARY_DIR}/test_rc.o)
        #SET(test_EXE_SRCS ${test_EXE_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/test_rc.o)
    #else(MINGW)
        #SET(test_EXE_SRCS ${test_EXE_SRCS} test.rc)
    #endif(MINGW)
#else()
#    message( FATAL_ERROR "This program can ONLY be compiled in Windows!" )
endif()

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS} ${MSVC_FLAGS} -D_REENTRANT" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS} ${MSVC_FLAGS} -D_REENTRANT" )
set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MSVC_LD_FLAGS}" )

#add_definitions( -DHAVE_CONFIG_H )
add_definitions( -DDUMP4 )

# to distinguish between debug and release lib
set( CMAKE_DEBUG_POSTFIX "d" )

include_directories ( "${PROJECT_SOURCE_DIR}" )

# presently ONLY building an EXE so...
#if(BUILD_SHARED_LIB)
#   set(LIB_TYPE SHARED)
#   message("*** Building DLL library ${LIB_TYPE}")
#else(BUILD_SHARED_LIB)
#   message("*** Building static library ${LIB_TYPE}")
#endif(BUILD_SHARED_LIB)

check_include_file(stdint.h     HAVE_STDINT_H)

# build a config.h from config_cmake.h.in
configure_file ( "${PROJECT_SOURCE_DIR}/config_cmake.h.in" "${PROJECT_BINARY_DIR}/config.h" )
include_directories ( "${PROJECT_BINARY_DIR}" )  # for config.h, ...
add_definitions ( -DHAVE_CONFIG_H )

# for service __imp__UnDecorateSymbolName@16
list(APPEND add_LIBS Dbghelp.lib)

# dump4 EXECUTABLE from [C:\GTools\ConApps\dump4],
# have 39 C/C++ sources, 39 headers
if (IS_64BIT_BUILD)
    set(name dump4)
    message(STATUS "*** 64-bit build of ${name}")
    add_definitions( -DIS_64BIT_BUILD )
else ()
    set(name dump432)
    message(STATUS "*** 32-bit build of ${name}")
endif ()
set(dir src)
set(${name}_SRCS
    ${dir}/D4Bmp.c
    ${dir}/Dump4.c
    ${dir}/Dump4Cab.c
    ${dir}/Dump4g.c
    ${dir}/Dump4v.c
    ${dir}/DumpASCII.c
    ${dir}/DumpAVI.c
    ${dir}/DumpArch.cxx
    ${dir}/DumpBmp.c
    ${dir}/DumpCOFF.cxx
    ${dir}/DumpCode.c
    ${dir}/DumpDFS.c
    ${dir}/DumpEXE.c
    ${dir}/DumpElf.cxx
    ${dir}/DumpFile.c
    ${dir}/DumpGif.c
    ${dir}/DumpHelp.c
    ${dir}/DumpHex.c
    ${dir}/DumpLNK.c
    ${dir}/DumpLib.c
    ${dir}/DumpM2TS.c
    ${dir}/DumpMk4.c
    ${dir}/DumpOS2.c
    ${dir}/DumpObj.c
    ${dir}/DumpPE.cxx
    ${dir}/DumpPE2.cxx
    ${dir}/DumpRGB.c
    ${dir}/DumpSHP.c
    ${dir}/DumpSonic.c
    ${dir}/DumpSynE.c
    ${dir}/DumpTar.c
    ${dir}/DumpTar2.c
    ${dir}/DumpTar4.c
    ${dir}/DumpTarSp.c
    ${dir}/DumpTarX.c
    ${dir}/DumpUtil.c
    ${dir}/DumpWAV.c
    ${dir}/Dumpcis.c
    ${dir}/Intel.c 
    )
set(${name}_HDRS
    ${dir}/CIS.H
    ${dir}/Dump4.h
    ${dir}/Dump4Cab.h
    ${dir}/Dump4fdi.h
    ${dir}/DumpASCII.h
    ${dir}/DumpAVI.h
    ${dir}/DumpArch.h
    ${dir}/DumpBmp.h
    ${dir}/DumpCOFF.h
    ${dir}/DumpCode.h
    ${dir}/DumpEXE.h
    ${dir}/DumpElf.h
    ${dir}/DumpFile.h
    ${dir}/DumpGif.h
    ${dir}/DumpHelp.h
    ${dir}/DumpHex.h
    ${dir}/DumpLNK.h
    ${dir}/DumpLib.h
    ${dir}/DumpList.h
    ${dir}/DumpM2TS.h
    ${dir}/DumpMk4.h
    ${dir}/DumpOS2.h
    ${dir}/DumpObj.h
    ${dir}/DumpPE.h
    ${dir}/DumpRGB.h
    ${dir}/DumpSHP.h
    ${dir}/DumpSonic.h
    ${dir}/DumpStruc.h
    ${dir}/DumpSynE.h
    ${dir}/DumpTar.h
    ${dir}/DumpTar2.h
    ${dir}/DumpTarOb.h
    ${dir}/DumpTarX.h
    ${dir}/DumpUtil.h
    ${dir}/DumpVers.h
    ${dir}/DumpWork.h
    ${dir}/Intel.h
    ${dir}/elffile.h
    ${dir}/headers.h 
    )
if (ADD_BEYE_ELF)
    list(APPEND ${name}_SRCS ${dir}/beyebstr.cxx ${dir}/beyebpkt.cxx ${dir}/beyeELF.cxx)
    list(APPEND ${name}_HDRS ${dir}/beyebstr.h   ${dir}/beyebpkt.h   ${dir}/beyeELF.h
        ${dir}/beyebswap.h ${dir}/beye_defs.h)
    add_definitions( -DDUMP4_BEYE_ELF )
    ### add_definitions( -DENABLE_BEYE_ELF ) # when completed!
endif ()
# add the generated config.h
list(APPEND ${name}_HDRS "${PROJECT_BINARY_DIR}/config.h")  
   
add_executable( ${name} ${${name}_SRCS} ${${name}_HDRS} )
target_link_libraries( ${name} ${add_LIBS} )
set_target_properties( ${name} PROPERTIES DEBUG_POSTFIX d )
# install - CMAKE_INSTALL_PREFIX / DESTDIR
install (TARGETS ${name} DESTINATION "C:/MDOS")
message(STATUS "*** CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")

# eof
