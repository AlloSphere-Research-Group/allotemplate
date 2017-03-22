#ifndef INLCUDE_AL_CUBEMAP_HPP
#define INLCUDE_AL_CUBEMAP_HPP

#include "al/core/math/al_Matrix4.hpp"

namespace al {

inline std::string cubevert() { return R"(
#version 330
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform mat4 C; // cubemap vertex displacement

// @omni_eyeSep: the eye parallax distance.
//  This will be zero for mono, and positive/negative for right/left eyes.
uniform float omni_eyeSep;

// @omni_radius: the radius of the sphere in OpenGL units.
//  This will be infinity for the original layout (we default to 1e10).
uniform float omni_radius;

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec3 normal;

out vec4 color_;
out vec2 texcoord_;
out vec3 normal_;

vec4 displace(in vec4 vertex) {
  float l = length(vertex.xz);
  vec3 vn = normalize(vertex.xyz);

  // Precise formula.
  float displacement = omni_eyeSep * (omni_radius * omni_radius - sqrt(l * l * omni_radius * omni_radius + omni_eyeSep * omni_eyeSep * (omni_radius * omni_radius - l * l))) / (omni_radius * omni_radius - omni_eyeSep * omni_eyeSep);

  // Approximation, safe if omni_radius / omni_eyeSep is very large, which is true for the allosphere.
  // float displacement = omni_eyeSep * (1.0 - l / omni_radius);

  // Displace vertex.
  return vertex + vec4(displacement * vn.z, 0.0, displacement * -vn.x, 0.0);
}

void main() {
  vec4 vertex = V * M * vec4(position, 1.0);
  gl_Position = P * C * displace(vertex);
  color_ = color;
  texcoord_ = texcoord;
  normal_ = normal;
}
)";}

inline std::string cubefrag() { return R"(
#version 330
uniform sampler2D tex0;
uniform float tex0_mix;
uniform float light_mix;

in vec4 color_;
in vec2 texcoord_;
in vec3 normal_;

out vec4 frag_color;

void main() {
  vec4 tex_color0 = texture(tex0, texcoord_);
  vec4 light_color = vec4(normal_, 1.0); // TODO
  vec4 final_color = mix(mix(color_, tex_color0, tex0_mix), light_color, light_mix);
  frag_color = final_color;
}
)";}

inline std::string cubesamplevert() { return R"(
#version 330
uniform mat4 MVP;

layout (location = 0) in vec3 position;
layout (location = 2) in vec2 texcoord;

out vec2 texcoord_;

void main() {
  gl_Position = MVP * vec4(position, 1.0);
  texcoord_ = texcoord;
}
)";}

inline std::string cubesamplefrag() { return R"(
#version 330
uniform samplerCube cubemap;
in vec2 texcoord_;
out vec4 frag_color;
void main() {
  float longi = texcoord_.x * 3.1415926535 * 2.0;
  float latti = (texcoord_.y - 0.5) * 3.1415926535;
  vec3 equirectdir = vec3(cos(longi)*cos(latti), sin(latti), sin(longi)*cos(latti));
  vec4 cube_color = texture(cubemap, equirectdir);
  frag_color = cube_color;
}
)";}

Mat4f get_cube_mat(int face) {
  switch (face) {
    // GL_TEXTURE_CUBE_MAP_POSITIVE_X
    // vertex.xyz = vec3(-vertex.z, -vertex.y, -vertex.x);
    case 0: return Mat4f {
      0, 0,-1, 0,
      0,-1, 0, 0,
      -1, 0, 0, 0,
      0, 0, 0, 1
    };
    // GL_TEXTURE_CUBE_MAP_NEGATIVE_X
    // vertex.xyz = vec3(vertex.z, -vertex.y, vertex.x);
    case 1: return Mat4f {
      0, 0, 1, 0,
      0, -1, 0, 0,
      1, 0, 0, 0,
      0, 0, 0, 1
    };
    // GL_TEXTURE_CUBE_MAP_POSITIVE_Y
    // vertex.xyz = vec3(vertex.x, vertex.z, -vertex.y);
    case 2: return Mat4f {
      1, 0, 0, 0,
      0, 0, 1, 0,
      0, -1, 0, 0,
      0, 0, 0, 1
    };
    // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
    // vertex.xyz = vec3(vertex.x, -vertex.z, vertex.y);
    case 3: return Mat4f {
      1, 0, 0, 0,
      0, 0, -1, 0,
      0, 1, 0, 0,
      0, 0, 0, 1
    };
    // GL_TEXTURE_CUBE_MAP_POSITIVE_Z
    // vertex.xyz = vec3(vertex.x, -vertex.y, -vertex.z);
    case 4: return Mat4f {
      1, 0, 0, 0,
      0, -1, 0, 0,
      0, 0, -1, 0,
      0, 0, 0, 1
    };
    // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    // vertex.xyz = vec3(-vertex.x, -vertex.y, vertex.z);
    case 5: return Mat4f {
      -1, 0, 0, 0,
      0, -1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1
    };
  }
  return Mat4f::identity();
}

}

#endif