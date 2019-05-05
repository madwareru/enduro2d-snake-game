#version 120

uniform sampler2D u_texture;
varying vec4 v_color;
varying vec2 v_uv;

void main(){
  vec2 uv = vec2(v_uv.s, 1.0 - v_uv.t);
  gl_FragColor = v_color * texture2D(u_texture, uv);
}
