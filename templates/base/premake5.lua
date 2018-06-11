workspace("base")
    targetdir("bin")
    debugdir("bin")
    
    configurations {"Debug", "Release"}
    
        defines {
            "GLM_FORCE_CTOR_INIT",
            "GLM_ENABLE_EXPERIMENTAL",
            "GLM_FORCE_RADIANS",
            --"QOR_NO_CAIRO",
            --"QOR_NO_PYTHON",
            --"QOR_NO_PHYSICS",
            --"QOR_NO_NET",
            "NOMINMAX"
        }

        -- Debug Config
        configuration "Debug"
            defines { "DEBUG", "BACKWARD_HAS_BFD=1" }
            symbols "On"
            
            symbols "On"
            links {
                "z",
                "bfd",
                "iberty"
            }
            linkoptions { }

        -- Release Config
        configuration "Release"
            defines { "NDEBUG" }
            optimize "Speed"
            floatingpoint "Fast"
            targetname("base_dist")

        -- gmake Config
        configuration "gmake"
            buildoptions { "-std=c++11" }
            -- Uncomment the following line to get in depth debugging 
            -- buildoptions { "-std=c++11", "-pedantic", "-Wall", "-Wextra", '-v', '-fsyntax-only'}

        -- OS X Config
        configuration "macosx"
            buildoptions { "-U__STRICT_ANSI__", "-stdlib=libc++" }
            linkoptions { "-stdlib=libc++" }


    project "base"
        kind "WindowedApp"
        language "C++"
        links {
            "pthread",
            "GL",
            "GLU",
            "SDL2",
            "SDL2_ttf",
            "GLEW",
            "assimp",
            "freeimage",
            "openal",
            "alut",
            "ogg",
            "vorbis",
            "vorbisfile",
            "boost_system",
            "boost_thread",
            "boost_filesystem",
            "boost_python",
            "boost_coroutine",
            "boost_regex",
            "jsoncpp",
            "BulletSoftBody",
            "BulletDynamics",
            "BulletCollision",
            "LinearMath",
            "z",
            "raknet",
        }

        -- Project Files
        files {
            "src/**.h",
            "src/**.cpp",
            "lib/Qor/Qor/**.h",
            "lib/Qor/Qor/**.cpp",
            "lib/Qor/lib/kit/**.h",
            "lib/Qor/lib/kit/**.cpp"
        }

        -- Exluding Files
        excludes {
            "lib/Qor/Qor/Main.cpp",
            "lib/Qor/Qor/Info.cpp",
            "lib/Qor/Qor/DemoState.*",
            "lib/Qor/Qor/tests/**",
            "lib/Qor/Qor/scripts/**",
            "lib/Qor/Qor/addons/**",
            "lib/Qor/lib/kit/tests/**",
            "lib/Qor/lib/kit/toys/**"
        }

        includedirs {
            "lib/Qor/",
            "lib/Qor/lib/kit",
            "/usr/local/include/",
            "/usr/include/bullet/",
            "/usr/include/rapidxml/",
            "/usr/include/raknet/DependentExtensions"
        }

        libdirs {
            "/usr/local/lib",
            "/usr/local/lib64/",
        }
        
        buildoptions {
            "`python2-config --includes`",
            "`pkg-config --cflags cairomm-1.0 pangomm-1.4`"
        }

        linkoptions {
            "`python2-config --libs`",
            "`pkg-config --libs cairomm-1.0 pangomm-1.4`"
        }

