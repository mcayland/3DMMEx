# Kauai

if (ENABLE_ASM_X86)
    # Generate Kauai decoders for Intel x86
    add_executable(kcdc-386 EXCLUDE_FROM_ALL)
    target_sources(kcdc-386 PRIVATE "${PROJECT_SOURCE_DIR}/kauai/src/kcdc_386.c")
    target_include_directories(kcdc-386 PRIVATE $<TARGET_PROPERTY:kauai,INCLUDE_DIRECTORIES>)

    add_executable(kcd2-386 EXCLUDE_FROM_ALL)
    target_sources(kcd2-386 PRIVATE "${PROJECT_SOURCE_DIR}/kauai/src/kcd2_386.c")
    target_include_directories(kcd2-386 PRIVATE $<TARGET_PROPERTY:kauai,INCLUDE_DIRECTORIES>)

    add_custom_command(
        OUTPUT "${PROJECT_BINARY_DIR}/generated/kauai/src/kcdc_386.h"
        COMMAND cmake -E make_directory "${PROJECT_BINARY_DIR}/generated/kauai/src"
        COMMAND kcdc-386 "${PROJECT_BINARY_DIR}/generated/kauai/src/kcdc_386.h"
        COMMENT "Generating kcdc_386.h"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${PROJECT_BINARY_DIR}/generated/kauai/src/kcd2_386.h"
        COMMAND cmake -E make_directory "${PROJECT_BINARY_DIR}/generated/kauai/src"
        COMMAND kcd2-386 "${PROJECT_BINARY_DIR}/generated/kauai/src/kcd2_386.h"
        COMMENT "Generating kcd2_386.h"
        VERBATIM
    )
endif()

add_executable(chelp WIN32)
target_sources(chelp PRIVATE
    "${PROJECT_SOURCE_DIR}/kauai/tools/chtop.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/tools/chelp.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/tools/chelpexp.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/tools/chelp.rc"
)
target_include_directories(chelp PRIVATE $<TARGET_PROPERTY:kauai,INCLUDE_DIRECTORIES>)
target_link_libraries(chelp PRIVATE $<TARGET_NAME_IF_EXISTS:kauai>)

# Kauai test applications
add_executable(ft WIN32)
target_sources(ft PRIVATE
    "${PROJECT_SOURCE_DIR}/kauai/src/test.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/ft.rc"
    "${PROJECT_SOURCE_DIR}/kauai/src/ft.cpp"
)
target_link_libraries(ft PRIVATE kauai)

add_executable(ut)
target_sources(ut PRIVATE
    "${PROJECT_SOURCE_DIR}/kauai/src/ut.cpp"
)
target_link_libraries(ut PRIVATE kauai)

add_executable(chomp)
target_sources(chomp PRIVATE "${PROJECT_SOURCE_DIR}/kauai/tools/chomp.cpp")
target_include_directories(chomp PRIVATE $<TARGET_PROPERTY:kauai,INCLUDE_DIRECTORIES>)
target_link_libraries(chomp PRIVATE $<TARGET_NAME_IF_EXISTS:kauai>)

# Chunk Editor
add_executable(ched WIN32)
target_sources(ched PRIVATE
    "${PROJECT_SOURCE_DIR}/kauai/tools/ched.rc"
    "${PROJECT_SOURCE_DIR}/kauai/tools/chdoc.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/tools/ched.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/tools/chgrp.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/tools/chhex.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/tools/chmbmp.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/tools/chpic.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/tools/chtxt.cpp"
)
target_link_libraries(ched PRIVATE kauai)

# mkmbmp
add_executable(mkmbmp)
target_sources(mkmbmp PRIVATE
    "${PROJECT_SOURCE_DIR}/kauai/tools/mkmbmp.cpp"
)
target_link_libraries(mkmbmp PRIVATE kauai)

# kpack
add_executable(kpack)
target_sources(kpack PRIVATE
    "${PROJECT_SOURCE_DIR}/kauai/tools/kpack.cpp"
)
target_link_libraries(kpack PRIVATE kauai)

# chmerge
add_executable(chmerge)
target_sources(chmerge PRIVATE
    "${PROJECT_SOURCE_DIR}/kauai/tools/chmerge.cpp"
)
target_link_libraries(chmerge PRIVATE kauai)

# chelpdmp
add_executable(chelpdmp)
target_sources(chelpdmp PRIVATE
    "${PROJECT_SOURCE_DIR}/kauai/tools/chelpdmp.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/tools/chelpexp.cpp"
)
target_link_libraries(chelpdmp PRIVATE kauai)

add_compile_options($<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/wd4430>)
add_library(kauai)
target_include_directories(
  kauai
    PUBLIC
      "${PROJECT_SOURCE_DIR}/kauai/src"
      "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/generated/kauai/src>")

target_sources(kauai
  PRIVATE
    "${PROJECT_SOURCE_DIR}/kauai/src/appb.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/base.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/chcm.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/chse.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/chunk.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/clip.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/clok.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/cmd.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/codec.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/codkauai.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/crf.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/ctl.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/cursor.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/dlg.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/docb.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/file.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/frame.rc"
    "${PROJECT_SOURCE_DIR}/kauai/src/gfx.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/gob.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/groups.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/groups2.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/kidhelp.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/kidspace.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/kidworld.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/lex.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/mbmp.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/mbmpgui.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/midi.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/mididev.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/mididev2.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/mssio.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/pic.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/region.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/rtxt.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/rtxt2.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/scrcom.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/scrcomg.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/screxe.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/screxeg.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/sndam.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/sndm.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/spell.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/stream.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/text.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/textdoc.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/util.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/utilcopy.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/utilerro.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/utilglob.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/utilint.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/utilmem.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/utilrnd.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/utilstr.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/src/video.cpp"

    # Windows implementations
    $<$<PLATFORM_ID:Windows>:${PROJECT_SOURCE_DIR}/kauai/src/appbwin.cpp>
    $<$<PLATFORM_ID:Windows>:${PROJECT_SOURCE_DIR}/kauai/src/dlgwin.cpp>
    $<$<PLATFORM_ID:Windows>:${PROJECT_SOURCE_DIR}/kauai/src/filewin.cpp>
    $<$<PLATFORM_ID:Windows>:${PROJECT_SOURCE_DIR}/kauai/src/fniwin.cpp>
    $<$<PLATFORM_ID:Windows>:${PROJECT_SOURCE_DIR}/kauai/src/gfxwin.cpp>
    $<$<PLATFORM_ID:Windows>:${PROJECT_SOURCE_DIR}/kauai/src/memwin.cpp>
    $<$<PLATFORM_ID:Windows>:${PROJECT_SOURCE_DIR}/kauai/src/menuwin.cpp>
    $<$<PLATFORM_ID:Windows>:${PROJECT_SOURCE_DIR}/kauai/src/picwin.cpp>
    $<$<PLATFORM_ID:Windows>:${PROJECT_SOURCE_DIR}/kauai/src/gobwin.cpp>

    # Stubs for Visual C++ 2.1 CRT functions
    "${PROJECT_SOURCE_DIR}/kauai/src/stub.cpp"   
)

if (ENABLE_ASM_X86)
    target_sources(
        kauai
        PRIVATE
        "${PROJECT_BINARY_DIR}/generated/kauai/src/kcdc_386.h"
        "${PROJECT_BINARY_DIR}/generated/kauai/src/kcd2_386.h"
    )
endif()

target_compile_definitions(kauai PUBLIC
  _LPCVOID_DEFINED
  STRICT)

target_link_libraries(kauai
  PUBLIC
    3DMMForever::AudioMan
    $<$<PLATFORM_ID:Windows>:Msacm32>
    $<$<PLATFORM_ID:Windows>:Vfw32>
    $<$<PLATFORM_ID:Windows>:Winmm>
    $<$<PLATFORM_ID:Windows>:mpr>)

# Unit tests
add_executable(kauai_test
    "${PROJECT_SOURCE_DIR}/kauai/test/kauai_test.cpp"
    "${PROJECT_SOURCE_DIR}/kauai/test/compression_test.cpp"
)
target_link_libraries(
    kauai_test
    PRIVATE
    kauai
    GTest::gtest_main
)
gtest_discover_tests(kauai_test)