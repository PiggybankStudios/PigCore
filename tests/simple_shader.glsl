@vs vertex

in vec2 position;
in vec4 color0;

out vec4 color;

void main()
{
	gl_Position = vec4(position, 0.0f, 1.0f);
	color = color0;
}
@end

@fs pixel
in vec4 color;
out vec4 frag_color;

void main()
{
	frag_color = vec4(sin(color.r*12)*0.5f+0.5f, sin(color.g*6)*0.5f+0.5f, sin(color.b*5)*0.5f+0.5f, color.a);
}
@end

@program simple vertex pixel
