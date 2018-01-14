cmake_minimum_required(VERSION 3.6)

set(CMAKE_CONFIGURATION_TYPES "Debug" "Release")

set(ProjectName szlib)
project(${ProjectName})

set(SOURCES "main.cpp;test.cpp;../szlib.h")

include_directories(AFTER ${CMAKE_CURRENT_SOURCE_DIR})
link_directories("zlib")
set(FILES ${SOURCES})

set(OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}")

add_executable(${ProjectName} ${FILES})

if(MSVC)
    set(DEFAULT_CXX_FLAGS "/DWIN32 /D_WINDOWS /D_MBCS /W4 /WX- /nologo /fp:precise /Zc:wchar_t /TP /Gd")
    if("1800" VERSION_LESS MSVC_VERSION)
        set(DEFAULT_CXX_FLAGS "${DEFAULT_CXX_FLAGS} /EHsc")
    endif()

    set(DEFAULT_CXX_FLAGS_DEBUG "/D_DEBUG /MTd /Zi /Ob0 /Od /RTC1 /Gy /GR- /GS /Gm-")
    set(DEFAULT_CXX_FLAGS_RELEASE "/MT /O2 /GL /GR- /DNDEBUG")
    set(DEFAULT_CXX_LINK_FLAGS_DEBUG "")
    set(DEFAULT_CXX_LINK_FLAGS_RELEASE "/LTCG")

    set(CMAKE_CXX_FLAGS "${DEFAULT_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "${DEFAULT_CXX_FLAGS_DEBUG}")
    set(CMAKE_CXX_FLAGS_RELEASE "${DEFAULT_CXX_FLAGS_RELEASE}")
    set_target_properties(${ProjectName} PROPERTIES
        LINK_FLAGS_DEBUG "${DEFAULT_CXX_LINK_FLAGS_DEBUG}"
        LINK_FLAGS_RELEASE "${DEFAULT_CXX_LINK_FLAGS_RELEASE}")
    target_link_libraries(${ProjectName} "zlibstat.lib")

elseif(UNIX)
    set(DEFAULT_CXX_FLAGS "-Wall -O2 -std=c++11")
    set(CMAKE_CXX_FLAGS "${DEFAULT_CXX_FLAGS}")
    target_link_libraries(${ProjectName} "z")
elseif(APPLE)
endif()

set_target_properties(${ProjectName} PROPERTIES OUTPUT_NAME_DEBUG "${ProjectName}" OUTPUT_NAME_RELEASE "${ProjectName}")
