-- Build order: common.lib (common) -> svim.lib (virtual_machine) -> svim.exe (interpreter)

local SVIM_DEBUG = "SVIM_DEBUG"

workspace "svim"
    configurations { "Debug", "Release" }
    platforms { "x86", "x64" }
    location "build/"
    startproject "tests"

    project "common"
        kind "StaticLib"
        targetname "svim_common"
        location ("build/%{prj.name}/" .. _ACTION)
        targetdir "bin/%{prj.name}/%{prj.cfg}"
        objdir "obj/"
        language "C++"
        cppdialect "C++20"
        files {
            "src/pch.h",
            "src/pch.cpp",
            "src/common/*.h",
            "src/common/*.cpp"
        }
        includedirs {
            "src/",
            "src/common/",
            "src/virutal_machine/"
        }
        pchheader "pch.h"
        pchsource "pch.cpp"
        filter "configurations:Debug"
            symbols "On"
            defines (SVIM_DEBUG)

    project "virtual_machine"
        kind "StaticLib"
        targetname "svim"
        location ("build/%{prj.name}/" .. _ACTION)
        targetdir "bin/%{prj.name}/%{prj.cfg}"
        objdir "obj/"
        language "C++"
        cppdialect "C++20"
        files {
            "src/pch.h",
            "src/pch.cpp",
            "src/virtual_machine/*.h",
            "src/virtual_machine/*.cpp"
        }
        includedirs {
            "src/",
            "src/virtual_machine/",
            "src/interpreter/",
            "src/common/"
        }
        pchheader "pch.h"
        pchsource "pch.cpp"
        links "svim_common"
        libdirs "bin/common/%{prj.cfg}"
        dependson "common"
        filter "configurations:Debug"
            symbols "On"
            defines (SVIM_DEBUG)

    project "interpreter"
        kind "ConsoleApp"
        targetname "svim"
        location ("build/%{prj.name}/" .. _ACTION)
        targetdir "bin/%{prj.name}/%{prj.cfg}"
        objdir "obj/"
        language "C++"
        cppdialect "C++20"
        files {
            "src/pch.h",
            "src/pch.cpp",
            "src/interpreter/*.h",
            "src/interpreter/*.cpp"
        }
        includedirs {
            "src/",
            "src/virtual_machine/",
            "src/interpreter/",
            "src/common/"
        }
        pchheader "pch.h"
        pchsource "pch.cpp"
        links "svim"
        libdirs "bin/virtual_machine/%{prj.cfg}"
        dependson "virtual_machine"
        filter "configurations:Debug"
            symbols "On"
            defines (SVIM_DEBUG)
    
    project "tests"
        kind "ConsoleApp"
        targetname "svim_tests"
        location ("build/%{prj.name}/" .. _ACTION)
        targetdir "bin/%{prj.name}/%{prj.cfg}"
        objdir "obj/"
        language "C++"
        cppdialect "C++20"
        files {
            "src/**.h",
            "src/**.cpp",
            "tests/**.h",
            "tests/**.cpp"
        }
        removefiles {
            "src/interpreter/main.cpp",
            "pch.cpp"
        }
        includedirs {
            "src/",
            "src/virtual_machine/",
            "src/interpreter/",
            "src/common/",
            "tests/cases/",
            "tests/examples/"
        }
        filter "configurations:Debug"
            symbols "On"
            defines (SVIM_DEBUG)
