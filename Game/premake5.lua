project "Game"
kind "ConsoleApp"

targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")
shaderobjectfileoutput ("CompiledShaders/%%(Filename).cso")

	links { "DirectX11","d3d12.lib",
        "dxgi.lib","d3dcompiler.lib","DirectXTK.lib","JoltPhysics.lib", }

	defines { "GLM_FORCE_DEPTH_ZERO_TO_ONE", }

	files  { 
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
		"../Directx11/src",
		"../Directx11",
	}

	filter { "files:**.hlslh" }
		flags "ExcludeFromBuild"
		shadermodel "5.0"

	filter { "files:**_p.hlsl" }
		shadertype "Pixel"
		shadermodel "5.0"
		
		shaderentry "main"
	filter { "files:**_v.hlsl" }
		shadertype "Vertex"
		shadermodel "5.0"
		shaderentry "main"
	filter { "system:windows", "configurations:Debug or configurations:Debug-AS" }
		postbuildcommands {
			'{COPY} "../Dependances/assimp/bin/windows/Debug/assimp-vc143-mtd.dll" "%{cfg.targetdir}"',
		}

	filter { "system:windows", "configurations:Release or configurations:Dist" }
		postbuildcommands {
			'{COPY} "../Dependances/assimp/bin/windows/Release/assimp-vc143-mt.dll" "%{cfg.targetdir}"',
		}

		result, err = os.outputof("pkg-config --libs gtk+-3.0")
		linkoptions {
			result,
			"-pthread",
			"-ldl"
		}

	filter "configurations:Debug"
		symbols "On"
		ProcessDependencies("Debug")

	filter "configurations:Release"
		optimize "On"
        vectorextensions "AVX2"

		ProcessDependencies("Release")

	filter "configurations:Debug or configurations:Release"
		defines {
			
            "JPH_DEBUG_RENDERER",
            "JPH_FLOATING_POINT_EXCEPTIONS_ENABLED",
			"JPH_EXTERNAL_PROFILE"
		}

	filter "configurations:Dist"
        kind "None"
		optimize "On"
        vectorextensions "AVX2"

		ProcessDependencies("Dist")