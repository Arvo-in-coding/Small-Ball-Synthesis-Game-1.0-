# Synthetic SHU / 合成大西瓜 (上大版)

A physics-based puzzle game inspired by "Synthetic Watermelon", featuring Shanghai University (SHU) campus culture elements. Developed with C++ and SFML.

本项目是基于 C++ 和 SFML 开发的物理合成类休闲游戏，灵感来源于《合成大西瓜》，结合了上海大学 (SHU) 的校园文化元素。

---

## 🎮 How to Play / 游戏玩法

**Goal / 目标**:
Drop and merge balls to evolve them into higher-level elements (SHU landmarks/icons). The ultimate goal is to synthesize the **Shanghai University Logo**.
控制不同等级的“上大元素”下落，通过物理碰撞将低级元素合成为高级元素，最终目标是合成上海大学校徽。

**Controls / 操作**:
- **Mouse / 鼠标**: Move horizontally to aim, click to spawn a ball. / 移动鼠标瞄准，点击左键生成元素。
- **Merge / 合成**: Two balls of the same level merge into one higher-level ball upon contact. / 两个相同等级的球碰撞后会合并为高一级的球。
- **Game Over / 游戏结束**: If the stack of balls reaches the top "Life Line", the game ends. / 如果元素堆叠高度超过顶部的“生命线”，游戏结束。

---

## 🛠️ Build & Run / 编译与运行

### Prerequisites / 环境要求
- **OS**: macOS (Tested) / Windows / Linux
- **C++ Standard**: C++17 or higher
- **Library**: [SFML](https://www.sfml-dev.org/) (Simple and Fast Multimedia Library) 2.5.1+

### macOS Build Instructions / macOS 编译指南

1.  **Install SFML** (e.g., via Homebrew):
    ```bash
    brew install sfml
    ```
    *Note: If you copied the local `SFML` folder provided in this repo, ensure the paths in the build command match your setup.*
    *注意：如果你使用本项目自带的 `SFML` 文件夹，请确保编译命令中的路径正确。*

2.  **Compile & Run / 编译并运行**:
    Run the following command in the terminal (adjust paths if necessary):
    
    ```bash
    g++ -std=c++17 -Wall -Wextra \
    -I./SFML/include \
    main.cpp Game.cpp Ball.cpp \
    -o game \
    -F./SFML/Frameworks \
    -framework sfml-graphics -framework sfml-window -framework sfml-system && ./game
    ```

3.  **Visual Studio Code**:
    This project includes a `.vscode` configuration. You can simply press **Cmd + Shift + B** to build the game if your environment matches the configuration.
    本项目包含 `.vscode` 配置。如果环境配置一致，你可以直接在 VS Code 中按下 **Cmd + Shift + B** 进行编译。

> [!IMPORTANT]
> The `assets` folder must be in the same directory as the executable `game`.
> `assets` 文件夹必须与可执行文件 `game` 位于同一目录下。

---

## 📂 Project Structure / 项目架构

- **`main.cpp`**: Entry point. / 程序入口。
- **`Game.cpp/h`**: Core game logic (Game loop, rendering, event handling). / 游戏核心逻辑（主循环、渲染、事件处理）。
- **`Ball.cpp/h`**: Physical entity class (Physics, collision handling). / 物理实体类（物理运动、碰撞处理）。
- **`assets/`**: Game textures and resources. / 游戏素材与资源。
- **`SFML/`**: Local copy of SFML libraries (Mac frameworks). / 本地包含的 SFML 库文件。

---

## 📝 Credits / 致敬

This project creates a gamified experience to celebrate the culture of **Shanghai University**.
本项目旨在通过游戏化的方式致敬 **上海大学** 校园文化。