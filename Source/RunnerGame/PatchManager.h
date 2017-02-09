#pragma once

#include <Scene\Object.h>
#include <vector>
#include <queue>

class PatchManager
{
private:
	static std::queue<std::vector<Object*>> _layers;
	static const std::size_t max_layer_count = 5;

public:
	static void CreateNewLayer();
	static void AddPatchInCurrentLayer(Object* object);
	static void DestroyOldestLayer();
};
