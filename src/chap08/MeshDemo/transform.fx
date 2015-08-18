uniform extern float4x4 gWVP;

struct OutputVS
{
	float4 posH : POSITION0;
};

OutputVS TransformVS(float3 posL : POSITION0)
{
	OutputVS outVS = (OutputVS)0;
	outVS.posH = mul(float4(posL, 1.0f), gWVP);
	return outVS;
}

float4 TransformPS() : COLOR
{
	return float4(0.0f, 0.0f, 0.0f, 1.0f);
}

technique TransformTech
{
	pass P0
	{
		vertexShader = compile vs_2_0 TransformVS();
		pixelShader  = compile ps_2_0 TransformPS();

		FillMode = Wireframe;
	}
}