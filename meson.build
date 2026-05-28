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

gnome = import('gnome')
ui_resource = gnome.compile_resources(
    'ui',
    'resources/ui.gresource.xml',
    source_dir: 'resources',
    c_name: 'ui'
)

dependencies = [
    dependency('gtkmm-4.0'),
    dependency('giomm-2.4')
]

sources = [
    'src/launcher.cpp',
    'src/provider_repository.cpp',
    'src/ui/list_item.cpp',
    'src/utils/fuzzy_matcher.cpp',
    'src/providers/desktop-entries/provider.cpp',
    ui_resource
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
