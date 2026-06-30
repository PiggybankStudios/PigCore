@ctype mat4 mat4
@ctype vec2 v2
@ctype vec3 v3
@ctype vec4 v4r

// +--------------------------------------------------------------+
// |                        Vertex Shader                         |
// +--------------------------------------------------------------+
@vs vertex_shader

layout(binding=0) uniform simple_VertParams
{
	uniform mat4 world;
	uniform mat4 view;
	uniform mat4 projection;
};

in vec2 position;
in vec4 color0;

out vec4 fragColor;

void main()
{
	gl_Position = vec4(1.0f, 1.0f, 1.0f, 1.0f); //projection * (view * (world * vec4(position, 0.0f, 1.0f)));
	// fragColor = color0;
}
@end

// +--------------------------------------------------------------+
// |                       Fragment Shader                        |
// +--------------------------------------------------------------+
@fs fragment_shader

layout(binding=1) uniform simple_FragParams
{
	uniform vec4 tint;
};

in vec4 fragColor;
out vec4 frag_color;

void main()
{
	frag_color = vec4(1.0f, 0.0f, 0.0f, 1.0f); //fragColor * tint;
}
@end

@program simple vertex_shader fragment_shader
