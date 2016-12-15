#version 330 core 
uniform sampler2D tex;
uniform vec2 resolution;
uniform vec3 FXAASettings;

#define FXAA_SPAN_MAX   FXAASettings.x
#define FXAA_REDUCE_MUL FXAASettings.y
#define FXAA_REDUCE_MIN FXAASettings.z

in DATA
{
   vec2 Position; 
   vec4 Color;
   vec2 TexCoords;
}fs_in;

out vec4 Color;
 
void main( void ) {

    vec3 rgbNW=texture2D(tex,fs_in.TexCoords+(vec2(-1.0,-1.0)/resolution)).xyz;
    vec3 rgbNE=texture2D(tex,fs_in.TexCoords+(vec2(1.0,-1.0)/resolution)).xyz;
    vec3 rgbSW=texture2D(tex,fs_in.TexCoords+(vec2(-1.0,1.0)/resolution)).xyz;
    vec3 rgbSE=texture2D(tex,fs_in.TexCoords+(vec2(1.0,1.0)/resolution)).xyz;
    vec3 rgbM=texture2D(tex,fs_in.TexCoords).xyz;

    vec3 luma=vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX),
          max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
          dir * rcpDirMin)) / resolution;

    vec3 rgbA = 0.5 * (
        texture2D(tex, fs_in.TexCoords.xy + dir * (1.0/3.0 - 0.5)).xyz +
        texture2D(tex, fs_in.TexCoords.xy + dir * (2.0/3.0 - 0.5)).xyz);
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture2D(tex, fs_in.TexCoords.xy + dir * (0.0/3.0 - 0.5)).xyz +
        texture2D(tex, fs_in.TexCoords.xy + dir * (3.0/3.0 - 0.5)).xyz);
    float lumaB = dot(rgbB, luma);

    if((lumaB < lumaMin) || (lumaB > lumaMax)){
      Color = vec4(rgbA, 1.0);
    }else{
        Color.xyz=rgbB;
    }
}