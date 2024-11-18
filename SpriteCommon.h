#pragma once
#include "DirectXCommon.h"

class SpriteCommon
{

public:

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon);

	//共通描画設定
	void CommonDraw();

	DirectXCommon* GetDxCommon()const { return dxCommon_; }


private:
	//ルートシグネチャの作成
	void RootSignatureInitialize();

	//グラフィックスパイプライン
	void GraphicsPipelineInitialize();

private:
	DirectXCommon* dxCommon_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

	//バッファリソース
	//Sprite用の頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = dxCommon_->CreateBufferResource(sizeof(VertexData) * 4);
	
	//Sprite用のindexResourceを作成								  
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = dxCommon_->CreateBufferResource(sizeof(uint32_t) * 6);
};