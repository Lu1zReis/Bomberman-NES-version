# Bomberman
### This game was developed in C, using the Allegro library
---
### Initial idea 
This project aims to recreate the classic NES game Bomberman. To develop it, I used the Allegro 5 game programming library.

My initial idea was to start with a simple game that I enjoy, while also allowing me to apply many of the concepts and skills I have learned in Computer Science.
Bomberman was a great choice because its top-down perspective simplifies movement and collision mechanics, allowing me to focus on game logic without having 
to deal with more complex physics concepts such as jumping and gravity.  

---
### Features and Concepts

1. The game uses linked lists to manage enemies and bombs. The bomb list also behaves as a queue, allowing bombs to be processed and removed in the correct order.

2. Enums and structs are used to organize game data, representing entities such as the player, enemies, bombs, and map blocks.

3. Collision detection and movement are implemented through coordinate calculations and tile-based positioning.

4. The map is generated procedurally, creating different obstacle and enemy layouts for each game session.

5. Bomb explosions interact with the environment, destroying breakable blocks and affecting both the player and enemies.

---
### Game
The game may have improvements such as scoring and other aspects to make it more faithful to the real game. Bellow have some pictures of the game.

<img width="211" height="243" alt="image" src="https://github.com/user-attachments/assets/b59f49f4-43d5-48de-a667-39862695f57d" />
<img width="210" height="247" alt="image" src="https://github.com/user-attachments/assets/081b997f-e4bd-47be-a2d6-3452deb62d53" />
<img width="210" height="247" alt="image" src="https://github.com/user-attachments/assets/12a347e6-5c9e-4df7-a233-6e2cecebc410" />
<img width="210" height="247" alt="image" src="https://github.com/user-attachments/assets/8e4d0d8c-2465-4d13-8900-dc8c3fcdb14b" />
