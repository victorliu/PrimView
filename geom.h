#ifndef GEOM_H_INCLUDED
#define GEOM_H_INCLUDED

double geom_norm2d(const double v[2]);
float  geom_norm3f(const float  v[3]);
double geom_norm3d(const double v[3]);
double geom_norm4d(const double v[4]);
float  geom_normalize3f(float  v[3]);
double geom_normalize3d(double v[3]);
double geom_normalize4d(double v[4]);

double geom_dot3d(const double a[3], const double b[3]);
void geom_cross3d(const double a[3], const double b[3], double result[3]);

void geom_maketriad3d(const double a[3], double b[3], double c[3]);

#endif // GEOM_H_INCLUDED
