uniform sampler2D texture;
uniform float time_elapsed;

void main() {
    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
    float factor = abs(pixel.r - 0.5) * 2.0;
    gl_FragColor = mix(gl_Color, pixel, factor);
    gl_FragColor.a = pixel.a * gl_Color.a;
}
