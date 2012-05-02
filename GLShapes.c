#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/GL.h>
#include "geom.h"

#define CIRCLE_DIVISIONS 16u
const float CIRCLE_DT = (float)(2.0*M_PI/CIRCLE_DIVISIONS);

static float icos_r[12][3] = {
    {  1.0,             0.0,             0.0            },
    {  0.447213595500,  0.894427191000,  0.0            },
    {  0.447213595500,  0.276393202252,  0.850650808354 },
    {  0.447213595500, -0.723606797748,  0.525731112119 },
    {  0.447213595500, -0.723606797748, -0.525731112119 },
    {  0.447213595500,  0.276393202252, -0.850650808354 },
    { -0.447213595500, -0.894427191000,  0.0 },
    { -0.447213595500, -0.276393202252,  0.850650808354 },
    { -0.447213595500,  0.723606797748,  0.525731112119 },
    { -0.447213595500,  0.723606797748, -0.525731112119 },
    { -0.447213595500, -0.276393202252, -0.850650808354 },
    { -1.0,             0.0,             0.0            }
};

static int icos_v [20][3] = {
    {  0,  1,  2 },
    {  0,  2,  3 },
    {  0,  3,  4 },
    {  0,  4,  5 },
    {  0,  5,  1 },
    {  1,  8,  2 },
    {  2,  7,  3 },
    {  3,  6,  4 },
    {  4, 10,  5 },
    {  5,  9,  1 },
    {  1,  9,  8 },
    {  2,  8,  7 },
    {  3,  7,  6 },
    {  4,  6, 10 },
    {  5, 10,  9 },
    { 11,  9, 10 },
    { 11,  8,  9 },
    { 11,  7,  8 },
    { 11,  6,  7 },
    { 11, 10,  6 }
};

GLuint shapeDL0;
void GLDrawCylinder(const double base[3], const double vector[3], const double radius){ // Draws a cylinder from point p along vector dir
	double mat[16] = {
		vector[0], vector[1], vector[2], 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		base[0], base[1], base[2], 1.
	};
	geom_maketriad3d(&mat[0], &mat[4], &mat[8]);
	mat[4] *= radius;
	mat[5] *= radius;
	mat[6] *= radius;
	mat[8] *= radius;
	mat[9] *= radius;
	mat[10] *= radius;
	glPushMatrix();
	glMultMatrixd(mat);
	glCallList(shapeDL0+0);
	glPopMatrix();
}
void GLDrawBall(){
	glCallList(shapeDL0+1);
}
void GLDrawSphere(){
	glCallList(shapeDL0+3);
}
void GLDrawVector(const double vector[3], const double base[3]){ // Draws a cylindrical arrow from point p along vector dir
	double len = geom_norm3d(vector);
	double mat[16] = {
		vector[0], vector[1], vector[2], 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		base[0], base[1], base[2], 1.
	};
	geom_maketriad3d(&mat[0], &mat[4], &mat[8]);
	mat[4] *= len;
	mat[5] *= len;
	mat[6] *= len;
	mat[8] *= len;
	mat[9] *= len;
	mat[10] *= len;
	glPushMatrix();
	glMultMatrixd(mat);
	glCallList(shapeDL0+2);
	glPopMatrix();
}
void GLDrawAxes(const double origin[3]){
	unsigned i;
	static const GLfloat axis_colors[3][4] = {
		{1.0f, 0.5f, 0.5f, 1.0f},
		{0.5f, 1.0f, 0.5f, 1.0f},
		{0.5f, 0.5f, 1.0f, 1.0f}
	};
	const double id[9] = {1,0,0,0,1,0,0,0,1};
	GLfloat saved[4];
	glGetMaterialfv(GL_FRONT, GL_DIFFUSE, saved);
	for(i = 0; i < 3u; ++i){
		glColor4fv(axis_colors[i]);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, axis_colors[i]);
		GLDrawVector(&id[3*i], origin);
	}
	glMaterialfv(GL_FRONT, GL_DIFFUSE, saved);
}

#define icoX .525731112119133606 
#define icoZ .850650808352039932

static GLfloat vdata[12][3] = {    
    {-icoX, 0.0, icoZ}, {icoX, 0.0, icoZ}, {-icoX, 0.0, -icoZ}, {icoX, 0.0, -icoZ},    
    {0.0, icoZ, icoX}, {0.0, icoZ, -icoX}, {0.0, -icoZ, icoX}, {0.0, -icoZ, -icoX},    
    {icoZ, icoX, 0.0}, {-icoZ, icoX, 0.0}, {icoZ, -icoX, 0.0}, {-icoZ, -icoX, 0.0} 
};
static GLuint tindices[20][3] = { 
    {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},    
    {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},    
    {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6}, 
    {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11}
};

void drawtri(GLfloat *a, GLfloat *b, GLfloat *c, int div, float r) {
    if(div<=0){
        glNormal3fv(a); glVertex3f(a[0]*r, a[1]*r, a[2]*r);
        glNormal3fv(b); glVertex3f(b[0]*r, b[1]*r, b[2]*r);
        glNormal3fv(c); glVertex3f(c[0]*r, c[1]*r, c[2]*r);
    }else{
		unsigned i;
        GLfloat ab[3], ac[3], bc[3];
        for(i = 0; i < 3u; i++){
            ab[i]=(a[i]+b[i])/2;
            ac[i]=(a[i]+c[i])/2;
            bc[i]=(b[i]+c[i])/2;
        }
        geom_normalize3f(ab); geom_normalize3f(ac); geom_normalize3f(bc);
        drawtri(a, ab, ac, div-1, r);
        drawtri(b, bc, ab, div-1, r);
        drawtri(c, ac, bc, div-1, r);
        drawtri(ab, bc, ac, div-1, r);  //<--Comment this line and sphere looks floatly cool!
    }  
}

void GLShapesInit(){
	unsigned i, j;
	shapeDL0 = glGenLists(4);
	
	// Cylinder
	glNewList(shapeDL0+0, GL_COMPILE);
	glBegin(GL_QUAD_STRIP);
	for(i = 0; i <= CIRCLE_DIVISIONS; ++i){
		float ca,sa;
		ca = cosf((float)i*CIRCLE_DT);
		sa = sinf((float)i*CIRCLE_DT);
		glNormal3f(0.f, ca, sa);
		glVertex3f(1.f, ca, sa);
		glVertex3f(0.f, ca, sa);
	}
	glEnd();
	glEndList();
	
	// Sphere
	glNewList(shapeDL0+1, GL_COMPILE);
	glBegin(GL_TRIANGLES);
	for(i = 0; i < 20; i++){
		/*
		for(j = 0; j < 3; j++){
			glNormal3fv(icos_r[icos_v[i][j]]);
			glVertex3fv(icos_r[icos_v[i][j]]);
		}
		*/
		// Do 1 level of subdivision
		float vm[3][3], len[3];
		for(j = 0; j < 3u; j++){
			int k;
			int j1 = (j+1)%3;
			int j2 = (j+2)%3;
			len[j] = 0;
			for(k = 0; k < 3; k++){
				vm[j][k] = 0.5*(icos_r[icos_v[i][j1]][k] + icos_r[icos_v[i][j2]][k]);
				len[j] += vm[j][k] * vm[j][k];
			}
			len[j] = sqrtf(len[j]);
			for(k = 0; k < 3; k++){
				vm[j][k] /= len[j];
			}
		}
		for(j = 0; j < 3u; j++){
			glNormal3fv(vm[j]);
			glVertex3fv(vm[j]);
		}
		for(j = 0; j < 3u; j++){
			int j1 = (j+1)%3;
			int j2 = (j+2)%3;
			glNormal3fv(icos_r[icos_v[i][j]]);
			glVertex3fv(icos_r[icos_v[i][j]]);
			glNormal3fv(vm[j2]);
			glVertex3fv(vm[j2]);
			glNormal3fv(vm[j1]);
			glVertex3fv(vm[j1]);
		}
	}
	glEnd();
	glEndList();
	
	// Arrow
	const float arrowhead_base_width = 0.1f;
	const float arrow_body_width = 0.03f;
	const float arrow_body_length = 0.8f;
	const float arrow_head_length = 1.f - arrow_body_length;
	const float arrow_normal_normalization = sqrtf(arrowhead_base_width*arrowhead_base_width + arrow_head_length*arrow_head_length);
	const float arrow_normal_x = arrowhead_base_width / arrow_normal_normalization;
	const float arrow_normal_y = arrow_head_length / arrow_normal_normalization;

	glNewList(shapeDL0+2, GL_COMPILE);
	glBegin(GL_QUAD_STRIP);
	for(i = 0; i <= CIRCLE_DIVISIONS; ++i){
		float ca,sa;
		ca = cosf((float)i*CIRCLE_DT);
		sa = sinf((float)i*CIRCLE_DT);
		glNormal3f(0.f, ca, sa);
		glVertex3f(arrow_body_length, arrow_body_width*ca, arrow_body_width*sa);
		glVertex3f(0.f, arrow_body_width*ca, arrow_body_width*sa);
	}
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(1.f, 0.f, 0.f);
	glVertex3f(1.f, 0.f, 0.f);
	for(i = 0; i <= CIRCLE_DIVISIONS; ++i){
		float ca,sa;
		ca = cosf((float)i*CIRCLE_DT);
		sa = sinf((float)i*CIRCLE_DT);
		glNormal3f(arrow_normal_x, arrow_normal_y*ca, arrow_normal_y*sa);
		glVertex3f(arrow_body_length, arrowhead_base_width*ca, arrowhead_base_width*sa);
	}
	glEnd();
	glEndList();
	
	
	glNewList(shapeDL0+3, GL_COMPILE);
	//glutSolidSphere(1.0, 64, 32);
	
	glBegin(GL_TRIANGLES);
	for(i = 0; i < 20u; i++){
		drawtri(vdata[tindices[i][0]], vdata[tindices[i][1]], vdata[tindices[i][2]], 4, 1.0);
	}
	glEnd();
	
	glEndList();
}
void GLShapesDestroy(){
	glDeleteLists(shapeDL0, 4);
}

void GLPerspective(GLdouble fovy, int width, int height, GLdouble zNear, GLdouble zFar){
	GLdouble m[16];
	GLdouble aspect = (GLdouble)width / (GLdouble)height;

	GLdouble depth = zFar - zNear;
	GLdouble q = -(zFar + zNear) / depth;
	GLdouble qn = -2 * (zFar * zNear) / depth;

	GLdouble w = 2 * zNear / (double)width;
	w = w / aspect;
	GLdouble h = 2 * zNear / (double)height;

	m[0]  = w;
	m[1]  = 0;
	m[2]  = 0;
	m[3]  = 0;

	m[4]  = 0;
	m[5]  = h;
	m[6]  = 0;
	m[7]  = 0;

	m[8]  = 0;
	m[9]  = 0;
	m[10] = q;
	m[11] = -1;

	m[12] = 0;
	m[13] = 0;
	m[14] = qn;
	m[15] = 0;
	glMultMatrixd(m);
}

