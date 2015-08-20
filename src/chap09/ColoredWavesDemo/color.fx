uniform extern float4x4 gWVP;
uniform extern float gTime;

static float a[2] = {0.8f, 0.2f};  // amplitudes
static float k[2] = {1.0f, 8.0f};  // angular wave numbers
static float w[2] = {1.0f, 8.0f};  // angular frequency
static float p[2] = {0.0f, 1.0f};  // phase shifts

float SumOfRadialSineWaves(float x, float z)
{
	// Distance of vertex from source of waves (which we set
	// as the origin of the local space)
	float d = sqrt(x*x + z*z);

	float sum = 0.0f;
	for (int i = 0; i < 2; ++i)
		sum += a[i]*sin(k[i]*d - gTime*w[i] + p[i]);

	return sum;
}

float4 GetColorFromHeight(float y)
{
	float absy = abs(y);
	if (absy <= 0.2f) return float4(0.0f, 0.0f, 0.0f, 1.0f);
	else if (absy <= 0.4f) return float4(0.0f, 0.0f, 1.0f, 1.0f);
	else if (absy <= 0.6f) return float4(0.0f, 1.0f, 0.0f, 1.0f);
	else if (absy <= 0.8f) return float4(1.0f, 0.0f, 0.0f, 1.0f);
	else return float4(1.0f, 1.0f, 0.0f, 1.0f);
}

struct OutputVS
{
	float4 posH  : POSITION0;
	float4 color : COLOR0;
};

OutputVS ColorVS(float3 posL : POSITION0, float4 c : COLOR0)
{
	OutputVS outVS = (OutputVS)0;
	posL.y = SumOfRadialSineWaves(posL.x, posL.z);
	outVS.posH  = mul(float4(posL, 1.0f), gWVP);
	outVS.color = GetColorFromHeight(posL.y);
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

		FillMode = Wireframe;
	}
}