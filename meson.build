project('ChuckleCore', ['cpp', 'c', 'objcpp'], default_options : ['cpp_std=c++14'])

add_project_arguments('-Wno-missing-braces', '-fno-exceptions', language: 'cpp')

cc = meson.get_compiler('cpp')

stickProj = subproject('Stick')
lukeProj = subproject('Luke')
crunchProj = subproject('Crunch')
dabProj = subproject('Dab')
paperProj = subproject('Paper2')
picProj = subproject('Pic')

deps = [stickProj.get_variable('stickDep'), 
        crunchProj.get_variable('crunchDep'),
        lukeProj.get_variable('lukeDep'), 
        dabProj.get_variable('dabDep'),
        paperProj.get_variable('paperDep'),
        picProj.get_variable('picDep')]

incDirs = include_directories('.', 'ChuckleCore/Libs/imgui', 'ChuckleCore/Libs/whereami')

if host_machine.system() == 'linux'
    deps += cc.find_library('dl', required : true)
endif

chuckleCoreInc = [
    'ChuckleCore/ChuckleCore.hpp'
]

if meson.is_subproject() == false
    install_headers(chuckleCoreInc)
endif

chuckleCoreSrc = [
    'ChuckleCore/ChuckleCore.cpp',
    'ChuckleCore/Libs/imgui/imgui.cpp',
    'ChuckleCore/Libs/imgui/imgui_demo.cpp',
    'ChuckleCore/Libs/imgui/imgui_draw.cpp',
    'ChuckleCore/Libs/imgui/imgui_widgets.cpp',
    'ChuckleCore/Libs/whereami/whereami.c'
]

if host_machine.system() == 'darwin'
    chuckleCoreSrc += 'ChuckleCore/Private/noc_file_dialog_osx.mm'
    deps += dependency('appKit')
endif

chuckleCore = library('ChuckleCore', 
    chuckleCoreSrc, 
    dependencies: deps, 
    include_directories : incDirs, 
    install: meson.is_subproject() == false)

chuckleCoreDep = declare_dependency(link_with : chuckleCore, 
    dependencies: deps, 
    include_directories: incDirs)

if get_option('buildExamples') == true
    subdir('Examples')
endif
