solution "car2d"
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }
    location "vs2013"
    
	configuration { "x32", "Debug" }
		targetdir "bin/x86/debug/"
        debugdir "bin/x86/debug/"
        libdirs "external/lib/x86/"
		flags { "Symbols" }
	configuration { "x32", "Release" }
		targetdir "bin/x86/release/"
        debugdir "bin/x86/release/"
        libdirs "external/lib/x86/"
		flags { "Optimize" }
	configuration { "x64", "Debug" }
		targetdir "bin/x64/debug/"
        debugdir "bin/x64/debug/"
        libdirs "external/lib/x64/"
		flags { "Symbols" }
	configuration { "x64", "Release" }
		targetdir "bin/x64/release/"
        debugdir "bin/x64/release/"
        libdirs "external/lib/x64/"
		flags { "Optimize" }
	configuration {}
    
    includedirs { "external/include/" }
    buildoptions { "/W4" }
        
    project "car2d_main"
        kind "ConsoleApp"
        language "C++"
        files { "code/car2d_main/**.hpp", "code/car2d_main/**.cpp", "assets/shaders/**.vert", "assets/shaders/**.frag" }
        objdir "build/car2d_main/obj/"
        
        configuration { "Debug" }
            links { "opengl32", "SDL2", "SDL2main", "gl3w", "libyaml-cppmdd", "freetype", "freetype-gl" }
        configuration { "Release" }
            links { "opengl32", "SDL2", "SDL2main", "gl3w", "libyaml-cppmd", "freetype", "freetype-gl" }