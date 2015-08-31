#!lua

solution "sndnet"
	configurations { "Debug", "Release" }
	targetdir "bin"

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

	project "testclient"
		location "build/testclient"
		kind "ConsoleApp"
		language "C"
		links "sndnet"
		includedirs "include"
		files { "testclient/**.c" }

	project "test"
		location "build/test"
		kind "ConsoleApp"
		language "C++"
		links "sndnet"
		includedirs "include"
		files { "test/**.hpp", "test/**.cpp" }
