uniform samplerCube CubeMap;
uniform float theta, phi;
varying vec3 normal, lightDir, r;

void main (void)
{
	vec3 newR = r;
	mat3 rotate_x = mat3(1, 0, 0,
			     0, cos(phi), -sin(phi),
			     0, sin(phi), cos(phi));
	mat3 rotate_y = mat3(cos(theta), -sin(theta), 0, 
				sin(theta), cos(theta), 0, 
				0, 0, 1);
	newR = rotate_x * rotate_y * newR;
	vec4 final_color = textureCube( CubeMap, newR);
	vec3 N = normalize(normal);
	vec3 L = normalize(lightDir);
	float lambertTerm = dot(N,L);
	if(lambertTerm > 0.0)
	{
		// Specular
		final_color += textureCube( CubeMap, newR);
	}
	gl_FragColor = final_color;
}
