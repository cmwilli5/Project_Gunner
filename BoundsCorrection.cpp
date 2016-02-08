#include "BoundsCorrection.h"
#include "SDL.h"

BoundsSystem::BoundsSystem(EntityManager* const manager, int32_t priority) : EntitySystem{manager, priority} {
    positionPool = manager->getComponentPool<Component<Position::name, Position>>();
    boundsPool = manager->getComponentPool<Component<Bounds::name, Bounds>>();
};

void BoundsSystem::initialize() {}

void BoundsSystem::addEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID == entityIDs.end()) {
        auto entity = manager->getEntity(id);
        if (entity) {
            auto position = entity->find("position");
            auto bounds = entity->find("bounds");
            if (position != entity->end() && bounds != entity->end() && position->second.first && bounds->second.first) {

                if (freeIDXs.empty()) {
                    entityIDs[id] = entities.size();
                    entities.emplace_back(&position->second, &bounds->second);
                }
                else {
                    auto idx = freeIDXs.back();
                    entityIDs[id] = idx;
                    freeIDXs.pop_back();
                    std::pair<EntityManager::component_pair const *, EntityManager::component_pair const *>
                        pos_vel_pair{&position->second, &bounds->second};
                    entities[idx] = pos_vel_pair;
                }

                Position& pos = (*positionPool)[position->second.second].data;
                Bounds& b = (*boundsPool)[bounds->second.second].data;

                if (b.behavior == Bounds::Behavior::wrap) {
                    if (pos.posX < b.minX) pos.posX = b.minX;
                    else if (pos.posX > b.maxX) pos.posX = b.maxX;
                    if (pos.posY < b.minY) pos.posY = b.minY;
                    else if (pos.posY > b.maxY) pos.posY = b.maxY;
                    if (pos.pastPosX < b.minX) pos.pastPosX = b.minX;
                    else if (pos.pastPosX > b.maxX) pos.pastPosX = b.maxX;
                    if (pos.pastPosY < b.minY) pos.pastPosY = b.minY;
                    else if (pos.pastPosY > b.maxY) pos.pastPosY = b.maxY;
                } else if (b.behavior == Bounds::Behavior::wrap_x) {
                    if (pos.posX < b.minX) pos.posX = b.minX;
                    else if (pos.posX > b.maxX) pos.posX = b.maxX;
                    if (pos.pastPosX < b.minX) pos.pastPosX = b.minX;
                    else if (pos.pastPosX > b.maxX) pos.pastPosX = b.maxX;
                } else if (b.behavior == Bounds::Behavior::wrap_y) {
                    if (pos.posY < b.minY) pos.posY = b.minY;
                    else if (pos.posY > b.maxY) pos.posY = b.maxY;
                    if (pos.pastPosY < b.minY) pos.pastPosY = b.minY;
                    else if (pos.pastPosY > b.maxY) pos.pastPosY = b.maxY;
                }

            }
        }
    }
}

void BoundsSystem::removeEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID != entityIDs.end()) {
        freeIDXs.push_back(entityID->second);
        entityIDs.erase(entityID);
    }
}

void BoundsSystem::refreshEntity(uint32_t id) {
    auto entityID = entityIDs.find(id);
    if (entityID != entityIDs.end() && !(entities[entityID->second].first->first && entities[entityID->second].second->first)) {
        freeIDXs.push_back(entityID->second);
        entityIDs.erase(entityID);
    } else if (entityID == entityIDs.end() ) {
        addEntity(id);
    }

}

void BoundsSystem::process(float dt) {

    auto startT = SDL_GetPerformanceCounter();

    for(auto& entityID : entityIDs) {
        auto& entity = entities[entityID.second];
        Position& position = (*positionPool)[entity.first->second].data;
        Bounds& bounds = (*boundsPool)[entity.second->second].data;
        if (bounds.behavior == Bounds::Behavior::block) {

            if (position.posX < bounds.minX) position.posX = bounds.minX;
            else if (position.posX > bounds.maxX) position.posX = bounds.maxX;
            if (position.posY < bounds.minY) position.posY = bounds.minY;
            else if (position.posY > bounds.maxY) position.posY = bounds.maxY;

            if (position.pastPosX < bounds.minX) position.pastPosX = bounds.minX;
            else if (position.pastPosX > bounds.maxX) position.pastPosX = bounds.maxX;
            if (position.pastPosY < bounds.minY) position.pastPosY = bounds.minY;
            else if (position.pastPosY > bounds.maxY) position.pastPosY = bounds.maxY;

        } else if (bounds.behavior == Bounds::Behavior::destroy
            && (position.posX < bounds.minX || position.posX > bounds.maxX || position.posY < bounds.minY || position.posY > bounds.maxY) ) {

            manager->entitiesToDestroy.insert(entityID.first);

        } else if (bounds.behavior == Bounds::Behavior::wrap) {
            if (position.posX < bounds.minX) {
                position.pastPosX = bounds.maxX;
                position.posX = bounds.maxX - bounds.minX + position.posX;
            } else if (position.posX > bounds.maxX) {
                position.pastPosX = bounds.minX;
                position.posX = bounds.minX + position.posX - bounds.maxX;
            }
            if (position.posY < bounds.minY) {
                position.pastPosY = bounds.maxY;
                position.posY = bounds.maxY - bounds.minY + position.posY;
            } else if (position.posY > bounds.maxY) {
                position.pastPosY = bounds.minY;
                position.posY = bounds.minY + position.posY - bounds.maxY;
            }
        } else if (bounds.behavior == Bounds::Behavior::wrap_x) {
            if (position.posX < bounds.minX) {
                position.pastPosX = bounds.maxX;
                position.posX = bounds.maxX - bounds.minX + position.posX;
            } else if (position.posX > bounds.maxX) {
                position.pastPosX = bounds.minX;
                position.posX = bounds.minX + position.posX - bounds.maxX;
            }

        } else if (bounds.behavior == Bounds::Behavior::wrap_y) {
            if (position.posY < bounds.minY) {
                position.pastPosY = bounds.maxY;
                position.posY = bounds.maxY - bounds.minY + position.posY;
            } else if (position.posY > bounds.maxY) {
                position.pastPosY = bounds.minY;
                position.posY = bounds.minY + position.posY - bounds.maxY;
            }

        }
    }

    auto endT = SDL_GetPerformanceCounter();

    //std::cout << "M-" << (1000.f / SDL_GetPerformanceFrequency() * (endT - startT) ) << '\n';

}




