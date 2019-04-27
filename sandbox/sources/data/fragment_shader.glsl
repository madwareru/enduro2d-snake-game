#version 120

uniform float u_game_time;
uniform sampler2D u_texture1;
uniform sampler2D u_texture2;
varying vec4 v_color;
varying vec2 v_uv;

void main(){
  vec2 uv = vec2(v_uv.s, 1.0 - v_uv.t);
  if ( u_game_time > 20.0 ) {
    gl_FragColor = v_color * texture2D(u_texture2, uv);
  } else {
    gl_FragColor = v_color * texture2D(u_texture1, uv);
  }
}
