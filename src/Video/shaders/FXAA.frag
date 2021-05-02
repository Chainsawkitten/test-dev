/*
FXAA fragment shader.
*/
layout(location = 0) in vec2 inTexCoords;

layout(location = 0) out vec4 outColor;

precision mediump int;
precision mediump float;

MATERIAL(0, colorSampler)

UNIFORMS
{
    highp vec2 screenSize;
    highp float time;
    highp uint fxaaEnable;
    highp uint ditherEnable;
} uniforms;

const float FXAA_SPAN_MAX = 8.0f;
const float FXAA_REDUCE_MUL = 1.0f/8.0f;
const float FXAA_REDUCE_MIN = 1.0f/128.0f;
const float PI = 3.14159265359f;

vec3 fxaa() {
    highp vec2 invScreenSize = 1.0f / uniforms.screenSize;
    
    vec3 luma = vec3(0.299f, 0.587f, 0.114f);
    float lumaTL = dot(luma, texture(colorSampler, inTexCoords + (vec2(-1.0f, -1.0f) * invScreenSize)).rgb);
    float lumaTR = dot(luma, texture(colorSampler, inTexCoords + (vec2(1.0f, -1.0f) * invScreenSize)).rgb);
    float lumaBL = dot(luma, texture(colorSampler, inTexCoords + (vec2(-1.0f, 1.0f) * invScreenSize)).rgb);
    float lumaBR = dot(luma, texture(colorSampler, inTexCoords + (vec2(1.0f, 1.0f) * invScreenSize)).rgb);
    float lumaM  = dot(luma, texture(colorSampler, inTexCoords).rgb);

    vec2 dir;
    dir.x = -((lumaTL + lumaTR) - (lumaBL + lumaBR));
    dir.y = ((lumaTL + lumaBL) - (lumaTR + lumaBR));
    
    float dirReduce = max((lumaTL + lumaTR + lumaBL + lumaBR) * (FXAA_REDUCE_MUL * 0.25f), FXAA_REDUCE_MIN);
    float inverseDirAdjustment = 1.0f / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    
    dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), 
        max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * inverseDirAdjustment)) * invScreenSize;

    vec3 result1 = (1.0f/2.0f) * (
        texture(colorSampler, inTexCoords + (dir * vec2(1.0f/3.0f - 0.5f))).rgb +
        texture(colorSampler, inTexCoords + (dir * vec2(2.0f/3.0f - 0.5f))).rgb);

    vec3 result2 = result1 * (1.0f/2.0f) + (1.0f/4.0f) * (
        texture(colorSampler, inTexCoords + (dir * vec2(0.0f/3.0f - 0.5f))).rgb +
        texture(colorSampler, inTexCoords + (dir * vec2(3.0f/3.0f - 0.5f))).rgb);

    float lumaMin = min(lumaM, min(min(lumaTL, lumaTR), min(lumaBL, lumaBR)));
    float lumaMax = max(lumaM, max(max(lumaTL, lumaTR), max(lumaBL, lumaBR)));
    float lumaResult2 = dot(luma, result2);
    
    if(lumaResult2 < lumaMin || lumaResult2 > lumaMax)
        return result1;
    else
        return result2;
}

highp float rand(vec2 co) {
    highp float a = 12.9898f;
    highp float b = 78.233f;
    highp float c = 43758.5453f;
    highp float dt = dot(co.xy, vec2(a,b));
    highp float sn = mod(dt, PI);
    return fract(sin(sn) * c);
}

void main () {
    // FXAA.
    vec3 color;
    if (uniforms.fxaaEnable != 0) {
        color = fxaa();
    } else {
        color = texture(colorSampler, inTexCoords).rgb;
    }

	// Dither.
	if (uniforms.ditherEnable != 0) {
		float dither = rand(gl_FragCoord.xy / uniforms.screenSize + vec2(uniforms.time, 0.0f)) / 255.0f;
		color = color + vec3(dither);
	}
    
    outColor = vec4(color, 1.0);
}
