#pragma once

#include <SFML/Graphics.hpp>

class Ball {
public:
	Ball(float x, float y, int level, const sf::Color& color, const sf::Texture* texture = nullptr);

	void update(float deltaTime);
	void render(sf::RenderWindow& window);

	sf::Vector2f getPosition() const;
	float getRadius() const;
	int getLevel() const;

	// 这里将 isDead 暴露为公有以兼容现有代码访问（简单方案）
	bool isDead = false;
	bool isOnGround = false;
	// 球的存在时间（秒），用于避免生成时立即触发生命线判定
	float age = 0.f;
	// 记录上一帧的位置用于检测是否被向上推过生命线
	sf::Vector2f prevPosition = {0.f, 0.f};
	// 记录球是否在生成时已位于生命线上方，以及在生命线上方持续的时间
	bool wasSpawnedAboveLine = false;
	float timeAboveLine = 0.f;

private:
	sf::CircleShape circle;
	int level = 0;
	float radius = 0.f;
	sf::Vector2f velocity = {0.f, 0.f};

public:
	void setPosition(const sf::Vector2f& p) { circle.setPosition(p); }

	// 反弹系数（0..1），越小损失越大（设置较小以降低回弹高度）
	static constexpr float RESTITUTION = 0.15f;
	// 地面摩擦系数（每秒减速比例），用于滚动时减速
	static constexpr float FRICTION = 4.0f; // m/s^2 级别的摩擦减速度
	// 质量（影响碰撞响应），可基于半径变化；默认 1.0
	float mass = 1.0f;

public:
	// 允许外部设置初始速度（spawn 时使用）
	void setVelocity(const sf::Vector2f& v) { velocity = v; }
	sf::Vector2f getVelocity() const { return velocity; }
	void setScale(float s) { circle.setScale(s, s); }
	sf::Vector2f getPrevPosition() const { return prevPosition; }
	float getAge() const { return age; }

	static constexpr float GRAVITY = 980.0f;
	static constexpr float FLOOR_Y = 800.0f;
};