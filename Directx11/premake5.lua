project "DirectX11"
	kind "StaticLib"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "pch.h"
	pchsource "pch.cpp"

	files {
		"**.h",
		"**.c",
		"**.hpp",
		"**.cpp",
				-- Shaders
				"Shader/**.hlsl",
				"Shader/**.hlslh",
				"Shader/**.slh",
	}
	includedirs  {
		"src/",
		"src/../"
	}
	IncludeDependencies()

	defines { "GLM_FORCE_DEPTH_ZERO_TO_ONE", }
	filter "files:src/ImGui/**.cpp or STB_Image.cpp"
	flags { "NoPCH" }

	filter { "files:**.hlslh" }
	flags "ExcludeFromBuild"
	shadermodel "6.0"
	filter { "files:**-p.hlsl" }
	shadertype "Pixel"
	shadermodel "6.0"
	shaderentry "main"
	filter { "files:**-v.hlsl" }
	shadertype "Vertex"
	shadermodel "6.0"
	shaderentry "main"

	filter { "configurations:Debug or configurations:Debug-AS or configurations:Release" }
		defines {

			"JPH_DEBUG_RENDERER",
			"JPH_FLOATING_POINT_EXCEPTIONS_ENABLED",
			"JPH_EXTERNAL_PROFILE"
		}

		filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		symbols "On"
		defines {  "_DEBUG", "ACL_ON_ASSERT_ABORT", }

	filter { "system:windows", "configurations:Debug-AS" }	
		sanitize { "Address" }
		flags { "NoRuntimeChecks", "NoIncrementalLink" }

	filter "configurations:Release"
		optimize "On"
		vectorextensions "AVX2"
		isaextensions { "BMI", "POPCNT", "LZCNT", "F16C" }
		defines {"NDEBUG" }

	filter { "configurations:Debug or configurations:Debug-AS or configurations:Release" }
		defines {
			"JPH_DEBUG_RENDERER",
			"JPH_FLOATING_POINT_EXCEPTIONS_ENABLED",
			"JPH_EXTERNAL_PROFILE"
		}

	filter "configurations:Dist"
		optimize "On"
		symbols "Off"
	