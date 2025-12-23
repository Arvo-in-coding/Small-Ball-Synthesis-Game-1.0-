#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "Ball.h"

class Game {
public:
	Game();
	void run();

private:
	void processEvents();
	void update(sf::Time deltaTime);
	void render();
	void loadResources();
	void spawnBall(float x, float y, int level);
	void checkCollisions();
	bool isSupported(size_t idx) const;
	void resetGame();

private:
	sf::RenderWindow window;
	std::vector<Ball> balls;
	sf::Texture textures[12];
	sf::Color colors[12];

	// UI / 游戏状态
	sf::Font font;
	sf::Text scoreText;
	int score = 0;

	// 游戏限制
	const size_t MAX_BALLS = 200;
	// 控制生成：当一个或多个球未稳定时禁止产生新的球
	bool spawnLocked = false;

	// 当前选择生成的等级（1/2/3），3 级需要解锁
	int currentSpawnLevel = 1;
	const int LEVEL3_UNLOCK_SCORE = 1000; // 解锁第3级所需分数，可调整
	// 现在始终随机生成 1/2/3；第3级受解锁分数限制
	bool random23Mode = true; // 保留字段以兼容旧逻辑但默认开启（实际我们会随机 1-3）

	// 最大等级（现在扩展为 10 级）
	const int MAX_LEVEL = 10;

	// 下一个将要生成的等级（用于 UI 预览）
	int nextSpawnLevel = 1;
	void pickNextSpawnLevel();

	// 生命线（虚线）Y 坐标（相对于窗口顶部）
	float lifelineY = 240.f; // 将生命线下移靠近中部/下方，使更容易触发 game over
	bool gameOver = false;
	bool gameWin = false;
	const int LEVEL_WIN = MAX_LEVEL; // 合成到该等级视为胜利（现在为 10 级）

	// 胜利界面文本
	sf::Text winText;

	// 游戏结束界面元素
	sf::RectangleShape againButton;
	sf::Text againText;

	// 容器边界（留白 margin）
	float leftMargin = 20.f;
	float rightMargin = 20.f;
};