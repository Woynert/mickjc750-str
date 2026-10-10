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

#woycontainer_includes = include_directories('../')
#olivec_lib = library('mickstr',
  #'./strview.c',
  #include_directories: olivec_includes,
#)
#woycontainer_dep = declare_dependency(
  #include_directories: woycontainer_includes
#)

cc = meson.get_compiler('c')
math_dep = cc.find_library('m', required : false)
mickstr_includes = include_directories('../')
mickstr_lib = library('mickstr',
  '../../mickstr_implementation.c',
  dependencies: math_dep,
  include_directories: mickstr_includes,
  #c_args: [ '-Wall', '-Wextra' ]
)
mickstr_dep = declare_dependency(
  include_directories: mickstr_includes,
  link_with: mickstr_lib
)

# Tests.

if not meson.is_subproject()

  #MY_WARNINGS = [
    #'-Wall',
    #'-Wextra',
    #'-Wpedantic',
    #'-Werror',
    #'-Wshadow',
    #'-Wno-language-extension-token',

    ## Preferences.
    #'-Wno-error=unused-parameter',
    #'-Wno-error=unused-function',
    #'-Wno-error=unused-variable',
    #'-Wno-unused-function',
    #'-Wno-gnu-statement-expression',
  #]

  #add_project_arguments(MY_WARNINGS, language: 'c')

  main_test = executable('main_test', './main_test.c', dependencies : [ mickstr_dep ] )

  test('main_test', main_test)

endif

