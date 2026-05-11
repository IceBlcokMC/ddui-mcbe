add_rules("mode.debug", "mode.release")

add_repositories("levimc-repo https://github.com/LiteLDev/xmake-repo.git")
add_repositories("miracleforest-repo https://github.com/MiracleForest/xmake-repo.git")

option("target_type")
    set_default("server")
    set_showmenu(true)
    set_values("server", "client")
option_end()

option("addon")
    set_default(false)
    set_showmenu(true)
option_end()

option("test")
    set_default(false)
    set_showmenu(true)
option_end()

option("capture_packet")
    set_default(false)
    set_showmenu(true)
option_end()

add_requires("levilamina", {configs = {target_type = get_config("target_type")}})
add_requires("levibuildscript")

if has_config("capture_packet") then
    add_requires("ilistenattentively 0.12.0")
end

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

target("ddui")
    add_rules("@levibuildscript/modpacker")
    add_rules("@levibuildscript/linkrule")
    add_cxflags( "/EHa", "/utf-8", "/W4", "/w44265", "/w44289", "/w44296", "/w45263", "/w44738", "/w45204")
    add_defines("NOMINMAX", "UNICODE", "DDUI_EXPORT")
    add_packages("levilamina")
    set_exceptions("none") -- To avoid conflicts with /EHa.
    set_kind("shared")
    set_languages("c++20")
    set_symbols("debug")

    add_headerfiles("src/(ddui/**.h)")
    add_files("src/**.cc")
    add_includedirs("src")

    if has_config("addon") then 
        add_files("src-addon/**.cc")
        add_includedirs("src-addon")
        add_defines("DDUI_WITH_ADDON")
    end

    if has_config("test") or has_config("capture_packet") then
        add_files("tests/**.cc")
        add_includedirs("tests")
        add_defines("DDUI_WITH_TESTS")
        if has_config("capture_packet") then
            add_packages("ilistenattentively")
            add_defines("DDUI_WITH_CAPTURE_PACKET")
        end
    end

    if is_config("target_type", "server") then
    --  add_includedirs("src-server")
    --  add_files("src-server/**.cpp")
    else
    --  add_includedirs("src-client")
    --  add_files("src-client/**.cpp")
    end
