cmake_minimum_required(VERSION 2.8)

project(lazyusf)

set(CMAKE_C_FLAGS "-g -D_GNU_SOURCE -fPIC -fno-strict-aliasing  -fmessage-length=0 -Wall -fstack-protector -funwind-tables -fasynchronous-unwind-tables")
set(LD_FLAGS "")

find_library(FLAC_LIBRARIES FLAC)
find_library(AO_LIBRARIES ao)

if(UNIX AND NOT AO_LIBRARIES)
    message(ERROR "Could not find libao. On-the-Fly playback won't be available. On Linux systems try installing: libao-devel (RPM based systems) or libao-dev (DEB based systems)")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPLAYBACK_SUPPORT")
    set(LD_FLAGS ${LD_FLAGS} ${AO_LIBRARIES})
endif(UNIX AND NOT AO_LIBRARIES)

if(UNIX AND NOT FLAC_LIBRARIES)
    message(ERROR "Could not find libFLAC. Writing directly to FLAC won't be possible. On Linux systems try installing: flac-devel (RPM based systems) or flac-dev (DEB based systems)")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFLAC_SUPPORT")
    set(LD_FLAGS ${LD_FLAGS} ${FLAC_LIBRARIES})
endif(UNIX AND NOT FLAC_LIBRARIES)

set(LD_FLAGS ${LD_FLAGS} -lm)

set(LAZYUSF_SRC
    audio.c
    cpu.c
    dma.c
    exception.c
    interpreter_cpu.c
    interpreter_ops.c
    main.c
    memory.c
    pif.c
    psftag.c
    recompiler_cpu.c
    registers.c
    rsp.c
    rsp_interpreter_cpu.c
    rsp_mmx.c
    rsp_sse.c
    rsp_x86.c
    tlb.c
    usf.c
    x86.c
    x86_fpu.c
)

add_executable(lazyusf ${LAZYUSF_SRC})

target_link_libraries(lazyusf ${LD_FLAGS})