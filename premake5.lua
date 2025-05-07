include "./vendor/premake_customization/solution_items.lua"
include "Dependances.lua"

workspace "DOEDirectX11"
	configurations { "Debug", "Release", "Dist" }
	targetdir "build"
	startproject "Game"
    conformancemode "On"

	language "C++"
	cppdialect "C++20"
	staticruntime "Off"

	solution_items { ".editorconfig" }

	configurations { "Debug", "Release", "Dist" }

	flags { "MultiProcessorCompile" }

	defines {
		"_CRT_SECURE_NO_WARNINGS",
		"NOMINMAX",
		"_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING",
	}

	filter "language:C++ or language:C"
		architecture "x86_64"

	filter "configurations:Debug"
		optimize "Off"
		symbols "On"

	filter "configurations:Release"
		optimize "On"
		symbols "Default"

	filter "configurations:Dist"
		optimize "Full"
		symbols "Off"

	filter "system:windows"
		buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
include "Dependances/JoltPhysics/JoltPhysicsPremake.lua"
include "Dependances/JoltPhysics/JoltViewerPremake.lua"
include "Dependances/NVRHI/Premake5.lua"
group ""

group "Core"
include "Directx11"
group ""

group "Tools"
include "Game"
group ""

