project "mickstr"
    kind "SharedLib"
    language "C"
        files {
            --./strnum.c,
            "strbuf.h",
            "strnum.c",
            "strview.c",
        }

    -- Required to treat the .h file as .c for stb-style libs.
    filter { "files:**.h" }
        compileas "C"
    filter { }

    usage "PRIVATE"
        links { "m" }
        defines { "STRBUF_IMPLEMENTATION" }
    usage "PUBLIC"
        includedirs { "./" }
    usage "INTERFACE"
        links { "mickstr" }



