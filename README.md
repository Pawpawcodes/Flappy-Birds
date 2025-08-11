# Flappy-Bird-Console Edition (C++)
Flappy Bird – Console Edition is a lightweight C++ game that runs entirely in the Windows console. Built with a custom text-based game engine, it features simple graphics made from Unicode characters, smooth gameplay, basic physics for gravity and flapping, collision detection with pipes, and score tracking.

## 🎮 Features
- **Console-based graphics** using Unicode characters
- **Smooth gameplay loop** without external dependencies
- **Score tracking** (Attempts & Max Flaps)
- **Collision detection** with pipes & ground
- **Restart system** after game over
- Customisable screen size and colors

---

## 🖼 Demo
```plaintext
   <//=Q        Pipe
    //        █████
             █████
Yellow bird

Green pipes

"GAME OVER" screen when you crash

⚙ How It Works
The game is built using:

Custom Console Game Engine

Handles drawing characters, filling areas, and writing to the console buffer.

No std::thread — single-threaded loop for compatibility.

Game Logic

Bird position & velocity updated using basic physics (gravity & flap boost).

Pipes are stored in a std::list and move across the screen.

Collision detection checks console buffer for overlaps with pipe characters.

Rendering

Draws background, pipes, and bird each frame.

Uses Unicode block characters (█) for solid graphics.

🕹 Controls
Spacebar → Flap upward

Avoid pipes & ground

Release Spacebar after game over to restart

📦 Requirements
Windows (uses WinAPI for console graphics)

MinGW or any Windows C++ compiler

C++11 support

🔨 Compile & Run
bash
Copy
Edit
g++ game.cpp -o flappybird -std=c++11 -static -luser32 -lgdi32
flappybird.exe
