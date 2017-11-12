
///////////////////////////////////////////////////////////////////////////////

Texture2D    glyphsTexture : register(t0);
SamplerState glyphsSampler : register(s0);

cbuffer ConstantBufferData : register(b0)
{
    matrix mvpMatrix;
    float4 screenDimensions;
};

///////////////////////////////////////////////////////////////////////////////

struct VertexInput
{
    float4 pos   : POSITION;
    float4 uv    : TEXCOORD;
    float4 color : COLOR;
};

struct VertexOutput
{
    float4 vpos  : SV_POSITION;
    float4 uv    : TEXCOORD;
    float4 color : COLOR;
};

///////////////////////////////////////////////////////////////////////////////
// Line/point drawing:
///////////////////////////////////////////////////////////////////////////////

VertexOutput VS_LinePoint(VertexInput input)
{
    VertexOutput output;
    output.vpos  = mul(input.pos, mvpMatrix);
    output.uv    = input.uv;
    output.color = input.color;
    return output;
}

float4 PS_LinePoint(VertexOutput input) : SV_TARGET
{
    return input.color;
}

///////////////////////////////////////////////////////////////////////////////
// Text glyphs drawing:
///////////////////////////////////////////////////////////////////////////////

VertexOutput VS_TextGlyph(VertexInput input)
{
    // Map to normalized clip coordinates:
    float x = ((2.0 * (input.pos.x - 0.5)) / screenDimensions.x) - 1.0;
    float y = 1.0 - ((2.0 * (input.pos.y - 0.5)) / screenDimensions.y);

    VertexOutput output;
    output.vpos  = float4(x, y, 0.0, 1.0);
    output.uv    = input.uv;
    output.color = input.color;
    return output;
}

float4 PS_TextGlyph(VertexOutput input) : SV_TARGET
{
    float4 texColor  = glyphsTexture.Sample(glyphsSampler, input.uv.xy);
    float4 fragColor = input.color;

    fragColor.a = texColor.r;
    return fragColor;
}

///////////////////////////////////////////////////////////////////////////////
