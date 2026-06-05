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
    dependency('boost', modules: ['program_options', 'regex']),
    dependency('gtkmm-4.0'),
    dependency('giomm-2.68'),
    dependency('gtk4-layer-shell-0'),
    dependency('gtk4-x11'),
    dependency('x11')
]

sources = [
    'src/launcher.cpp',
    'src/provider_repository.cpp',
    'src/config.cpp',
    'src/history.cpp',
    'src/macros.cpp',
    'src/ui/list_item.cpp',
    'src/ui/main_window.cpp',
    'src/ui/main_window_x11.cpp',
    'src/ui/main_window_wayland.cpp',
    'src/utils/fuzzy_matcher.cpp',
    'src/utils/xdg_base_directory.cpp',
    'src/utils/spawn_helper.cpp',
    'src/providers/desktop-entries/provider.cpp',
    'src/providers/console/provider.cpp',
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
