run_tests = function (bin_path, test_list)
    local ANSI_RES,ANSI_RED,ANSI_YEL = "\27[0m","\27[31m","\27[33m"
    local total_passed = 0
    local test_res = { }
    table.foreachi(table.keys(test_list), function(index)
        test_res[index] = os.execute(bin_path .. test_list[index]) == true
    end)
    printf("\n---");
    table.foreachi(table.keys(test_list), function(index)
        local passed = test_res[index]
        printf("%s%s [%s].", (passed and ANSI_YEL.."OK  " or ANSI_RED.."FAIL"), ANSI_RES, test_list[index])
        if passed then total_passed = total_passed + 1 end
    end)
    printf("%s%d/%d Test passed.\n---", ANSI_RES, total_passed, #test_list);
    os.exit((#test_list == total_passed) and 0 or 1)   -- Return error on failure.
end

enable_coverage = function()
    -- Enable coverage support.
    filter "toolset:gcc" buildoptions { "-fprofile-arcs", "-ftest-coverage" } linkoptions { "--coverage" }
    --filter "toolset:clang" buildoptions { "???" }
    filter { }
end

setup_test = function(test_name, test_file)
    project (test_name) kind "ConsoleApp" language "C" uses "mickstr" files { test_file }
    -- Enable compiler color diagnostics.
    filter "toolset:gcc" buildoptions { "-fdiagnostics-color=always" }
    filter "toolset:clang" buildoptions { "-fcolor-diagnostics" }
    filter { }
    enable_coverage()
end

local MY_WARNINGS = {
    '-Wall',
    '-Wextra',
    '-Wpedantic',
    '-Werror',
    '-Wshadow',
    '-Wno-language-extension-token',

    -- Preferences.
    '-Wno-error=unused-parameter',
    '-Wno-error=unused-function',
    '-Wno-error=unused-variable',
    '-Wno-unused-function',
    '-Wno-gnu-statement-expression',
}

workspace "main"
    configurations "Debug"
    location "build"
    cdialect "gnu11"
    buildoptions { MY_WARNINGS }
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"                                -- -g
        optimize "Debug"                            -- -Og
        omitframepointer "On"                       -- -fno-omit-frame-pointer
        sanitize { "Address", "UndefinedBehavior" } -- -fsanitize=address,undefined

include "../"     -- Include the library.
project "mickstr"
    enable_coverage()


setup_test ( "test_strbuf", "strbuf_test.c" )
setup_test ( "test_strview", "strview_test.c" )
test_list = {
    "test_strbuf",
    "test_strview",
}

newaction {
    trigger = "test",
    description = "Run the automated test suite",
    execute = function () run_tests ("./build/bin/Debug/", test_list) end
}
