#version 130
in mediump vec2 UV;
flat in mediump vec3 normal;
flat in mediump vec3 light;
in mediump vec3 direction;
flat in mediump float diffuse;
out mediump vec3 fragColor;
uniform sampler2D tex;
void main()
{
  mediump float specular = max(0.0, dot(normalize(direction), reflect(light, normal)));
  if (specular != 0.0)
    specular = 6.0 * pow(specular, 128.0);
  fragColor = texture(tex, UV).rgb * (0.1 + 0.5 * diffuse) + 0.4 * specular;
}