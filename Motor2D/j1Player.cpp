#include "j1Player.h"
#include "j1Input.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "p2Animation.h"
#include "p2Defs.h"
#include "j1Collision.h"
#include "math.h"
#include "p2Log.h"

#include "j1Map.h"

j1Player::j1Player() : j1Module()
{
	name.create("player");
}

bool j1Player::Awake(pugi::xml_node& player_node)
{
	//Values from xml
	SDL_Rect colliderRect;
	colliderRect.w = player_node.child("collider_width").text().as_int();
	colliderRect.h = player_node.child("collider_height").text().as_int();
	playerCol = App->collision->AddCollider(colliderRect, COLLIDER_PLAYER, this);
	SDL_Rect feetColRect;
	feetColRect.w = player_node.child("feet_collider_width").text().as_int();
	feetColRect.h = player_node.child("feet_collider_height").text().as_int();
	feetCol = App->collision->AddCollider(feetColRect, COLLIDER_PLAYER, this);
	tileSize = player_node.child("tile_size").text().as_uint();
	gravity = tile_to_pixel(player_node.child("gravity").text().as_float());
	moveSpeedGnd = tile_to_pixel(player_node.child("move_speed_ground").text().as_float());
	moveSpeedAir = tile_to_pixel(player_node.child("move_speed_air").text().as_float());
	//- This formula traduces gives us the speed necessary to reach a certain height
	//- It is calculated using the conservation of mechanic energy
	jumpSpeed = -sqrtf(gravity * tile_to_pixel(player_node.child("jump_height").text().as_float()) * 2.0f);

	//Projectile
	projectileSpeed = player_node.child("projectile").child("speed").text().as_float();
	projectileStartHeight = player_node.child("projectile").child("start_height").text().as_uint();
	projectileColRect.w = player_node.child("projectile").child("collider_width").text().as_int();
	projectileColRect.h = player_node.child("projectile").child("collider_height").text().as_int();
	projectilePath = player_node.child("projectile").child("image").text().as_string();

	//Animation
	animTileWidth = player_node.child("animation").attribute("tile_width").as_uint();
	animTileHeight = player_node.child("animation").attribute("tile_height").as_uint();

	idlePath = player_node.child("animation").child("idle_image").text().as_string();
	runPath = player_node.child("animation").child("run_image").text().as_string();
	jumpPath = player_node.child("animation").child("jump_image").text().as_string();

	LoadAnimation(player_node.child("animation").child("idle_animation").child("sprite"), idleAnim);
	idleAnim.speed = player_node.child("animation").child("idle_animation").attribute("speed").as_float();
	LoadAnimation(player_node.child("animation").child("run_animation").child("sprite"), runAnim);
	runAnim.speed = player_node.child("animation").child("run_animation").attribute("speed").as_float();
	LoadAnimation(player_node.child("animation").child("jump_animation").child("sprite"), jumpAnim);
	jumpAnim.speed = player_node.child("animation").child("jump_animation").attribute("speed").as_float();
	LoadAnimation(player_node.child("animation").child("fall_animation").child("sprite"), fallAnim);
	fallAnim.speed = player_node.child("animation").child("fall_animation").attribute("speed").as_float();
	currAnim = &idleAnim;
	return true;
}

bool j1Player::Start()
{
	//INFO: We can't load the texture in awake because the render is not initialized yet
	idleTex = App->tex->LoadTexture(idlePath.GetString());
	runTex = App->tex->LoadTexture(runPath.GetString());
	jumpTex = App->tex->LoadTexture(jumpPath.GetString());
	currTex = idleTex;

	projectileTex = App->tex->LoadTexture(projectilePath.GetString());

	//General values
	position.x = App->map->playerData.x;//80;
	position.y = App->map->playerData.y;//50;
	velocity.x = 0;
	velocity.y = 0;
	acceleration.x = 0;
	acceleration.y = 0;

	return true;
}

bool j1Player::PreUpdate()
{
	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT && App->input->GetKey(SDL_SCANCODE_D) == KEY_IDLE) {
		if (isOnPlatform) {
			velocity.x = -moveSpeedGnd;
		}
		else {
			velocity.x = -moveSpeedAir;
		}
		flip = SDL_RendererFlip::SDL_FLIP_HORIZONTAL;
	}
	else if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT && App->input->GetKey(SDL_SCANCODE_A) == KEY_IDLE) {
		if (isOnPlatform) {
			velocity.x = moveSpeedGnd;
		}
		else {
			velocity.x = moveSpeedAir;
		}
		flip = SDL_RendererFlip::SDL_FLIP_NONE;
	}
	else {
		velocity.x = 0;
	}

	// Check that it is hitting the ground to be able to jump (he could jump on the air otherwise)
	if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && isOnPlatform) {
		velocity.y = jumpSpeed;
		isOnPlatform = false;
		checkFall = false;
	}

	return true;
}

bool j1Player::Update(float dt)
{
	CalculateDeltaTime();
	UpdatePlayer();
	UpdateProjectile();
	return true;
}

bool j1Player::PostUpdate()
{
	//Once the movement and the physical resolution has happened, determine the animations it must play

	if (velocity.x > 0) {
		flip = SDL_RendererFlip::SDL_FLIP_NONE;
	}
	else if (velocity.x < 0) {
		flip = SDL_RendererFlip::SDL_FLIP_HORIZONTAL;
	}

	if (isOnPlatform) {
		if(velocity.x != 0) {
			currTex = runTex;
			currAnim = &runAnim;
		}
		else {
			currTex = idleTex;
			currAnim = &idleAnim;
		}
	}
	else {
		currTex = jumpTex;
		if(velocity.y <= 0){
			currAnim = &jumpAnim;
		}
		else {
			currAnim = &fallAnim;
		}
	}

	iPoint blitPos = GetPosFromPivot(pivot::bottom_middle, (int)position.x, (int)position.y, animTileWidth, animTileHeight);
	App->render->Blit(currTex, blitPos.x, blitPos.y, &currAnim->GetCurrentFrame(), 1.0f, flip);
	return true;
}

bool j1Player::CleanUp()
{
	App->tex->UnloadTexture(idleTex);
	App->tex->UnloadTexture(runTex);
	App->tex->UnloadTexture(jumpTex);
	App->tex->UnloadTexture(projectileTex);
	//playerCol->to_delete = true;
	//feetCol->to_delete = true;
	return true;
}

bool j1Player::Load(pugi::xml_node&)
{
	return true;
}

bool j1Player::Save(pugi::xml_node&) const
{
	return true;
}

void j1Player::OnCollision(Collider* c1, Collider* c2) {
	if (c1 == playerCol) {
		switch (c2->type) {
		case COLLIDER_WALL:
			//1. Check which direction it was going to
			bool direction[(uint)dir::max];
			direction[(uint)dir::up]	= velocity.y < 0;
			direction[(uint)dir::down]	= velocity.y > 0;
			direction[(uint)dir::left]	= velocity.x < 0;
			direction[(uint)dir::right]	= velocity.x > 0;

			//2. Check which point (opposite to those directions) is the nearest
			//Ex.: If it has entered in the direction "up" the distance is from the character to the bottom of the other collider
			uint dist[(uint)dir::max];
			dist[(uint)dir::up] = (c2->rect.y + c2->rect.h) - (position.y - playerCol->rect.h);
			dist[(uint)dir::down] = position.y - c2->rect.y;
			dist[(uint)dir::left] = (c2->rect.x + c2->rect.w) - (position.x - playerCol->rect.w/2);
			dist[(uint)dir::right] = (position.x + playerCol->rect.w / 2) - c2->rect.x;
			int nearestDir = -1;
			for (int i = 0; i < (int)dir::max; ++i) {
				if (direction[i]) {
					if (nearestDir == -1) {
						nearestDir = i;
					}
					else if (dist[i] < dist[nearestDir]) {
						nearestDir = i;
					}
				}
			}

			//3. Move it to that point
			switch (nearestDir) {
			case (int)dir::down:
				position.y = c2->rect.y;
				velocity.y = 0;
				acceleration.y = 0;
				checkFall = true;
				isOnPlatform = true;
				break;
			case (int)dir::up:
				position.y = c2->rect.y + c2->rect.h + playerCol->rect.h;
				velocity.y = 0;
				break;
			case (int)dir::left:
				position.x = c2->rect.x + c2->rect.w + playerCol->rect.w / 2;
				velocity.x = 0;
				break;
			case (int)dir::right:
				position.x = c2->rect.x - playerCol->rect.w / 2;
				velocity.x = 0;
				break;
			}
			iPoint colPos = GetPosFromPivot(pivot::bottom_middle, (int)position.x, (int)position.y, playerCol->rect.w, playerCol->rect.h);
			playerCol->SetPos(colPos.x, colPos.y);
			feetCol->SetPos(position.x - feetCol->rect.w / 2, position.y);
			break;
		}
	}

	//Only starts checking if it falls when it snaps to the platform
	//Otherwise it would stop adding gravity the moment the foot collider touched the floor, making the character be way above what it should be
	if (c1 == feetCol && checkFall) {
		isOnPlatform = true;
	}
}

//We can consider that 1 tile = 1 meter to make it easier for us to imagine the different values
inline float j1Player::tile_to_pixel(uint pixel) {
	return pixel * tileSize;
}

bool j1Player::LoadAnimation(pugi::xml_node &node, Animation &anim) {
	for (; node; node = node.next_sibling("sprite")) {
		SDL_Rect frame;
		frame.x = node.attribute("x").as_int();
		frame.y = node.attribute("y").as_int();
		frame.w = node.attribute("w").as_int();
		frame.h = node.attribute("h").as_int();
		anim.PushBack(frame);
	}
	return true;
}

void j1Player::CalculateDeltaTime()
{
	if (isFirstFrame) {
		currTime = lastTime = SDL_GetTicks();
		isFirstFrame = false;
	}
	else {
		currTime = SDL_GetTicks();
	}
	deltaTime = currTime - lastTime;
	deltaTime /= 1000;//1 second is 1000 miliseconds
	lastTime = currTime;
}

void j1Player::UpdatePlayer()
{
	//Add gravity
	if (isOnPlatform) {
		acceleration.y = 0;
	}
	else {
		acceleration.y = gravity;
		checkFall = false;
	}

	//- Move the player
	velocity = velocity + acceleration * deltaTime;
	position = position + velocity * deltaTime;
	iPoint colPos = GetPosFromPivot(pivot::bottom_middle, (int)position.x, (int)position.y, playerCol->rect.w, playerCol->rect.h);
	playerCol->SetPos(colPos.x, colPos.y);
	feetCol->SetPos(position.x - feetCol->rect.w / 2, position.y);

	// - If this value remains false after checking the collision we'll consider the player has fallen from the platform
	isOnPlatform = false;
}

void j1Player::UpdateProjectile()
{
	//Shoot the projectile
	if (App->input->GetMouseButton(1) == KEY_DOWN) {
		projectilePos.x = position.x;
		projectilePos.y = position.y - projectileStartHeight;

		iPoint mousePos;
		App->input->GetMousePosition(mousePos.x, mousePos.y);
		projectileVelocity = (fPoint)mousePos - projectilePos;
		projectileVelocity.Normalize();
		projectileVelocity *= projectileSpeed;
	}
	//Move projectile
	projectilePos += projectileVelocity;

	iPoint blitPos = GetPosFromPivot(pivot::middle_middle, projectilePos.x, projectilePos.y, projectileColRect.w, projectileColRect.h);
	App->render->Blit(projectileTex, blitPos.x, blitPos.y);
}

iPoint j1Player::GetPosFromPivot(pivot pivot, int x, int y, uint w, uint h) {
	;
	switch (pivot) {
	case pivot::top_left:
		return iPoint(x, y);
		break;
	case pivot::top_middle:
		return iPoint(x - w / 2, y);
		break;
	case pivot::top_right:
		return iPoint(x - w, y);
		break;
	case pivot::middle_left:
		return iPoint(x, y - h / 2);
		break;
	case pivot::middle_middle:
		return iPoint(x - w / 2, y - h / 2);
		break;
	case pivot::middle_right:
		return iPoint(x - w, y - h / 2);
		break;
	case pivot::bottom_left:
		return iPoint(x, y - h);
		break;
	case pivot::bottom_middle:
		return iPoint(x - w / 2, y - h);
		break;
	case pivot::bottom_right:
		return iPoint(x - w, y - h);
		break;
	}
}

fPoint j1Player::GetPlayerPos() const
{
	return position;
}

fPoint j1Player::GetVelocity() const
{
	return velocity;
}

void j1Player::SetPosition(fPoint newPos)
{
	position.x = newPos.x;
	position.y = newPos.y;
}