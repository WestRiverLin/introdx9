uniform extern float4x4 gWVP;

struct OutputVS
{
	float4 posH  : POSITION0;
	float4 color : COLOR0;
};

OutputVS ColorVS(float3 posL : POSITION0, float4 c : COLOR0)
{
	OutputVS outVS = (OutputVS)0;
	outVS.posH  = mul(float4(posL, 1.0f), gWVP);
	outVS.color = c;
	return outVS;
}

float4 ColorPS(float4 c : COLOR0) : COLOR
{
	return c;
}

technique ColorTech
{
	pass P0
	{
		vertexShader = compile vs_2_0 ColorVS();
		pixelShader  = compile ps_2_0 ColorPS();
	}
}