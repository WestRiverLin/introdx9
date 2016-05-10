uniform extern float4x4 gWorld;
uniform extern float4x4 gWorldInvTrans;
uniform extern float4x4 gWVP;

uniform extern float4 gAmbientMtrl;
uniform extern float4 gAmbientLight;
uniform extern float4 gDiffuseMtrl;
uniform extern float4 gDiffuseLight;
uniform extern float4 gSpecularMtrl;
uniform extern float4 gSpecularLight;
uniform extern float  gSpecularPower;
uniform extern float3 gLightVecW;
uniform extern float3 gEyePosW;
uniform extern texture gTex;

sampler TexS = sampler_state
{
	Texture = <gTex>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU = WRAP;
	AddressV = WRAP;
};

struct OutputVS
{
	float4 posH    : POSITION0;
	float4 diffuse : COLOR0;
	float4 spec    : COLOR1;
	float2 tex0    : TEXCOORD0;
};

OutputVS DirLightTexVS(float3 posL : POSITION0, float3 normalL : NORMAL0, float2 tex0 : TEXCOORD0)
{
	OutputVS outVS = (OutputVS)0;

	float3 normalW = mul(float4(normalL, 0.0f), gWorldInvTrans).xyz;
	normalW = normalize(normalW);

	float3 posW = mul(float4(posL, 1.0f), gWorld).xyz;

	float3 toEye = normalize(gEyePosW - posW);

	float3 r = reflect(-gLightVecW, normalW);

	float t = pow(max(dot(r, toEye), 0.0f), gSpecularPower);

	float s = max(dot(gLightVecW, normalW), 0.0f);

	float3 spec = t*(gSpecularMtrl*gSpecularLight).rgb;
	float3 diffuse = s*(gDiffuseMtrl*gDiffuseLight).rgb;
	float3 ambient = (gAmbientMtrl*gAmbientLight).rgb;

	outVS.diffuse.rgb = ambient + diffuse;
	outVS.diffuse.a   = gDiffuseMtrl.a;
	outVS.spec        = float4(spec, 0.0f);

	outVS.posH = mul(float4(posL, 1.0f), gWVP);

	outVS.tex0 = tex0;

	return outVS;
}

float4 DirLightTexPS(float4 c : COLOR0, float4 spec : COLOR1, float2 tex0 : TEXCOORD0) : COLOR
{
	float4 texColor = tex2D(TexS, tex0);
	float3 diffuse = c.rgb * texColor.rgb;
	return float4(diffuse + spec.rgb, texColor.a * c.a);
}

technique DirLightTexTech
{
	pass P0
	{
		vertexShader = compile vs_2_0 DirLightTexVS();
		pixelShader  = compile ps_2_0 DirLightTexPS();
	}
}
