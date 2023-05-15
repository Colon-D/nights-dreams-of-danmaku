uniform sampler2D texture;
uniform float disintegration;

// https://stackoverflow.com/a/4275343
float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    if (rand(round(gl_TexCoord[0].xy * 32) / 32) < disintegration) {
		discard;
	}
    gl_FragColor = gl_Color * texture2D(texture, gl_TexCoord[0].xy);
}
