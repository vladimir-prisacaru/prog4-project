# Dig Dug
Exam project for the Programming 4 course in Howest DAE

<img width="316" height="351" alt="image" src="https://github.com/user-attachments/assets/e571f458-e09f-418b-b431-051cb99682bf" />

Created a 2D game engine, and an implementation of Dig Dug built on top of it.



**Controls:**

Gamepad: D-pad - move, X - attack
Keyboard: WASD (P1) / Arrows (P2) - move, F (P1) / L (P2) - attack

---

## Engine

### Core Loop
The engine (`Minigin`) runs a fixed-timestep loop with a capped frame rate. Physics and collision run at the fixed step; gameplay logic and rendering run at the variable frame rate. An `EngineCtx` struct is threaded through every system call so components can reach any engine service without global state.

### GameObject & Component System
Scenes are made up of `GameObject`s, each owning an arbitrary set of `Component`s. Components register themselves at static-init time via a `Registrar<T>` CRTP mixin and declare their XML-parseable parameters through `RegisterParameter`. This lets any scene be fully described in a `.xml` file — objects, their parent–child hierarchy, component types and initial values are all parsed at load time with no code changes required.

### Scene Management
`SceneManager` loads and unloads scenes from `.xml` files. Loads are deferred to end-of-frame to avoid mutating the scene list mid-update. Multiple scenes can be live at once; `GameManager` uses this to keep itself alive across level transitions while loading and unloading level scenes underneath it.

### Physics & Collision
A `Physics` system maintains a flat list of `ICollider`s. Each frame it tests every pair and fires `OnOverlap` / `OnOverlapEnd` on components that implement `ICollisionReceiver`, with enter/exit semantics tracked across frames. Two collider types are provided: `BoxCollider` (AABB) and `GridCollider` (tilemap solid-tile traversal). The physics system also exposes typed raycasts — `Raycast<T>()` filters to a specific collider type — used for line-of-sight checks and attack wall blocking.

### Event System
`EventManager` implements a queued observer pattern. Components subscribe to specific `GameEvent` types; events are buffered during the frame and dispatched in a single `FlushEvents` call after update, preventing mid-frame mutations. A separate `Subject` mixin is available for direct (non-queued) local notifications.

### Input
`InputManager` maps keyboard keys and Xbox controller buttons to `InputCommand` objects with configurable trigger states (`Pressed`, `Down`, `Up`). Commands are owned by the manager and executed each frame. Controller support is handled via a `Gamepad` wrapper around XInput.

### Audio
`SoundSystem` is a pure-virtual interface accessed through a `ServiceLocator`. The SDL3_mixer implementation (`SoundSystemSDL`) offloads all audio work to a dedicated thread: the main thread enqueues requests (`Play`, `PlayIfNotPlaying`, `Stop`), the audio thread processes them against a cache of loaded `MIX_Audio` assets and a map of named `MIX_Track`s. `PlayIfNotPlaying` is used for polled sounds that should run continuously while a condition holds, without restarting on every call.

### Rendering & Sprites
`SpriteComponent` loads a spritesheet texture and a JSON animation definition. Animations are collections of frames with per-frame timing and pivot points; the component advances frames each update and renders via the `IRenderable` interface. A `GridRenderer` handles tilemap rendering separately.

### Resource Management
`ResourceManager` caches loaded textures by path via `shared_ptr`, so multiple components referencing the same asset pay the load cost only once.

---

## Game

Dig Dug is a tile-based arcade game. The player moves through a grid, digging tunnels, and must defeat all enemies to advance to the next level.

Three game modes are supported: **Solo**, **Co-op** (shared lives, keyboard + controller), and **Versus** (two players compete; killing the other player scores points).

Enemy AI pathfinds toward the nearest player using a navigation graph built from the tunnel network, and attacks when in range with line-of-sight checks.
