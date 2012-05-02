#include <math.h>

double geom_norm2d(const double v[2]){
	double x = fabs(v[0]);
	double y = fabs(v[1]);
	if(x < y){
		if(0 == x){
			return y;
		}
		x /= y;
		return y * sqrt(1 + x*x);
	}else{
		y /= x;
		return x * sqrt(1 + y*y);
	}
}
float geom_norm3f(const float v[3]){
	float a[3] = {fabsf(v[0]),fabsf(v[1]),fabsf(v[2])};
	float w = a[0];
	if(a[1] > w){ w = a[1]; }
	if(a[2] > w){ w = a[2]; }
	if(0 == w){
		return 0;
	}else{
		a[0] /= w; a[1] /= w; a[2] /= w;
		w *= sqrtf(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
		return w;
	}
}

double geom_norm3d(const double v[3]){
	double a[3] = {fabs(v[0]),fabs(v[1]),fabs(v[2])};
	double w = a[0];
	if(a[1] > w){ w = a[1]; }
	if(a[2] > w){ w = a[2]; }
	if(0 == w){
		return 0;
	}else{
		a[0] /= w; a[1] /= w; a[2] /= w;
		w *= sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
		return w;
	}
}
double geom_norm4d(const double v[4]){
	double a[4] = {fabs(v[0]),fabs(v[1]),fabs(v[2]),fabs(v[3])};
	double w = a[0];
	if(a[1] > w){ w = a[1]; }
	if(a[2] > w){ w = a[2]; }
	if(a[3] > w){ w = a[3]; }
	if(0 == w){
		return 0;
	}else{
		a[0] /= w; a[1] /= w; a[2] /= w; a[3] /= w;
		w *= sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2] + a[3]*a[3]);
		return w;
	}
}
float geom_normalize3f(float v[3]){
	float n = geom_norm3f(v);
	v[0] /= n;
	v[1] /= n;
	v[2] /= n;
	return n;
}
double geom_normalize3d(double v[3]){
	double n = geom_norm3d(v);
	v[0] /= n;
	v[1] /= n;
	v[2] /= n;
	return n;
}
double geom_normalize4d(double v[4]){
	double n = geom_norm4d(v);
	v[0] /= n;
	v[1] /= n;
	v[2] /= n;
	v[3] /= n;
	return n;
}
double geom_dot3d(const double a[3], const double b[3]){
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}
void geom_cross3d(const double a[3], const double b[3], double result[3]){
	result[0] = a[1]*b[2] - a[2]*b[1];
	result[1] = a[2]*b[0] - a[0]*b[2];
	result[2] = a[0]*b[1] - a[1]*b[0];
}

void geom_maketriad3d(const double a[3], double b[3], double c[3]){
	double alen = geom_norm3d(a);
	const double an[3] = {
		a[0] / alen,
		a[1] / alen,
		a[2] / alen
	};
	if(fabs(a[0]) > fabs(a[1])){
		double invLen = 1. / hypot(an[0], an[2]);
		b[0] = -an[2] * invLen;
		b[1] = 0;
		b[2] = an[0] * invLen;
	}else{
		double invLen = 1. / hypot(an[1], an[2]);
		b[0] = 0;
		b[1] = an[2] * invLen;
		b[2] = -an[1] * invLen;
	}
	geom_cross3d(an, b, c);
}
