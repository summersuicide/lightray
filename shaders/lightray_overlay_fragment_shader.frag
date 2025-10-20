#version 450

layout(location = 0) in vec2 in_uv;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec4 out_color;

layout(binding = 1) uniform sampler2DArray u_combined_sampler; 

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

void main()
{

	const vec3 smpl = texture(u_combined_sampler, vec3(in_uv, 0.0)).rgb;
	const float sdf = median(smpl.r, smpl.g, smpl.b);
	float base_smoothing = 0.05;
	float dynamic_smoothing = fwidth(sdf);
	float smoothing = mix(base_smoothing, dynamic_smoothing, smoothstep(0.45, 0.55, sdf));
	float edge_softness = clamp(4.2 * fwidth(sdf), 0.02, 0.35);

	const float alpha = smoothstep(0.2 - edge_softness, 0.2 + edge_softness, sdf);
	vec3 premultiplied_color = in_color * alpha;

	out_color = vec4(premultiplied_color, alpha);
}