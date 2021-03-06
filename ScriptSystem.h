#ifndef SCRIPTSYSTEM_H_INCLUDED
#define SCRIPTSYSTEM_H_INCLUDED

#include "EntitySystem.h"
#include <unordered_map>
#include "scriptcomponent.h"
/*
class ScriptSystem : public EntitySystem {

public:
    ScriptSystem(EntityManager* const manager, int32_t priority);

    void initialize();

    void addEntity(uint32_t id);

    void removeEntity(uint32_t id);

    void refreshEntity(uint32_t id);

    void process(float dt);

    void execute(Script& scr, uint32_t id, std::vector<std::string>::size_type beg, std::vector<std::string>::size_type end );

private:

    std::vector<size_t> entityIDXs;

    std::vector<uint8_t> hasEntity;

    std::vector<uint32_t> idxToID;

    std::vector<EntityManager::ComponentHandle const *> entities;

    std::deque<Component<Script::name, Script>>* scriptPool;

};*/

#endif // SCRIPTSYSTEM_H_INCLUDED
