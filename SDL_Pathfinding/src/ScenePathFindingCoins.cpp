#include "ScenePathFindingCoins.h"

using namespace std;

ScenePathFindingCoin::ScenePathFindingCoin()
{
	draw_grid = false;
	draw_nodes = false;
	draw_path = true;

	algorithm = MULTI;
	solution = 0; //Efficient Solution

	node_count = 0;

	num_cell_x = SRC_WIDTH / CELL_SIZE;
	num_cell_y = SRC_HEIGHT / CELL_SIZE;
	initMaze();
	loadTextures("../res/mazeTerrain.png", "../res/coin.png");

	srand((unsigned int)time(NULL));

	Agent *agent = new Agent;
	agent->loadSpriteTexture("../res/soldier.png", 4);
	agents.push_back(agent);

	loadTextures("../res/Water.png", "../res/Mud.png");

	// set agent position coords to the center of a random cell
	Vector2D rand_cell(-1, -1);
	while (!isValidCell(rand_cell))
		rand_cell = Vector2D((float)(rand() % num_cell_x), (float)(rand() % num_cell_y));
	agents[0]->setPosition(cell2pix(rand_cell));

	// set the coin in a random cell (but at least 3 cells far from the agent)
	numTargets = 3 + rand() % 5;
	for (int i = 0; i < numTargets; i++) {
		Vector2D coinPosition(-1, -1);
		while ((!isValidCell(coinPosition)) || (Vector2D::Distance(coinPosition, pix2cell(agents[0]->getPosition()))<3))
			coinPosition = Vector2D((float)(rand() % num_cell_x), (float)(rand() % num_cell_y));

		agent[0].setTarget(coinPosition);

multipleTargets.push_back(graph[Vector2D(agents[0]->getTarget().x * CELL_SIZE + CELL_SIZE / 2, agents[0]->getTarget().y * CELL_SIZE + CELL_SIZE / 2)]);
	}

	// PathFollowing next Target
	currentTarget = Vector2D(0, 0);
	currentTargetIndex = -1;

	// Firtst path
	Vector2D originPosition = pix2cell(Vector2D((float)(agents[0]->getPosition().x), (float)(agents[0]->getPosition().y)));
	Node* origin = graph[Vector2D(originPosition.x * CELL_SIZE + CELL_SIZE / 2, originPosition.y * CELL_SIZE + CELL_SIZE / 2)];

	path = Algorithm::MultipleTargets(multipleTargets, origin, &node_count, solution);

}

ScenePathFindingCoin::~ScenePathFindingCoin()
{
	if (background_texture)
		SDL_DestroyTexture(background_texture);
	if (coin_texture)
		SDL_DestroyTexture(coin_texture);

	agents.clear();
	nodes.clear();
	graph.clear();
	multipleTargets.clear();
}

void ScenePathFindingCoin::update(float dtime, SDL_Event *event)
{
	/* Keyboard & Mouse events */
	switch (event->type) {
	case SDL_KEYDOWN:
		if (event->key.keysym.scancode == SDL_SCANCODE_SPACE)		draw_grid = !draw_grid;
		if (event->key.keysym.scancode == SDL_SCANCODE_N)			draw_nodes = !draw_nodes;
		if (event->key.keysym.scancode == SDL_SCANCODE_P)			draw_path = !draw_path;
		if (event->key.keysym.scancode == SDL_SCANCODE_E)			solution = 0;
		else if (event->key.keysym.scancode == SDL_SCANCODE_R)			solution = 1;
		break;
	default:
		break;
	}

	if ((currentTargetIndex == -1) && (path.points.size() > 0))
		currentTargetIndex = 0;

	if (currentTargetIndex >= 0) {
		// Segundos transcurridos
		if (firstTimer) {
			endingTime = std::chrono::steady_clock::now();
			elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endingTime - startingTime).count();
			OutputData::WriteData(algorithm, elapsedTime, path.points.size(), node_count);
			std::cout << "\nTime difference = " << elapsedTime << std::endl;
			node_count = 0;
			firstTimer = false;
		}

		float dist = Vector2D::Distance(agents[0]->getPosition(), path.points[currentTargetIndex]);
		if (dist < path.ARRIVAL_DISTANCE) {
			if (currentTargetIndex == path.points.size() - 1) {
				if (dist < 3) {
					path.points.clear();
					currentTargetIndex = -1;
					agents[0]->setVelocity(Vector2D(0, 0));
					// if we have arrived to the coins, replace it ina random cell!
					if (path.points.empty()) {
						multipleTargets.clear();
						numTargets = 3 + rand() % 5;
						for (int i = 0; i < numTargets; i++) {
							Vector2D coinPosition(-1, -1);
							while ((!isValidCell(coinPosition)) || (Vector2D::Distance(coinPosition, pix2cell(agents[0]->getPosition())) < 3))
								coinPosition = Vector2D((float)(rand() % num_cell_x), (float)(rand() % num_cell_y));

							agents[0]->setTarget(coinPosition);

							multipleTargets.push_back(graph[Vector2D(agents[0]->getTarget().x * CELL_SIZE + CELL_SIZE / 2, agents[0]->getTarget().y * CELL_SIZE + CELL_SIZE / 2)]);
						}

						Vector2D originPosition = pix2cell(Vector2D((float)(agents[0]->getPosition().x), (float)(agents[0]->getPosition().y)));
						Node* origin = graph[Vector2D(originPosition.x * CELL_SIZE + CELL_SIZE / 2, originPosition.y * CELL_SIZE + CELL_SIZE / 2)];

						startingTime = std::chrono::steady_clock::now();
						firstTimer = true;
						path.points.clear();
						path = Algorithm::MultipleTargets(multipleTargets, origin, &node_count, solution);
					}
				}
				else{
					Vector2D steering_force = agents[0]->Behavior()->Arrive(agents[0], currentTarget, path.ARRIVAL_DISTANCE, dtime);
					agents[0]->update(steering_force, dtime, event);
				}
				return;
			}
			coinDrawTarget = path.points[currentTargetIndex];
			currentTargetIndex++;
		}

		currentTarget = path.points[currentTargetIndex];
		
		if (abs(agents[0]->getPosition().x - currentTarget.x) > CELL_SIZE * 36 
			&& abs(agents[0]->getPosition().y - currentTarget.y) < CELL_SIZE) agents[0]->teleport();
		Vector2D steering_force = agents[0]->Behavior()->Seek(agents[0], currentTarget, dtime);
		agents[0]->update(steering_force, dtime, event);

		for (int i = 0; i < multipleTargets.size(); i++) {
			if (coinDrawTarget == multipleTargets[i]->position) multipleTargets.erase(multipleTargets.begin() + i);
		}
	}
	else
	{
		agents[0]->update(Vector2D(0, 0), dtime, event);
	}	
}

void ScenePathFindingCoin::draw(){
	drawMaze();

	for each(Node* coin in multipleTargets) {
		drawCoin(coin->position);
	}

	//Visual debug of the nodes, draw them all
	if (draw_nodes) {
		for each (pair<Vector2D, Node*> n in graph)
		{
			if (n.second->terrain == 1)
				draw_circle(TheApp::Instance()->getRenderer(), n.second->position.x, n.second->position.y, (CELL_SIZE / 2) - 7, 249, 167, 84, 200);
			else if (n.second->terrain == 10)
				draw_circle(TheApp::Instance()->getRenderer(), n.second->position.x, n.second->position.y, (CELL_SIZE / 2) - 7, 84, 188, 249, 200);
			else if (n.second->terrain == 20)
				draw_circle(TheApp::Instance()->getRenderer(), n.second->position.x, n.second->position.y, (CELL_SIZE / 2) - 7, 71, 222, 53, 200);
		}
	}
	if (draw_grid)
	{
		SDL_SetRenderDrawColor(TheApp::Instance()->getRenderer(), 255, 255, 255, 127);
		for (int i = 0; i < SRC_WIDTH; i += CELL_SIZE)
		{
			SDL_RenderDrawLine(TheApp::Instance()->getRenderer(), i, 0, i, SRC_HEIGHT);
		}
		for (int j = 0; j < SRC_HEIGHT; j = j += CELL_SIZE)
		{
			SDL_RenderDrawLine(TheApp::Instance()->getRenderer(), 0, j, SRC_WIDTH, j);
		}
	}

	
	if (draw_path) {
		for (int i = 0; i < (int)path.points.size(); i++)
		{
			draw_circle(TheApp::Instance()->getRenderer(), (int)(path.points[i].x), (int)(path.points[i].y), 15, 255, 255, 0, 255);
			if (i > 0)
				SDL_RenderDrawLine(TheApp::Instance()->getRenderer(), (int)(path.points[i - 1].x), (int)(path.points[i - 1].y), (int)(path.points[i].x), (int)(path.points[i].y));
		}
	}


	draw_circle(TheApp::Instance()->getRenderer(), (int)currentTarget.x, (int)currentTarget.y, 15, 255, 0, 0, 255);

	agents[0]->draw();
}

const char* ScenePathFindingCoin::getTitle()
{
	return "SDL Steering Behaviors :: PathFinding MultipleCoins";
}

void ScenePathFindingCoin::drawMaze()
{
	if (draw_grid)
	{
		SDL_SetRenderDrawColor(TheApp::Instance()->getRenderer(), 0, 0, 255, 255);
		for (unsigned int i = 0; i < maze_rects.size(); i++)
			SDL_RenderFillRect(TheApp::Instance()->getRenderer(), &maze_rects[i]);
	}
	else
	{
		SDL_RenderCopy(TheApp::Instance()->getRenderer(), background_texture, NULL, NULL);
	}
}

void ScenePathFindingCoin::drawCoin(const Vector2D position) {
	int offset = CELL_SIZE / 2;
	SDL_Rect dstrect = { (int)position.x - offset, (int)position.y - offset, CELL_SIZE, CELL_SIZE };
	SDL_RenderCopy(TheApp::Instance()->getRenderer(), coin_texture, NULL, &dstrect);
}

void ScenePathFindingCoin::initMaze()
{
	// Initialize a list of Rectagles describing the maze geometry (useful for collision avoidance)
	SDL_Rect rect = { 0, 0, 1280, 32 };
	maze_rects.push_back(rect);
	rect = { 608, 32, 64, 32 };
	maze_rects.push_back(rect);
	rect = { 0, 736, 1280, 32 };
	maze_rects.push_back(rect);
	rect = { 608, 512, 64, 224 };
	maze_rects.push_back(rect);
	rect = { 0,32,32,288 };
	maze_rects.push_back(rect);
	rect = { 0,416,32,320 };
	maze_rects.push_back(rect);
	rect = { 1248,32,32,288 };
	maze_rects.push_back(rect);
	rect = { 1248,416,32,320 };
	maze_rects.push_back(rect);
	rect = { 128,128,64,32 };
	maze_rects.push_back(rect);
	rect = { 288,128,96,32 };
	maze_rects.push_back(rect);
	rect = { 480,128,64,32 };
	maze_rects.push_back(rect);
	rect = { 736,128,64,32 };
	maze_rects.push_back(rect);
	rect = { 896,128,96,32 };
	maze_rects.push_back(rect);
	rect = { 1088,128,64,32 };
	maze_rects.push_back(rect);
	rect = { 128,256,64,32 };
	maze_rects.push_back(rect);
	rect = { 288,256,96,32 };
	maze_rects.push_back(rect);
	rect = { 480, 256, 320, 32 };
	maze_rects.push_back(rect);
	rect = { 608, 224, 64, 32 };
	maze_rects.push_back(rect);
	rect = { 896,256,96,32 };
	maze_rects.push_back(rect);
	rect = { 1088,256,64,32 };
	maze_rects.push_back(rect);
	rect = { 128,384,32,256 };
	maze_rects.push_back(rect);
	rect = { 160,512,352,32 };
	maze_rects.push_back(rect);
	rect = { 1120,384,32,256 };
	maze_rects.push_back(rect);
	rect = { 768,512,352,32 };
	maze_rects.push_back(rect);
	rect = { 256,640,32,96 };
	maze_rects.push_back(rect);
	rect = { 992,640,32,96 };
	maze_rects.push_back(rect);
	rect = { 384,544,32,96 };
	maze_rects.push_back(rect);
	rect = { 480,704,32,32 };
	maze_rects.push_back(rect);
	rect = { 768,704,32,32 };
	maze_rects.push_back(rect);
	rect = { 864,544,32,96 };
	maze_rects.push_back(rect);
	rect = { 320,288,32,128 };
	maze_rects.push_back(rect);
	rect = { 352,384,224,32 };
	maze_rects.push_back(rect);
	rect = { 704,384,224,32 };
	maze_rects.push_back(rect);
	rect = { 928,288,32,128 };
	maze_rects.push_back(rect);

	//Water rects
	rect = { 1120, 160, 32, 96 };
	maze_water.push_back(rect);
	rect = { 1120, 288, 32, 96 };
	maze_water.push_back(rect);
	rect = { 608, 64, 64, 160 };
	maze_water.push_back(rect);
	rect = { 512, 288, 256, 96 };
	maze_water.push_back(rect);
	rect = { 672, 512, 96, 64 };
	maze_water.push_back(rect);
	rect = { 288, 160, 32, 96 };
	maze_water.push_back(rect);
	rect = { 128, 640, 32, 96 };
	maze_water.push_back(rect);

	//Mud rects
	rect = { 128, 160, 32, 96 };
	maze_mud.push_back(rect);
	rect = { 384, 128, 96, 32 };
	maze_mud.push_back(rect);
	rect = { 224, 352, 96, 160 };
	maze_mud.push_back(rect);
	rect = { 384, 640, 64, 96 };
	maze_mud.push_back(rect);
	rect = { 928, 416, 32, 96 };
	maze_mud.push_back(rect);
	rect = { 992, 128, 96, 32 };
	maze_mud.push_back(rect);
	rect = { 1152, 448, 96, 64 };
	maze_mud.push_back(rect);

	// Initialize the terrain matrix (for each cell a zero value indicates it's a wall)

	// (1st) initialize all cells to 1 by default
	for (int i = 0; i < num_cell_x; i++)
	{
		vector<int> terrain_col(num_cell_y, 1);
		terrain.push_back(terrain_col);
	}
	// (2nd) set to zero all cells that belong to a wall
	int offset = CELL_SIZE / 2;
	for (int i = 0; i < num_cell_x; i++)
	{
		for (int j = 0; j < num_cell_y; j++)
		{
			Vector2D cell_center((float)(i*CELL_SIZE + offset), (float)(j*CELL_SIZE + offset));
			for (unsigned int b = 0; b < maze_rects.size(); b++)
			{
				if (Vector2DUtils::IsInsideRect(cell_center, (float)maze_rects[b].x, (float)maze_rects[b].y, (float)maze_rects[b].w, (float)maze_rects[b].h))
				{
					terrain[i][j] = 0;
					break;
				}
			}
			for (unsigned int b = 0; b < maze_water.size(); b++)
			{
				if (Vector2DUtils::IsInsideRect(cell_center, (float)maze_water[b].x, (float)maze_water[b].y, (float)maze_water[b].w, (float)maze_water[b].h))
				{
					terrain[i][j] = 10;
					break;
				}
			}
			for (unsigned int b = 0; b < maze_mud.size(); b++)
			{
				if (Vector2DUtils::IsInsideRect(cell_center, (float)maze_mud[b].x, (float)maze_mud[b].y, (float)maze_mud[b].w, (float)maze_mud[b].h))
				{
					terrain[i][j] = 20;
					break;
				}
			}

			//Creation of the nodes in the free cells
			if (terrain[i][j] != 0) {
				Node* n = new Node;
				n->position = cell_center;
				n->terrain = terrain[i][j];
				nodes.push_back(n);
			}
		}
	}

	//Adding the adjacent nodes to each one
	for each (Node* n in nodes)
	{
		for each (Node* n2 in nodes)
		{
			//The "bridge"
			if (n->position.x == CELL_SIZE / 2 && n2->position.x == SRC_WIDTH - (CELL_SIZE / 2)) {
				if (n->position.y == n2->position.y) {
					n->adyacents.push_back(n2);
					n2->adyacents.push_back(n);
				}
			}

			//---------------------------------

			if (n->position.x + CELL_SIZE == n2->position.x && n->position.y == n2->position.y) {
				n->adyacents.push_back(n2);
				continue;
			}
			if (n->position.x - CELL_SIZE == n2->position.x && n->position.y == n2->position.y) {
				n->adyacents.push_back(n2);
				continue;
			}
			if (n->position.x == n2->position.x && n->position.y + CELL_SIZE == n2->position.y) {
				n->adyacents.push_back(n2);
				continue;
			}
			if (n->position.x == n2->position.x && n->position.y - CELL_SIZE == n2->position.y) {
				n->adyacents.push_back(n2);
				continue;
			}
		}
	}

	for each (Node* n in nodes) {
		graph.insert(pair<Vector2D, Node*>(n->position, n));
	}

}

bool ScenePathFindingCoin::loadTextures(char* filename_bg, char* filename_coin)
{
	SDL_Surface *image = IMG_Load(filename_bg);
	if (!image) {
		cout << "IMG_Load: " << IMG_GetError() << endl;
		return false;
	}
	background_texture = SDL_CreateTextureFromSurface(TheApp::Instance()->getRenderer(), image);

	if (image)
		SDL_FreeSurface(image);

	image = IMG_Load(filename_coin);
	if (!image) {
		cout << "IMG_Load: " << IMG_GetError() << endl;
		return false;
	}
	coin_texture = SDL_CreateTextureFromSurface(TheApp::Instance()->getRenderer(), image);

	if (image)
		SDL_FreeSurface(image);

	return true;
}

Vector2D ScenePathFindingCoin::cell2pix(Vector2D cell)
{
	int offset = CELL_SIZE / 2;
	return Vector2D(cell.x*CELL_SIZE + offset, cell.y*CELL_SIZE + offset);
}

Vector2D ScenePathFindingCoin::pix2cell(Vector2D pix)
{
	return Vector2D((float)((int)pix.x / CELL_SIZE), (float)((int)pix.y / CELL_SIZE));
}

bool ScenePathFindingCoin::isValidCell(Vector2D cell)
{
	if ((cell.x < 0) || (cell.y < 0) || (cell.x >= terrain.size()) || (cell.y >= terrain[0].size()))
		return false;
	return !(terrain[(unsigned int)cell.x][(unsigned int)cell.y] == 0);
}