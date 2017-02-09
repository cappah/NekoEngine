#include "PatchManager.h"
#include <Scene/Object.h>

std::queue<std::vector<Object*>> PatchManager::_layers;

void PatchManager::CreateNewLayer()
{
	_layers.push(std::vector<Object*>());
}

void PatchManager::AddPatchInCurrentLayer(Object* patch)
{
	_layers.back().push_back(patch);

	//TODO: Instantiate
}

void PatchManager::DestroyOldestLayer()
{
	for (auto item : _layers.front()) {
		//TODO: Destroy patch in oldest layer
	}

	_layers.pop();
}