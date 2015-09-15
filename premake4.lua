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
        buildoptions { "-std=c11" }
        includedirs "include"
        files { "src/**.c"}

    project "prototype"
        location "build/prototype"
        kind "ConsoleApp"
        language "C"
        buildoptions { "-std=c11" }
        links { "sndnet", "pthread" }
        includedirs "include"
        files { "prototype/**.c" }

    project "test"
        location "build/test"
        kind "ConsoleApp"
        language "C++"
        links { "sndnet", "pthread" }
        includedirs "include"
        files { "test/**.hpp", "test/**.cpp" }
