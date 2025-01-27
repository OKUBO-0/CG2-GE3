#pragma once

#include "Matrix4x4.h"
#include "Vector4.h"
#include "Vector3.h"
#include "Vector2.h"
#include "RenderingData.h"
#include "Model.h"

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

	void SetModel(Model* model) { model_ = model; }
	void SetModel(const std::string& filepath);

	//transform
	void SetTransform(const Transform& transform) { this->transform = transform; }
	Transform GetTransform() { return transform; }

	//スケール
	void SetScale(const Vector3& scale) { transform.scale = scale; }
	//回転
	void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
	//位置
	void SetTranslate(const Vector3& translate) { transform.translate = translate; }


private:

	Object3dCommon* object3dCommon_ = nullptr;


	// モデル
	Model* model_ = nullptr;

	// トランスフォーム
	// ModelTransform用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;
	// データを書き込む
	TransformationMatrix* transformationMatrixData = nullptr;

	// 平行光源
	// 平行光源用のResourceを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;
	DirectionalLight* directionalLightData = nullptr;

	// SRT
	Transform transform;
	// カメラ用のTransformを作る
	Transform cameratransform;
	Matrix4x4 cameraMatrix;
	Matrix4x4 viewMatrix;

	Matrix4x4 worldMatrix;
	Matrix4x4 projectionMatrix;
	Matrix4x4 worldViewProjectionMatrix;
};