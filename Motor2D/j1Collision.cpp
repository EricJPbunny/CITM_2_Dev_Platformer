#include "j1App.h"
#include "j1Input.h"
#include "j1Render.h"
#include "j1Collision.h"

#include "p2Log.h"

j1Collision::j1Collision()
{
	for (uint i = 0; i < MAX_COLLIDERS; ++i)
		colliders[i] = nullptr;

	matrix[COLLIDER_WALL][COLLIDER_WALL] = false;
	matrix[COLLIDER_WALL][COLLIDER_PLAYER] = true;
	matrix[COLLIDER_WALL][COLLIDER_ENEMY] = true;
	matrix[COLLIDER_WALL][COLLIDER_PLAYER_SHOT] = true;
	matrix[COLLIDER_WALL][COLLIDER_ENEMY_SHOT] = true;
	//matrix[COLLIDER_WALL][COLLIDER_POWER_UP] = false;
	matrix[COLLIDER_WALL][COLLIDER_PLAYER_GOD] = false;

	matrix[COLLIDER_PLAYER][COLLIDER_WALL] = true;
	matrix[COLLIDER_PLAYER][COLLIDER_PLAYER] = false;
	matrix[COLLIDER_PLAYER][COLLIDER_ENEMY] = true;
	matrix[COLLIDER_PLAYER][COLLIDER_PLAYER_SHOT] = false;
	matrix[COLLIDER_PLAYER][COLLIDER_ENEMY_SHOT] = true;
	//matrix[COLLIDER_PLAYER][COLLIDER_POWER_UP] = false;
	matrix[COLLIDER_PLAYER][COLLIDER_PLAYER_GOD] = false;

	matrix[COLLIDER_PLAYER_GOD][COLLIDER_WALL] = false;
	matrix[COLLIDER_PLAYER_GOD][COLLIDER_PLAYER] = false;
	matrix[COLLIDER_PLAYER_GOD][COLLIDER_ENEMY] = false;
	matrix[COLLIDER_PLAYER_GOD][COLLIDER_PLAYER_SHOT] = false;
	matrix[COLLIDER_PLAYER_GOD][COLLIDER_ENEMY_SHOT] = false;
	//matrix[COLLIDER_PLAYER_GOD][COLLIDER_POWER_UP] = false;
	matrix[COLLIDER_PLAYER_GOD][COLLIDER_PLAYER_GOD] = false;
	
	matrix[COLLIDER_ENEMY][COLLIDER_WALL] = true;
	matrix[COLLIDER_ENEMY][COLLIDER_PLAYER] = true;
	matrix[COLLIDER_ENEMY][COLLIDER_ENEMY] = false;
	matrix[COLLIDER_ENEMY][COLLIDER_PLAYER_SHOT] = true;
	matrix[COLLIDER_ENEMY][COLLIDER_ENEMY_SHOT] = false;
	//matrix[COLLIDER_ENEMY][COLLIDER_POWER_UP] = false;
	matrix[COLLIDER_ENEMY][COLLIDER_PLAYER_GOD] = false;

	matrix[COLLIDER_PLAYER_SHOT][COLLIDER_WALL] = true;
	matrix[COLLIDER_PLAYER_SHOT][COLLIDER_PLAYER] = false;
	matrix[COLLIDER_PLAYER_SHOT][COLLIDER_ENEMY] = true;
	matrix[COLLIDER_PLAYER_SHOT][COLLIDER_PLAYER_SHOT] = false;
	matrix[COLLIDER_PLAYER_SHOT][COLLIDER_ENEMY_SHOT] = false;
	//matrix[COLLIDER_PLAYER_SHOT][COLLIDER_POWER_UP] = false;
	matrix[COLLIDER_PLAYER_SHOT][COLLIDER_PLAYER_GOD] = false;

	matrix[COLLIDER_ENEMY_SHOT][COLLIDER_WALL] = true;
	matrix[COLLIDER_ENEMY_SHOT][COLLIDER_PLAYER] = true;
	matrix[COLLIDER_ENEMY_SHOT][COLLIDER_ENEMY] = false;
	matrix[COLLIDER_ENEMY_SHOT][COLLIDER_PLAYER_SHOT] = false;
	matrix[COLLIDER_ENEMY_SHOT][COLLIDER_ENEMY_SHOT] = false;
	//matrix[COLLIDER_ENEMY_SHOT][COLLIDER_POWER_UP] = false;
	matrix[COLLIDER_ENEMY_SHOT][COLLIDER_PLAYER_GOD] = false;

	/*matrix[COLLIDER_POWER_UP][COLLIDER_WALL] = false;
	matrix[COLLIDER_POWER_UP][COLLIDER_PLAYER] = true;
	matrix[COLLIDER_POWER_UP][COLLIDER_ENEMY] = false;
	matrix[COLLIDER_POWER_UP][COLLIDER_PLAYER_SHOT] = false;
	matrix[COLLIDER_POWER_UP][COLLIDER_ENEMY_SHOT] = false;
	matrix[COLLIDER_POWER_UP][COLLIDER_POWER_UP] = false;
	matrix[COLLIDER_POWER_UP][COLLIDER_PLAYER_GOD] = true;*/

	actualColliders = 0;
}

// Destructor
j1Collision::~j1Collision()
{}

bool j1Collision::PreUpdate()
{
	// Remove all colliders scheduled for deletion
	for (uint i = 0; i < MAX_COLLIDERS; ++i)
	{
		if (colliders[i] != nullptr && colliders[i]->to_delete == true)
		{
			delete colliders[i];
			colliders[i] = nullptr;
			actualColliders--;
		}
	}

	return true;
}

// Called before render is available
bool j1Collision::Update(float dt)
{
	// Calculate collisions
	Collider* c1;
	Collider* c2;

	for (uint i = 0; i < MAX_COLLIDERS; ++i)
	{
		// skip empty colliders
		if (colliders[i] == nullptr)
			continue;

		c1 = colliders[i];

		// avoid checking collisions already checked
		for (uint k = i + 1; k < MAX_COLLIDERS; ++k)
		{
			// skip empty colliders
			if (colliders[k] == nullptr)
				continue;

			c2 = colliders[k];

			if (c1->CheckCollision(c2->rect) == true)
			{
				if (matrix[c1->type][c2->type] && c1->callback)
					c1->callback->OnCollision(c1, c2);

				if (matrix[c2->type][c1->type] && c2->callback)
					c2->callback->OnCollision(c2, c1);
			}
		}
	}

	return true;
}

bool j1Collision::PostUpdate() {
	DebugDraw();

	return true;
}

void j1Collision::DebugDraw()
{
	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
		debug = !debug;

	if (debug == false)
		return;

	Uint8 alpha = 80;
	for (uint i = 0; i < MAX_COLLIDERS; ++i)
	{
		if (colliders[i] == nullptr)
			continue;

		switch (colliders[i]->type)
		{
		case COLLIDER_NONE: // white
			App->render->DrawQuad(colliders[i]->rect, 255, 255, 255, alpha);
			break;
		case COLLIDER_WALL: // blue
			App->render->DrawQuad(colliders[i]->rect, 0, 0, 255, alpha);
			break;
		case COLLIDER_PLAYER: // green
			App->render->DrawQuad(colliders[i]->rect, 0, 255, 0, alpha);
			break;
		case COLLIDER_ENEMY: // red
			App->render->DrawQuad(colliders[i]->rect, 255, 0, 0, alpha);
			break;
		case COLLIDER_PLAYER_SHOT: // yellow
			App->render->DrawQuad(colliders[i]->rect, 255, 255, 0, alpha);
			break;
		case COLLIDER_ENEMY_SHOT: // magenta
			App->render->DrawQuad(colliders[i]->rect, 0, 255, 255, alpha);
			break;
		//case COLLIDER_POWER_UP: //dark orange
			//App->render->DrawQuad(colliders[i]->rect, 255, 140, 0, alpha);
			//break;
		case COLLIDER_PLAYER_GOD: //pink
			App->render->DrawQuad(colliders[i]->rect, 255, 0, 255, alpha);
			break;
		// secret purple
		//App->render->DrawQuad(colliders[i]->rect, 95, 0, 232, alpha);
		}
	}
}

// Called before quitting
bool j1Collision::CleanUp()
{
	if (!exitGameLoop) // to avoid general cleanup order when exits the game
	{
		LOG("Freeing all colliders");

		for (uint i = 0; i < MAX_COLLIDERS; ++i)
		{
			if (colliders[i] != nullptr)
			{
				delete colliders[i];
				colliders[i] = nullptr;
			}
		}
	}
	else
		exitGameLoop = false;

	return true;
}

Collider* j1Collision::AddCollider(SDL_Rect rect, COLLIDER_TYPE type, j1Module* callback, Uint32 damage)
{
	Collider* ret = nullptr;

	for (uint i = 0; i < MAX_COLLIDERS; ++i)
	{
		if (colliders[i] == nullptr)
		{
			ret = colliders[i] = new Collider(rect, type, callback, damage);
			actualColliders++;
			break;
		}
	}

	return ret;
}

// -----------------------------------------------------

bool Collider::CheckCollision(const SDL_Rect& r) const
{
	return !(r.x >= (rect.x + rect.w) || (r.x + r.w) <= rect.x || r.y >= (rect.y + rect.h) || (r.y + r.h) <= rect.y);
}