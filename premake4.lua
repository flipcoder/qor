solution("qor")
    configurations {"Debug", "Release"}

    targetdir("bin")

    configuration "debug"
        defines { "DEBUG" }
        flags { "Symbols" }
    configuration "release"
        defines { "NDEBUG" }
        flags { "OptimizeSpeed" }

    project("qor")
        --uuid("")
        kind("WindowedApp")
        language("C++")
        links {
            "pthread",
            "GL",
            "GLU",
            "SDL2",
            "GLEW",
            --"assimp",
            "IL",
            "ILU",
            "openal",
            "alut",
            "ogg",
            "vorbis",
            "vorbisfile",
            "boost_system",
            "boost_filesystem",
            "boost_python",
            "boost_thread",
            "boost_regex",
            "jsoncpp",
            --"RocketCore",
            --"RocketControls",
            "BulletSoftBody",
            "BulletDynamics",
            "BulletCollision",
            "LinearMath",
            "z"
        }
        files {
            "src/**.h",
            "src/**.cpp"
        }
        excludes {
            "src/tests/**",
            "src/addons/**",
            "src/scripts/**",
            "src/shaders/**"
        }
        includedirs {
            "vendor/include/",
            "/usr/local/include/",
            --"/usr/include/cegui-0/",
            "/usr/include/bullet/",
        }
        libdirs {
            "/usr/lib/cegui-0.8/",
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
                "bfd",
                "iberty"
            }
            --buildoptions { "-finstrument-functions" }
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

