#include <string>
#include <format>
#include <numbers>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <windows.h>
#pragma comment(lib,"dxguid.lib")

#include "Vector4.h"
#include "Vector3.h"
#include "Vector2.h"
#include "Matrix4x4.h"
#include "MyMath.h"
#include "RenderingPipeline.h"
#include "Input.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "D3DResourceLeakChecker.h"
#include "Logger.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "TextureManager.h"
#include "Object3DCommon.h"
#include "Object3D.h"
#include "RenderingData.h"
#include "Model.h"
#include "ModelCommon.h"


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	D3DResourceLeakChecker leakCheck;

	CoInitializeEx(0, COINIT_MULTITHREADED);

	OutputDebugStringA("HEllo,DirectX!\n");


#pragma region 基盤システム初期化
	// WindowsAPI初期化
	WinApp* winApp = nullptr;
	winApp = new WinApp;
	winApp->Initialize();

	// DX初期化
	DirectXCommon* dxCommon = nullptr;
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);

	//テクスチャマネージャの初期化
	TextureManager::GetInstance()->Initialize(dxCommon);

	// 入力初期化
	Input* input = nullptr;
	input = new Input();
	input->Initialize(winApp);

	// スプライトの初期化
	SpriteCommon* spriteCommon = nullptr;
	spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(dxCommon);

	//3Dオブジェクトの初期化
	Object3dCommon* object3dCommon = nullptr;
	object3dCommon = new Object3dCommon;
	object3dCommon->Initialize(dxCommon);

	//モデルの共通部初期化
	ModelCommon* modelCommon = nullptr;
	modelCommon = new ModelCommon;
	modelCommon->Initialize(dxCommon);
#pragma endregion 


#pragma region Resource
	const uint32_t kSubdivision = 512;

#pragma region VertexResourceを生成
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * kSubdivision * kSubdivision * 6);
#pragma endregion


#pragma region vertexResource頂点バッファーを作成
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{ };
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * kSubdivision * kSubdivision * 6;
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
#pragma endregion


#pragma region 基準点
	//経度分割1つ分の経度φd
	const float kLonEvery = 2 * std::numbers::pi_v<float> / (float)kSubdivision;
	//緯度分割１つ分の緯度Θd
	const float kLatEvery = std::numbers::pi_v<float> / (float)kSubdivision;
	//緯度方向に分割しながら線を描く
	const float w = 2.0f;
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;//θ
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			//テクスチャ用のTexcoord

			//書き込む最初の場所
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
			float lon = lonIndex * kLonEvery;
			//基準点a
			vertexData[start].position.x = std::cosf(lat) * std::cosf(lon);
			vertexData[start].position.y = std::sinf(lat);
			vertexData[start].position.z = std::cosf(lat) * std::sinf(lon);
			vertexData[start].position.w = w;
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			vertexData[start].texcoord = { float(lonIndex) / float(kSubdivision), 1.0f - float(latIndex) / float(kSubdivision) };
			//基準点b
			start++;
			vertexData[start].position.x = std::cosf(lat + kLatEvery) * std::cosf(lon);
			vertexData[start].position.y = std::sinf(lat + kLatEvery);
			vertexData[start].position.z = std::cosf(lat + kLatEvery) * std::sinf(lon);
			vertexData[start].position.w = w;
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			vertexData[start].texcoord = { float(lonIndex) / float(kSubdivision), 1.0f - float(latIndex + 1.0f) / float(kSubdivision) };
			//基準点c
			start++;
			vertexData[start].position.x = std::cosf(lat) * std::cosf(lon + kLonEvery);
			vertexData[start].position.y = std::sinf(lat);
			vertexData[start].position.z = std::cosf(lat) * std::sinf(lon + kLonEvery);
			vertexData[start].position.w = w;
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			vertexData[start].texcoord = { float(lonIndex + 1.0f) / float(kSubdivision), 1.0f - float(latIndex) / float(kSubdivision) };

			//基準点c
			start++;
			vertexData[start].position.x = std::cosf(lat) * std::cosf(lon + kLonEvery);
			vertexData[start].position.y = std::sinf(lat);
			vertexData[start].position.z = std::cosf(lat) * std::sinf(lon + kLonEvery);
			vertexData[start].position.w = w;
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			vertexData[start].texcoord = { float(lonIndex + 1.0f) / float(kSubdivision), 1.0f - float(latIndex) / float(kSubdivision) };

			//基準点b
			start++;
			vertexData[start].position.x = std::cosf(lat + kLatEvery) * std::cosf(lon);
			vertexData[start].position.y = std::sinf(lat + kLatEvery);
			vertexData[start].position.z = std::cosf(lat + kLatEvery) * std::sinf(lon);
			vertexData[start].position.w = w;
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			vertexData[start].texcoord = { float(lonIndex) / float(kSubdivision), 1.0f - float(latIndex + 1.0f) / float(kSubdivision) };

			//基準点d
			start++;
			vertexData[start].position.x = std::cosf(lat + kLatEvery) * std::cosf(lon + kLonEvery);
			vertexData[start].position.y = std::sinf(lat + kLatEvery);
			vertexData[start].position.z = std::cosf(lat + kLatEvery) * std::sinf(lon + kLonEvery);
			vertexData[start].position.w = w;
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			vertexData[start].texcoord = { float(lonIndex + 1) / float(kSubdivision), 1.0f - float(latIndex + 1) / float(kSubdivision) };
		}
	}
#pragma endregion


#pragma region WVP用のResourceを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* wvpData = nullptr;
	//書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	//単位行列を書き込む
	wvpData->WVP = MakeIdentity4x4();
	wvpData->World = MakeIdentity4x4();
#pragma endregion


#pragma region Texturを読む
	std::string textureFilePath[2]{ "Resources/monsterBall.png" ,"Resources/uvChecker.png" };
#pragma endregion 


	std::vector<Sprite*>sprites;
	for (uint32_t i = 0; i < 1; ++i) {
		Sprite* sprite = new Sprite();
		sprite->Initialize(spriteCommon, textureFilePath[1]);
		sprites.push_back(sprite);
	}

#pragma region 最初のシーン初期化
	//モデル初期化
	Model* model_ = new Model();
	model_->Initialize(modelCommon);

	//3Dオブジェクトの初期化
	Object3d* object3d = new Object3d();
	object3d->Initialize(object3dCommon);
	object3d->Setmodel(model_);

	//3Dオブジェクトの初期化
	Object3d* object3d2nd = new Object3d();
	object3d2nd->Initialize(object3dCommon);
	object3d2nd->Setmodel(model_);
#pragma endregion

	int i = 0;
	for (Sprite* sprite : sprites) {
		Vector2 position = sprite->GetPosition();
		Vector2 size = sprite->GetSize();

		position.x = 200.0f * i;
		position.y = 200.0f;
		size = Vector2(100, 100);

		sprite->SetPosition(position);
		sprite->SetSize(size);
		sprite->SetAnchorPoint(Vector2{ 0.0f,0.0f });
		sprite->SetIsFlipY(0);
		sprite->SetTextureLeftTop(Vector2{ i * 64.0f,0.0f });
		sprite->SetTextureSize(Vector2{ 64.0f,64.0f });
		i++;
	}

	float rotation{ 0 };


#pragma region Transform変数
	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,-1.5f,0.0f},{0.0f,0.0f,0.0f } };

	Transform transformModel = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
#pragma endregion

	bool useMonsterBall = false;

	while (true) {
		//Windowsのメッセージ処理
		if (winApp->ProcessMessage()) {
			//ゲームループを抜ける
			break;
		}

		input->Update();

		////ゲームの処理
		//if (input->PushKey(DIK_A)) {
		//	transformModel.translate.x -= 0.01f;
		//}
		//if (input->PushKey(DIK_D)) {
		//	transformModel.translate.x += 0.01f;
		//}
		//if (input->PushKey(DIK_W)) {
		//	transformModel.translate.y += 0.01f;
		//}
		//if (input->PushKey(DIK_S)) {
		//	transformModel.translate.y -= 0.01f;
		//}

/*#pragma region Transform用Matrix
		Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
		Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f);

		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
		wvpData->WVP = worldViewProjectionMatrix;
		wvpData->World = worldMatrix;*/
#pragma endregion


		for (Sprite* sprite : sprites) {
			sprite->Update();
		}

		rotation += 0.03f;

		object3d->SetRotate(Vector3{ 0,rotation ,0 });
		object3d->Update();

		object3d2nd->SetRotate(Vector3{ 0,0 ,rotation });
		object3d2nd->Update();


		/*#pragma region material用Matrix
				Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
				uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
				uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
				materialDataSprite->uvTransform = uvTransformMatrix;
		#pragma endregion*/


		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Settings");

		// Color Edit ウィンドウ
		if (ImGui::CollapsingHeader("SetColor")) {
			//	ImGui::ColorEdit4("materialData", &materialDataSphere->color.x);
		}
		ImGui::Separator();

		// Texture変更
		if (ImGui::CollapsingHeader("Texture change")) {
			ImGui::Checkbox("useMonsterBall", &useMonsterBall);
		}
		ImGui::Separator();

		// Lighting
		if (ImGui::CollapsingHeader("Lighting")) {
			//	ImGui::ColorEdit4("LightSetColor", &directionalLightData->color.x);
			//	ImGui::DragFloat3("directionalLight", &directionalLightData->direction.x, 0.01f, -1.0f, 1.0f);
		}
		ImGui::Separator();

		// スフィアウィンドウ
		if (ImGui::CollapsingHeader("3DObject")) {
			ImGui::DragFloat3("Translation", &transform.translate.x, 0.01f);
			ImGui::DragFloat3("Rotation", &transform.rotate.x, 0.01f);
			ImGui::DragFloat2("Scale", &transform.scale.x, 0.01f);
			if (ImGui::Button("Reset Transform")) {
				transform = { {1.0f, 1.0f, 1.0f}, {0.0f, -1.5f, 0.0f}, {0.0f, 0.0f, 0.0f} };
			}
		}
		ImGui::Separator();

		// モデルウィンドウ
		if (ImGui::CollapsingHeader("Model")) {
			transformModel = object3d->GetTransform();

			ImGui::DragFloat3("ModelTranslate", &transformModel.translate.x, 0.01f);
			ImGui::DragFloat3("ModelRotate", &transformModel.rotate.x, 0.01f);
			ImGui::DragFloat3("ModelScale", &transformModel.scale.x, 0.01f);

			object3d->SetTransform(transformModel);
			//	ImGui::DragFloat3("ModelTranslate", &transformModel.translate.x, 0.01f);
			//	ImGui::DragFloat3("ModelRotate", &transformModel.rotate.x, 0.01f);
			//	ImGui::DragFloat3("ModelScale", &transformModel.scale.x, 0.01f);
			//	if (ImGui::Button("Reset Transform")) {
			//		transformModel = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
			//	}
		}
		ImGui::Separator();

		// スプライトウィンドウ
		if (ImGui::CollapsingHeader("2DSprite")) {
			//	Vector2 size = sprite->GetSize();
			//	Vector2 position = sprite->GetPosition();
			//	float rotation = sprite->GetRotation();
			//	Vector4 spritecolor = sprite->GetColor();
			// 
			//	ImGui::ColorEdit4("*spriteColor", &spritecolor.x);
			//	ImGui::DragFloat2("*ScaleSprite", &size.x, 0.1f);
			//	ImGui::DragFloat("*RotateSprite", &rotation, 0.1f);
			//	ImGui::DragFloat2("*TranslateSprite", &position.x);
			// 
			//	sprite->setColor(spritecolor);
			//	sprite->SetPosition(position);
			//	sprite->SetRotation(rotation);
			//	sprite->SetSize(size);
			// 
			//	if (ImGui::Button("Reset Transform")) {
			//		transformSprite = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
			//	}
		}
		ImGui::Separator();

		// UVTransform
		if (ImGui::CollapsingHeader("UVTransform")) {
			//	ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
			//	ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
			//	ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);
		}
		ImGui::Separator();


		ImGui::End();
		ImGui::Render();

		//DirectXの描画準備
		dxCommon->Begin();

#pragma region 3Dオブジェクト描画
		//3dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
		object3dCommon->CommonDraw();
		object3d->Draw();
		object3d2nd->Draw();
#pragma endregion


#pragma region スプライト描画
		//Spriteの描画準備
		spriteCommon->CommonDraw();

		for (Sprite* sprite : sprites) {
			sprite->Draw();
		}
#pragma endregion


		//#pragma region スフィアの描画
		//		dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
		//		//現状を設定。POSに設定しているものとはまた別。おなじ物を設定すると考えておけばいい
		//		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
		//		//wvp用のCBufferの場所を設定
		//		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
		//		dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
		//		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
		//		//描画！
		//		//dxCommon->GetCommandList()->DrawInstanced(kSubdivision * kSubdivision * 6, 1, 0, 0);
		//#pragma endregion


		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

		dxCommon->End();
	}

#pragma region 解放処理

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	/*CloseHandle(fenceEvent);*/

#ifdef _DEBUG
#endif //_DEBUG
#pragma endregion

	// 終了処理
	winApp->Finalize();
	// 解放処理
	TextureManager::GetInstance()->Finalize();
	delete winApp;
	delete dxCommon;
	delete input;
	delete spriteCommon;
	for (Sprite* sprite : sprites) {
		delete sprite;
	}
	delete object3dCommon;
	delete object3d;
	delete object3d2nd;
	delete modelCommon;
	delete model_;

	return 0;
}