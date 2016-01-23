#ifndef PLAYSTATE_H_INCLUDED
#define PLAYSTATE_H_INCLUDED

#include "gameState.h"
#include "EntityManager.h"
#include "EntitySystem.h"
#include "window.h"
#include "MovementSystem.h"
#include "RenderSystem.h"

class PlayState : public GameState{
public:

    PlayState(Window* w);

    virtual void handleInput();

    virtual void update(float rate);

    virtual void render(float remainder);

    virtual ~PlayState();

private:
    EntityManager manager;
    Window* window;
    MovementSystem moveSys;
    RenderSystem renderSys;
};

#endif // PLAYSTATE_H_INCLUDED
