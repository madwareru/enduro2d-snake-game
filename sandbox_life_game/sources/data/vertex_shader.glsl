#version 120

attribute vec3 a_vertex;
attribute vec2 a_st0;
attribute vec4 a_color0;

uniform float u_time;
uniform mat4 u_matrix_m;
uniform mat4 u_matrix_vp;

varying vec4 v_color;
varying vec2 v_uv;

void main(){
  v_color = a_color0;
  v_uv = a_st0;
  gl_Position = vec4(a_vertex, 1.0) * u_matrix_m * u_matrix_vp;
}
