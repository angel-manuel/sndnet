#!lua

solution "sndnet"
    configurations { "Debug", "Release" }
    targetdir "bin"

    configuration "Debug"
        defines "DEBUG"
        flags { "Symbols", "ExtraWarnings" }

    configuration "Release"
        flags { "Optimize" }

    project "sndnet"
        location "build/sndnet"
        kind "StaticLib"
        language "C"
        buildoptions { "-D_POSIX_C_SOURCE=200112L -std=c99 -Wall -Wextra -Werror -Wfatal-errors" }
        includedirs {"include", "third_party/include"}
        files { "src/**.c"}

    project "prototype"
        location "build/prototype"
        kind "ConsoleApp"
        language "C"
        buildoptions { "-std=c99 -Wall -Wextra -Werror -Wfatal-errors" }
        links { "sndnet", "pthread" }
        includedirs {"include", "third_party/include"}
        files { "prototype/**.c" }

    project "test"
        location "build/test"
        kind "ConsoleApp"
        language "C++"
        links { "sndnet", "pthread" }
        includedirs {"include", "third_party/include"}
        files { "test/**.hpp", "test/**.cpp" }
