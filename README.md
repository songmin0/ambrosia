# Team06 - Ambrosia

### Summary - Milestone 1:

Our work on this milestone aligned with our development plan. We have reviewed our plan for Milestone 2, and no changes are needed.

#### AI
- Created the mob base class
- Made mob movement logic to move towards its closest player (linked with active turn component and pathfinding system)
\* Note: Egg is a mob

#### Observer Pattern
- EventSystem which uses callbacks (event listeners) to be notified of events such as MouseClickEvent, HitEvent, projectile launch events, and so on. It is currently connected to the UISystem, TurnSystem, and ProjectileSystem.

#### Geometric/sprite/other assets
- Created assets: background, navmesh, animated sprites, placeholder buttons
- Adjusted motion.scale's system to be a relative scale
- Implemented animation system and animations component
- Added render support for animated sprites using OpenGL 2D Array Textures
- Implemented geometric deformation (jiggling cheese blob)

#### UI and IO
- Implemented clickable UI buttons (change character buttons, attack buttons)
- Implemented UI system to check if any buttons are clicked, and handles calling its callback function

#### Collision
- Implemented AABB collision on entities
- Added scalable hitboxes for entities

#### Pathfinding
- Implemented BFS pathfinding which is used for players (point-and-click movement) and AI.
- Modified the PhysicsSystem to allow an entity to move along a predefined path which was generated by the PathFindingSystem.

#### Physics/projectiles
- Implemented the ProjectileSystem which allows for launching projectile attacks such as the bone throw attack. Projectiles can travel with a linear trajectory or a boomerang trajectory.
- Projectiles are entities and can collide with players and enemies. A HitEvent is sent (using the EventSystem) upon collision.

#### Turn system
- Implemented a turn system which works for players and AI
- Hooked up the player buttons so clicking on them switches to the appropriate player's turn
- Hooked up colour coordination between the Player UI buttons and the players to help identify which player button is for which player


---

### Key Bindings for Debugging
- R = restart
- D = debug mode
- 3 = makes one of the player characters play his bone-throw attack animation (animation only)
- 4 = makes all the eggs play the hit animation
- M = dessert arena
- N = pizza arena

### Additional Info
The lighter area in the center of the map is the walkable region. In this basic version of our game, click on the walkable region to make the current active player navigate to that position. Alternatively, click on an unwalkable region of the map to make the current active player launch a bone-throw attack which travels on a boomerang trajectory. This was implemented for testing purposes.

Each player character gets their turn, and then the mobs get their turns. A "turn" only has a movement phase at the moment, but in the near future, a turn will consist of an optional move phase, followed by a skill phase.

### References
* http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/
