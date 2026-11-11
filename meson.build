# https://mesonbuild.com/  https://mesonbuild.com/Tutorial.html
# sudo dnf install meson
# sudo dnf install ninja-build
#gtkdep = dependency('gtk+-3.0')
# executable('demo', 'ui.c', dependencies : gtkdep) one dependency
# meson setup builddir
# cd builddir && ninja
# gtkdeps = [dependency('gtk+-4.0'), dependency('gtksourceview-4.0')] # multiple deps
# gtksourceview is a add on library for building code editors / IDEs

project('Claires Apps', 'c')
gtkdep = dependency('gtk4')
executable('demo', 'ui.c', dependencies : gtkdep) 




