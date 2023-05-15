uniform sampler2D texture;
uniform float time_elapsed;

void main() {
    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
    float factor = abs(pixel.r - 0.5) * 2.0;
    vec4 color = gl_Color + vec4(vec3(0.2 * sin(time_elapsed * 12.0)), 1.0);
    gl_FragColor = mix(color, pixel, factor);
    gl_FragColor.a = pixel.a;
}
