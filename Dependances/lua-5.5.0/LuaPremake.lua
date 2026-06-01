project "LuaLib"
    kind "StaticLib"
    cdialect "C99"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "src/**.c",
        "src/**.h",
    }
    removefiles
    {
        "src/lua.c",
        "src/luac.c",
    }
    filter "system:windows"
        systemversion "latest"

        

    filter "configurations:Debug"
        symbols "on"
        optimize "off"

    filter "configurations:Release"
        optimize "speed"
        vectorextensions "AVX2"
        isaextensions { "BMI", "POPCNT", "LZCNT", "F16C" }

    filter "configurations:Dist"
        optimize "speed"
        symbols "off"
        vectorextensions "AVX2"
        isaextensions { "BMI", "POPCNT", "LZCNT", "F16C" }

--[[project "JoltPhysics-Samples"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"
    flags { "ExcludeFromBuild" }

    files
    {
        "JoltPhysics/Samples/**.cpp",
        "JoltPhysics/Samples/**.h",
        "JoltPhysics/Samples/**.inl",
        "JoltPhysics/Samples/**.gliffy"
    }

    includedirs { "JoltPhysics/Jolt", "JoltPhysics/Samples", "JoltPhysics/TestFramework", "JoltPhysics/" }

    defines
    {
        "JPH_DEBUG_RENDERER",
        "JPH_FLOATING_POINT_EXCEPTIONS_ENABLED",
        "JPH_USE_LZCNT",
        "JPH_USE_TZCNT",
        "JPH_USE_FMADD"
    }

    filter "system:windows"
        systemversion "latest"
        
    filter "configurations:Debug"
        symbols "on"
        optimize "off"

    filter "configurations:Release"
        optimize "speed"

    filter "configurations:Dist"
        optimize "speed"
        symbols "off"    
]]--