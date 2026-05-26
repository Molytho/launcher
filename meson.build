project(
    'launcher',
    'cpp',
    version : '0.1',
    default_options : [
        'warning_level=3',
        'werror=true',
        'cpp_std=c++20'
    ]
)

dependencies = [
    dependency('giomm-2.4')
]

sources = [
    'src/launcher.cpp',
    'src/provider_repository.cpp',
    'src/utils/fuzzy_matcher.cpp',
    'src/providers/desktop-entries/provider.cpp'
]

includes = [
    include_directories('include')
]

exe = executable(
    'launcher',
    sources,
    include_directories : includes,
    install : true,
    dependencies : dependencies
)

test('basic', exe)
