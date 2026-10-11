project('mickstr', 'c', default_options : ['c_std=gnu11'])

if not meson.is_subproject()
  MY_WARNINGS = [
    '-Wall',
    '-Wextra',
    '-Wpedantic',
    '-Werror',
    '-Wshadow',
    '-Wno-language-extension-token',

    # Preferences.
    '-Wno-error=unused-parameter',
    '-Wno-error=unused-function',
    '-Wno-error=unused-variable',
    '-Wno-unused-function',
    '-Wno-gnu-statement-expression',
  ]

  add_project_arguments(MY_WARNINGS, language: 'c')
endif


cc = meson.get_compiler('c')
math_dep = cc.find_library('m', required : false)
mickstr_includes = include_directories('../')
mickstr_lib = library('mickstr',
  '../../mickstr_implementation.c',
  dependencies: math_dep,
  include_directories: mickstr_includes,
)
mickstr_dep = declare_dependency(
  include_directories: mickstr_includes,
  link_with: mickstr_lib
)

# Tests.

if not meson.is_subproject()

  strbuf_test = executable('strbuf_test', './strbuf_test.c', dependencies : [ mickstr_dep ] )
  test('strbuf_test', strbuf_test)
  strview_test = executable('strview_test', './strview_test.c', dependencies : [ mickstr_dep ] )
  test('strview_test', strview_test)

endif

