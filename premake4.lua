solution("qor")
    targetdir("bin")
    
    configurations {"Debug", "Release"}

        defines { "GLM_FORCE_RADIANS" }
        
        -- Debug Config
        configuration "Debug"
            defines { "DEBUG" }
            flags { "Symbols" }
            linkoptions { }

            configuration "linux"
                links {
                    "z",
                    "bfd",
                    "iberty"
                }
        
        -- Release Config
        configuration "Release"
            defines { "NDEBUG" }
            flags { "OptimizeSpeed" }
            targetname("qor_dist")

        -- gmake Config
        configuration "gmake"
            buildoptions { "-std=c++11" }
            -- buildoptions { "-std=c++11", "-pedantic", "-Wall", "-Wextra", '-v', '-fsyntax-only'}
            links {
                "pthread",
                "SDL2_ttf",
                "SDL2",
                "GLEW",
                "assimp",
                "freeimage",
                "portaudio",
                "sndfile",
                "boost_system",
                "boost_filesystem",
                "boost_coroutine",
                "boost_python",
                "boost_regex",
                "jsoncpp",
                "RakNetDLL",
                "BulletSoftBody",
                "BulletDynamics",
                "BulletCollision",
                "LinearMath",
            }
            includedirs {
                "lib/qor/",
                "lib/qor/lib/kit",
                "/usr/local/include/",
                "/usr/include/bullet/",
                "/usr/include/raknet/DependentExtensions"
            }

            libdirs {
                "/usr/local/lib"
            }
            
            buildoptions {
                "`python2-config --includes`",
                "`pkg-config --cflags cairomm-1.0 pangomm-1.4`"
            }

            linkoptions {
                "`python2-config --libs`",
                "`pkg-config --libs cairomm-1.0 pangomm-1.4`"
            }
            
        configuration "macosx"
            links {
                "boost_thread-mt",
            }
            buildoptions { "-framework OpenGL" }
            linkoptions { "-framework OpenGL" }
            --buildoptions { "-U__STRICT_ANSI__", "-stdlib=libc++" }
            --linkoptions { "-stdlib=libc++" }

        configuration "linux"
            links {
                "GL",
                "boost_thread",
            }

        configuration "windows"
            links {
                "ws2_32",
                "glibmm.dll",
                "cairomm.dll",
                "pangomm.dll",
                "SDL2main",
                "OpenGL32",
                "GLU32",
                "SDL2",
                "SDL2_ttf",
                "GLEW32",
                "assimp",
                "freeimage",
                "boost_system-vc140-mt-1_61",
                "boost_thread-vc140-mt-1_61",
                "boost_python-vc140-mt-1_61",
                "boost_coroutine-vc140-mt-1_61",
                "boost_regex-vc140-mt-1_61",
                "lib_json",
                "RakNetDLL",
                "BulletSoftBody",
                "BulletDynamics",
                "BulletCollision",
                "LinearMath",
            }

            includedirs {
                "c:/Python27/include",
                "c:/gtkmm/lib/pangomm/include",
                "c:/gtkmm/lib/sigc++/include",
                "c:/gtkmm/lib/cairomm/include",
                "c:/gtkmm/include/pango",
                "c:/gtkmm/include/pangomm",
                "c:/gtkmm/include/sigc++",
                "c:/gtkmm/include",
                "c:/gtkmm/include/cairo",
                "c:/gtkmm/lib/glib/include",
                "c:/gtkmm/include/glib",
                "c:/gtkmm/lib/glibmm/include",
                "c:/gtkmm/include/glibmm",
                "c:/gtkmm/include/cairomm",
                "c:/gtkmm/include",
                "c:/local/boost_1_61_0",
                --"c:/Program Files (x86)/OpenAL 1.1 SDK/include",
                "c:/msvc/include",
            }
            configuration { "windows", "Debug" }
                libdirs {
                    "c:/msvc/lib32/debug"
                }
            configuration { "windows" }
            libdirs {
                --"c:/Program Files (x86)/OpenAL 1.1 SDK/libs/Win32",
                "c:/msvc/lib32",
                "c:/gtkmm/lib",
                "c:/local/boost_1_61_0/lib32-msvc-14.0",
            }
            -- buildoptions {
                -- "/MP",
                -- "/Gm-",
            -- }
            
            configuration { "windows", "Debug" }
                links {
                    "libboost_filesystem-vc140-mt-gd-1_61",
                    "RakNet_VS2008_LibStatic_Debug_Win32",
                }
            configuration {}
            configuration { "windows", "Release" }
                links {
                    "libboost_filesystem-vc140-mt-1_61",
                    "RakNet_VS2008_LibStatic_Release_Win32",
                }

    project "qor"
        kind "WindowedApp"
        language "C++"

        -- Project Files
        files {
            "Qor/**.h",
            "Qor/**.cpp",
            "lib/kit/**.h",
            "lib/kit/**.cpp",
            "lib/coal/**.h",
            "lib/coal/**.cpp",
        }

        -- Exluding Files
        excludes {
            "Qor/tests/**",
            "Qor/scripts/**",
            "Qor/addons/**",
            "lib/kit/tests/**",
            "lib/coal/tests/**",
            "lib/kit/toys/**"
        }
        
        includedirs {
            "lib/kit",
            "lib/coal",
            "/usr/local/include/",
            "/usr/include/bullet/",
            "/usr/include/raknet/DependentExtensions"
        }

