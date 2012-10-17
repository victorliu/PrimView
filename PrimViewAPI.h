#ifndef PRIMVIEW_API_H_INCLUDED
#define PRIMVIEW_API_H_INCLUDED

typedef struct PrimView_Common_Properties_struct{
	float color[3];
} PrimView_Common_Properties;

typedef struct PrimView_Point_struct{
	double p[3];
	PrimView_Common_Properties props;
} PrimView_Point;

typedef struct PrimView_Arrow_struct{
	unsigned int v[2];
	PrimView_Common_Properties props;
} PrimView_Arrow;

typedef struct PrimView_Line_struct{
	unsigned int v[2];
	PrimView_Common_Properties props;
} PrimView_Line;

typedef struct PrimView_Triangle_struct{
	unsigned int v[3];
	PrimView_Common_Properties props;
} PrimView_Triangle;

typedef struct PrimView_Quad_struct{
	unsigned int v[4];
	PrimView_Common_Properties props;
} PrimView_Quad;

typedef struct PrimView_Tet_struct{
	unsigned int v[4];
	PrimView_Common_Properties props;
} PrimView_Tet;

typedef struct PrimView_Sphere_struct{
	unsigned int c;
	double r;
	PrimView_Common_Properties props;
} PrimView_Sphere;

#define OBJ_TEXT_LENGTH 64
typedef struct PrimView_Text_struct{
	unsigned int p;
	char str[OBJ_TEXT_LENGTH];
	PrimView_Common_Properties props;
} PrimView_Text;

typedef struct PrimView_Options_struct{
	double Origin[3];
	double PointSize;
	double LineSize;
	double LineShrink;
	double TriShrink;
	double QuadShrink;
	double TetShrink;
} PrimView_Options;

typedef struct PrimView_Geometry_struct{
	unsigned int n_points, n_points_alloc;
	PrimView_Point *point;
	
	unsigned int n_lines, n_lines_alloc;
	PrimView_Line *line;
	
	unsigned int n_arrows, n_arrows_alloc;
	PrimView_Arrow *arrow;
	
	unsigned int n_tris, n_tris_alloc;
	PrimView_Triangle *tri;
	
	unsigned int n_quads, n_quads_alloc;
	PrimView_Quad *quad;
	
	unsigned int n_tets, n_tets_alloc;
	PrimView_Tet *tet;
	
	unsigned int n_spheres, n_spheres_alloc;
	PrimView_Sphere *sphere;
	
	unsigned int n_texts, n_texts_alloc;
	PrimView_Text *text;
	
	PrimView_Options options;
} PrimView_Geometry;

int PrimView_Geometry_load(PrimView_Geometry *geom, int argc, char **argv);
int PrimView_Geometry_free(PrimView_Geometry *geom);

#endif // PRIMVIEW_API_H_INCLUDED
