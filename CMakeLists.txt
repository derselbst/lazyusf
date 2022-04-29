cmake_minimum_required(VERSION 2.8...3.22)

project(lazyusf)

if(UNIX)
    option(BUILD_STATIC "Build statically linked binary" OFF)
    if(BUILD_STATIC)
        set(BUILD_SHARED_LIBS OFF)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
    endif()
endif()

set(CMAKE_C_FLAGS_DEBUG "-g3 -Wall -Wextra")
# no compiler optimizations, since it breaks recompiler on i586
set(CMAKE_C_FLAGS "-D_GNU_SOURCE -fno-strict-aliasing -fstack-protector -funwind-tables -fasynchronous-unwind-tables -Wno-unused-result")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_library(FLAC_LIBRARIES FLAC)
find_library(AO_LIBRARIES ao)

if(UNIX AND NOT AO_LIBRARIES)
    message(ERROR "Could not find libao. On-the-Fly playback won't be available. On Linux systems try installing: libao-devel (RPM based systems) or libao-dev (DEB based systems)")
elseif(NOT BUILD_STATIC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPLAYBACK_SUPPORT")
    set(LD_FLAGS ${LD_FLAGS} ao pthread)
endif()

if(UNIX AND NOT FLAC_LIBRARIES)
    message(ERROR "Could not find libFLAC. Writing directly to FLAC won't be possible. On Linux systems try installing: flac-devel (RPM based systems) or libflac-dev (DEB based systems)")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFLAC_SUPPORT")
    set(LD_FLAGS ${LD_FLAGS} FLAC ogg m)
endif()


set(LAZYUSF_SRC
    audio.c
    audio_hle_main.c
    audio_ucode1.c
    audio_ucode2.cpp
    audio_ucode3.cpp
    audio_ucode3mp3.cpp
    cpu.c
    dma.c
    exception.c
    interpreter_cpu.c
    interpreter_ops.c
    interpreter_memory.c
    main.c
    memory.c
    pif.c
    psftag.c
    recompiler_cpu.c
    recompiler_fpu_ops.c
    recompiler_ops.c
    recompiler_memory.c
    registers.c
    rsp.c
    rsp_interpreter_cpu.c
    rsp_mmx.c
    rsp_recompiler_analysis.c
    rsp_recompiler_cpu.c
    rsp_recompiler_ops.c
    rsp_sse.c
    rsp_x86.c
    tlb.c
    usf.c
    x86.c
    x86_fpu.c
)

add_executable(lazyusf ${LAZYUSF_SRC})

target_link_libraries(lazyusf ${LD_FLAGS})
install(TARGETS lazyusf RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
