const int MAX_KERNEL_SIZE = 25;
uniform sampler2D tex;
uniform vec2 offsets[MAX_KERNEL_SIZE]; 
uniform float kernel[MAX_KERNEL_SIZE];
void main(void) { 
	vec4 blur_vec = vec4(0,0,0,0);
	for (int i = 0; i < MAX_KERNEL_SIZE; i++) {
		vec2 loc = gl_TexCoord[0].xy;
		loc[1] = -loc[1];
		loc[1] += 1;
		blur_vec += kernel[i] * texture2D(tex, loc + offsets[i]);
	}
	gl_FragColor = blur_vec;
}
