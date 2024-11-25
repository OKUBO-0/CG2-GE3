#pragma once

#include "Matrix4x4.h"
#include "Vector4.h"
#include "Vector3.h"
#include "Vector2.h"
#include "RenderingData.h"

class Object3dCommon;
class Object3d
{

public:

	// 初期化
	void Initialize(Object3dCommon* object3DCommon);

	// 更新
	void Update();

	// 描画
	void Draw();

	static MaterialData LoadMaterialTemplateFile(const std::string& directorypath, const std::string& filename);
	static ModelData LoadObjeFile(const std::string& ditrectoryPath, const std::string& filename);


private:

	Object3dCommon* object3dCommon_ = nullptr;


	// モデル
	// objファイルのデータ
	ModelData modelData;
	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;


	// マテリアル
	// modelマテリアル用のリソースを作る。今回color1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	// マテリアルにデータを書き込む	
	Material* materialData = nullptr;
	// トランスフォーム
	// ModelTransform用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	// データを書き込む
	TransformationMatrix* transformaitionMatrixData = nullptr;


	// 平行光源
	// 平行光源用のResoureceを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;
	DirectionalLight* directionalLightData = nullptr;


	// SRT
	Transform transform;
	// カメラ用のTransformを作る
	Transform cameraTransform;
};

