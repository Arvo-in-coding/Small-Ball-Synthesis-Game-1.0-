#include "Game.h"
#include <iostream>
#include <cmath>
#include <string>
#include <algorithm>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <streambuf>
#include <stack>

// 构造函数
Game::Game()
    : window(sf::VideoMode(480, 800), "Synthetic SHU")
{
    window.setFramerateLimit(60);
    loadResources();
}

// 加载资源
void Game::loadResources()
{
    // 颜色调色板（第 1-3 级为明确可区分的颜色）
    sf::Color palette[12] = {
        sf::Color(180,180,180), // 0
        sf::Color(220, 80, 80),  // 1 - red
        sf::Color(80, 180, 90),  // 2 - green
        sf::Color(70, 130, 230), // 3 - blue
        sf::Color(230, 190, 60), // 4
        sf::Color(190, 90, 180), // 5
        sf::Color(90, 190, 200), // 6
        sf::Color(200, 120, 60), // 7
        sf::Color(150, 150, 220),// 8
        sf::Color(220, 120, 120),// 9
        sf::Color(120, 220, 180),// 10
        sf::Color(200,200,200)   // 11
    };

    for (int i = 0; i < 12; ++i) {
        colors[i] = palette[i];
    }
    // 尝试从 assets 加载纹理 (1.png ... 11.png)
    for (int i = 1; i < 12; ++i) {
        std::string path = "assets/" + std::to_string(i) + ".png";
        // 如果加载成功，textures[i] 将包含图像；如果失败，保持为空
        if (textures[i].loadFromFile(path)) {
            textures[i].setSmooth(true); // 开启平滑更美观
        }
    }

    // 尝试加载系统字体以显示分数；在 macOS 上常见路径为 /Library/Fonts/Arial.ttf
    // 为避免在字体加载失败时 SFML 向 stderr 打印错误信息，我们在尝试加载时暂时屏蔽 sf::err()
    struct NullBuf : public std::streambuf { int overflow(int c) override { return c; } } nullBuf;
    std::streambuf* oldBuf = sf::err().rdbuf(&nullBuf);
    bool fontLoaded = false;
    const char* candidates[] = {"./resources/arial.ttf", "/Library/Fonts/Arial.ttf", "/System/Library/Fonts/Supplemental/Arial.ttf"};
    for (auto path : candidates) {
        if (font.loadFromFile(path)) {
            fontLoaded = true;
            break;
        }
    }
    // 恢复 sf::err() 的缓冲区
    sf::err().rdbuf(oldBuf);

    if (fontLoaded) {
        scoreText.setFont(font);
        scoreText.setCharacterSize(20);
        scoreText.setFillColor(sf::Color::Black);
        scoreText.setPosition(8.f, 8.f);
        scoreText.setString("Score: 0");
    }

    // 随机种子（用于 spawn 的随机初速度）
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // 初始化 Again 按钮（位置将在 run/render 时基于窗口大小调整）
    againButton.setSize(sf::Vector2f(140.f, 44.f));
    againButton.setFillColor(sf::Color(200, 50, 50));
    againButton.setOutlineColor(sf::Color::Black);
    againButton.setOutlineThickness(2.f);
    if (fontLoaded) {
        againText.setFont(font);
        againText.setCharacterSize(20);
        againText.setFillColor(sf::Color::White);
        againText.setString("Again");
    }

    // 选择初始的下一个生成等级（用于 UI 预览）
    pickNextSpawnLevel();
}

// 随机选择下一次要生成的球的等级（遵循已有的 1..3 随机并考虑第3级解锁）
void Game::pickNextSpawnLevel()
{
    // 默认行为：随机 1..3，若 3 未解锁则退为 1/2
    int pick = (std::rand() % 3) + 1; // 1..3
    if (pick == 3 && score < LEVEL3_UNLOCK_SCORE) {
        pick = (std::rand() % 2) + 1; // 1 or 2
    }
    nextSpawnLevel = pick;
}

// 生成新球
void Game::spawnBall(float x, float y, int level)
{
    if (level < 1) level = 1;
    if (level > MAX_LEVEL) level = MAX_LEVEL;
    if (balls.size() >= MAX_BALLS) return; // 限制球的总数

    // 保证在容器内部横坐标
    float winW = static_cast<float>(window.getSize().x);
    float minX = leftMargin + 8.f;
    float maxX = winW - rightMargin - 8.f;
    if (x < minX) x = minX;
    if (x > maxX) x = maxX;

    // 先创建一个 probe 以获取半径
    const sf::Texture* pTex = (textures[level].getSize().x > 0) ? &textures[level] : nullptr;
    Ball probe(x, y, level, colors[level], pTex);
    float r = probe.getRadius();

    // 尝试不同横向偏移（左右交替）寻找不重叠位置
    float chosenX = x;
    float chosenY = y;
    bool placed = false;
    const int maxOffsetSteps = 24;
    for (int step = 0; step < maxOffsetSteps && !placed; ++step) {
        int k = step/2;
        int side = (step % 2 == 0) ? 1 : -1;
        float offset = (k + 0.0f) * (r * 0.85f + 6.f) * side;
        float nx = x + offset;
        if (nx < minX + r) nx = minX + r;
        if (nx > maxX - r) nx = maxX - r;

        bool ok = true;
        for (auto &b : balls) {
            float dx = nx - b.getPosition().x;
            float dy = y - b.getPosition().y;
            float dist2 = dx*dx + dy*dy;
            float minDist = (r + b.getRadius()) * 0.82f; // 允许略紧密
            if (dist2 < minDist * minDist) { ok = false; break; }
        }
        if (ok) { chosenX = nx; placed = true; break; }
    }

    // 如果横向没有合适位置，尝试向上抬高更多步以便堆叠
    if (!placed) {
        const int upAttempts = 30;
        for (int u = 1; u <= upAttempts && !placed; ++u) {
            float ny = y - u * (r * 0.9f + 4.f);
            if (ny < r + 8.f) break;
            for (int step = 0; step < maxOffsetSteps && !placed; ++step) {
                int k = step/2;
                int side = (step % 2 == 0) ? 1 : -1;
                float offset = (k + 0.0f) * (r * 0.85f + 6.f) * side;
                float nx = x + offset;
                if (nx < minX + r) nx = minX + r;
                if (nx > maxX - r) nx = maxX - r;
                bool ok = true;
                for (auto &b : balls) {
                    float dx = nx - b.getPosition().x;
                    float dy = ny - b.getPosition().y;
                    float dist2 = dx*dx + dy*dy;
                    float minDist = (r + b.getRadius()) * 0.82f;
                    if (dist2 < minDist * minDist) { ok = false; break; }
                }
                if (ok) { chosenX = nx; chosenY = ny; placed = true; break; }
            }
        }
    }

    if (!placed) {
        // 如果确实放不下，则尝试在窗口顶部少许位置生成（让它自然落下）
        float topY = r + 12.f;
        chosenX = std::min(std::max(x, minX + r), maxX - r);
        chosenY = topY;
    }

    const sf::Texture* bTex = (textures[level].getSize().x > 0) ? &textures[level] : nullptr;
    balls.emplace_back(chosenX, chosenY, level, colors[level], bTex);
    // 给一点初速度避免完全垂直停滞
    float vy = -90.f + (std::rand() % 80 - 40);
    float vx = (std::rand() % 80 - 40) * 0.4f;
    balls.back().setVelocity(sf::Vector2f(vx, vy));
    // 初始化生命线相关字段，避免 spawn 时被立即判死
    balls.back().prevPosition = balls.back().getPosition();
    balls.back().timeAboveLine = 0.f;
    balls.back().wasSpawnedAboveLine = (chosenY - balls.back().getRadius() <= lifelineY);
}

// 游戏主循环
void Game::run()
{
    sf::Clock clock;
    while (window.isOpen()) {
        sf::Time deltaTime = clock.restart();
        processEvents();
        update(deltaTime);
        render();
    }
}

// 处理事件（鼠标点击）
void Game::processEvents()
{
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();

        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2i pos = sf::Mouse::getPosition(window);
            if (!gameOver) {
                // 使用已经预选的 nextSpawnLevel 来生成球，然后再选一个新的 nextSpawnLevel
                int pick = nextSpawnLevel;
                // 作为保险，如果 pick 超过 MAX_LEVEL 或小于 1，则修正
                if (pick < 1) pick = 1;
                if (pick > MAX_LEVEL) pick = MAX_LEVEL;
                spawnBall(static_cast<float>(pos.x), static_cast<float>(pos.y), pick);
                // 生成后立刻选择下一个预览
                pickNextSpawnLevel();
            } else {
                // 如果处于 gameOver，则检查 Again 按钮点击
                sf::Vector2f mp(static_cast<float>(pos.x), static_cast<float>(pos.y));
                if (againButton.getGlobalBounds().contains(mp)) {
                    resetGame();
                }
            }
        }

        // 现在始终以随机方式生成（1-3），因此不暴露等级选择按键
        if (event.type == sf::Event::KeyPressed) {
            // 允许按 Esc 关闭窗口
            if (event.key.code == sf::Keyboard::Escape) window.close();
        }
    }
}

// 更新逻辑
void Game::update(sf::Time deltaTime)
{
    float dt = deltaTime.asSeconds();
    if (!gameOver) {
        for (auto& ball : balls)
            ball.update(dt);
    }

    // 先处理碰撞（碰撞可能会产生新球）
    checkCollisions();

    // 移除已经死亡的球
    balls.erase(std::remove_if(balls.begin(), balls.end(), [](const Ball& b) { return b.isDead; }), balls.end());

    // 更新分数字符串（如果 font 可用）
    if (!font.getInfo().family.empty()) {
        std::ostringstream oss;
        oss << "Score: " << score;
        scoreText.setString(oss.str());
    }

    // 如果所有球都在地面并速度接近 0，则解锁生成
    bool anyMoving = false;
    for (auto& b : balls) {
        if (!b.isOnGround) { anyMoving = true; break; }
        // 速度接近 0
        if (std::abs(b.getVelocity().y) > 1.f) { anyMoving = true; break; }
    }
    if (!anyMoving) spawnLocked = false;

    // 检查生命线（只在非 gameOver 时）
    if (!gameOver) {
        for (auto &b : balls) {
            float prevTop = b.getPrevPosition().y - b.getRadius();
            float curTop = b.getPosition().y - b.getRadius();
            // 如果上一帧在生命线下而当前帧在生命线上/线上方 -> 被向上推过，立即判死
            if (prevTop > lifelineY && curTop <= lifelineY) { gameOver = true; break; }

            // 如果当前在/高于生命线，则开始积累在生命线之上的时间
            if (curTop <= lifelineY) {
                b.timeAboveLine += dt;
                // 只有当在生命线上停留超过阈值才判定死亡（避免快速连续生成导致的立即死亡）
                const float ABOVE_THRESHOLD = 1.5f;
                if (b.timeAboveLine >= ABOVE_THRESHOLD) { gameOver = true; break; }
            } else {
                // 在线下则重置计时
                b.timeAboveLine = 0.f;
            }
        }
    }
}

// 简单碰撞检测：如果两个球重叠，则将其中一个标记为死亡（这是占位逻辑，便于编译和演示）
void Game::checkCollisions()
{
    // 为避免在迭代中直接修改 balls，先收集要生成的新球请求
    struct SpawnReq { float x; float y; int level; sf::Vector2f vel; };
    std::vector<SpawnReq> spawns;

    // 优先合并：遍历所有球，如果接触且等级相同则立即合并
    // 但仅当两球都被“支撑”（supported）时才允许合并——即接触地面或通过一系列接触链条接触地面
    for (size_t i = 0; i < balls.size(); ++i) {
        for (size_t j = i + 1; j < balls.size(); ++j) {
            if (balls[i].isDead || balls[j].isDead) continue;
            if (balls[i].getLevel() != balls[j].getLevel()) continue;
            // 只允许在“被支撑”的情况下合并
            if (!isSupported(i) || !isSupported(j)) continue;
            sf::Vector2f p1 = balls[i].getPosition();
            sf::Vector2f p2 = balls[j].getPosition();
            float dx = p1.x - p2.x;
            float dy = p1.y - p2.y;
            float dist2 = dx*dx + dy*dy;
            float rsum = balls[i].getRadius() + balls[j].getRadius();
            if (dist2 <= rsum * rsum) {
                int lvl = balls[i].getLevel();
                // 仅当当前等级小于最大等级时才合成为更高等级
                if (lvl >= MAX_LEVEL) continue;
                int newLevel = std::min(MAX_LEVEL, lvl + 1);
                sf::Vector2f mid((p1.x + p2.x) / 2.f, (p1.y + p2.y) / 2.f);
                balls[j].isDead = true;
                balls[i].isDead = true;
                // 生成合成球时不要给予强烈向上速度，设置为不动以避免跳起
                spawns.push_back({mid.x, mid.y - 4.f, newLevel, sf::Vector2f(0.f, 0.f)});
                score += newLevel * 50;
            }
        }
    }

    // 将生成请求转换为实际球（受 MAX_BALLS 限制）
    for (auto& r : spawns) {
        if (balls.size() >= MAX_BALLS) break;
        // 如果生成的是胜利等级，则标记为胜利状态
        if (r.level >= LEVEL_WIN) {
            gameWin = true;
            gameOver = false;
            if (!font.getInfo().family.empty()) {
                winText.setFont(font);
                winText.setCharacterSize(28);
                winText.setFillColor(sf::Color::White);
                winText.setString("恭喜你合成出上海大学");
            }
            // 仍然生成这个球以便视觉显示
        }
        const sf::Texture* sTex = (textures[r.level].getSize().x > 0) ? &textures[r.level] : nullptr;
        balls.emplace_back(r.x, r.y, r.level, colors[r.level], sTex);
        balls.back().setVelocity(r.vel);
        // 初始化生命线相关字段
        balls.back().prevPosition = balls.back().getPosition();
        balls.back().timeAboveLine = 0.f;
        balls.back().wasSpawnedAboveLine = (r.y - balls.back().getRadius() <= lifelineY);
    }
    // 更严格的迭代碰撞分离：多次通过以确保没有明显侵入
    const int separationPasses = 4;
    for (int pass = 0; pass < separationPasses; ++pass) {
        for (size_t i = 0; i < balls.size(); ++i) {
            for (size_t j = i + 1; j < balls.size(); ++j) {
                if (balls[i].isDead || balls[j].isDead) continue;
                sf::Vector2f p1 = balls[i].getPosition();
                sf::Vector2f p2 = balls[j].getPosition();
                float dx = p2.x - p1.x;
                float dy = p2.y - p1.y;
                float dist = std::sqrt(dx*dx + dy*dy);
                float rsum = balls[i].getRadius() + balls[j].getRadius();
                if (dist <= 0.0001f) {
                    // 随机微小偏移，避免完全重合
                    float jitter = 0.5f;
                    balls[i].setPosition(p1 + sf::Vector2f(-jitter, -jitter));
                    balls[j].setPosition(p2 + sf::Vector2f(jitter, jitter));
                    continue;
                }
                if (dist < rsum) {
                    float overlap = rsum - dist;
                    sf::Vector2f normal(dx / dist, dy / dist);
                    float m1 = balls[i].mass;
                    float m2 = balls[j].mass;
                    float total = m1 + m2;
                    // 更强力的分离（确保无穿透），并轻微去除沿法线速度以避免再次侵入
                    sf::Vector2f moveI = -normal * (overlap * (m2/total) * 1.02f);
                    sf::Vector2f moveJ = normal * (overlap * (m1/total) * 1.02f);
                    balls[i].setPosition(balls[i].getPosition() + moveI);
                    balls[j].setPosition(balls[j].getPosition() + moveJ);

                    // 修正速度，移除沿法线的侵入分量
                    sf::Vector2f v1 = balls[i].getVelocity();
                    sf::Vector2f v2 = balls[j].getVelocity();
                    float vn1 = v1.x * normal.x + v1.y * normal.y;
                    float vn2 = v2.x * normal.x + v2.y * normal.y;
                    // 将沿法线的速度减小（防止再次穿透）
                    v1 -= normal * (vn1 * 0.6f);
                    v2 -= normal * (vn2 * 0.6f);
                    balls[i].setVelocity(v1);
                    balls[j].setVelocity(v2);
                }
            }
        }
    }

    // 墙面约束（左右），并减少水平速度（小的反弹）
    float winW = static_cast<float>(window.getSize().x);
    for (auto &b : balls) {
        sf::Vector2f pos = b.getPosition();
        float r = b.getRadius();
        sf::Vector2f vel = b.getVelocity();
        if (pos.x - r < leftMargin) {
            pos.x = leftMargin + r;
            b.setPosition(pos);
            vel.x = -vel.x * 0.2f; // 小反弹
            b.setVelocity(vel);
        } else if (pos.x + r > winW - rightMargin) {
            pos.x = winW - rightMargin - r;
            b.setPosition(pos);
            vel.x = -vel.x * 0.2f;
            b.setVelocity(vel);
        }
    }
}

void Game::resetGame()
{
    balls.clear();
    score = 0;
    spawnLocked = false;
    currentSpawnLevel = 1;
    gameOver = false;
    gameWin = false;
}

bool Game::isSupported(size_t idx) const
{
    if (idx >= balls.size()) return false;
    const float EPS = 2.0f;
    // DFS/BFS 从 idx 向下查找是否可以通过接触链条到达地面
    std::vector<char> visited(balls.size(), 0);
    std::vector<size_t> stack;
    stack.push_back(idx);
    while (!stack.empty()) {
        size_t cur = stack.back(); stack.pop_back();
        if (visited[cur]) continue;
        visited[cur] = 1;
        sf::Vector2f p = balls[cur].getPosition();
        float bottom = p.y + balls[cur].getRadius();
        // 如果接触或穿透地面则认为被支撑
        if (bottom >= Ball::FLOOR_Y - EPS) return true;
        // 向下找接触的球
        for (size_t k = 0; k < balls.size(); ++k) {
            if (k == cur) continue;
            if (visited[k]) continue;
            sf::Vector2f pk = balls[k].getPosition();
            float dx = pk.x - p.x;
            float dy = pk.y - p.y;
            // 要求对方在当前球下方
            if (dy <= -0.5f) continue; // pk not below cur
            float dist = std::sqrt(dx*dx + dy*dy);
            float rsum = balls[cur].getRadius() + balls[k].getRadius();
            // 如果两球接触或足够接近（允许少量误差），则认为存在支撑链
            if (dist <= rsum + EPS) {
                // 确保 pk 的垂直位置在 cur 之下（即真正位于下方）
                if (pk.y > p.y - 0.5f) {
                    stack.push_back(k);
                }
            }
        }
    }
    return false;
}

// 渲染画面
void Game::render()
{
    window.clear(sf::Color(240, 240, 240));
    // 绘制生命线（虚线）
    sf::Color lineColor(180, 30, 30);
    float startX = 0.f;
    float endX = static_cast<float>(window.getSize().x);
    float dashW = 12.f;
    float gapW = 8.f;
    for (float x = startX; x < endX; x += (dashW + gapW)) {
        sf::RectangleShape seg(sf::Vector2f(std::min(dashW, endX - x), 2.f));
        seg.setPosition(x, lifelineY);
        seg.setFillColor(lineColor);
        window.draw(seg);
    }
    for (auto& ball : balls)
        ball.render(window);
    // 绘制分数与当前生成等级（如果 font 可用）
    if (!font.getInfo().family.empty()) {
        // 仅显示分数；在其下方绘制“下一个球”的小预览图标（固定大小，颜色随 nextSpawnLevel 变化）
        window.draw(scoreText);

        const float PREVIEW_R = 12.f;
        sf::CircleShape preview(PREVIEW_R);
        preview.setOrigin(PREVIEW_R, PREVIEW_R);
        // 将预览放在 scoreText 下方（与原来等级文本位置相近）
        preview.setPosition(8.f + PREVIEW_R, 34.f + PREVIEW_R);
        int lv = std::max(1, std::min(nextSpawnLevel, MAX_LEVEL));
        preview.setFillColor(colors[lv]);
        preview.setOutlineColor(sf::Color::Black);
        preview.setOutlineThickness(2.f);
        window.draw(preview);

        // 在球中央显示等级数字（若字体可用）
        sf::Text lvlText;
        lvlText.setFont(font);
        lvlText.setCharacterSize(12);
        lvlText.setFillColor(sf::Color::Black);
        lvlText.setString(std::to_string(lv));
        sf::FloatRect tb = lvlText.getLocalBounds();
        lvlText.setOrigin(tb.left + tb.width/2.f, tb.top + tb.height/2.f);
        lvlText.setPosition(preview.getPosition());
        window.draw(lvlText);
    }
    // 胜利/失败界面
    if (gameWin) {
        // 半透明遮罩
        sf::RectangleShape overlay(sf::Vector2f((float)window.getSize().x, (float)window.getSize().y));
        overlay.setFillColor(sf::Color(0,0,0,120));
        window.draw(overlay);

        if (!font.getInfo().family.empty()) {
            // 显示胜利文本（中文）
            sf::FloatRect tb = winText.getLocalBounds();
            winText.setOrigin(tb.left + tb.width/2.f, tb.top + tb.height/2.f);
            winText.setPosition(window.getSize().x/2.f, window.getSize().y/2.f - 60.f);
            window.draw(winText);
        }

        // 绘制 Again 按钮，居中
        againButton.setPosition(window.getSize().x/2.f - againButton.getSize().x/2.f, window.getSize().y/2.f + 10.f);
        window.draw(againButton);
        if (!font.getInfo().family.empty()) {
            sf::FloatRect tb2 = againText.getLocalBounds();
            againText.setOrigin(tb2.left + tb2.width/2.f, tb2.top + tb2.height/2.f);
            againText.setPosition(againButton.getPosition().x + againButton.getSize().x/2.f, againButton.getPosition().y + againButton.getSize().y/2.f - 4.f);
            window.draw(againText);
        }
    } else if (gameOver) {
        // 半透明遮罩
        sf::RectangleShape overlay(sf::Vector2f((float)window.getSize().x, (float)window.getSize().y));
        overlay.setFillColor(sf::Color(0,0,0,120));
        window.draw(overlay);

        if (!font.getInfo().family.empty()) {
            sf::Text loseText;
            loseText.setFont(font);
            loseText.setCharacterSize(36);
            loseText.setFillColor(sf::Color::White);
            loseText.setString("You lose");
            // 居中
            sf::FloatRect tb = loseText.getLocalBounds();
            loseText.setOrigin(tb.left + tb.width/2.f, tb.top + tb.height/2.f);
            loseText.setPosition(window.getSize().x/2.f, window.getSize().y/2.f - 60.f);
            window.draw(loseText);
        }

        // 绘制 Again 按钮，居中
        againButton.setPosition(window.getSize().x/2.f - againButton.getSize().x/2.f, window.getSize().y/2.f + 10.f);
        window.draw(againButton);
        if (!font.getInfo().family.empty()) {
            sf::FloatRect tb = againText.getLocalBounds();
            againText.setOrigin(tb.left + tb.width/2.f, tb.top + tb.height/2.f);
            againText.setPosition(againButton.getPosition().x + againButton.getSize().x/2.f, againButton.getPosition().y + againButton.getSize().y/2.f - 4.f);
            window.draw(againText);
        }
    }

    window.display();
}