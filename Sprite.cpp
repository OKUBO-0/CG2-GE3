#include "Sprite.h"
#include "SpriteCommon.h"
#include "MyMath.h"
#include "RenderingPipeline.h"
#include "TextureManager.h"

void Sprite::Initialize(SpriteCommon* spriteCommon, std::string textureFilePath)
{
	//Textureを読んで転送する
	TextureManager::GetInstance()->LoadTexture(textureFilePath);

	spriteCommon_ = spriteCommon;
	textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);

	vertexResource = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * 4);
	indexResource = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(uint32_t) * 6);
	materialResource = spriteCommon->GetDxCommon()->CreateBufferResource(sizeof(Material));
	transformationMatrixResource = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));

	//リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 4;
	//1頂点当たりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	//Index
	//リソース先頭のアドレスから使う
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス６つ分のサイズ
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	//インデックスはuint32_tとする
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	//Material
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//色
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = false;
	materialData->uvTransform = MakeIdentity4x4();

	//Transformation
	//書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	//単位行列を書き込んでおく
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();
}


void Sprite::Update()
{
	transform.rotate = { 0.0f,0.0f,rotation };
	transform.translate = { position.x,position.y,0.0f };
	transform.scale = { size.x,size.y,1.0f, };

	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	//一個目
	vertexData[0].position = { 0.0f,1.0f,0.0f,1.0f };
	vertexData[0].texcoord = { 0.0f,1.0f };
	vertexData[0].normal = { 0.0f,0.0f,-1.0f };

	vertexData[1].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexData[1].texcoord = { 0.0f,0.0f };
	vertexData[1].normal = { 0.0f,0.0f,-1.0f };

	vertexData[2].position = { 1.0f,1.0f,0.0f,1.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };
	vertexData[2].normal = { 0.0f,0.0f,-1.0f };
	//二個目
	vertexData[3].position = { 1.0f,0.0f,0.0f,1.0f };
	vertexData[3].texcoord = { 1.0f,0.0f };
	vertexData[3].normal = { 0.0f,0.0f,-1.0f };

	//インデックスリソースにデータ書き込む
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
	indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;

	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 viewMatrix = MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData->WVP = worldViewProjectionMatrix;
	transformationMatrixData->World = worldMatrix;
}


void Sprite::Draw()
{
	//sprite用の描画
	spriteCommon_->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	spriteCommon_->GetDxCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView);
	spriteCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	//TransFormationMatrixBufferの場所を設定
	spriteCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
	spriteCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex));
	//spriteCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
	//描画！
	//commandList->DrawInstanced(6, 1, 0, 0);
	spriteCommon_->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
}