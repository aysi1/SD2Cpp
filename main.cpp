#include <SDL.h>
#include <SDL_image.h>
#ifdef WIN32
#include <Windows.h>
#endif
#include <iostream>
#include <vector>
#include <cmath>
#include <ctime>

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

class Triangle {
private:
	SDL_Point Origin;
	short Gamma;
	int Length;
	SDL_Color Color;
	std::vector<SDL_Point> Points;
public:
	Triangle() {
		Gamma = 0;
		Length = 40;
		Color = { 0, 0, 255, 255};
		Origin = {20, 20};
		Update();
	}
	void Update() {
		if (!Points.empty()) Points.clear();
		int x, y;
		int a = Origin.x - (Length / 2), b = Origin.y - (Length / 2);
		switch (Gamma) {
		case 0:
			for (y = b; y <= b + Length; y++) {
				for (x = a; x <= y - b + a; x++)
					Points.push_back({ x, y });
			}
			break;
		case 1:
			for (y = b; y <= b + Length; y++) {
				for (x = a + Length - y + b; x <= a + Length; x++)
					Points.push_back({ x, y });
			}
			break;
		case 2:
			for (y = b; y <= b + Length; y++) {
				for (x = y - b + a; x <= a + Length; x++)
					Points.push_back({ x, y });
			}
			break;
		case 3:
			for (y = b; y <= b + Length; y++) {
				for (x = a; x <= a + Length - y + b; x++)
					Points.push_back({ x, y });
			}
			break;
		}
	}
	void setLength(int l) {
		Length = l;
		Update();
	}
	void setOrigin(SDL_Point O) {
		Origin = O;
		Update();
	}
	void setGamma(int G) {
		Gamma = G;
		Update();
	}
	void setColor(SDL_Color C) {
		Color = C;
	}
	int getLength() const { return Length; };
	SDL_Point getOrigin() const { return Origin; }
	int getGamma() const { return Gamma; };
	SDL_Color getColor() const { return Color; }
	int contain(SDL_Point Point) {
		int x = Point.x, y = Point.y;
		int a = Origin.x - (Length / 2), b = Origin.y - (Length / 2);
		if (x >= a && x <= a + Length && y >= b && y <= b + Length) return true;
		return false;
	}
	void draw() {
		SDL_SetRenderDrawColor(renderer, Color.r, Color.g, Color.b, Color.a);
		SDL_RenderDrawPoints(renderer, Points.data(), Points.size());
	}
};

class Texture {
private:
	SDL_Surface *Surface;
	SDL_Texture *texture;
	int x, y, w, h;
	SDL_Rect rect;
	SDL_Point center;
	double angle;
	float sx, sy;
public:
	bool ok;
	Texture(const std::string path) {
		Surface = NULL;
		texture = NULL;
		ok = true;
		Surface = IMG_Load(path.c_str());
		if (Surface == NULL) ok = false;
		if (ok) {
			texture = SDL_CreateTextureFromSurface(renderer, Surface);
			w = Surface->w;
			h = Surface->h;
			sx = 1;
			sy = 1;
			angle = 0;
			Update();
		}
	}
	void setRotation(double a) {
		angle = a;
	}
	void setScale(double _sx, double _sy) {
		sx = _sx;
		sy = _sy;
		Update();
	}
	void setPosition(int _x, int _y) {
		x = _x;
		y = _y;
		Update();
	}
	void setPosition(SDL_Point Pos) {
		x = Pos.x;
		y = Pos.y;
		Update();
	}
	SDL_Point getPosition() const { return {x, y}; }
	SDL_Point getSize() const { return{int(w * sx), int(h * sy)}; }
	void Update() {
		rect = {x, y, (int)(w * sx), (int) (h*sy)};
		center = { rect.w / 2, rect.h / 2 };
	}
	void draw() {
		if (ok) SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, &center, SDL_FLIP_NONE);
	}
	~Texture() {
		if (Surface != NULL) SDL_FreeSurface(Surface);
		if (texture != NULL) SDL_DestroyTexture(texture);
	}
};

class Ball {
private:
	double x, y;
	double Speed;
	int Direction; /* UP : 1 DOWN : 3 RIGHT : 2 LEFT : 0 */
	SDL_Color Color;
public:
	static const int UP = 1;
	static const int DOWN = 3;
	static const int RIGHT = 2;
	static const int LEFT = 0;
	Ball() {
		x = 0;
		y = 0;
		Speed = 1;
		Direction = 0;
		Color = { 0, 255, 255, 255 };
	}
	void setPosition(double _x, double _y) { x = _x; y = _y; }
	void setSpeed(double S) { Speed = S; }
	void setDirection(int D) { Direction = D; }
	void setColor(SDL_Color C) { Color = C; }
	SDL_Point getPosition() const { return {(int) x, (int) y}; }
	double getSpeed() const { return Speed; }
	int getDirection() const { return Direction; }
	SDL_Color getColor() const { return Color; }
	void draw(bool move = false) {
		if (move) {
			switch (Direction) {
			case 0:
				x -= Speed;
				break;
			case 1:
				y -= Speed;
				break;
			case 2:
				x += Speed;
				break;
			case 3:
				y += Speed;
				break;
			}
		}
		SDL_Point *P = new SDL_Point[5];
		int _x = (int)x, _y = (int)y;
		P[0] = { _x, _y };
		P[1] = { _x, _y - 1 };
		P[2] = { _x, _y + 1};
		P[3] = { _x - 1, _y };
		P[4] = { _x + 1, _y};
		SDL_SetRenderDrawColor(renderer, Color.r, Color.g, Color.b, Color.a);
		SDL_RenderDrawPoints(renderer, P, 5);
	}
};

int randChoice(int a, int b) {
	if (rand() % 2) return a;
	return b;
}

class Game {
private:
	std::vector<Triangle> *Artillery;
	Ball *Rocket;
	std::vector<Triangle> *Walls;
	bool stopRocket;
	int initDirection;
	SDL_Point ArtilleryPos;
	Texture *TargetTex, *Win, *Lose, *Replay, *Exit;
public:
	bool _Exit, stop, playerWin;
	Game() {
		Artillery = new std::vector<Triangle>;
		Rocket = new Ball();
		Walls = new std::vector<Triangle>;
		TargetTex = new Texture("res\\target.png");
		Win = new Texture("res\\winner.png");
		Lose = new Texture("res\\loser.png");
		Replay = new Texture("res\\try.png");
		Exit = new Texture("res\\Exit.png");
		Win->setScale(0.25, 0.25);
		Lose->setScale(0.25, 0.25);
		Win->setPosition({ 400 - Win->getSize().x / 2, 200 - Win->getSize().y / 2 });
		Lose->setPosition({ 400 - Lose->getSize().x / 2, 200 - Lose->getSize().y / 2 });
		Replay->setPosition(300, 336);
		Exit->setPosition(436, 336);
		_Exit = false;
		stop = false;
	}
	void loadWinnerBg(SDL_Event &e) {
		Win->draw();
		Replay->draw();
		Exit->draw();
		if (e.motion.y >= 336 && e.motion.y <= 400) {
			if (e.motion.x >= 300 && e.motion.x <= 364) {
				if (e.type == SDL_MOUSEBUTTONDOWN) {
					stop = false;
					init();
				}
			}
			else if (e.motion.x >= 372 && e.motion.y <= 436) {
				if (e.type == SDL_MOUSEBUTTONDOWN) {
					_Exit = true;
					init();
				}
			}
		}
	}
	void loadLoserBg(SDL_Event &e) {
		Lose->draw();
		Replay->draw();
		Exit->draw();
		if (e.motion.y >= 336 && e.motion.y <= 400) {
			if (e.motion.x >= 300 && e.motion.x <= 364) {
				if (e.type == SDL_MOUSEBUTTONDOWN) {
					stop = false;
					init();
				}
			}
			else if (e.motion.x >= 372 && e.motion.y <= 436) {
				if (e.type == SDL_MOUSEBUTTONDOWN) {
					_Exit = true;
					init();
				}
			}
		}
	}
	void init() {
		stopRocket = true;
		Rocket->setColor({ 0, 0, 0, 255 });
		Rocket->setSpeed(2);
		initDirection = Ball::LEFT;
		ArtilleryPos = { 400, 200 };
		if (!Artillery->empty()) Artillery->clear();
		for (int i = 0; i < 6; i++) Artillery->push_back(Triangle());
		setupArtillery(ArtilleryPos, 10, initDirection);
		Rocket->setPosition(ArtilleryPos.x, ArtilleryPos.y);
		generateWalls();
	}

	void generateWalls() {
		if (!Walls->empty()) Walls->clear();
		int num = 4 + rand() % 8;
		Walls->push_back(Triangle());
		Walls->at(0).setGamma(rand() % 4);
		Walls->at(0).setLength(20);
		int s = (int)Rocket->getSpeed();
		switch (initDirection) {
		case Ball::RIGHT:
			Walls->at(0).setOrigin({ 500 + s * (rand() % (200 / s)),
				ArtilleryPos.y });
			break;
		case Ball::LEFT:
			Walls->at(0).setOrigin({ 100 + s * (rand() % (200 / s)),
				ArtilleryPos.y });
			break;
		case Ball::UP:
			Walls->at(0).setOrigin({ ArtilleryPos.x,
				40 + s * (rand() % (100 / s)) });
			break;
		case Ball::DOWN:
			Walls->at(0).setOrigin({ ArtilleryPos.x,
				240 + s * (rand() % (100 / s)) });
			break;
		}
		for (int i = 1; i < num; i++) {
			Walls->push_back(Triangle());
			Walls->at(i).setGamma(rand() % 4);
			Walls->at(i).setLength(20);
			int x = Walls->at(i - 1).getOrigin().x, y = Walls->at(i - 1).getOrigin().y;
			s *= 10;
			int a = (x - 30) / s, b = (760 - x) / s, c = (360 - y) / s, d = (y - 30) / s;
			s /= 10;
			if (a == 0) a++;
			if (b == 0) b++;
			if (c == 0) c++;
			if (d == 0) d++;
			if (i == 1) {
				if (ArtilleryPos.x == x) {
					Walls->at(i).setOrigin({ randChoice(x - 20 - 10 * s * (rand() % a), x + 20 + 10 * s * (rand() % b)), y });
				}
				else {
					Walls->at(i).setOrigin({ x, randChoice(y - 20 - 10 * s * (rand() % d), y + 20 + 10 * s * (rand() % c)) });
				}
			}
			else {
				if (Walls->at(i - 2).getOrigin().x == x) {
					Walls->at(i).setOrigin({ randChoice(x - 20 - 10 * s * (rand() % a), x + 20 + 10 * s * (rand() % b)), y });
				}
				else {
					Walls->at(i).setOrigin({ x, randChoice(y - 20 - 10 * s * (rand() % d), y + 20 + 10 * s * (rand() % c)) });
				}
			}
		}
		int x = Walls->at(Walls->size() - 1).getOrigin().x;
		int y = Walls->at(Walls->size() - 1).getOrigin().y;
		TargetTex->setPosition(x - TargetTex->getSize().x / 2, y - TargetTex->getSize().y / 2);
	}
	void play() {
		if (stopRocket) {
			Rocket->setPosition(ArtilleryPos.x, ArtilleryPos.y);
			stopRocket = false;
			Rocket->setDirection(initDirection);
		}
	}

	/* direction : 0 => left 1 => up 2 => right 3 => down */
	void setupArtillery(SDL_Point Position, int size, int direction, SDL_Color color = { 0, 255, 255, 255 }) {
		for (int i = 0; i < 6; i++) {
			Artillery->at(i).setLength(size);
			Artillery->at(i).setColor(color);
			switch (direction) {
			case 0:
				if (i % 2 == 0) Artillery->at(i).setGamma(1);
				else Artillery->at(i).setGamma(2);
				Artillery->at(i).setOrigin({ Position.x - size / 2 + (i / 2)*(size / 2), Position.y + ((i % 2) ? size / 2 : -size / 2) });
				break;
			case 1:
				if (i % 2 == 0) Artillery->at(i).setGamma(1);
				else Artillery->at(i).setGamma(0);
				Artillery->at(i).setOrigin({ Position.x + ((i % 2) ? size / 2 : -size / 2) , Position.y - size / 2 + (i / 2)*(size / 2) });
				break;
			case 2:
				if (i % 2 == 0) Artillery->at(i).setGamma(0);
				else Artillery->at(i).setGamma(3);
				Artillery->at(i).setOrigin({ Position.x - size / 2 + (i / 2)*(size / 2), Position.y + ((i % 2) ? size / 2 : -size / 2) });
				break;
			case 3:
				if (i % 2 == 0) Artillery->at(i).setGamma(2);
				else Artillery->at(i).setGamma(3);
				Artillery->at(i).setOrigin({ Position.x + ((i % 2) ? size / 2 : -size / 2) , Position.y - size / 2 - (i / 2)*(size / 2) });
				break;
			}
		}
	}
	void draw() {
		Rocket->draw(!stopRocket);
		std::vector<Triangle>::iterator it;
		int x, y;
		for (it = Artillery->begin(); it != Artillery->end(); it++)
			it->draw();
		for (it = Walls->begin(); it != Walls->end() - 1; it++) {
			it->draw();

			//
			x = (int)Rocket->getPosition().x, y = (int)Rocket->getPosition().y;
			if (x == it->getOrigin().x && y == it->getOrigin().y) {
				switch (it->getGamma()) {
				case 0:
					if (Rocket->getDirection() == Ball::DOWN) Rocket->setDirection(Ball::RIGHT);
					else if (Rocket->getDirection() == Ball::LEFT) Rocket->setDirection(Ball::UP);
					break;
				case 1:
					if (Rocket->getDirection() == Ball::RIGHT) Rocket->setDirection(Ball::UP);
					else if (Rocket->getDirection() == Ball::DOWN) Rocket->setDirection(Ball::LEFT);
					break;
				case 2:
					if (Rocket->getDirection() == Ball::RIGHT) Rocket->setDirection(Ball::DOWN);
					else if (Rocket->getDirection() == Ball::UP) Rocket->setDirection(Ball::LEFT);
					break;
				case 3:
					if (Rocket->getDirection() == Ball::LEFT) Rocket->setDirection(Ball::DOWN);
					else if (Rocket->getDirection() == Ball::UP) Rocket->setDirection(Ball::RIGHT);
				}
			}
			else if (x == it->getOrigin().x - it->getLength() / 2 && y == it->getOrigin().y) {
				if (it->getGamma() == 0 || it->getGamma() == 3) Rocket->setDirection(0);
			}
			else if (x == it->getOrigin().x + it->getLength() / 2 && y == it->getOrigin().y) {
				if (it->getGamma() == 1 || it->getGamma() == 2) Rocket->setDirection(2);
			}
			else if (x == it->getOrigin().x && y == it->getOrigin().y - it->getLength() / 2) {
				if (it->getGamma() == 2 || it->getGamma() == 3) Rocket->setDirection(1);
			}
			else if (x == it->getOrigin().x && y == it->getOrigin().y + it->getLength() / 2) {
				if (it->getGamma() == 0 || it->getGamma() == 1) Rocket->setDirection(3);
			}
		}
		TargetTex->draw();
		if (it->contain(Rocket->getPosition())) {
			stopRocket = true;
			stop = true;
			playerWin = true;
		}
		else if (Rocket->getPosition().x <= 0 || Rocket->getPosition().x >= 800
			|| Rocket->getPosition().y <= 0 || Rocket->getPosition().y >= 400) {
			stopRocket = true;
			stop = true;
			playerWin = false;
		}
	}

	void handleMouseEvent(SDL_Event &e) {
		if (e.type == SDL_MOUSEBUTTONDOWN) {
			for (std::vector<Triangle>::iterator i = Walls->begin(); i != Walls->end(); i++) {
				if (i->contain({ e.motion.x, e.motion.y })) i->setGamma((i->getGamma() + 1)%4);
			}
		}
	}
	~Game() {
		delete Artillery;
		delete Rocket;
		delete Walls;
		delete TargetTex;
		delete Win;
		delete Lose;
		delete Replay;
		delete Exit;
	}
};

int init() {
	window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, 800, 400, SDL_WINDOW_RESIZABLE);
	if (!window) return -1;
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) return -2;
	if (IMG_Init(!IMG_INIT_PNG) & IMG_INIT_PNG) return -3;
	return 0;
}


#ifdef WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
int main(int argc, char *argv[])
#endif
{
	if (init() < 0) {
		std::cout << SDL_GetError() << std::endl;
		return 0;
	}

	SDL_Event event;
	srand(time(NULL));

	Game *game = new Game();
	game->init();
	while (!game->_Exit) {
		while (SDL_PollEvent(&event)) {
			game->handleMouseEvent(event);
			if (event.type == SDL_QUIT) game->_Exit = true;
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_SPACE: game->play(); break;
				case SDLK_s: game->init(); break;
				}
			}
		}
		SDL_Delay(1000 / 60); // 60 fps
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderClear(renderer);
		if (!game->stop) game->draw();
		else {
			if (game->playerWin) game->loadWinnerBg(event);
			else game->loadLoserBg(event);
		}
		SDL_RenderPresent(renderer);
	}
	delete game;
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
	return 0;
}