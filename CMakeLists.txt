cmake_minimum_required(VERSION 3.13)
project(uemacs C)

FIND_PROGRAM(CRANKER cranker_generic PATHS "/usr/local/bin")
IF(CRANKER AND CMAKE_C_COMPILER STREQUAL "/opt/amiga/gcc6/bin/m68k-amigaos-gcc")
    SET(CRANKER_COMMAND ${CRANKER} -f ${CMAKE_CURRENT_BINARY_DIR}/uemacs -o ${CMAKE_CURRENT_BINARY_DIR}/em)
ENDIF()

message(${CRANKER} ${CMAKE_TA})

add_compile_options(-D__ALKIS__)
if (CMAKE_C_COMPILER STREQUAL "/opt/amiga/gcc6/bin/m68k-amigaos-gcc")
    add_compile_options(-mcrt=nix13 -DAMIGA=1 -DUSEINLINE -fbaserel -fomit-frame-pointer -Wno-pointer-sign -Wall -DAUTOCONF -DPOSIX -D_BSD_SOURCE -D_SVID_SOURCE -D_XOPEN_SOURCE=600 )
    set(CMAKE_EXE_LINKER_FLAGS "-s -mcrt=nix13 -fbaserel" CACHE INTERNAL "" FORCE)
    #add_compile_options(-fbaserel -fomit-frame-pointer -Wall)
    #set(CMAKE_EXE_LINKER_FLAGS "-s -fbaserel -noixemul " CACHE INTERNAL "" FORCE)
    else()
    add_compile_options(-DLINUX -DAUTOCONF -DPOSIX -DUSG -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=600)
    #set(CMAKE_VERBOSE_MAKEFILE ON)
endif(CMAKE_C_COMPILER STREQUAL "/opt/amiga/gcc6/bin/m68k-amigaos-gcc")


set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(uemacs
        amigados.c
        ansi.c
        basic.c
        bind.c
        buffer.c
        crypt.c
        display.c
        ebind.h
        edef.h
        efunc.h
        epath.h
        estruct.h
        eval.c
        evar.h
        exec.c
        file.c
        fileio.c
        globals.c
        ibmpc.c
        input.c
        isearch.c
        line.c
        line.h
        lock.c
        main.c
        names.c
        pklock.c
        posix.c
        random.c
        region.c
        search.c
        spawn.c
        tcap.c
        termio.c
        usage.c
        usage.h
        utf8.c
        utf8.h
        util.h
        version.c
        version.h
        vmsvt.c
        vt52.c
        window.c
        word.c
        wrapper.c
        wrapper.h)


if (CMAKE_C_COMPILER STREQUAL "/opt/amiga/gcc6/bin/m68k-amigaos-gcc")

else()
    target_link_libraries(uemacs "ncurses")
endif(CMAKE_C_COMPILER STREQUAL "/opt/amiga/gcc6/bin/m68k-amigaos-gcc")

add_custom_command(TARGET uemacs
        POST_BUILD
        COMMAND ${CRANKER_COMMAND}
        COMMAND cranker_generic -f uemacs -o em
        COMMAND ls -l uemacs em
        )