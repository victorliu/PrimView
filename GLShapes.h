#ifndef GLSHAPES_H_INCLUDED
#define GLSHAPES_H_INCLUDED

void GLShapesInit();
void GLShapesDestroy();

// Draws a cylinder from point p along vector dir
void GLDrawCylinder(const double base[3], const double vector[3], const double radius);
// Draws a cylindrical arrow from point p along vector dir
void GLDrawVector(const double base[3], const double vector[3]);
void GLDrawAxes(const double origin[3]);
// Draws a low quality sphere
void GLDrawBall();
// Draws a high quality sphere
void GLDrawSphere();
void GLPerspective(GLdouble fovy, int width, int height, GLdouble zNear, GLdouble zFar);

#endif // GLSHAPES_H_INCLUDED
