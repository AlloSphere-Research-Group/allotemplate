#ifndef INLCUDE_AL_CUBEMAP_HPP
#define INLCUDE_AL_CUBEMAP_HPP

#include "al/core/math/al_Matrix4.hpp"
#include "al/core/gl/al_Viewpoint.hpp"
#include "al/core/gl/al_FBO.hpp"
#include "al/core/gl/al_Shader.hpp"
#include "al/core/gl/al_Texture.hpp"
#include "al/core/gl/al_VAOMesh.hpp"


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

layout (location = 0) in vec3 position;
layout (location = 2) in vec2 texcoord;

out vec2 texcoord_;

void main() {
  gl_Position = vec4(position, 1.0);
  texcoord_ = texcoord;
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
  int res_;
  Viewpoint view_;
  Texture cubemap_;
  RBO rbo_;
  FBO fbo_;
  ShaderProgram cubeshader_;
  float radius_;
  Graphics* g;

  void init(int res=1024, float near=0.1, float far=100, float radius = 1e10) {
    res_ = res;
    radius_ = radius;
    view_.fovy(90).near(near).far(far);
    view_.viewport(0, 0, res_, res_);
    cubemap_.createCubemap(res_, GL_RGBA32F, GL_RGBA, GL_FLOAT);
    rbo_.create(res_, res_);
    fbo_.bind();
    fbo_.attachCubemapFace(cubemap_, GL_TEXTURE_CUBE_MAP_POSITIVE_X);
    fbo_.attachRBO(rbo_);
    fbo_.unbind();
    cubeshader_.compile(cubevert(), cubefrag());
    cubeshader_.begin();
    cubeshader_.uniform("tex0", 0);
    cubeshader_.uniform("tex0_mix", 0.0);
    cubeshader_.uniform("light_mix", 0.0f);
    cubeshader_.uniform("omni_radius", radius_);
    cubeshader_.end();
  }

  void begin(Graphics& graphics) {
    g = &graphics;
    fbo_.bind();
    g->shader(cubeshader_);
    g->camera(view_);
  }

  void set_eye(int i) {
    g->shader(cubeshader_);
    g->shader().uniform("omni_eyeSep", 0.0);
  }

  void set_face(int f) {
    g->shader(cubeshader_);
    g->shader().uniform("C", get_cube_mat(f));
    fbo_.attachCubemapFace(cubemap_, GL_TEXTURE_CUBE_MAP_POSITIVE_X+f);
  }

  void end() {
    fbo_.unbind();
  }

  void view(Viewpoint& v) {
    view_.pose(v);
  }
};

class CubeSampler {
public:
  Texture* cubemap_;
  Texture* cubesampletex_;
  ShaderProgram sampleshader_;
  VAOMesh quad_;

  void init() {
    // vertex shader doesn't use mvp matrix.
    // draw methos fills the viewport
    sampleshader_.compile(cubesamplevert(), cubetexsamplefrag());
    sampleshader_.begin();
    sampleshader_.uniform("sample_tex", 0);
    sampleshader_.uniform("cubemap", 1);
    sampleshader_.end();

    quad_.reset();
    quad_.primitive(Mesh::TRIANGLES);
    quad_.vertex(-1, -1, 0);
    quad_.texCoord(0.0, 0.0);
    quad_.vertex(1, -1, 0);
    quad_.texCoord(1.0, 0.0);
    quad_.vertex(-1, 1, 0);
    quad_.texCoord(0.0, 1.0);
    quad_.vertex(-1, 1, 0);
    quad_.texCoord(0.0, 1.0);
    quad_.vertex(1, -1, 0);
    quad_.texCoord(1.0, 0.0);
    quad_.vertex(1, 1, 0);
    quad_.texCoord(1.0, 1.0);
    quad_.update();
  }

  // fills viewport
  void draw() {
    auto& g = graphics();
    g.shader(sampleshader_);
    g.texture(*cubesampletex_, 0);
    g.texture(*cubemap_, 1);
    g.draw(quad_);
  }
};

}

#endif