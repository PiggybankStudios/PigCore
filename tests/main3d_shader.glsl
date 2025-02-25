@ctype mat4 mat4
@ctype vec2 v2
@ctype vec3 v3
@ctype vec4 v4r

// +--------------------------------------------------------------+
// |                        Vertex Shader                         |
// +--------------------------------------------------------------+
@vs vertex_shader

layout(binding=0) uniform main3d_VertParams
{
	uniform mat4 world;
	uniform mat4 view;
	uniform mat4 projection;
	uniform vec2 main3d_texture0_size;
	uniform vec4 sourceRec0;
};

in vec3 position;
in vec2 texCoord0;
in vec4 color0;

out vec4 fragColor;
out vec2 fragSampleCoord;

void main()
{
	gl_Position = projection * (view * (world * vec4(position, 1.0f)));
	fragColor = color0;
	fragSampleCoord = vec2(
		((texCoord0.x * sourceRec0.z) + sourceRec0.x) / main3d_texture0_size.x,
		((texCoord0.y * sourceRec0.w) + sourceRec0.y) / main3d_texture0_size.y
	);
}
@end

// +--------------------------------------------------------------+
// |                       Fragment Shader                        |
// +--------------------------------------------------------------+
@fs fragment_shader

layout(binding=1) uniform main3d_FragParams
{
	uniform vec4 tint;
};
layout(binding=0) uniform texture2D main3d_texture0;
layout(binding=0) uniform sampler main3d_sampler0;

in vec4 fragColor;
in vec2 fragSampleCoord;
out vec4 frag_color;

void main()
{
	vec4 sampleColor = texture(sampler2D(main3d_texture0, main3d_sampler0), fragSampleCoord);
	frag_color = fragColor * sampleColor * tint;
}
@end

@program main3d vertex_shader fragment_shader
