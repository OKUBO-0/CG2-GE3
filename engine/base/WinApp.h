#pragma once

#include <Windows.h>
#include <cstdint>

class WinApp
{

public:
	//クライアント領域のサイズ
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

public:

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	//初期化
	void Initialize();
	//更新
	void Update();
	//終了
	void Finalize();
	//Getter
	HWND GetHwnd()const { return hwnd; }
	HINSTANCE GetHInstance()const { return wc.hInstance; }

	bool ProcessMessage();

private:

	//ウィンドウ生成
	HWND hwnd = nullptr;
	WNDCLASS wc{};
};