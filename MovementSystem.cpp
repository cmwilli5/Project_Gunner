#include "MovementSystem.h"
#include "SDL.h"

MovementSystem::MovementSystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority} {
    positionPool = manager->getComponentPool<Component<Position::name, Position>>();
    velocityPool = manager->getComponentPool<Component<Velocity::name, Velocity>>();
};

void MovementSystem::initialize() {}

void MovementSystem::addEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID == entityIDs.end()) {
        auto entity = manager->getEntity(id);
        if (entity) {
            auto position = entity->find("position");
            auto velocity = entity->find("velocity");
            auto delay = entity->find("fullDelay");
            auto pause = entity->find("pauseDelay");
            if ( (delay == entity->end() || !delay->second.first)
                && (pause == entity->end() || !pause->second.first)
                && position != entity->end() && velocity != entity->end()
                && position->second.first && velocity->second.first) {

                if (freeIDXs.empty()) {
                    entityIDs[id] = entities.size();
                    entities.emplace_back(&position->second, &velocity->second);
                }
                else {
                    auto idx = freeIDXs.back();
                    entityIDs[id] = idx;
                    freeIDXs.pop_back();
                    std::pair<EntityManager::component_pair const *, EntityManager::component_pair const *>
                        pos_vel_pair{&position->second, &velocity->second};
                    entities[idx] = pos_vel_pair;
                }
            }
        }
    }
}

void MovementSystem::removeEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID != entityIDs.end()) {
        freeIDXs.push_back(entityID->second);
        entityIDs.erase(entityID);
    }
}

void MovementSystem::refreshEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID != entityIDs.end() && !(entities[entityID->second].first->first && entities[entityID->second].second->first)) {
        freeIDXs.push_back(entityID->second);
        entityIDs.erase(entityID);
    } else if (entityID != entityIDs.end() ) {
        auto entity = manager->getEntity(id);
        auto delay = entity->find("fullDelay");
        auto pause = entity->find("pauseDelay");
        if ( (delay != entity->end() && delay->second.first) || (pause != entity->end() && pause->second.first) ) {
            freeIDXs.push_back(entityID->second);
            entityIDs.erase(entityID);
        }
    } else {
        addEntity(id);
    }

}

void MovementSystem::process(float dt) {

    auto startT = SDL_GetPerformanceCounter();

    for(auto& entityID : entityIDs) {
        auto& entity = entities[entityID.second];
        Position& position = (*positionPool)[entity.first->second].data;
        Velocity& velocity = (*velocityPool)[entity.second->second].data;
        position.pastPosX = position.posX;
        position.pastPosY = position.posY;
        position.posX += dt * velocity.velX;
        position.posY += dt * velocity.velY;
    }

    auto endT = SDL_GetPerformanceCounter();

    //std::cout << "M-" << (1000.f / SDL_GetPerformanceFrequency() * (endT - startT) ) << '\n';

}

PositionSyncSystem::PositionSyncSystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority} {
    positionPool = manager->getComponentPool<Component<Position::name, Position>>();
};

void PositionSyncSystem::initialize() {}

void PositionSyncSystem::addEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID == entityIDs.end()) {
        auto entity = manager->getEntity(id);
        if (entity) {
            auto position = entity->find("position");
            if ( position != entity->end()
                && position->second.first ) {

                if (freeIDXs.empty()) {
                    entityIDs[id] = entities.size();
                    entities.emplace_back(&position->second);
                }
                else {
                    auto idx = freeIDXs.back();
                    entityIDs[id] = idx;
                    freeIDXs.pop_back();
                    entities[idx] = &position->second;
                }
            }
        }
    }
}

void PositionSyncSystem::removeEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID != entityIDs.end()) {
        freeIDXs.push_back(entityID->second);
        entityIDs.erase(entityID);
    }
}

void PositionSyncSystem::refreshEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID != entityIDs.end() && !(entities[entityID->second]->first)) {
        freeIDXs.push_back(entityID->second);
        entityIDs.erase(entityID);
    } else {
        addEntity(id);
    }

}

void PositionSyncSystem::process(float dt) {

    auto startT = SDL_GetPerformanceCounter();

    for(auto& entityID : entityIDs) {
        auto& entity = entities[entityID.second];
        Position& position = (*positionPool)[entity->second].data;
        position.pastPosX = position.posX;
        position.pastPosY = position.posY;
    }

    auto endT = SDL_GetPerformanceCounter();

    //std::cout << "M-" << (1000.f / SDL_GetPerformanceFrequency() * (endT - startT) ) << '\n';

}

