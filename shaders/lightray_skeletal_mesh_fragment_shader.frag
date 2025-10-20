#version 460

layout(location = 0) in vec3 frag_normal;
layout(location = 1) in vec3 frag_position;
layout(location = 2) in vec3 frag_color;

layout(location = 0) out vec4 out_color;

void main()
{
    float light_strength = 0.9f;
    vec3 light_position = vec3(1.2f, 1.0f, 4.0f);
    vec3 light_color = vec3(0.6f);

    float ambient_str = 0.1f;
    vec3 ambient = ambient_str * light_color;

    vec3 normal = normalize(frag_normal);
    vec3 light_direction = normalize(light_position - frag_position);
    float difference = max(dot(normal, light_direction), 0.0f);
    vec3 diffuse = difference * light_color;
    vec3 lighting = (ambient + diffuse) * light_strength;
    vec3 result = lighting * frag_color;

	out_color = vec4(result, 1.0);
}