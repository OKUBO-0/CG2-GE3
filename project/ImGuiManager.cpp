#include "ImGuiManager.h"
#include <imgui.h>
#include <imgui_impl_win32.h>

void ImGuiManager::Initialize(DirectXCommon* dxCommon, WinApp* winapp)
{
	//imguiのコンテキストを生成
	ImGui::CreateContext();
	//imguiのスタイルを設定
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(winapp->GetHwnd());
	//デスクリプタヒープ設定
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	//デスクリプターフープ生成
	HRESULT hr = dxCommon->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&srvHeap_));
	assert(SUCCEEDED(hr));

	ImGui_ImplDX12_Init()
}