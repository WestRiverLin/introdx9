uniform extern float4x4 gWorldInverseTranspose;
uniform extern float4x4 gWVP;

uniform extern float4 gAmbientMtrl;
uniform extern float4 gAmbientLight;
uniform extern float4 gDiffuseMtrl;
uniform extern float4 gDiffuseLight;
uniform extern float3 gLightVecW;

struct OutputVS
{
	float4 posH  : POSITION0;
	float4 color : COLOR0;
};

OutputVS DiffuseVS(float3 posL : POSITION0, float3 normalL : NORMAL0)
{
	OutputVS outVS = (OutputVS)0;

	// Transform normal to world space
	float3 normalW = mul(float4(normalL, 0.0f), gWorldInverseTranspose).xyz;
	normalW = normalize(normalW);

	// Compute ambient + diffuse color
	float s = max(dot(gLightVecW, normalW), 0.0f);
	float3 diffuse = s*(gDiffuseMtrl*gDiffuseLight).rgb;
	float3 ambient = gAmbientMtrl*gAmbientLight;
	outVS.color.rgb = ambient + diffuse;
	outVS.color.a   = gDiffuseMtrl.a;

	// Transform to homogeneous clip space
	outVS.posH = mul(float4(posL, 1.0f), gWVP);

	return outVS;
}

float4 DiffusePS(float4 c : COLOR0) : COLOR
{
	return c;
}

technique DiffuseTech
{
	pass P0
	{
		vertexShader = compile vs_2_0 DiffuseVS();
		pixelShader  = compile ps_2_0 DiffusePS();
	}
}

