#pragma once

#include <string>
#include"externals/DirectXTex/DirectXTex.h"
#include"externals/DirectXTex/d3dx12.h"
#include "DirectXCommon.h"


class TextureManager
{
private:

	static TextureManager* instance;
	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = default;
	TextureManager& operator=(TextureManager&) = delete;
	//テクスチャ1枚分のデータ
	struct TextureDate {
		std::string filePath;
		DirectX::TexMetadata metadata;
		Microsoft::WRL::ComPtr<ID3D12Resource>resource;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
	};

public:

	//シングルトンインタンス
	static TextureManager* GetInstance();

	//終了
	void Finalize();

	// 初期化
	void Initialize(DirectXCommon* dxCommon);

	//テクスチャファイルの読み込み
	void LoadTexture(const std::string& filePath);
	//SRVインデックスの開始番号
	uint32_t GetTextureIndexByFilePath(const std::string& filePath);
	//テクスチャ番号からCPUハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE GeTSrvHandleGPU(uint32_t textureIndex);
	static uint32_t kSRVIndexTop;

private:

	//テクスチャデータ
	std::vector<TextureData>textureDatas;;

	DirectXCommon* dxCommon_ = nullptr;
};