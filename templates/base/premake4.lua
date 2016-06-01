solution("base")
    configurations {"Debug", "Release"}

    targetdir("bin")
    configuration "Debug"
        defines { "DEBUG" }
        --optimize "debug"
        flags { "Symbols" }
    configuration "Release"
        defines { "NDEBUG" }
        --optimize "speed"
        flags { "OptimizeSpeed" }
        targetname("base_dist")

    project("base")
        --uuid("")
        kind("WindowedApp")
        language("C++")
        links {
            "pthread",
            "GL",
            "GLU",
            "SDL2",
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
            "RakNetDLL"
        }
        files {
            "src/**.h",
            "src/**.cpp"
        }
        excludes {
            "src/Qor/Main.cpp",
            "src/Qor/Info.cpp",

                "src/Qor/DemoState.*",
            
            "src/Qor/tests/**",
            "src/Qor/scripts/**",
            "src/Qor/addons/**",
            "src/Qor/shaders/**"
        }
        includedirs {
            "vendor/include/",
            "/usr/local/include/",
            "/usr/local/include/",
            --"/usr/include/cegui-0/",
            "/usr/include/bullet/",
            "/usr/include/raknet/DependentExtensions"
        }
        libdirs {
            --"/usr/lib/cegui-0.8/",
            "/usr/local/lib/",
            "/usr/local/lib64/",
        }
        
        buildoptions {
            "`python2-config --includes`",
            "`pkg-config --cflags cairomm-1.0 pangomm-1.4`"
        }
        linkoptions {
            "`python2-config --libs`",
            "`pkg-config --libs cairomm-1.0 pangomm-1.4`"
            --"`pkg-config --libs cairomm pangomm`"
        }
        configuration {"debug"}
            defines { "BACKWARD_HAS_BFD=1" }
            links {
                "z",
                "bfd",
                "iberty"
            }
            linkoptions { "`llvm-config --libs core` `llvm-config --ldflags`" }
        configuration {}

        --configuration { "linux" }
        --    includedirs {
        --        "/usr/include/lua5.1",
        --    }
        --configuration {}

        configuration { "gmake" }
            buildoptions { "-std=c++11" }
            --buildoptions { "-std=c++11",  "-pedantic", "-Wall", "-Wextra" }
            configuration { "macosx" }
                buildoptions { "-U__STRICT_ANSI__", "-stdlib=libc++" }
                linkoptions { "-stdlib=libc++" }
        configuration {}
        
