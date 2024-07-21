add_rules("mode.debug", "mode.release")
if is_mode("debug")
then 
    add_defines("DEBUG")
end 
-- target("Sysy2023")
    -- set_kind("binary")
    -- add_files("src/*.cpp")

-- target("lextest")
--     set_kind("binary")
--     set_languages( "c++17")
--     add_includedirs("include")
--     add_files("src/frontend/*.cpp")
--     add_files("src/test/testlex.cpp")
--     set_optimize("none")

-- target("asttest")
--     set_kind("binary")
--     set_languages( "c++17")
--     add_includedirs("include")
--     add_files("src/frontend/*.cpp")
--     add_files("src/test/testast.cpp")
--     set_optimize("none")



target("passtest")
    set_kind("binary")
    set_languages( "c++17")
    add_includedirs("include")
    add_files("src/frontend/*.cpp")
    add_files("src/midend/*.cpp")
    add_files("src/backend/*")
    add_files("src/analysis/*.cpp")
   -- add_files("src/backend/*.cpp")
    add_files("src/test/testir.cpp")
    set_optimize("none")

target("targettest")
    set_kind("binary")
    set_languages( "c++17")
    add_includedirs("include")
    add_files("src/frontend/*.cpp")
    add_files("src/midend/*.cpp")
    add_files("src/backend/*")
    add_files("src/analysis/*.cpp")
    add_files("src/optimization/*.cpp")   -- ignore other opt pass!!!
    add_files("src/test/testpass.cpp")
    set_optimize("none")

target("testAnalysis")
    set_kind("binary")
    set_languages( "c++17")
    add_includedirs("include")
    add_files("src/frontend/*.cpp")
    add_files("src/backend/*")
    add_files("src/midend/*.cpp")
    add_files("src/analysis/*.cpp")
    add_files("src/optimization/*.cpp")   -- ignore other opt pass!!!
    add_files("src/test/testAnalysis.cpp")
    set_optimize("none")

target("compiler")
    set_kind("binary")
    set_languages( "c++17")
    add_includedirs("include")
    add_files("src/frontend/*.cpp")
    add_files("src/midend/*.cpp")
    add_files("src/analysis/*.cpp")
    add_files("src/optimization/*.cpp")
    add_files("src/compiler.cpp")
    set_optimize("none")
--
--
-- If you want to known more usage about xmake, please see https://xmake.io
--
-- ## FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add debug and release modes
--    add_rules("mode.debug", "mode.release")
--
--    -- add macro definition
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "c++11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add system link libraries
--    add_syslinks("z", "pthread")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--

