project "DirectX11"
	kind "StaticLib"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "pch.h"
	pchsource "pch.cpp"

	files {
		"**.h",
		"**.h",
		"**.c",
		"**.hpp",
		"**.cpp",
	}
	includedirs  {
		"{os.getcwd}"
	}
	IncludeDependencies()

	defines { "GLM_FORCE_DEPTH_ZERO_TO_ONE", }

	filter { "configurations:Debug or configurations:Debug-AS or configurations:Release" }
		defines {

			"JPH_DEBUG_RENDERER",
			"JPH_FLOATING_POINT_EXCEPTIONS_ENABLED",
			"JPH_EXTERNAL_PROFILE"
		}

	filter "configurations:Dist"
		optimize "On"
		symbols "Off"
		vectorextensions "AVX2"
	