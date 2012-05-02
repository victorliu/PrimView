#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <time.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>
#include "geom.h"
#include "GLShapes.h"
#include "lodepng.h"
#include "PrimViewAPI.h"

PrimView_Geometry G;

struct render_settings_struct{
	double extent_min[3], extent_max[3];
	unsigned int flags;
#define RENDER_FLAG_HIDE_POINTS 0x0001
#define RENDER_FLAG_HIDE_LINES  0x0002
#define RENDER_FLAG_HIDE_ARROWS 0x0004
#define RENDER_FLAG_HIDE_TRIS   0x0008
#define RENDER_FLAG_HIDE_QUADS  0x0010
#define RENDER_FLAG_HIDE_TETS   0x0020
#define RENDER_FLAG_HIDE_BALLS  0x0040
#define RENDER_FLAG_HIDE_TEXTS  0x0080
#define RENDER_FLAG_SHOW_AXES   0x0100

	double move_inc;
} render_settings;



// VBO Extension Definitions, From glext.h
#define GL_ARRAY_BUFFER_ARB 0x8892
#define GL_STATIC_DRAW_ARB 0x88E4
typedef void (APIENTRY * PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRY * PFNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint *buffers);
typedef void (APIENTRY * PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRY * PFNGLBUFFERDATAARBPROC) (GLenum target, int size, const GLvoid *data, GLenum usage);
// VBO Extension Function Pointers
PFNGLGENBUFFERSARBPROC glGenBuffersARB = NULL;					// VBO Name Generation Procedure
PFNGLBINDBUFFERARBPROC glBindBufferARB = NULL;					// VBO Bind Procedure
PFNGLBUFFERDATAARBPROC glBufferDataARB = NULL;					// VBO Data Loading Procedure
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB = NULL;				// VBO Deletion Procedure
int g_fVBOSupported = 0;					// ARB_vertex_buffer_object supported?
GLuint m_nVBOVertices; // VBO name

// Camera info
double cam_quat[4], cam_quat_prev[4], cam_down_vec[3], cam_dist;
int cam_dragging = 0;

//mouse params
int mouse_left_down = 0;
int mouse_right_down = 0;
int mouse_middle_down = 0;
int mouse_prev_x = 0, mouse_prev_y = 0;
int mouse_down_x = 0, mouse_down_y = 0;
int window_width, window_height;

void draw();

void point_to_sphere(double xy[2], double v[3]){
	// Remap the point to (-1,-1) -> (1,1)
	xy[0] = 2 * xy[0] - 1;
	xy[1] = 2 * xy[1] - 1;
	double len2 = xy[0]*xy[0] + xy[1]*xy[1];
	if(len2 > 1.){
		len2 = sqrt(len2);
		v[0] = xy[0] / len2;
		v[1] = xy[1] / len2;
		v[2] = 0;
	}else{
		v[0] = xy[0];
		v[1] = xy[1];
		v[2] = sqrt(1 - len2);
	}
}
void cam_drag_begin(double xy[2]){
	cam_quat_prev[0] = cam_quat[0];
	cam_quat_prev[1] = cam_quat[1];
	cam_quat_prev[2] = cam_quat[2];
	cam_quat_prev[3] = cam_quat[3];
	point_to_sphere(xy, cam_down_vec);
	cam_dragging = 1;
}
void cam_drag_end(double xy[2]){
	cam_dragging = 0;
}
void cam_drag_update(double xy[2]){
	double v[3], q[4], *p = cam_quat_prev;
	point_to_sphere(xy, v);
	geom_cross3d(v, cam_down_vec, q);
	q[3] = geom_dot3d(cam_down_vec, v);
	cam_quat[3] = p[3]*q[3] - p[0]*q[0] - p[1]*q[1] - p[2]*q[2];
	cam_quat[0] = p[3]*q[0] + p[0]*q[3] + p[1]*q[2] - p[2]*q[1];
	cam_quat[1] = p[3]*q[1] + p[1]*q[3] + p[2]*q[0] - p[0]*q[2];
	cam_quat[2] = p[3]*q[2] + p[2]*q[3] + p[0]*q[1] - p[1]*q[0];
	geom_normalize4d(cam_quat);

	cam_dragging = 1;
}
void cam_load_matrix(){
	const double *q = cam_quat;
	double m[16] = {
		1 - 2*(q[1]*q[1] + q[2]*q[2]),
		2*(q[0]*q[1] - q[3]*q[2]),
		2*(q[1]*q[2] + q[3]*q[1]),
		0,
		2*(q[0]*q[1] + q[3]*q[2]),
		1 - 2*(q[0]*q[0] + q[2]*q[2]),
		2*(q[1]*q[2] - q[3]*q[0]),
		0,
		2*(q[0]*q[2] - q[3]*q[1]),
		2*(q[1]*q[2] + q[3]*q[0]),
		1 - 2*(q[0]*q[0] + q[1]*q[1]),
		0,
		0,0,0,1
	};
	glMultMatrixd(m);
}
void view_save(){
	FILE *fp = fopen("primview.view", "wb");
	if(NULL == fp){ return; }
	fprintf(fp, "%.14g %.14g %.14g %.14g\n%.14g\n",
		cam_quat[0],
		cam_quat[1],
		cam_quat[2],
		cam_quat[3],
		cam_dist
	);
	fclose(fp);
}
void view_load(){
	FILE *fp = fopen("primview.view", "rb");
	if(NULL == fp){ return; }
	fscanf(fp, "%lf %lf %lf %lf %lf",
		&cam_quat[0],
		&cam_quat[1],
		&cam_quat[2],
		&cam_quat[3],
		&cam_dist
	);
	fclose(fp);
}
void screenshot(){
	glDrawBuffer(GL_BACK);
	draw(); //doing drawing here
	glReadBuffer(GL_BACK);
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	GLvoid *imageData = malloc(viewport[2]*viewport[3]*3);
	glReadPixels(0, 0, viewport[2], viewport[3], GL_RGB, GL_UNSIGNED_BYTE, imageData);

	time_t rawtime;
	struct tm *timeinfo;
	char filename[32];

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(filename, 32, "PV%Y%m%d%H%M%S.png", timeinfo);
	lodepng_encode24_file(filename, imageData, viewport[2], viewport[3]);
	free(imageData);
}

void init_render_settings(){
	unsigned int i, d;
	render_settings.flags = RENDER_FLAG_SHOW_AXES;
	double domain_size = 0;
	
	for(d = 0; d < 3u; ++d){
		render_settings.extent_min[d] = DBL_MAX;
		render_settings.extent_max[d] = -DBL_MAX;
	}
	for(i = 0; i < G.n_points; ++i){
		for(d = 0; d < 3u; ++d){
			if(G.point[i].p[d] < render_settings.extent_min[d]){
				render_settings.extent_min[d] = G.point[i].p[d];
			}
			if(G.point[i].p[d] > render_settings.extent_max[d]){
				render_settings.extent_max[d] = G.point[i].p[d];
			}
		}
	}
	
	for(d = 0; d < 3u; ++d){
		double diff = render_settings.extent_max[d] - render_settings.extent_min[d];
		domain_size += diff*diff;
	}
	render_settings.move_inc = 0.1 * domain_size;
}

void onquit(){
	GLShapesDestroy();
	glDeleteBuffersARB(1, &m_nVBOVertices);
	glutSetKeyRepeat(GLUT_KEY_REPEAT_DEFAULT);
}

/* process menu option 'op' */
void menu(int op) {
	switch(op){
	case 0: render_settings.flags ^= RENDER_FLAG_SHOW_AXES; break;
	case 1: render_settings.flags ^= RENDER_FLAG_HIDE_POINTS; break;
	case 2: render_settings.flags ^= RENDER_FLAG_HIDE_LINES; break;
	case 3: render_settings.flags ^= RENDER_FLAG_HIDE_TRIS; break;
	case 4: render_settings.flags ^= RENDER_FLAG_HIDE_QUADS; break;
	case 5: render_settings.flags ^= RENDER_FLAG_HIDE_TETS; break;
	case 6: render_settings.flags ^= RENDER_FLAG_HIDE_BALLS; break;
	case 7: render_settings.flags ^= RENDER_FLAG_HIDE_TEXTS; break;
	case 10: glClearColor(0.0f, 0.0f, 0.0f, 0.0f); break;
	case 11: glClearColor(0.5f, 0.5f, 0.5f, 0.0f); break;
	case 12: glClearColor(1.0f, 1.0f, 1.0f, 0.0f); break;
	case 'v':
		view_save();
		break;
	case 's':
		screenshot();
		break;
	case 'Q':
	case 'q':
		onquit();
		exit(0);
	}
}

/* reshaped window */
void reshape(int width, int height) {
	const GLfloat fieldOfView = 45.0f;
	glViewport (0, 0, (GLsizei) width, (GLsizei) height);
	window_width = width;
	window_height = height;
	//cam.ResizeWindow(width, height);

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	GLPerspective(fieldOfView, width, height, 0.1, 1000.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/* executed when button 'button' is put into state 'state' at screen position ('x', 'y') */
void mouseClick(int button, int state, int x, int y) {
	if(GLUT_LEFT_BUTTON == button){
		if(GLUT_DOWN == state){
			mouse_left_down = 1;
			mouse_down_x = x;
			mouse_down_y = y;
			double f[2] = {
				(double)x/(double)window_width,
				(double)1-(double)y/(double)window_height
			};
			cam_drag_begin(f);
		}else if(GLUT_UP == state){
			mouse_left_down = 0;
			double f[2] = {
				(double)x/(double)window_width,
				(double)1-(double)y/(double)window_height
			};
			cam_drag_end(f);
			glutPostRedisplay();
		}
	}else if(GLUT_MIDDLE_BUTTON == button){
		if(GLUT_DOWN == state){
			mouse_middle_down = 1;
		}else if(GLUT_UP == state){
			mouse_middle_down = 0;
			glutPostRedisplay();
		}
	}
	mouse_prev_x = x;
	mouse_prev_y = y;
}

/* executed when the mouse moves to position ('x', 'y') */
void mouseMotion(int x, int y) {
	if(mouse_left_down){
		double f[2] = {
			(double)x/(double)window_width,
			(double)1-(double)y/(double)window_height
		};
		cam_drag_update(f);
		glutPostRedisplay();
	}
	mouse_prev_x = x;
	mouse_prev_y = y;
}
void mouseWheel(int button, int dir, int x, int y){
	double f = (dir >= 0) ? 0.9 : 1./0.9;
	cam_dist *= f;
	glutPostRedisplay();
}

/* render the scene */
void draw() {
	const double org[3] = {0,0,0};
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Set projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float xmin, xmax, ymin, ymax;
	ymax = 0.1 * tan(45 * M_PI / 360.0);
	ymin = -ymax;
	float ratio = (float)window_width/(float)window_height;
	xmin = ymin * ratio;
	xmax = ymax * ratio;
	glFrustum(xmin, xmax, ymin, ymax, 0.1, 1000);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslated(0,0,-cam_dist);
	cam_load_matrix();

	glEnable(GL_LIGHTING);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if(RENDER_FLAG_SHOW_AXES & render_settings.flags){
		GLDrawAxes(org);
	}
	
	glEnable(GL_BLEND);
	glEnable(GL_COLOR_MATERIAL);
	glPushAttrib(GL_NORMALIZE);
	glEnable(GL_NORMALIZE);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	if(!(render_settings.flags & RENDER_FLAG_HIDE_POINTS) && G.n_points > 0){
		unsigned int i;
		for(i = 0; i < G.n_points; ++i){
			glPushMatrix();
			glTranslated(G.point[i].p[0], G.point[i].p[1], G.point[i].p[2]);
			glScaled(G.options.PointSize, G.options.PointSize, G.options.PointSize);
			if(G.point[i].props.color[0] >= 0){
				glColor3fv(G.point[i].props.color);
			}else{
				glColor3f(0.5f, 0.5f, 0.0f);
			}
			GLDrawBall();
			glPopMatrix();
		}
	}
	if(!(render_settings.flags & RENDER_FLAG_HIDE_LINES) && G.n_lines > 0){
		unsigned int i;
		for(i = 0; i < G.n_lines; ++i){
			double base[3];
			double vec[3];
			unsigned d;
			for(d = 0; d < 3u; ++d){
				base[d] = G.point[G.line[i].v[0]].p[d];
				vec[d] = G.point[G.line[i].v[1]].p[d] - G.point[G.line[i].v[0]].p[d];
			}
			if(G.line[i].props.color[0] >= 0){
				glColor3fv(G.line[i].props.color);
			}else{
				glColor3f(0.0f, 0.6f, 1.0f);
			}
			base[0] += G.options.LineShrink * vec[0];
			base[1] += G.options.LineShrink * vec[1];
			base[2] += G.options.LineShrink * vec[2];
			vec[0] *= (1. - 2*G.options.LineShrink);
			vec[1] *= (1. - 2*G.options.LineShrink);
			vec[2] *= (1. - 2*G.options.LineShrink);
			GLDrawCylinder(base, vec, G.options.LineSize);
		}
	}
	if(!(render_settings.flags & RENDER_FLAG_HIDE_ARROWS) && G.n_arrows > 0){
		unsigned int i;
		for(i = 0; i < G.n_arrows; ++i){
			double base[3];
			double vec[3];
			unsigned d;
			for(d = 0; d < 3u; ++d){
				base[d] = G.point[G.line[i].v[0]].p[d];
				vec[d] = G.point[G.line[i].v[1]].p[d] - G.point[G.line[i].v[0]].p[d];
			}
			if(G.line[i].props.color[0] >= 0){
				glColor3fv(G.line[i].props.color);
			}else{
				glColor3f(0.0f, 0.6f, 1.0f);
			}
			GLDrawVector(base, vec);
		}
	}
	if(!(render_settings.flags & RENDER_FLAG_HIDE_TRIS) && G.n_tris > 0){
		unsigned int i, j;
		glBegin(GL_TRIANGLES);
		double shrink1 = 1. - 2.*G.options.TriShrink;
		for(i = 0; i < G.n_tris; ++i){
			double u[3], v[3], w[3];
			for(j = 0; j < 3u; ++j){
				u[j] = G.point[G.tri[i].v[1]].p[j] - G.point[G.tri[i].v[0]].p[j];
				v[j] = G.point[G.tri[i].v[2]].p[j] - G.point[G.tri[i].v[0]].p[j];
			}
			
			if(G.tri[i].props.color[0] >= 0){
				glColor3fv(G.tri[i].props.color);
			}else{
				glColor3f(0.8f, 0.8f, 0.8f);
			}
			
			double n[3];
			geom_cross3d(u, v, n);
			geom_normalize3d(n);
			glNormal3dv(n);
			if(G.options.TriShrink <= 0){
				for(j = 0; j < 3; ++j){
					glVertex3dv(G.point[G.tri[i].v[j]].p);
				}
			}else{
				unsigned d;
				for(d = 0; d < 3u; ++d){
					u[d] = shrink1 * G.point[G.tri[i].v[0]].p[d] + G.options.TriShrink * G.point[G.tri[i].v[1]].p[d] + G.options.TriShrink * G.point[G.tri[i].v[2]].p[d];
					v[d] = shrink1 * G.point[G.tri[i].v[1]].p[d] + G.options.TriShrink * G.point[G.tri[i].v[2]].p[d] + G.options.TriShrink * G.point[G.tri[i].v[0]].p[d];
					w[d] = shrink1 * G.point[G.tri[i].v[2]].p[d] + G.options.TriShrink * G.point[G.tri[i].v[0]].p[d] + G.options.TriShrink * G.point[G.tri[i].v[1]].p[d];
				}
				glVertex3dv(u);
				glVertex3dv(v);
				glVertex3dv(w);
			}
		}
		glEnd();
	}
	if(!(render_settings.flags & RENDER_FLAG_HIDE_QUADS) && G.n_quads > 0){
		unsigned int i;
		glBegin(GL_QUADS);
		double shrink1 = 1. - 3.*G.options.QuadShrink;
		for(i = 0; i < G.n_quads; ++i){
			double u[3], v[3], w[3], z[3];
			unsigned d;
			for(d = 0; d < 3u; ++d){
				u[d] = G.point[G.quad[i].v[1]].p[d] - G.point[G.quad[i].v[0]].p[d];
				v[d] = G.point[G.quad[i].v[3]].p[d] - G.point[G.quad[i].v[0]].p[d];
			}
			if(G.quad[i].props.color[0] >= 0){
				glColor3fv(G.quad[i].props.color);
			}else{
				glColor3f(0.8f, 0.8f, 0.8f);
			}
			double n[3];
			geom_cross3d(u, v, n);
			geom_normalize3d(n);
			glNormal3dv(n);
			if(G.options.QuadShrink <= 0){
				unsigned j;
				for(j = 0; j < 4u; ++j){
					glVertex3dv(G.point[G.quad[i].v[j]].p);
				}
			}else{
				unsigned d;
				for(d = 0; d < 3u; ++d){
					u[d] = shrink1 * G.point[G.quad[i].v[0]].p[d] + G.options.QuadShrink * G.point[G.quad[i].v[1]].p[d] + G.options.QuadShrink * G.point[G.quad[i].v[2]].p[d] + G.options.QuadShrink * G.point[G.quad[i].v[3]].p[d];
					v[d] = shrink1 * G.point[G.quad[i].v[1]].p[d] + G.options.QuadShrink * G.point[G.quad[i].v[2]].p[d] + G.options.QuadShrink * G.point[G.quad[i].v[3]].p[d] + G.options.QuadShrink * G.point[G.quad[i].v[0]].p[d];
					w[d] = shrink1 * G.point[G.quad[i].v[2]].p[d] + G.options.QuadShrink * G.point[G.quad[i].v[3]].p[d] + G.options.QuadShrink * G.point[G.quad[i].v[0]].p[d] + G.options.QuadShrink * G.point[G.quad[i].v[1]].p[d];
					z[d] = shrink1 * G.point[G.quad[i].v[3]].p[d] + G.options.QuadShrink * G.point[G.quad[i].v[0]].p[d] + G.options.QuadShrink * G.point[G.quad[i].v[1]].p[d] + G.options.QuadShrink * G.point[G.quad[i].v[2]].p[d];
				}
				glVertex3dv(u);
				glVertex3dv(v);
				glVertex3dv(w);
				glVertex3dv(z);
			}
		}
		glEnd();
	}
	if(!(render_settings.flags & RENDER_FLAG_HIDE_TETS) && G.n_tets > 0){
		unsigned int i;
		glBegin(GL_TRIANGLES);
		double shrink1 = 1. - G.options.TetShrink;
		for(i = 0; i < G.n_tets; ++i){
			unsigned j, d;
			if(G.tet[i].props.color[0] >= 0){
				glColor3fv(G.tet[i].props.color);
			}else{
				glColor3f(0.7f, 0.8f, 0.7f);
			}
			double v[4][3], n[4][3];
			// v2,v1,v0, n0=(2-0)x(1-0)
			// v3,v0,v1, n1=(3-1)x(0-1)
			// v0,v3,v2, n2=(0-2)x(3-2)
			// v1,v2,v3, n3=(1-3)x(2-3)
			for(j = 0; j < 4u; ++j){
				for(d = 0; d < 3u; ++d){
					v[j][d] = G.point[G.tet[i].v[j]].p[d];
				}
			}
			for(j = 0; j < 4; ++j){
				double d1[3] = {
					v[(j+2)%4][0] - v[j][0],
					v[(j+2)%4][1] - v[j][1],
					v[(j+2)%4][2] - v[j][2]
				};
				double d2[3] = {
					v[(5-j)%4][0] - v[j][0],
					v[(5-j)%4][1] - v[j][1],
					v[(5-j)%4][2] - v[j][2]
				};
				geom_cross3d(d1, d2, n[j]);
			}
			
			if(G.options.TetShrink <= 0){
			}else{
				double c[3];
				c[0] = 0.25*(v[0][0]+v[1][0]+v[2][0]+v[3][0]);
				c[1] = 0.25*(v[0][1]+v[1][1]+v[2][1]+v[3][1]);
				c[2] = 0.25*(v[0][2]+v[1][2]+v[2][2]+v[3][2]);
				for(j = 0; j < 4; ++j){
					v[j][0] = shrink1 * v[j][0] + G.options.TetShrink * c[0];
					v[j][1] = shrink1 * v[j][1] + G.options.TetShrink * c[1];
					v[j][2] = shrink1 * v[j][2] + G.options.TetShrink * c[2];
				}
			}
			for(j = 0; j < 4; ++j){
				glNormal3dv(n[j]);
				glVertex3dv(v[(j+2)%4]);
				glVertex3dv(v[(5-j)%4]);
				glVertex3dv(v[j]);
			}
		}
		glEnd();
	}
	if(!(render_settings.flags & RENDER_FLAG_HIDE_BALLS) && G.n_spheres > 0){
		unsigned int i;
		for(i = 0; i < G.n_spheres; ++i){
			glPushMatrix();
			glTranslated(
				G.point[G.sphere[i].c].p[0],
				G.point[G.sphere[i].c].p[1],
				G.point[G.sphere[i].c].p[2]
			);
			if(G.sphere[i].props.color[0] >= 0){
				glColor3fv(G.sphere[i].props.color);
			}else{
				glColor3f(0.5f, 0.0f, 0.0f);
			}
			glScalef(G.sphere[i].r, G.sphere[i].r, G.sphere[i].r);
			GLDrawSphere();
			glPopMatrix();
		}
	}
	if(!(render_settings.flags & RENDER_FLAG_HIDE_TEXTS) && G.n_texts > 0){
		unsigned int i;
		for(i = 0; i < G.n_texts; ++i){
			if(G.text[i].props.color[0] >= 0){
				glColor3fv(G.text[i].props.color);
			}else{
				glColor3f(0.1f, 0.1f, 0.1f);
			}
			glRasterPos3d(
				G.point[G.text[i].p].p[0],
				G.point[G.text[i].p].p[1],
				G.point[G.text[i].p].p[2]
			);
			glutBitmapString(GLUT_BITMAP_9_BY_15, (const unsigned char*)G.text[i].str);
		}
	}
	glPopAttrib();
	glDisable(GL_COLOR_MATERIAL);
/*
	if(g_data.tris.size() > 0){
		glEnableClientState( GL_VERTEX_ARRAY );						// Enable Vertex Arrays
		glEnableClientState( GL_NORMAL_ARRAY );						// Enable Vertex Arrays
  		// Set Pointers To Our Data
		if( g_fVBOSupported ){
			glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nVBOVertices );
			glVertexPointer( 3, GL_FLOAT, 0, 0 );		// Set The Vertex Pointer To The Vertex Buffer
			glNormalPointer(GL_FLOAT, 0, (const GLvoid*)(3*g_data.tris.size()*sizeof(float)));
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		}
		glDrawArrays( GL_TRIANGLES, 0, g_data.tris.size() );		// Draw All Of The Triangles At Once

		glDisableClientState( GL_NORMAL_ARRAY );					// Disable Vertex Arrays

		glDisable(GL_LIGHTING);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nVBOVertices );
		glVertexPointer( 3, GL_FLOAT, 0, 0 );		// Set The Vertex Pointer To The Vertex Buffer
		glDrawArrays( GL_TRIANGLES, 0, g_data.tris.size() );		// Draw All Of The Triangles At Once

		glDisableClientState( GL_VERTEX_ARRAY );					// Disable Vertex Arrays
	}
*/

	glFlush();
	glutSwapBuffers();
}

int IsExtensionSupported(const char* szTargetExtension){
	const unsigned char *pszExtensions = NULL;
	const unsigned char *pszStart;
	unsigned char *pszWhere, *pszTerminator;

	// Extension names should not have spaces
	pszWhere = (unsigned char *) strchr( szTargetExtension, ' ' );
	if( pszWhere || *szTargetExtension == '\0' ){ return 0; }

	// Get Extensions String
	pszExtensions = glGetString( GL_EXTENSIONS );

	// Search The Extensions String For An Exact Copy
	pszStart = pszExtensions;
	for(;;){
		pszWhere = (unsigned char *) strstr( (const char *) pszStart, szTargetExtension );
		if( !pszWhere )
			break;
		pszTerminator = pszWhere + strlen( szTargetExtension );
		if( pszWhere == pszStart || *( pszWhere - 1 ) == ' ' ){
			if( *pszTerminator == ' ' || *pszTerminator == '\0' ){
				return 1;
			}
		}
		pszStart = pszTerminator;
	}
	return 0;
}

void GLStdLighting(){
	GLfloat mat_diffuse[4] = { 0.7f, 0.7f, 0.7f, 1.0f };
	GLfloat mat_specular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat mat_shininess[] = { 100.0f };
	GLfloat ambient_light[] = { 0.2f, 0.2f, 0.2f, 1.0f };

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_light);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse );
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular );
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess );

	GLfloat light0[4] = { 4.0f, 4.0f, 10.0f, 1.0f };
	glLightfv (GL_LIGHT0, GL_POSITION, light0);
	glEnable (GL_LIGHT0);
	glEnable(GL_LIGHTING);
}
	
/* initialize GLUT settings, register callbacks, enter main loop */
int main(int argc, char** argv) {
	glutInit(&argc, argv);

	window_width = 800;
	window_height = 500;
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(window_width, window_height);
	glutCreateWindow("3D Primitive Viewer");

	GLStdLighting();
	// Check For VBOs Supported
	g_fVBOSupported = IsExtensionSupported("GL_ARB_vertex_buffer_object");
	if(g_fVBOSupported){
		// Get Pointers To The GL Functions
		glGenBuffersARB = (PFNGLGENBUFFERSARBPROC) wglGetProcAddress("glGenBuffersARB");
		glBindBufferARB = (PFNGLBINDBUFFERARBPROC) wglGetProcAddress("glBindBufferARB");
		glBufferDataARB = (PFNGLBUFFERDATAARBPROC) wglGetProcAddress("glBufferDataARB");
		glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) wglGetProcAddress("glDeleteBuffersARB");
		// Load Vertex Data Into The Graphics Card Memory
	}

	if(argc > 1){
		PrimView_Geometry_load(&G, argv[1]);
		cam_quat[0] = 1;
		cam_quat[1] = 0;
		cam_quat[2] = 0;
		cam_quat[3] = 0;
		cam_quat_prev[0] = cam_quat[0];
		cam_quat_prev[1] = cam_quat[1];
		cam_quat_prev[2] = cam_quat[2];
		cam_quat_prev[3] = cam_quat[3];
		cam_dist = 1;
		view_load();
	}
	init_render_settings();
	GLShapesInit();

	// register glut call backs
	glutMouseFunc(mouseClick);
	glutMouseWheelFunc(mouseWheel);
	glutMotionFunc(mouseMotion);
	glutReshapeFunc(reshape);
	glutDisplayFunc(draw);  

	// create a sub menu
	int subMenuShowHide = glutCreateMenu(menu);
	glutAddMenuEntry("Toggle Axes", 0);
	glutAddMenuEntry("Toggle Points", 1);
	glutAddMenuEntry("Toggle Lines", 2);
	glutAddMenuEntry("Toggle Triangles", 3);
	glutAddMenuEntry("Toggle Quads", 4);
	glutAddMenuEntry("Toggle Tets", 5);
	glutAddMenuEntry("Toggle Spheres", 6);
	glutAddMenuEntry("Toggle Texts", 7);

	int subMenuBackground = glutCreateMenu(menu);
	glutAddMenuEntry("Black", 10);
	glutAddMenuEntry("Gray" , 11);
	glutAddMenuEntry("White", 12);
	
	int subMenuRender = glutCreateMenu(menu);
	glutAddMenuEntry("Solid", 20);
	glutAddMenuEntry("Wireframe", 21);
	glutAddMenuEntry("Both", 22);
	
	// create main "right click" menu
	glutCreateMenu(menu);
	glutAddSubMenu("Show/Hide", subMenuShowHide);
	glutAddSubMenu("Background color", subMenuBackground);
	glutAddSubMenu("Render style", subMenuRender);
	glutAddMenuEntry("Save view", 'v');
	glutAddMenuEntry("Save screenshot", 's');
	glutAddMenuEntry("Quit", 'q');
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	reshape(window_width, window_height);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glutMainLoop();
	onquit();
	return 0;
}
