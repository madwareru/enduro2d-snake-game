#version 120

uniform sampler2D u_texture;
varying vec2 v_uv;

void main(){
  vec2 uv = vec2(v_uv.s, 1.0 - v_uv.t);
  gl_FragColor = texture2D(u_texture, uv);
}
