#include "DelaySystem.h"
#include "SDL.h"
#include <iostream>

DelaySystem::DelaySystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority} {
    delayPool = manager->getComponentPool<Component<delayComponent::fullDelay, float>>();
    entityIDXs.reserve(1 << 16);
    hasEntity.reserve(1 << 16);
    idxToID.reserve(1 << 16);
    entities.reserve(1 << 16);
};

void DelaySystem::initialize() {}

void DelaySystem::addEntity(uint32_t id) {
    if (id >= hasEntity.size()) {
        hasEntity.resize(id + 1, false);
        entityIDXs.resize(id + 1, 0);
    }
    if (hasEntity[id]) return;
    const auto& entity = manager->getEntity(id);
    if (entity) {
        auto delay = entity->find("fullDelay");
        auto pause = entity->find("pauseDelay");
        if ((delay == entity->end() || !delay->second.first) && (pause != entity->end() && pause->second.first)) {

            entityIDXs[id] = entities.size();
            hasEntity[id] = true;
            idxToID.emplace_back(id);
            entities.emplace_back(&pause->second);

        }
    }
}

void DelaySystem::removeEntity(uint32_t id) {
    if (!hasEntity[id]) return;
    entities[entityIDXs[id]] = entities.back();
    entities.pop_back();
    entityIDXs[idxToID.back()] = entityIDXs[id];
    idxToID[entityIDXs[id]] = idxToID.back();
    idxToID.pop_back();
    hasEntity[id] = false;
}

void DelaySystem::refreshEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) {
        addEntity(id);
        return;
    }
    const auto& entity = entities[entityIDXs[id]];
    if (!(entity->first)) {
        removeEntity(id);
    }

}

void DelaySystem::process(float dt) {
    dt *= 1000.0f;

    for(size_t i = 0; i < entities.size(); ++i) {
        const auto& entity = entities[i];
        float& delay = (*delayPool)[entity->second].data;
        delay -= dt;
        if (delay <= 0.0f) manager->removeComponent<Component<delayComponent::fullDelay, float>>(idxToID[i]);
    }

}

PauseSystem::PauseSystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority} {
    pausePool = manager->getComponentPool<Component<delayComponent::pauseDelay, float>>();
    entityIDXs.reserve(1 << 16);
    hasEntity.reserve(1 << 16);
    idxToID.reserve(1 << 16);
    entities.reserve(1 << 16);
};

void PauseSystem::initialize() {}

void PauseSystem::addEntity(uint32_t id) {
    if (id >= hasEntity.size()) {
        hasEntity.resize(id + 1, false);
        entityIDXs.resize(id + 1, 0);
    }
    if (hasEntity[id]) return;
    const auto& entity = manager->getEntity(id);
    if (entity) {
        auto pause = entity->find("fullPause");
        if (pause != entity->end() && pause->second.first) {

            entityIDXs[id] = entities.size();
            hasEntity[id] = true;
            idxToID.emplace_back(id);
            entities.emplace_back(&pause->second);


        }
    }
}

void PauseSystem::removeEntity(uint32_t id) {
    if (!hasEntity[id]) return;
    entities[entityIDXs[id]] = entities.back();
    entities.pop_back();
    entityIDXs[idxToID.back()] = entityIDXs[id];
    idxToID[entityIDXs[id]] = idxToID.back();
    idxToID.pop_back();
    hasEntity[id] = false;
}

void PauseSystem::refreshEntity(uint32_t id) {
    if (id >= hasEntity.size() || !hasEntity[id]) {
        addEntity(id);
        return;
    }
    const auto& entity = entities[entityIDXs[id]];
    if (!(entity->first)) {
        removeEntity(id);
    } else {
        const auto& fullEntity = manager->getEntity(id);
        auto delay = fullEntity->find("fullDelay");
        if (delay != fullEntity->end() && delay->second.first) {
            removeEntity(id);
        }
    }

}

void PauseSystem::process(float dt) {
    dt *= 1000.0f;

    for(size_t i = 0; i < entities.size(); ++i) {
        const auto& entity = entities[i];
        float& pause = (*pausePool)[entity->second].data;
        pause -= dt;
        if (pause <= 0.0f) manager->removeComponent<Component<delayComponent::pauseDelay, float>>(idxToID[i]);
    }

}

