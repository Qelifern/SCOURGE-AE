-- include subprojects
includes("lib/commonlibf4")

-- set project constants
set_project("scourge_ae")
set_version("1.0.1")
set_license("MIT")
set_languages("c++23")
set_warnings("allextra")

-- add common rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

add_requires("yaml-cpp")

-- define targets
target("scourge_ae")
    add_rules("commonlibf4.plugin", {
        name = "scourge-ae",
        author = "QELIFERN",
        description = "F4SE plugin using CommonLibF4"
    })
	
	
	add_packages("yaml-cpp")
	
	

    -- add src files
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
	add_headerfiles("include/**.h")
    add_includedirs("src")
	add_includedirs("include")
    set_pcxxheader("src/pch.h")
	
	
