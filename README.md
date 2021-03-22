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
## Milestone 3
### Summary - Milestone 3:

Our work is consistent with our development plan, and we are using one grace day for this milestone.

#### Gameplay:
- added the GameStateSystem which handles the flow of the game outside of each level
- Refactoring and extension of the skills implementation to allow projectiles to do more than just apply damage to targets. They are now configured in the same way as AreaOfEffect skills and can do things like heal, stun, buff, debuff, and damage targets
- Added skills for Chia and Taji (the green and blue characters)
- Taji's second skill implements stun CC that skips the turn of the afflicted enemy for 1 turn. It includes a VFX that is active for the duration of the stun. Bosses such as the Potato boss and its related chunks/respawned version are immune to CC and are not affected by the stun.
- Chia's third skill grants all allies an HP shield. This is indicated both with a VFX and a procedurally-generated blue bar in the affected player's HP bar that informs players how much of the shield is remaining.
- Added movement range to the Motion struct for simple distance limitations
- Added the range indicator for skills and adjusted AABB collision + targetting system to take bounding boxes into account when calculating range
- Added a black fade-out transition for all transitions in the game
- Added start screen, defeat screen, and victory screens

#### Physics:
- Implemented a knockback attack for Ember (the red character)'s third skill, which applies an impulse to nearby targets. The force of the attack is dependent on the instigator's strength. The affect it has on the target's velocity is dependent on the target's mass. Variations of this skill can be added to any player or enemy
- Added a friction force to the physics system to gradually stop targets who get knocked back
- Modified the timestep calculations in main.cpp to allow for framerate-independent game logic and physics. And for smoother motion, entities are rendered using interpolated positions and orientations, based on their positions/orientations in the most recent physics step and the step before that

#### AI
- Refactoring code to make behaviour tree definition easier
- Added Milk mob, Pepper mob, Potato boss behaviour trees

#### Advanced Group Behaviour
- The Milk mob is a healer-type mob. It will heal its allies when there exists any ally that is not full HP, otherwise it will attack players with a ranged projectile and do damage.

#### Swarm Behaviour
- Implemented swarm behaviour for potato boss (spawns in the third arena)
- On defeat, spawns 5 potato chunks evenly spaced around boss
- Chunks try to reconstitute into a mashed potato boss when close enough
- Health of the reformed mashed potato boss is reliant on the remaining HP of the chunks

#### Level Loader
- Refactored to add ability to specify stats for mobs as well
- Changed a per level format to per playthrough format

#### Save and Load
- Added auto save after a stage is finished
- Current stage and playthrough outputted into JSON
- Click "Load" from the start screen to load the last played level from the save file

#### User Experience
- Added story introduction (press "Start" from main screen to view)
- Added intro gameplay tutorial (press "Start" and complete story panels to view)
- Added a "Help" overlay. This can be accessed in game after completion of tutorial, either by pressing `H` or by clicking the `Help` icon in the top right corner of the screen.
- Based on player feedback, adjusted the TurnSystem and UISystem to account for a "Null State" during player turns, in which no action is considered "active" and player mouse clicks do nothing (previously, the player must be in either "Move" or "Skill" phase. Now, players begin in "Null" state and must manually select an action to perform). This also allows players to use a skill without moving.
- Based on player feedback, adjusted mob HP bars to be red

#### VFX:
- Implemented effect system to handle creating and removing visual skill effects
- Effect system also handles positioning buffs on top of the buffed entity
- Updated the particleSystem so particles can be added at anytime instead of just in the main method
- Made the cotton candy particles only spawn in the Dessert arena

#### UI:
- Active player is now indicated by a bobbing arrow above their head

#### Audio Feedback:
- All major interactions in the game have audio feedback (ie. mouse clicks, turn switch, skill usage, defeat... etc)

#### Memory Management:
- Using Visual Studio's Diagnostic Tools, examined memory usage in the game
- Fixed memory leaks due to improper handling of `stbi` pointers
- Fixed memory leaks due to allocating heap resources using `new` instead of `std::shared_pointer`

#### Assets
- new start screen music (composed by Emma)
- new animations, button textures, VFX
- new splash art assets for all game menus
- new tutorial and story assets

---

### Key Bindings for Debugging
#### *Please note that debug keys only work during combat, and are disabled in start menus/story scenes/etc
- R = restart
- D = debug mode
- S = print stats for all entities
- M = load dessert arena
- N = load pizza arena
- A = cycle available soundtracks
- 3, 4 = animation tests
- H = show help menu (this isn't really a debugging key but a real key binding)
- L = load last save file


### Additional Info
All features should be accessible by playing through the entire game from the "Start" button. The "Load" button or the `L` key can be used to test save and reload. The expected behaviour is that a fresh version of the last map accessed should be loaded.

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
