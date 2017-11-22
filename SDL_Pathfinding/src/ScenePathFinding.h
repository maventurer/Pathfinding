#pragma once
#include <vector>
#include <list>
#include <map>
#include <time.h>
#include "Scene.h"
#include "Agent.h"
#include "Path.h"
#include "Node.h"

class ScenePathFinding :
	public Scene
{
public:
	ScenePathFinding();
	~ScenePathFinding();
	void update(float dtime, SDL_Event *event);
	void draw();
	const char* getTitle();
private:
	std::vector<Agent*> agents;
	Vector2D coinPosition;
	Vector2D currentTarget;
	int currentTargetIndex;
	Path path;
	int num_cell_x;
	int num_cell_y;
	bool draw_grid;
	bool draw_nodes;
	bool draw_path;
	std::vector<SDL_Rect> maze_rects;
	std::vector<SDL_Rect> maze_water;
	std::vector<SDL_Rect> maze_mud;
	void drawMaze();
	void drawCoin();
	SDL_Texture *background_texture;
	SDL_Texture *coin_texture;
	void initMaze();
	bool loadTextures(char* filename_bg, char* filename_coin);
	std::vector< std::vector<int> > terrain;
	Vector2D cell2pix(Vector2D cell);
	Vector2D pix2cell(Vector2D pix);
	bool isValidCell(Vector2D cell);

	std::vector<Node*> nodes;
	std::map<Vector2D, Node*> graph;
	algorithms algorithm;
};
