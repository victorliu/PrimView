#include "PrimViewAPI.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

PrimView_Geometry *_G; // current file

static void init_options(PrimView_Options *opts);
static void register_PrimView_API(lua_State *L);
static void getcolor(lua_State *L, float *c);

static int cb_PrimView_Point(lua_State *L);
static int cb_PrimView_Line(lua_State *L);
static int cb_PrimView_Arrow(lua_State *L);
static int cb_PrimView_Triangle(lua_State *L);
static int cb_PrimView_Quad(lua_State *L);
static int cb_PrimView_Tetrahedron(lua_State *L);
static int cb_PrimView_Sphere(lua_State *L);
static int cb_PrimView_Text(lua_State *L);
static int cb_PrimView_SetOptions(lua_State *L);
static int cb_PVF(lua_State *L);

int PrimView_Geometry_load(PrimView_Geometry *geom, int argc, char **argv){
	int i;
	const char *filename = argv[1];
	lua_State *L;
	if(NULL == geom){ return -1; }
	if(NULL == filename){ return -2; }
	
	geom->n_points = 0; geom->n_points_alloc = 0;
	geom->point = NULL;
	geom->n_lines = 0; geom->n_lines_alloc = 0;
	geom->line = NULL;
	geom->n_arrows = 0; geom->n_arrows_alloc = 0;
	geom->arrow = NULL;
	geom->n_tris = 0; geom->n_tris_alloc = 0;
	geom->tri = NULL;
	geom->n_quads = 0; geom->n_quads_alloc = 0;
	geom->quad = NULL;
	geom->n_tets = 0; geom->n_tets_alloc = 0;
	geom->tet = NULL;
	geom->n_spheres = 0; geom->n_spheres_alloc = 0;
	geom->sphere = NULL;
	geom->n_texts = 0; geom->n_texts_alloc = 0;
	geom->text = NULL;
	
	init_options(&geom->options);
	
	_G = geom;
	
	L = luaL_newstate();
	luaL_openlibs(L);
	register_PrimView_API(L);
	
	// Add arguments
	lua_createtable(L, argc-1, 0);
	for(i = 1; i < argc; ++i){
		lua_pushinteger(L, i-1);
		lua_pushstring(L, argv[i]);
		lua_settable(L, -3);
	}
	lua_setglobal(L, "arg");
	
	if(luaL_loadfile(L, filename)){
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		lua_pop(L, 1); /* pop error message from the stack */
	}else{
		if(lua_pcall(L, 0, 0, 0)){
			fprintf(stderr, "%s\n", lua_tostring(L, -1));
			lua_pop(L, 1); /* pop error message from the stack */
		}
	}
	lua_close(L);
	return 0;
}

int PrimView_Geometry_free(PrimView_Geometry *geom){
	if(NULL == geom){ return -1; }
	free(geom->point);
	free(geom->line);
	free(geom->arrow);
	free(geom->tri);
	free(geom->quad);
	free(geom->tet);
	free(geom->sphere);
	free(geom->text);
	return 0;
}

static void init_options(PrimView_Options *opts){
	opts->Origin[0] = 0;
	opts->Origin[1] = 0;
	opts->Origin[2] = 0;
	opts->PointSize = 0.01;
	opts->LineSize = 0.008;
	opts->LineShrink = 0;
	opts->TriShrink = 0;
	opts->QuadShrink = 0;
	opts->TetShrink = 0;
}

static void register_PrimView_API(lua_State *L){
	luaL_Reg PrimView_table[] = {
		{"Point", &cb_PrimView_Point},
		{"Line", &cb_PrimView_Line},
		{"Arrow", &cb_PrimView_Arrow},
		{"Triangle", &cb_PrimView_Triangle},
		{"Quad", &cb_PrimView_Quad},
		{"Tetrahedron", &cb_PrimView_Tetrahedron},
		{"Sphere", &cb_PrimView_Sphere},
		{"Text", &cb_PrimView_Text},
		{"SetOptions", &cb_PrimView_SetOptions},
		{NULL, NULL}
	};
	luaL_newlib(L, PrimView_table);
	lua_setglobal(L, "PrimView");
	lua_pushcfunction(L, &cb_PVF);
    lua_setglobal(L, "PVF");
}

#define EXPAND_ALLOC(ARRAY, TYPE, COUNT, ALLOC_COUNT) do{ \
	if(COUNT >= ALLOC_COUNT){ \
		if(0 == ALLOC_COUNT){ \
			ALLOC_COUNT = 512; \
		}else{ \
			ALLOC_COUNT *= 2; \
		} \
		ARRAY = (TYPE*)realloc(ARRAY, sizeof(TYPE) * (ALLOC_COUNT)); \
		if(NULL == ARRAY){ \
			luaL_error(L, "Error allocating %d %s's\n", ALLOC_COUNT, #TYPE); \
			return 0; \
		} \
	} \
}while(0)

static int cb_PrimView_Point(lua_State *L){
	int i;
	luaL_argcheck(L, lua_istable(L,1), 1, "Expected table\n");
	EXPAND_ALLOC(_G->point, PrimView_Point, _G->n_points, _G->n_points_alloc);
	for(i = 0; i < 3; ++i){
		lua_pushinteger(L, 1+i);
		lua_gettable(L, 1);
		_G->point[_G->n_points].p[i] = luaL_checknumber(L, 2);
		lua_pop(L, 1);
	}
	getcolor(L, _G->point[_G->n_points].props.color);
	_G->n_points++;
	return 0;
}
static int cb_PrimView_Line(lua_State *L){
	int i;
	luaL_argcheck(L, lua_istable(L,1), 1, "Expected table\n");
	EXPAND_ALLOC(_G->line, PrimView_Line, _G->n_lines, _G->n_lines_alloc);
	for(i = 0; i < 2; ++i){
		lua_pushinteger(L, 1+i);
		lua_gettable(L, 1);
		int v = luaL_checkint(L, 2);
		if(v < 0){ v += _G->n_points; }
		_G->line[_G->n_lines].v[i] = v;
		lua_pop(L, 1);
	}
	getcolor(L, _G->line[_G->n_lines].props.color);
	_G->n_lines++;
	return 0;
}
static int cb_PrimView_Arrow(lua_State *L){
	int i;
	luaL_argcheck(L, lua_istable(L,1), 1, "Expected table\n");
	EXPAND_ALLOC(_G->arrow, PrimView_Arrow, _G->n_arrows, _G->n_arrows_alloc);
	for(i = 0; i < 2; ++i){
		lua_pushinteger(L, 1+i);
		lua_gettable(L, 1);
		int v = luaL_checkint(L, 2);
		if(v < 0){ v += _G->n_points; }
		_G->arrow[_G->n_arrows].v[i] = v;
		lua_pop(L, 1);
	}
	getcolor(L, _G->arrow[_G->n_arrows].props.color);
	_G->n_arrows++;
	return 0;
}
static int cb_PrimView_Triangle(lua_State *L){
	int i;
	luaL_argcheck(L, lua_istable(L,1), 1, "Expected table\n");
	EXPAND_ALLOC(_G->tri, PrimView_Triangle, _G->n_tris, _G->n_tris_alloc);
	for(i = 0; i < 3; ++i){
		lua_pushinteger(L, 1+i);
		lua_gettable(L, 1);
		int v = luaL_checkint(L, 2);
		if(v < 0){ v += _G->n_points; }
		_G->tri[_G->n_tris].v[i] = v;
		lua_pop(L, 1);
	}
	getcolor(L, _G->tri[_G->n_tris].props.color);
	_G->n_tris++;
	return 0;
}
static int cb_PrimView_Quad(lua_State *L){
	int i;
	luaL_argcheck(L, lua_istable(L,1), 1, "Expected table\n");
	EXPAND_ALLOC(_G->quad, PrimView_Quad, _G->n_quads, _G->n_quads_alloc);
	for(i = 0; i < 4; ++i){
		lua_pushinteger(L, 1+i);
		lua_gettable(L, 1);
		int v = luaL_checkint(L, 2);
		if(v < 0){ v += _G->n_points; }
		_G->quad[_G->n_quads].v[i] = v;
		lua_pop(L, 1);
	}
	getcolor(L, _G->quad[_G->n_quads].props.color);
	_G->n_quads++;
	return 0;
}
static int cb_PrimView_Tetrahedron(lua_State *L){
	int i;
	luaL_argcheck(L, lua_istable(L,1), 1, "Expected table\n");
	EXPAND_ALLOC(_G->tet, PrimView_Tet, _G->n_tets, _G->n_tets_alloc);
	for(i = 0; i < 4; ++i){
		lua_pushinteger(L, 1+i);
		lua_gettable(L, 1);
		int v = luaL_checkint(L, 2);
		if(v < 0){ v += _G->n_points; }
		_G->tet[_G->n_tets].v[i] = v;
		lua_pop(L, 1);
	}
	getcolor(L, _G->tet[_G->n_tets].props.color);
	_G->n_tets++;
	return 0;
}
static int cb_PrimView_Sphere(lua_State *L){
	luaL_argcheck(L, lua_istable(L,1), 1, "Expected table\n");
	EXPAND_ALLOC(_G->sphere, PrimView_Sphere, _G->n_spheres, _G->n_spheres_alloc);
	
	lua_pushinteger(L, 1);
	lua_gettable(L, 1);
	int v = luaL_checkint(L, 2);
	if(v < 0){ v += _G->n_points; }
	_G->sphere[_G->n_spheres].c = v;
	lua_pop(L, 1);
	lua_pushinteger(L, 2);
	lua_gettable(L, 1);
	_G->sphere[_G->n_spheres].r = luaL_checknumber(L, 2);
	lua_pop(L, 1);
	
	getcolor(L, _G->sphere[_G->n_spheres].props.color);
	_G->n_spheres++;
	return 0;
}
static int cb_PrimView_Text(lua_State *L){
	luaL_argcheck(L, lua_istable(L,1), 1, "Expected table\n");
	EXPAND_ALLOC(_G->text, PrimView_Text, _G->n_texts, _G->n_texts_alloc);
	
	lua_pushinteger(L, 1);
	lua_gettable(L, 1);
	int v = luaL_checkint(L, 2);
	if(v < 0){ v += _G->n_points; }
	_G->text[_G->n_texts].p = v;
	lua_pop(L, 1);
	lua_pushinteger(L, 2);
	lua_gettable(L, 1);
	size_t len;
	const char *s = luaL_checklstring(L, 2, &len);
	if(NULL != s){
		if(len < OBJ_TEXT_LENGTH){
			strncpy(_G->text[_G->n_texts].str, s, OBJ_TEXT_LENGTH);
		}else{
			strncpy(_G->text[_G->n_texts].str, s, OBJ_TEXT_LENGTH);
			_G->text[_G->n_texts].str[OBJ_TEXT_LENGTH-1] = '\0';
		}
	}
	getcolor(L, _G->text[_G->n_texts].props.color);
	lua_pop(L, 1);
	
	_G->n_texts++;
	return 0;
}
static int cb_PrimView_SetOptions(lua_State *L){
	luaL_argcheck(L, lua_istable(L,1), 1, "Expected table\n");
	
	lua_pushnil(L);
	while(lua_next(L, 1) != 0){
		const char *key = lua_tostring(L, -2);
		if(0 == strcmp("PointSize", key)){
			_G->options.PointSize = lua_tonumber(L, -1);
			if(_G->options.PointSize <= 0){
				luaL_error(L, "PointSize must be positive\n");
			}
		}else if(0 == strcmp("LineSize", key)){
			_G->options.LineSize = lua_tonumber(L, -1);
			if(_G->options.LineSize <= 0){
				luaL_error(L, "LineSize must be positive\n");
			}
		}else if(0 == strcmp("LineShrink", key)){
			_G->options.LineShrink = lua_tonumber(L, -1);
		}else if(0 == strcmp("TriangleShrink", key)){
			_G->options.TriShrink = lua_tonumber(L, -1);
		}else if(0 == strcmp("QuadShrink", key)){
			_G->options.QuadShrink = lua_tonumber(L, -1);
		}else if(0 == strcmp("TetrahedronShrink", key)){
			_G->options.TetShrink = lua_tonumber(L, -1);
		}else{
			/* uses 'key' (at index -2) and 'value' (at index -1) */
			luaL_error(L, "Unrecognized argument: %s\n", lua_tostring(L, -2));
		}
		/* removes 'value'; keeps 'key' for next iteration */
		lua_pop(L, 1);
	}
	
	return 0;
}

static void PVF_1(lua_State *L){
	lua_pushcfunction(L, &cb_PrimView_Point);
    lua_setglobal(L, "p");
	lua_pushcfunction(L, &cb_PrimView_Line);
    lua_setglobal(L, "l");
	lua_pushcfunction(L, &cb_PrimView_Arrow);
    lua_setglobal(L, "v");
	lua_pushcfunction(L, &cb_PrimView_Triangle);
    lua_setglobal(L, "t");
	lua_pushcfunction(L, &cb_PrimView_Quad);
    lua_setglobal(L, "q");
	lua_pushcfunction(L, &cb_PrimView_Tetrahedron);
    lua_setglobal(L, "T");
	lua_pushcfunction(L, &cb_PrimView_Sphere);
    lua_setglobal(L, "b");
	lua_pushcfunction(L, &cb_PrimView_Text);
    lua_setglobal(L, "x");
}

static int cb_PVF(lua_State *L){
	int v = luaL_checkint(L, 1);
	switch(v){
	case 1:
		PVF_1(L);
		break;
	default:
		luaL_error(L, "Unrecognized version to PVF(): \n", v);
		break;
	}
	return 0;
}

static void getcolor(lua_State *L, float *c){
	int i;
	lua_pushstring(L, "color");
	lua_gettable(L, 1);
	if(!lua_isnil(L, 2) && lua_istable(L, 2)){
		for(i = 0; i < 3; ++i){
			lua_pushinteger(L, 1+i);
			lua_gettable(L, 2);
			c[i] = luaL_checknumber(L, 3);
			lua_pop(L, 1);
		}
	}else{
		c[0] = -1;
		c[1] = -1;
		c[2] = -1;
	}
	lua_pop(L, 1);
}

