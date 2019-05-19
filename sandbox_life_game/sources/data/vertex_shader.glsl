#version 120

attribute vec3 a_vertex;
attribute vec2 a_st0;

uniform mat4 u_matrix_m;
uniform mat4 u_matrix_vp;

varying vec2 v_uv;

void main(){
  v_uv = a_st0;
  gl_Position = vec4(a_vertex, 1.0) * u_matrix_m * u_matrix_vp;
}
