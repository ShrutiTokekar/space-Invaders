# Space Invaders - C++ Game

A classic Space Invaders game implemented in C++ using SDL2.

## Features

- **Classic gameplay**: Defend against waves of alien invaders
- **Progressive difficulty**: Enemies speed up as you eliminate them
- **Score system**: Higher rows worth more points
- **Lives system**: 3 lives to start
- **Enemy AI**: Aliens shoot back randomly
- **Victory/Game Over states**: Win by clearing all enemies

## Controls

- **Arrow Keys / A,D**: Move left and right
- **Spacebar**: Shoot
- **Spacebar** (when game over): Restart game

## Requirements

- C++ compiler (g++)
- SDL2 library

## Gameplay

1. **Objective**: Destroy all alien invaders before they reach the bottom
2. **Movement**: Aliens move side-to-side and descend when hitting screen edges
3. **Shooting**: Both you and aliens can shoot
4. **Lives**: You have 3 lives - lose one when hit by enemy bullet
5. **Victory**: Destroy all enemies to win
6. **Game Over**: Lose all lives or let aliens reach your position

## Technical Details

### Architecture
- **Entity-Component Pattern**: Separate Player, Enemy, and Bullet entities
- **Collision Detection**: AABB (Axis-Aligned Bounding Box) collision
- **Game Loop**: Fixed ~60 FPS frame rate
- **State Management**: Game over and victory states

### Code Structure
```
SpaceInvaders class:
├── Entity management (Player, Enemies, Bullets)
├── Input handling
├── Update logic (movement, collisions)
├── Rendering (SDL2 primitives)
└── Game state management
```

### Key Features Implemented
- Object-oriented design with inheritance
- STL containers (vectors)
- Collision detection algorithms
- Game state machine
- Real-time rendering



