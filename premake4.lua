#!lua

solution "sndnet"
	configurations { "Debug", "Release" }
	targetdir "bin"
	buildoptions { "-std=c11" }

	configuration "Debug"
		defines "DEBUG"
		flags { "Symbols" }

	configuration "Release"
		flags { "Optimize" }

	project "sndnet"
		location "build/sndnet"
		kind "StaticLib"
		language "C"
		includedirs "include"
		files { "src/**.c"}

	project "prototype"
		location "build/prototype"
		kind "ConsoleApp"
		language "C"
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
