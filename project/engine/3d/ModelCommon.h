#pragma once

#include "DirectXCommon.h"
#include "SrvManager.h"

class ModelCommon
{

public:
	// 初期化
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvMnager);

	//DXCommon
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
	SrvManager* GetSRVManager() { return srvManager_; }

private:
	DirectXCommon* dxCommon_;
	SrvManager* srvManager_ = nullptr;
};