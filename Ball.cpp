#include "Ball.h"
#include <cmath>

// 根据等级决定大小（像素半径）
static float getRadiusByLevel(int level)
{
	// 放大基准半径与每级增量，让球体积更大，便于更容易触达生命线
	return 18.f + level * 8.f; // level 0 -> 18, 每级增大 8px
}

Ball::Ball(float x, float y, int lvl, const sf::Color& color, const sf::Texture* texture)
	: level(lvl)
{
	isDead = false;
	velocity = sf::Vector2f(0.f, 0.f);

	radius = getRadiusByLevel(level);
	circle.setRadius(radius);
	if (texture) {
		circle.setTexture(texture);
		circle.setFillColor(sf::Color::White); // 使用纹理时设为白色以免混色
	} else {
		circle.setFillColor(color);
	}
	circle.setOrigin(radius, radius);
	circle.setPosition(x, y);

	// 质量与面积相关（近似）：质量与半径的平方成正比
	mass = std::max(0.1f, radius * radius * 0.001f);
}

void Ball::update(float deltaTime)
{
	// 记录上一帧位置
	prevPosition = circle.getPosition();

	// 增加生存时间
	age += deltaTime;

	// 重力（Y 方向）
	velocity.y += GRAVITY * deltaTime;

	// 移动：水平与竖直
	circle.move(velocity.x * deltaTime, velocity.y * deltaTime);

	// 地面判定与弹性处理
	sf::Vector2f pos = circle.getPosition();
	if (pos.y + radius >= FLOOR_Y) {
		pos.y = FLOOR_Y - radius;
		circle.setPosition(pos);

		if (std::abs(velocity.y) > 0.0f) {
			velocity.y = -velocity.y * RESTITUTION;
		}

		if (std::abs(velocity.y) < 30.f) {
			velocity.y = 0.f;
			isOnGround = true;
		} else {
			isOnGround = false;
		}

		if (isOnGround) {
			float sign = (velocity.x >= 0.f) ? 1.f : -1.f;
			float dec = FRICTION * deltaTime; // 速度减少量
			if (std::abs(velocity.x) <= dec) velocity.x = 0.f;
			else velocity.x -= sign * dec;
		}
	} else {
		isOnGround = false;
	}

	if (std::abs(velocity.x) < 0.01f) velocity.x = 0.f;
}

void Ball::render(sf::RenderWindow& window)
{
	window.draw(circle);
}

sf::Vector2f Ball::getPosition() const { return circle.getPosition(); }
float Ball::getRadius() const { return radius; }
int Ball::getLevel() const { return level; }