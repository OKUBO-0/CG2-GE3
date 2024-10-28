#include <string>
#include <format>

#include <windows.h>
#pragma comment(lib,"dxguid.lib")
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "MyMath.h"

#include <numbers>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "Input.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "D3DResourceLeakChecker.h"
#include "Logger.h"


#pragma region MaterialData
MaterialData LoadMaterialTemplateFile(const std::string& directorypath, const std::string& filename) {

	MaterialData materialData;//構築するMaterialData
	std::string line;//ファイルから読んだ1行を格納するもの
	std::ifstream file(directorypath + "/" + filename);//ファイルを開く
	assert(file.is_open());//とりあえず開けなっかたら止める
	while (std::getline(file, line)) {
		std::string identifile;
		std::stringstream s(line);
		s >> identifile;

		//identifierの応じた処理
		if (identifile == "map_Kd") {

			std::string textureFilename;
			s >> textureFilename;
			//連結してファイルパスにする
			materialData.textureFilePath = directorypath + "/" + textureFilename;
		}
	}
	return materialData;
}
#pragma endregion


#pragma region LoadObjectFil関数
ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData;            //構築するModelData
	std::vector<Vector4>positions;  //位置
	std::vector<Vector3>normals;    //法線
	std::vector<Vector2>texcoords;  //テクスチャ座標
	std::string line;               //ファイルから読んだ1行を格納するもの

	std::ifstream file(directoryPath + "/" + filename);  //fileを開く
	assert(file.is_open());                               //開けなかったら止める

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;  //先頭の識別子を読む

		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			position.x *= -1;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1 - texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1;
			normals.push_back(normal);
		}
		else if (identifier == "f") {
			VertexData triangle[3];
			//面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				//頂点の要素へのIndexは「位置・UV・法線」で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');//区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);
				}
				//要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				// VertexData vertex = { position, texcoord, normal };
				// modelData.vertices.push_back(vertex);
				triangle[faceVertex] = { position, texcoord, normal };
			}
			//頂点を逆順で登録刷ることで、周り順を逆にする
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		}
		else if (identifier == "mtllib") {
			//materialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			//基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}
	return modelData;
}
#pragma endregion 


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	D3DResourceLeakChecker leakCheck;

	CoInitializeEx(0, COINIT_MULTITHREADED);

	OutputDebugStringA("HEllo,DirectX!\n");

	// ポインタ
	WinApp* winApp = nullptr;
	DirectXCommon* dxCommon = nullptr;
	Input* input = nullptr;

	// WindowsAPI初期化
	winApp = new WinApp;
	winApp->Initialize();

	// DX初期化
	dxCommon = new DirectXCommon;
	dxCommon->Initialize(winApp);

	// 入力初期化
	input = new Input();
	input->Initialize(winApp);


#pragma region RootSignatureを生成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
#pragma endregion

#pragma region RootParameter
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 0;

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].Descriptor.ShaderRegister = 1;

	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);
#pragma endregion

#pragma region Sampler
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0～1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);
#pragma endregion

#pragma region InputLayoutの設定
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	hr = dxCommon->GetDevice()->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature)
	);
	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region InputLayoutの設定
	//InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);
#pragma endregion

#pragma region BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};

	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
#pragma endregion

#pragma region RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};

	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
#pragma endregion

#pragma region ShaderをCompileする
	IDxcBlob* vertexShaderBlob = dxCommon->CompileShader(
		L"Resources/shaders/Object3d.VS.hlsl",
		L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	IDxcBlob* pixelShaderBlob = dxCommon->CompileShader(
		L"Resources/shaders/Object3d.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlob != nullptr);
#pragma endregion

#pragma region DepthStencilState
	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	// Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	// 書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// 比較関数はLessEqual
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
#pragma endregion

#pragma region PSOを生成する
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ（形状）のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定（気にしなくてよい）
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 実際に生成
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));
#pragma endregion


#pragma region Resource
	const uint32_t kSubdivision = 512;
	ModelData modelData = LoadObjFile("resources", "axis.obj");

#pragma region VertexResourceを生成
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * kSubdivision * kSubdivision * 6);

#pragma region VertexBufferResourceを生成
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite = dxCommon->CreateBufferResource(sizeof(VertexData) * 4);

#pragma region IndexResourceを生成
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = dxCommon->CreateBufferResource(sizeof(uint32_t) * 6);

#pragma region ModelResourceを生成
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceModel = dxCommon->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
#pragma endregion


#pragma region vertexResourceModel頂点バッファーを作成する
	D3D12_VERTEX_BUFFER_VIEW VertexBufferViewModel{};
	VertexBufferViewModel.BufferLocation = vertexResourceModel->GetGPUVirtualAddress();
	VertexBufferViewModel.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	VertexBufferViewModel.StrideInBytes = sizeof(VertexData);
	VertexData* vertexDataModel = nullptr;
	vertexResourceModel->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataModel));
	std::memcpy(vertexDataModel, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());
#pragma endregion


#pragma region vertexResource頂点バッファーを作成
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{ };
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * kSubdivision * kSubdivision * 6;
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());
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


#pragma region vertexResourceSprite頂点バッファーを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{ };
	//リソースの先頭のアドレスから使う
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 4;
	//1頂点当たりのサイズ
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);
	//頂点リソースにデータを書き込む
	VertexData* vertexDataSprite = nullptr;
	//書き込むためのアドレスを取得
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	//一個目
	vertexDataSprite[0].position = { 0.0f,360.0f,0.0f,1.0f };//左した
	vertexDataSprite[0].texcoord = { 0.0f,1.0f };
	vertexDataSprite[0].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[1].position = { 0.0f,0.0f,0.0f,1.0f };//左上
	vertexDataSprite[1].texcoord = { 0.0f,0.0f };
	vertexDataSprite[1].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[2].position = { 640.0f,360.0f,0.0f,1.0f };//右下
	vertexDataSprite[2].texcoord = { 1.0f,1.0f };
	vertexDataSprite[2].normal = { 0.0f,0.0f,-1.0f };
	//二個目
	vertexDataSprite[3].position = { 640.0f,0.0f,0.0f,1.0f };//右上
	vertexDataSprite[3].texcoord = { 1.0f,0.0f };
	vertexDataSprite[3].normal = { 0.0f,0.0f,-1.0f };
#pragma endregion


#pragma region IndexBufferViewを作成
	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	// リソース先頭のアドレスから使う
	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス６つ分のサイズ
	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	// インデックスはUint32_tとする
	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;
	// インデックスリソースにデータ書き込む
	uint32_t* indexDataSprite = nullptr;
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	indexDataSprite[0] = 0; indexDataSprite[1] = 1; indexDataSprite[2] = 2;
	indexDataSprite[3] = 1; indexDataSprite[4] = 3; indexDataSprite[5] = 2;
#pragma endregion


#pragma region Material用のResourceを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = dxCommon->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む	
	Material* materialDataSphere = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSphere));
	//色
	materialDataSphere->color = { Vector4(1.0f, 1.0f, 1.0f, 1.0f) };
	materialDataSphere->enableLighting = true;
	materialDataSphere->uvTransform = MakeIdentity4x4();
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


#pragma region Model用のResourceを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceModel = dxCommon->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む	
	Material* materialDataModel = nullptr;
	materialResourceModel->Map(0, nullptr, reinterpret_cast<void**>(&materialDataModel));
	//色
	materialDataModel->color = { Vector4(1.0f, 1.0f, 1.0f, 1.0f) };
	materialDataModel->enableLighting = true;
	materialDataModel->uvTransform = MakeIdentity4x4();
#pragma endregion


#pragma region ModelTransform用のResourceを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceModel = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* transformationMatrixDataModel = nullptr;
	//書き込むためのアドレスを取得
	transformationMatrixResourceModel->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataModel));
	//単位行列を書き込む
	transformationMatrixDataModel->WVP = MakeIdentity4x4();
	transformationMatrixDataModel->World = MakeIdentity4x4();
#pragma endregion


#pragma region Sprite用のTransformationMatrix用のResourceを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* transformationMatrixDataSprite = nullptr;
	//書き込むためのアドレスを取得
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	//単位行列を書き込んでおく
	transformationMatrixDataSprite->WVP = MakeIdentity4x4();
	transformationMatrixDataSprite->World = MakeIdentity4x4();
#pragma endregion


#pragma region Sprite用のmaterial用のResourceを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite = dxCommon->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む	
	Material* materialDataSprite = nullptr;
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	//色
	materialDataSprite->color = { Vector4(1.0f, 1.0f, 1.0f, 1.0f) };
	materialDataSprite->enableLighting = false;
	materialDataSprite->uvTransform = MakeIdentity4x4();
#pragma endregion


#pragma region 平行光源用のResourceを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = dxCommon->CreateBufferResource(sizeof(DirectionalLight));
	DirectionalLight* directionalLightData = nullptr;
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,-1.0f,1.0f };
	directionalLightData->intensity = 1.0f;
#pragma endregion



#pragma region Texturを読む
	// Textureを読んで転送する
	DirectX::ScratchImage mipImages = dxCommon->LoadTexture("Resources/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = dxCommon->CreateTextureResource(metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = dxCommon->UploadTextureData(textureResource, mipImages);

	//Texture2を読んで転送する
	DirectX::ScratchImage mipImages2 = dxCommon->LoadTexture("Resources/monsterBall.png");
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = dxCommon->CreateTextureResource(metadata2);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource2 = dxCommon->UploadTextureData(textureResource2, mipImages2);

	//Texture3を読んで転送する
	DirectX::ScratchImage mipImages3 = dxCommon->LoadTexture(modelData.material.textureFilePath);
	const DirectX::TexMetadata& metadata3 = mipImages3.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource3 = dxCommon->CreateTextureResource(metadata3);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource3 = dxCommon->UploadTextureData(textureResource3, mipImages3);
#pragma endregion 

#pragma region ShaderResourceView
	// metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{  };
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	// metaDara2を気にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{  };
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

	// metaDara3を気にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc3{  };
	srvDesc3.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc3.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc3.Texture2D.MipLevels = UINT(metadata3.mipLevels);

	// SRVを作成するDescriptHeap	の場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = dxCommon->GetSRVCPUDescriptorHandle(1);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = dxCommon->GetSRVGPUDescriptorHandle(1);

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = dxCommon->GetSRVCPUDescriptorHandle(2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = dxCommon->GetSRVGPUDescriptorHandle(2);

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU3 = dxCommon->GetSRVCPUDescriptorHandle(3);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU3 = dxCommon->GetSRVGPUDescriptorHandle(3);

	// 先頭はImGuiが使っているのでその次を使う
	textureSrvHandleCPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	// SRVの設定
	dxCommon->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);
	dxCommon->GetDevice()->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);
	dxCommon->GetDevice()->CreateShaderResourceView(textureResource3.Get(), &srvDesc3, textureSrvHandleCPU3);
#pragma endregion 


#pragma region Transform変数
	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,-1.5f,0.0f},{0.0f,0.0f,0.0f } };

#pragma region cameraTransform変数
	Transform cameraTransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-5.0f} };

#pragma region spriteTransform変数
	Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };

#pragma region uvspriteTransform変数
	Transform uvTransformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };

#pragma region model変数
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

		//ゲームの処理
		if (input->PushKey(DIK_A)) {
			transformModel.translate.x -= 0.01f;
		}
		if (input->PushKey(DIK_D)) {
			transformModel.translate.x += 0.01f;
		}
		if (input->PushKey(DIK_W)) {
			transformModel.translate.y += 0.01f;
		}
		if (input->PushKey(DIK_S)) {
			transformModel.translate.y -= 0.01f;
		}

#pragma region Transform用Matrix
		Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
		Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f);
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
		wvpData->WVP = worldViewProjectionMatrix;
		wvpData->World = worldMatrix;
#pragma endregion


#pragma region model用Matrix
		Matrix4x4 worldMatrixmodel = MakeAffineMatrix(transformModel.scale, transformModel.rotate, transformModel.translate);
		Matrix4x4 worldViewProjectionMatrixModel = Multiply(worldMatrixmodel, Multiply(viewMatrix, projectionMatrix));
		transformationMatrixDataModel->WVP = worldViewProjectionMatrixModel;
		transformationMatrixDataModel->World = worldMatrixmodel;
#pragma endregion


#pragma region Sprite用Matrix
		Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
		Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
		Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);
		Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
		transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;
		transformationMatrixDataSprite->World = worldMatrix;
#pragma endregion


#pragma region material用Matrix
		Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
		materialDataSprite->uvTransform = uvTransformMatrix;
#pragma endregion


		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Settings");

		// Color Edit ウィンドウ
		if (ImGui::CollapsingHeader("SetColor")) {
			ImGui::ColorEdit4("materialData", &materialDataSphere->color.x);
		}
		ImGui::Separator();

		// Texture変更
		if (ImGui::CollapsingHeader("Texture change")) {
			ImGui::Checkbox("useMonsterBall", &useMonsterBall);
		}
		ImGui::Separator();

		// Lighting
		if (ImGui::CollapsingHeader("Lighting")) {
			ImGui::ColorEdit4("LightSetColor", &directionalLightData->color.x);
			ImGui::DragFloat3("directionalLight", &directionalLightData->direction.x, 0.01f, -1.0f, 1.0f);
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
		if (ImGui::CollapsingHeader("Model"))
		{
			ImGui::DragFloat3("ModelTranslate", &transformModel.translate.x, 0.01f);
			ImGui::DragFloat3("ModelRotate", &transformModel.rotate.x, 0.01f);
			ImGui::DragFloat3("ModelScale", &transformModel.scale.x, 0.01f);
			if (ImGui::Button("Reset Transform")) {
				transformModel = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
			}
		}
		ImGui::Separator();

		// スプライトウィンドウ
		if (ImGui::CollapsingHeader("2DSprite")) {
			ImGui::DragFloat3("TranslationSprite", &transformSprite.translate.x);
			ImGui::DragFloat3("RotationSprite", &transformSprite.rotate.y, 0.1f);
			ImGui::DragFloat2("ScaleSprite", &transformSprite.scale.x, 0.1f);
			if (ImGui::Button("Reset Transform")) {
				transformSprite = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
			}
		}
		ImGui::Separator();

		// UVTransform
		if (ImGui::CollapsingHeader("UVTransform")) {
			ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);
		}
		ImGui::Separator();


		ImGui::End();
		ImGui::Render();

		dxCommon->Begin();

		//RootSignatureを設定。POSに設定しているけどベット設定が必要
		dxCommon->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
		dxCommon->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());

#pragma region スフィアの描画
		dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
		//現状を設定。POSに設定しているものとはまた別。おなじ物を設定すると考えておけばいい
		dxCommon->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
		//wvp用のCBufferの場所を設定
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
		dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
		//描画！
		dxCommon->GetCommandList()->DrawInstanced(kSubdivision * kSubdivision * 6, 1, 0, 0);
#pragma endregion


#pragma region Spriteの描画
		dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
		dxCommon->GetCommandList()->IASetIndexBuffer(&indexBufferViewSprite);
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
		//TransFormationMatrixBufferの場所を設定
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
		dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
		//描画！
		dxCommon->GetCommandList()->DrawInstanced(6, 1, 0, 0);
		dxCommon->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
#pragma endregion


#pragma region Modelの描画
		dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &VertexBufferViewModel);
		//現状を設定。POSに設定しているものとはまた別。おなじ物を設定すると考えておけばいい
		dxCommon->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceModel->GetGPUVirtualAddress());
		//wvp用のCBufferの場所を設定
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceModel->GetGPUVirtualAddress());
		dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU3);
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
		//描画！
		dxCommon->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);
#pragma endregion


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
	delete winApp;
	delete dxCommon;
	delete input;

	return 0;
}