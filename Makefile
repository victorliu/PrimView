CFLAGS = -O0 -ggdb -Wall
SUBSYSTEM = -Wl,-subsystem,windows

LUA_INC = -I/d/dev/libs/lua-5.2.0/src
LUA_LIB = -L/d/dev/libs/output/lua-5.2.0 -llua
FREEGLUT_INC = -I/d/dev/libs/freeglut-2.8.0/include
FREEGLUT_LIB = -L/d/dev/libs/output/freeglut -lfreeglut 
GL_LIBS = -lopengl32 -lwinmm -lgdi32 -lglu32

primview: main.o geom.o PrimViewAPI.o GLShapes.o lodepng.o
	gcc $(CFLAGS) main.o geom.o PrimViewAPI.o GLShapes.o lodepng.o \
	$(SUBSYSTEM) $(LUA_LIB) $(FREEGLUT_LIB) $(GL_LIBS) -o primview

main.o: main.c PrimViewAPI.h geom.h GLShapes.h lodepng.h
	gcc -c $(CFLAGS) main.c -DFREEGLUT_STATIC $(FREEGLUT_INC) -o main.o
PrimViewAPI.o: PrimViewAPI.c PrimViewAPI.h
	gcc -c $(CFLAGS) PrimViewAPI.c $(LUA_INC) -o PrimViewAPI.o
geom.o: geom.c geom.h
	gcc -c $(CFLAGS) geom.c -o geom.o
GLShapes.o: GLShapes.c GLShapes.h geom.h
	gcc -c $(CFLAGS) GLShapes.c -o GLShapes.o
lodepng.o: lodepng.c lodepng.h
	gcc -c $(CFLAGS) lodepng.c -o lodepng.o
