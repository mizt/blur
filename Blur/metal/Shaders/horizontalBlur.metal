#define prefix horizontalBlur

#import <metal_stdlib>
#import "ShaderUtils.h"

using namespace metal;

vertex VertInOut VertexShader(prefix)(constant float4 *pos[[buffer(0)]],constant packed_float2  *texcoord[[buffer(1)]],uint vid[[vertex_id]]) {
    VertInOut outVert;
    outVert.pos = pos[vid];
    outVert.texcoord = float2(texcoord[vid][0],1-texcoord[vid][1]);
    return outVert;
}

fragment float4 FragmentShader(prefix)(VertInOut inFrag[[stage_in]],constant FragmentShaderArguments &args[[buffer(0)]]) {
    float2 resolution = args.resolution[0];
    float4 src = float4(0,0,0,1);
    for(int i=-10; i<=10; i++) {
        src += args.texture.sample(SAMPLAR,inFrag.texcoord.xy+float2(i,0)/resolution).rgba;
    }
    src/=21.;
    return float4(src.r,src.g,src.b,1);
}
