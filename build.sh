emcc -o index.html fractal_tree.c -Os -Wall ./libraylib.a -I. -I /usr/include  -L. -L./ -s USE_GLFW=3 -s ASYNCIFY --shell-file ./minshell.html
