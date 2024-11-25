#pragma once

#include "Matrix4x4.h"
#include "Vector4.h"
#include "Vector3.h"
#include "Vector2.h"
#include "Mymath.h"

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
};

