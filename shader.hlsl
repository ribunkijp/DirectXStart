/**********************************************************************************
    shader.hlsl

                                                                LI WENHUI
                                                                2025/07/14

**********************************************************************************/

cbuffer ConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
    
    float2 texOffset;
    float2 texScale;
    float2 padding; // 16バイトアライメントを保持するため
};
// テクスチャオブジェクト (Texture2D) とサンプラー (SamplerState) を宣言
// register(t0) はテクスチャをレジスタ t0 にバインドすることを意味する
// register(s0) はサンプラーをレジスタ s0 にバインドすることを意味する
Texture2D shaderTexture : register(t0);
SamplerState SamplerClamp : register(s0);

// 頂点シェーダーの入力構造体 VS_INPUT
struct VS_INPUT
{
    float3 pos : POSITION; // スクリーン座標（ピクセル）
    float4 col : COLOR;
    float2 tex : TEXCOORD; // テクスチャ座標入力
};

// この構造体は頂点シェーダーからピクセルシェーダーに渡すデータで、
// 中のテクスチャ座標は自動で補間される
struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float2 tex : TEXCOORD; // テクスチャ座標出力
};
// 頂点シェーダーのメイン関数 VSMain
PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;

    // 3Dのposを4Dのfloat4に変換するのは、4x4行列を掛けるため
    float4 localPos = float4(input.pos, 1.0f);
    
    //
    float4 worldPos = mul(localPos, model);
    float4 viewPos = mul(worldPos, view);
    float4 projPos = mul(viewPos, projection);

    output.pos = projPos;
    
    output.col = input.col; // 頂点カラーをピクセルシェーダーへ渡す
    output.tex = input.tex * texScale + texOffset;
    //
    return output;
}

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    
     // サンプラーと補間後のテクスチャ座標を使ってテクスチャから色をサンプリング
    float4 textureColor = shaderTexture.Sample(SamplerClamp, input.tex);

    // テクスチャの色だけを使う：
    return textureColor;
    
     // 頂点シェーダーから渡された色をそのまま出力する場合。各ピクセルの色は頂点から補間されたものになる。
    //return input.col;
    

    // テクスチャカラーと頂点カラーを掛け合わせる（テクスチャの色付けを実現）：
    // return textureColor * input.col;
    
   
}