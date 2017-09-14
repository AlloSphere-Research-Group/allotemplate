#ifndef INLCUDE_AL_CUBEMAP_HPP
#define INLCUDE_AL_CUBEMAP_HPP

#include "al/core/math/al_Matrix4.hpp"
#include "al/core/gl/al_Viewpoint.hpp"
#include "al/core/gl/al_FBO.hpp"
#include "al/core/gl/al_Shader.hpp"
#include "al/core/gl/al_Texture.hpp"
#include "al/core/gl/al_VAOMesh.hpp"
#include "al/core/gl/al_Graphics.hpp"
#include "al/core/gl/al_Shapes.hpp"

namespace al {

class CubeRenderConstants {
public:
  static int const sampletex_binding_point = 20;
  static int const cubemap_binding_point = 21;
};

inline std::string cubevert() { return R"(
#version 330
uniform mat4 MV;
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
  vec4 vertex = MV * vec4(position, 1.0);
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
uniform mat4 MV;
uniform mat4 P;

layout (location = 0) in vec3 position;
layout (location = 2) in vec2 texcoord;

out vec2 texcoord_;

void main() {
  gl_Position = P * MV * vec4(position, 1.0);
  texcoord_ = texcoord;
}
)";}

inline std::string cubetexsamplefrag() { return R"(
#version 330
uniform sampler2D sample_tex;
uniform samplerCube cubemap;
in vec2 texcoord_;
out vec4 frag_color;
void main() {
  vec3 dir = texture(sample_tex, texcoord_).rgb;
  vec4 cube_color = texture(cubemap, dir);
  frag_color = cube_color;
}
)";}


inline std::string equirect_cubesample_frag() { return R"(
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

class CubeRender {
public:
  int res;
  Viewpoint view;
  Texture cubemap;
  RBO rbo;
  FBO fbo;
  ShaderProgram cubeshader;
  float radius;
  Graphics* g;

  void init(int resolution=1024, float near=0.1, float far=100, float sphere_radius = 1e10) {
    res = resolution;
    radius = sphere_radius;
    view.fovy(90).near(near).far(far);
    view.viewport(0, 0, res, res);
    cubemap.createCubemap(res, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
    rbo.create(res, res);
    fbo.bind();
    fbo.attachCubemapFace(cubemap, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
    fbo.attachRBO(rbo);
    fbo.unbind();
    cubeshader.compile(cubevert(), cubefrag());
    cubeshader.begin();
    cubeshader.uniform("omni_radius", radius);
    cubeshader.end();
  }

  void begin(Graphics& graphics) {
    g = &graphics;
    fbo.bind();
    g->shader(cubeshader);
    g->camera(view);
  }

  void set_eye(int i) {
    if (!g) return;
    g->shader().uniform("omni_eyeSep", 0.0);
  }

  void set_face(int f) {
    if (!g) return;
    g->shader().uniform("C", get_cube_mat(f));
    fbo.attachCubemapFace(cubemap, GL_TEXTURE_CUBE_MAP_POSITIVE_X+f);
  }

  void end() {
    if (!g) return;
    fbo.unbind();
    g = nullptr;
  }

  // void pose(Viewpoint& v) {
  //   view.pose(v);
  // }

  void pose(Pose const& v) {
    view.pose(v);
  }
};

class CubeSampler {
public:
  Texture* cubemap_tex;
  Texture* cubesample_tex;
  ShaderProgram sampleshader;
  VAOMesh texquad;

  void init() {
    // vertex shader doesn't use mvp matrix.
    // draw methos fills the viewport
    sampleshader.compile(cubesamplevert(), cubetexsamplefrag());
    sampleshader.begin();
    sampleshader.uniform("sample_tex", CubeRenderConstants::sampletex_binding_point);
    sampleshader.uniform("cubemap", CubeRenderConstants::cubemap_binding_point);
    sampleshader.end();

    // prepare textured quad to fill viewport with the result
    addTexQuad(texquad);
    texquad.update();
  }

  void sampleTexture(Texture& sample_texture) {
    cubesample_tex = &sample_texture;
    cubesample_tex->bind(CubeRenderConstants::sampletex_binding_point);
  }

  void cubemap(Texture& cubemap_texture) {
    cubemap_tex = &cubemap_texture;
    cubemap_tex->bind(CubeRenderConstants::cubemap_binding_point);
  }

  void draw(Graphics& g) {
    g.shader(sampleshader);
    g.camera(Viewpoint::IDENTITY);
    g.draw(texquad); // fill viewport
  }
};

}

#endif