#version 120

attribute vec3 a_vertex;
attribute vec2 a_st0;
attribute vec4 a_color0;

uniform float u_game_time;
uniform mat4 u_matrix_m;
uniform mat4 u_matrix_vp;

varying vec4 v_color;
varying vec2 v_uv;

void main(){
  v_color = a_color0;
  v_uv = a_st0;

  float s = 0.7 + 0.3 * (cos(u_game_time * 3.0) + 1.0);
  gl_Position = vec4(a_vertex * s, 1.0) * u_matrix_m * u_matrix_vp;
}
