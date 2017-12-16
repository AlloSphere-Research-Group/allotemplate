#include "al/core.hpp"
#include <iostream>
#include <cmath>

using namespace al;
using namespace std;

using vec3 = Vec3d;

double r = 30;
double e = 1;
vec3 v {10, 20, -30};

template <typename VEC_TYPE>
auto get_mag_and_normalize(VEC_TYPE& v) {
	using MAG_TYPE = decltype(v.mag());
	auto m = v.mag();
	if (m > MAG_TYPE(1e-6)) {
		v /= m;
		return m;
	}
	else {
		v *= MAG_TYPE(0);
		return MAG_TYPE(0);
	}
}

// allofw
vec3 omni_displace() {
    vec3 EYE = vec3(-v.z, 0.0, v.x) * e / v.mag();

    double a = (v - EYE).magSqr();
    double b = 2.0 * EYE.dot(v - EYE);
    double c = EYE.magSqr() - r * r;
    double t = (-b + std::sqrt(b * b - 4 * a * c)) / a / 2.0;
    vec3 SP = EYE + t * (v - EYE);

    // normalize direction and put at eye-to-v distance
    vec3 dir = SP.normalized();
    double dist = (EYE - v).mag();
    return dir * dist;
}

// allolib
vec3 displace2()
{
	vec3 OE {-v.z, 0.0, v.x}; // eye direction
	OE.normalize();           // to unit vector
	OE *= e;                  // set to eye separation length
	vec3 EV = v - OE;         // eye to vertex vector
	double ev = get_mag_and_normalize(EV); // normalize

	// coefs for polynomial t^2 + 2bt + c = 0
	// derived from cosine law r^2 = t^2 + e^2 + 2tecos(theta)
	// where theta is angle between OE and EV
	// t is distance to sphere surface from eye
	double b = -OE.dot(EV);
	double c = e * e - r * r;
	double t = -b + std::sqrt(b * b - c); // quadratic formula

	vec3 s = OE + t * EV; // result direction from origin to sphere surface
	return ev * s.normalize(); // normalize direction and put at eye-to-v distance
}

// allosystem
vec3 omni_render() {
  // double l = length(v.xz);
  double l = std::sqrt(v.x * v.x + v.z * v.z);
  vec3 vn = v.normalized();
  // Precise formula.
  double r2 = r * r;
  double e2 = e * e;
  double l2 = l * l;
  double displacement = e * (r2 - std::sqrt(l2 * r2 + e2 * (r2 - l2))) / (r2 - e2);

  vec3 ret = v;
  ret.x += displacement * vn.z;
  ret.z += displacement * -vn.x;
  return ret;
}

float foc_len = 10;
float eye_sep = 1;

Vec3f flat_stereo(Vec3f v) {
	Vec3f eye {eye_sep, 0.0f, 0.0f};
	float l = std::sqrt((v.x - eye.x) * (v.x - eye.x) + (v.z - eye.z) * (v.z - eye.z));
	float t = foc_len * (v.x - eye.x) / (-v.z);
	vec3 p = eye + Vec3f(t, 0.0f, -foc_len);
	p.normalize();
	p *= l;
	return {p.x, v.y, p.z};
}

Vec3f flat_stereo2(Vec3f v) {
	Vec3f eye {eye_sep, 0.0f, 0.0f};
	float l = std::sqrt((v.x - eye.x) * (v.x - eye.x) + (v.z - eye.z) * (v.z - eye.z));
	float t = foc_len * (v.x - eye.x) / (-v.z);
	v.x = -v.z * (eye_sep + t) / foc_len;
	v.normalize();
	v *= l;
	return v;
}

int main()
{

	cout << flat_stereo({0, 3, -20}) << endl;
	cout << flat_stereo2({0, 3, -20}) << endl;
	cout << flat_stereo({0, 3, -5}) << endl;
	cout << flat_stereo2({0, 3, -5}) << endl;



	// cout << omni_displace() << endl;
	// cout << displace2() << endl;
	// cout << omni_render() << endl;

	// r = 20;
	// e = 1;
	// v.set(50, 20, -30);

	// cout << endl;
	// cout << omni_displace() << endl;
	// cout << displace2() << endl;
	// cout << omni_render() << endl;

	// r = 100;
	// e = 1;
	// v.set(50, 10, -20);

	// cout << endl;
	// cout << omni_displace() << endl;
	// cout << displace2() << endl;
	// cout << omni_render() << endl;
}