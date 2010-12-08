uniform samplerCube CubeMap;
uniform float theta, phi;
varying vec3 normal, lightDir, r;

void main (void)
{
	vec3 newR = r;
	mat3 rotate_theta = mat3(cos(theta), 0, sin(theta),
						0, 1, 0,
						-sin(theta), 0, cos(theta));
	newR = rotate_theta * r;

	mat3 rotate_phi = mat3(1, 0, 0,
					   0, cos(phi), -sin(phi),
					   0, sin(phi), cos(phi));
	newR = rotate_phi * newR;

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
