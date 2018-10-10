#ifndef __j1MAP_H__
#define __j1MAP_H__

#include "PugiXml/src/pugixml.hpp"
#include "p2List.h"
#include "p2Point.h"
#include "j1Module.h"
#include "p2Animation.h"

#define MAX_OBJECTGROUP_COLLIDERS 100

struct Collider;

struct MapObjects
{
	p2SString name;
	uint id = 0;
	Collider* colliders[MAX_OBJECTGROUP_COLLIDERS] = { nullptr };

	~MapObjects()
	{
		//RELEASE_ARRAY(colliders[MAX_OBJECTGROUP_COLLIDERS]);
	}
};

// ----------------------------------------------------
//INFO:
//- Width & Height are always calculated in pixels
//- Columns & Rows are always calculated in number of tiles
struct MapLayer
{
	p2SString name;
	uint columns = 0u;
	uint rows = 0u;
	uint* tileArray = nullptr;

	MapLayer() : tileArray(NULL)
	{}

	~MapLayer()
	{
		RELEASE(tileArray);
	}

	inline uint GetArrayPos(int column, int row) const
	{
		return tileArray[(columns*row) + column];
	}
};

// ----------------------------------------------------
struct TileSet
{
	SDL_Rect GetTileRect(int id) const;

	p2SString			name;
	int					firstgid;
	int					margin;
	int					spacing;
	int					tile_width;
	int					tile_height;
	SDL_Texture*		texture;
	int					tex_width;
	int					tex_height;
	int					columns;
	int					rows;
	int					offset_x;
	int					offset_y;
	Animation*			anim = nullptr;
	float				animSpeedFactor = 0.0025f; // TODO, implement this custom propierty from tileset of animation
};

enum class MapTypes
{
	MAPTYPE_UNKNOWN = 0,
	MAPTYPE_ORTHOGONAL,
	MAPTYPE_ISOMETRIC,
	MAPTYPE_STAGGERED
};
// ----------------------------------------------------
struct MapData
{
	int					columns;
	int					rows;
	int					tile_width;
	int					tile_height;
	SDL_Color			background_color;
	MapTypes			type;
	p2List<TileSet*>	tilesets;
	p2List<MapLayer*>	mapLayers;
	p2List<MapObjects*> mapObjects;
	
};

// ----------------------------------------------------
class j1Map : public j1Module
{
public:

	j1Map();

	// Destructor
	virtual ~j1Map();

	// Called before render is available
	bool Awake(pugi::xml_node& conf);

	// Called each loop iteration
	void Draw();

	// Called before quitting
	bool CleanUp();

	// Load new map
	bool Load(const char* path);

	// Coordinate translation methods
	iPoint MapToWorld(int x, int y) const;
	iPoint WorldToMap(int x, int y) const;

private:

	bool LoadMap();
	bool LoadTilesetDetails(pugi::xml_node& tileset_node, TileSet* set);
	bool LoadTilesetImage(pugi::xml_node& tileset_node, TileSet* set);
	bool LoadLayer(pugi::xml_node& node, MapLayer* layer);
	bool LoadMapColliders(pugi::xml_node& node);//, MapObjects* obj);
	//bool LoadProperties(pugi::xml_node& node, Properties& properties);

	TileSet* GetTilesetFromTileId(int id) const;

public:

	MapData data;

private:

	pugi::xml_document	map_file;
	p2SString			folder;
	bool				map_loaded;
};

#endif // __j1MAP_H__