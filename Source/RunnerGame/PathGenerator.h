#pragma once

#include "RunnerGame.h"
#include <Scene/Object.h>

class PathGenerator : public Object
{
public:
	

private:
	RUNNERGAME_API PathGenerator(ObjectInitializer *initializer) noexcept;

	virtual int RUNNERGAME_API Load() override;
	virtual void RUNNERGAME_API Update(double deltaTime) noexcept override;

	virtual RUNNERGAME_API ~PathGenerator() noexcept {};
}