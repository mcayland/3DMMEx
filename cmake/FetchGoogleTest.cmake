include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/e39786088138f2749d64e9e90e0f9902daa77c40.zip # v1.15.0
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
