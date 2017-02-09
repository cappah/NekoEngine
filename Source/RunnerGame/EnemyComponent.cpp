
#include <Scene/Object.h>
#include "EnemyComponent.h"
#include "Player.h"

REGISTER_COMPONENT_CLASS(EnemyComponent);

EnemyComponent::EnemyComponent(ComponentInitializer *initializer) :
	ObjectComponent(initializer)
{

}

int EnemyComponent::Load()
{
	int ret{ ObjectComponent::Load() };
	if (ret != ENGINE_OK) return ret;

	return ENGINE_OK;
}

void EnemyComponent::Update(double deltaTime) noexcept
{
	ObjectComponent::Update(deltaTime);
}

bool EnemyComponent::Unload()
{
	if (!ObjectComponent::Unload())
		return false;

	return true;
}

void EnemyComponent::OnHit(Object* other, glm::vec3 &position)
{
	Player* player = dynamic_cast<Player*> (other);

	if (player == nullptr) {
		return;
	}

	player->Kill();
}

EnemyComponent::~EnemyComponent()
{
	//
}