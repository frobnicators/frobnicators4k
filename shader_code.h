/* File generated with Shader Minifier 1.1.4
 * http://www.ctrl-alt-test.fr
 */
#ifndef SHADER_CODE_H_
# define SHADER_CODE_H_

const char * const common_glsl =
  "#version 150\n"
  "#extension GL_ARB_explicit_attrib_location:enable\n"
  "uniform float t,b;"
  "float _db(vec3 n,vec3 v,vec3 m)"
  "{"
    "vec3 f=abs(v-n)-m;"
    "return max(f.x,max(f.y,f.z));"
  "}"
  "vec3 _n(vec3 n)"
  "{"
    "vec3 v,m;"
    "derp(vec3(1.,0,0),m);"
    "return normalize(v);"
  "}";

#endif // SHADER_CODE_H_
