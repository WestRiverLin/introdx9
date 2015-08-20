uniform extern float4x4 gWorldInverseTranspose;
uniform extern float4x4 gWVP;

uniform extern float4 gAmbientMtrl;
uniform extern float4 gAmbientLight;
uniform extern float4 gDiffuseMtrl;
uniform extern float4 gDiffuseLight;
uniform extern float3 gLightVecW;

float DiffuseToonIt(float s)
{
	if (s <= 0.25) return 0.4;
	else if (s <= 0.85) return 0.6;
	else return 1.f;
}

struct OutputVS
{
	float4 posH    : POSITION0;
	float3 normalW : TEXCOORD0;
};

OutputVS ToonVS(float3 posL : POSITION0, float3 normalL : NORMAL0)
{
	OutputVS outVS = (OutputVS)0;

	// Transform normal to world space
	outVS.normalW = mul(float4(normalL, 0.0f), gWorldInverseTranspose).xyz;
	outVS.normalW = normalize(outVS.normalW);

	// Transform to homogeneous clip space
	outVS.posH = mul(float4(posL, 1.0f), gWVP);

	return outVS;
}

float4 ToonPS(float3 normalW : TEXCOORD0) : COLOR
{
	normalW = normalize(normalW);

	// Compute ambient + diffuse color
	float s = max(dot(gLightVecW, normalW), 0.0f);
	s = DiffuseToonIt(s);
	float3 diffuse = s*(gDiffuseMtrl*gDiffuseLight).rgb;
	float3 ambient = (gAmbientMtrl*gAmbientLight).rgb;

	return float4(ambient + diffuse, gDiffuseMtrl.a);
}

technique ToonTech
{
	pass P0
	{
		vertexShader = compile vs_2_0 ToonVS();
		pixelShader  = compile ps_2_0 ToonPS();
	}
}

