cmake_minimum_required(VERSION 2.6)

project(vCardTool)

if (MINGW)
    set(_EXTRA_DEF
        -DNTDDI_VERSION=0x06000000
        -DWINVER=0x500
        )
    set(_EXTRA_FLAGS "-Wl,--subsystem,windows")
elseif (MSVC)
    set(_EXTRA_FLAGS "/EHa")
endif()

add_definitions(
    ${_EXTRA_DEF}
    -D_UNICODE
    -DUNICODE
    )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_EXTRA_FLAGS}")

add_subdirectory(vCardTool)
