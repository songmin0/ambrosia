# Team06 - Ambrosia

### Important Reference Info
#### Basic Controls
- **move camera** - arrow keys
- **change active player** - click player buttons (center top)
- **move active player** - while the move button is active, click a destination on the map
- **use player skill**  - click a skill button to select a skill, click a target on the map to use it
#### Playable Characters
- Raoul = "gold" character
- Taji = "blue" character
- Ember = "red" character
- Chia = "green" character

---

## Milestone 2
### Summary - Milestone 2:

Our work is consistent with our development plan, and we are using one grace day for this milestone.

#### Gameplay:
- Created SkillSystem supporting a wide variety of skills such as projectile attacks, melee attacks, buffs, and debuffs using the template pattern
- Added skills to the players and egg mobs
- Implemented buffs and debuffs as time-based stat modifiers in the StatsSystem
- Adjusted player death logic (delayed world restart after all players die)
- PathFindingSystem: update logic to find closest valid point if given invalid path destination

#### AI:
- Created a state system
- Implemented behaviour trees for mob AI (currently just the Egg)
- Hooked up with EventSystem and TurnSystem

#### Rendering:
- Created debugging graphics for pathfinding and bounding boxes
- Added RenderableComponent to handle render order
- Entities are sorted before rendering

#### Level Loading 
- Created json structure to specify level layout
- Added ability to read json into game
- Added ability to read navmesh img into game
- Levels can specify position, stats, map

#### User Experience
- Added on-hover skill tooltips

#### UI and IO
- Added HP bars
- Added visual feedback for Player Turn buttons
- Added visual feedback for Skill buttons
- Added visual feedback for mouse clicks

#### Camera System
- Implemented movable camera, controlled via arrow keys
- Camera will center on the active entity
- Camera follows the active moving entity

#### Particle System
- Created particle system
- Added cotton candy particle effect

#### Assets:
- new button/object textures, new characters, new map, new animations

#### Audio:
- Added new music and various sound effects/audio feedback

---

### Key Bindings for Debugging
- R = restart
- D = debug mode
- S = print stats for all entities
- M = load dessert arena
- N = load pizza arena
- A = cycle available soundtracks
- 3, 4 = animation tests


### Additional Info
All maps are rendered in "debug" mode with a navmesh overlay. Walkable regions are lighter and unwalkable regions are darker.

The game is turn-based. Players and mobs cycle through one turn each. A turn consists of [1] an initial **move phase**, and [2] a **skill phase**. A turn ends when an entity has moved and used a skill. The active player can be changed at any point within their turn, but once a player has ended their turn, they cannot be active again until the next cycle of turns.

After all players have ended their turns, the mobs will use their turns. The turn cycle repeats after all entities have ended their turns.

In this milestone, the game begins in the Pizza arena. Defeating all mobs in the Pizza arena will automatically load the Dessert arena. All subsequent victories reload the Dessert arena. This behaviour is temporary for gameplay and level-loading purposes, and will be improved in future milestones.


#### Notes
Some animations have not been implemented yet. The following animations are currently not available:
- Chia (green) and Ember (red)'s damaged and defeat animations

---

## Milestone 1
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

### References
* http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/
