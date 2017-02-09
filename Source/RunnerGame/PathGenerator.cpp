#include <math.h>
#include <Engine/Engine.h>
#include <Engine/Input.h>
#include <Engine/SceneManager.h>
#include <Engine/Console.h>

#include "RunnerGame.h"
#include "PathGenerator.h"

REGISTER_OBJECT_CLASS(PathGenerator);

PathGenerator::PathGenerator(ObjectInitializer *initializer) noexcept : Object(initializer)
{
}

int PathGenerator::Load()
{
	int ret = Object::Load();

	if (ret != ENGINE_OK)
		return ret;

	return ENGINE_OK;
}

void PathGenerator::Update(double deltaTime) noexcept
{
	Object::Update(deltaTime);

	if (Input::GetButtonDown("exit"))
		Console::ExecuteCommand("call exit");

	if (Input::GetButtonDown("show_stats"))
		Console::ExecuteCommand("call showstats");

	if (Input::GetButtonDown("next_scene"))
		SceneManager::LoadNextScene();

	if (Input::GetButtonDown("draw_lights"))
		SceneManager::GetActiveScene()->SetDrawLights(!SceneManager::GetActiveScene()->GetDrawLights());

	if (Input::GetButtonDown("screenshot"))
		Console::ExecuteCommand("call screenshot");
}

