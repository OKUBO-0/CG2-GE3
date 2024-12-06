#include "Object3dCommon.h"
#include "Object3d.h"
#include "MyMath.h"
#include "TextureManager.h"
#include "RenderingPipeline.h"
#include "ModelManager.h"


void Object3d::Initialize(Object3dCommon* object3dCommon)
{
	//引数で受け取って、メンバ変数に記録する
	object3dCommon_ = object3dCommon;

#pragma region ModelTransform
	//ModelTransform用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	transformationMatrixResource = object3dCommon_->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));
	//書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	//単位行列を書き込む
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();
#pragma endregion
	

#pragma region 平行光源
	//平行光源用のResourceを作成
	directionalLightResource = object3dCommon_->GetDxCommon()->CreateBufferResource(sizeof(DirectionalLight));
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,-1.0f,1.0f };
	directionalLightData->intensity = 1.0f;
#pragma endregion
	
	//カメラとモデルのTransform変数
	transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	//カメラ用のTransformを作る
	cameraTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{ 0.0f,0.0f,-5.0f} };
}


void Object3d::Update()
{

	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	Matrix4x4 viewMatrix = Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData->WVP = worldViewProjectionMatrix;
	transformationMatrixData->World = worldMatrix;
}


void Object3d::Draw()
{
	object3dCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
	//平行光源Cbufferの場所を設定
	object3dCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

	//3Dモデルが割り当てられているなら描画する
	if (model_) {
		model_->Draw();
	}
}

void Object3d::SetModel(const std::string& filepath)
{
	//もでるを検索してセットする
	model_ = ModelManager::GetInstants()->FindModel(filepath);
}