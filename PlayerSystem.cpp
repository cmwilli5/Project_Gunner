#include "PlayerSystem.h"
#include "displace.h"
#include "Spawner.h"
#include "collider.h"
#include "Transform.h"
#include "renderable.h"
#include "delayComponent.h"
#include "lifeTimer.h"
#include <ctime>

PlayerSystem::PlayerSystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority},
    playerHitFunc(std::function<bool(Message&)>([this](Message& message){
            return playerHit(message); } ) ) {
    playerPool = manager->getComponentPool<Component<PlayerCmp::name, PlayerCmp>>();
    manager->registerWithMessage(Message::Type::PlayerHit, playerHitFunc, 1000);
    randEngine.seed(time(nullptr));
}

void PlayerSystem::initialize() {

}

void PlayerSystem::addEntity(uint32_t id) {
    const auto& entity = manager->getEntity(id);
    if (entity) {
        auto playerCmp = entity->find("player");
        auto delay = entity->find("fullDelay");
        auto pause = entity->find("pauseDelay");
        if ( (delay == entity->end() || !delay->second.active) && (pause == entity->end() || !pause->second.active)
            && playerCmp != entity->end() && playerCmp->second.active) {

            entities.emplace(id, &playerCmp->second);

        }
    }

}

void PlayerSystem::removeEntity(uint32_t id) {
    entities.erase(id);
}

void PlayerSystem::refreshEntity(uint32_t id) {
    auto entity = entities.find(id);
    if (entity != entities.end() && !entity->second->active) {
        removeEntity(id);
    } else if (entity != entities.end()) {
        const auto& fullEntity = manager->getEntity(id);
        auto delay = fullEntity->find("fullDelay");
        auto pause = fullEntity->find("pauseDelay");
        if ( (delay != fullEntity->end() && delay->second.active) || (pause != fullEntity->end() && pause->second.active) ) {
            removeEntity(id);
        }
    } else {
        addEntity(id);
    }
}

void PlayerSystem::process(float dt) {

    float adjustedDT = dt * 1000.0f;

    for (auto& entity : entities) {

        PlayerCmp& playerData = playerPool->operator[](entity.second->index).data;
        if (!playerData.alive/* && playerData.lives > 0*/) {
            playerData.deathTimer -= adjustedDT;
            if (playerData.deathTimer <= 0) {
                playerData.deathTimer = 3000.0f;
                playerData.alive = true;
                --playerData.lives;
                Position playerPos;
                playerPos.pastPosX = playerPos.posX = 180.0f;
                playerPos.pastPosY = playerPos.posY = 360.0f;
                manager->addComponent<Component<Position::name, Position>>(playerPos, entity.first);
                manager->addComponent(std::string("component:velocity 0, 0"), entity.first);
                manager->addComponent(std::string("component:collider Player Point 0 0"), entity.first);
                manager->addComponent(std::string("component:shield 2500"), entity.first);
            }
        }

    }

}

bool PlayerSystem::playerHit(Message& message) {//
    PlayerHit& data = (PlayerHit&)message;
    auto entity = entities.find(data.col2ID);
    std::cout << "Player Hit event: \n";
    if (entity != entities.end() && entity->second->active) {
        PlayerCmp& playerCmp = playerPool->operator[](entity->second->index).data;
        if (playerCmp.alive) {
            std::cout << "Player hit\tHitter: " << data.col1ID << "\tPlayer: " << data.col2ID << '\n';
            manager->destroyEntity(data.col1ID);

            std::string explosionAnim{"component:animation 8 1 160 \
            2 180 3 180 4 200\
            5 280 6 400 7 550 8 1"};
            auto positionPool = manager->getComponentPool<Component<Position::name, Position>>();
            const auto& fullEntity = manager->getEntity(data.col2ID);
            auto posCmp = fullEntity->find("position");
            if (posCmp != fullEntity->end()) {
                Sprite explSprite;
                explSprite.spriteName = "possibleExplosions";
                explSprite.zOrder = 35;
                explSprite.spritePos = 0;

                WorldTransform orient;

                std::uniform_real_distribution<float> angleDist{0.0f, 359.999f};
                std::uniform_real_distribution<float> scaleDist{0.6f, 0.8f};
                std::uniform_int_distribution<int32_t> coordDist{-6, 6};
                std::uniform_int_distribution<int16_t> zOrderDist{30, 40};

                for (uint8_t i = 0; i < 4; ++i) {
                    std::uniform_int_distribution<int32_t> coordDist{-3, 3};
                    std::uniform_real_distribution<float> delayDist{0.0f, 60.0f};
                    auto explId = manager->createEntity();
                    WorldTransform orient;
                    orient.present.rotate(angleDist(randEngine));
                    orient.present.setScale(scaleDist(randEngine), scaleDist(randEngine));
                    orient.past = orient.present;
                    explSprite.zOrder = zOrderDist(randEngine);
                    manager->addComponent(explosionAnim, explId);
                    manager->addComponent<Component<Sprite::name, Sprite>>(explSprite, explId);
                    manager->addComponent<Component<Position::name, Position>>(positionPool->operator[](posCmp->second.index), explId);
                    manager->addComponent<Component<cmpName::worldTF, WorldTransform>>(orient, explId);
                    manager->addComponent<Component<delayComponent::fullDelay, float>>(delayDist(randEngine), explId);
                    manager->addComponent<Component<lifeTimerName, float>>(2500, explId);
                }

                for (uint8_t i = 0; i < 8; ++i) {
                    auto explId = manager->createEntity();
                    std::uniform_real_distribution<float> scaleDist{0.35f, 0.70f};
                    std::uniform_real_distribution<float> delayDist{50.0f, 450.0f};

                    explSprite.zOrder = zOrderDist(randEngine);
                    manager->addComponent(explosionAnim, explId);
                    manager->addComponent<Component<Sprite::name, Sprite>>(explSprite, explId);
                    Position cpyPos = positionPool->operator[](posCmp->second.index).data;
                    cpyPos.posX += coordDist(randEngine);
                    cpyPos.posY += coordDist(randEngine);
                    cpyPos.pastPosX = cpyPos.posX;
                    cpyPos.pastPosY = cpyPos.posY;
                    manager->addComponent<Component<Position::name, Position>>(cpyPos, explId);
                    WorldTransform orient;
                    orient.present.setAngle(angleDist(randEngine));
                    orient.present.setScale(scaleDist(randEngine), scaleDist(randEngine));
                    orient.past = orient.present;
                    manager->addComponent<Component<cmpName::worldTF, WorldTransform>>(orient, explId);
                    manager->addComponent<Component<delayComponent::fullDelay, float>>(delayDist(randEngine), explId);
                    manager->addComponent<Component<lifeTimerName, float>>(2000, explId);

                }

            }


            manager->removeComponent<Component<Position::name, Position>>(data.col2ID);
            manager->removeComponent<Component<Collider::name, Collider>>(data.col2ID);
            manager->removeComponent<Component<Velocity::name, Velocity>>(data.col2ID);
            playerCmp.alive = false;
            return true;
        }
    }
    return false;
}