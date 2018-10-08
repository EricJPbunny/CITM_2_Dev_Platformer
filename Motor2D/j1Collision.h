#ifndef __ModuleCollision_H__
#define __ModuleCollision_H__

#define MAX_COLLIDERS 50

#include "j1Module.h"

enum COLLIDER_TYPE
{
	COLLIDER_NONE = -1,
	COLLIDER_WALL,
	COLLIDER_PLAYER,

	COLLIDER_MAX
};

class Collider
{
	fPoint position;
	fPoint velocity;
	fPoint acceleration;

	//TYPE: Dynamic or static

	SDL_Rect rect;
	COLLIDER_TYPE type;

	Collider(SDL_Rect rectangle, COLLIDER_TYPE type) :
		rect(rectangle),
		type(type)
	{}

	virtual void OnCollision() {

	}
};

class j1Collision : public j1Module
{
public:

	j1Collision();
	~j1Collision();

	bool PreUpdate();
	bool Update();
	bool CleanUp();

	Collider* AddCollider(SDL_Rect rect, COLLIDER_TYPE type, j1Module* callback = nullptr);
	void DebugDraw();

private:

	Collider * colliders[MAX_COLLIDERS];
	bool matrix[COLLIDER_MAX][COLLIDER_MAX];
	bool debug = false;
};

#endif // __ModuleCollision_H__