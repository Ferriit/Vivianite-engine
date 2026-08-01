# Vivianite TODO-List

## Rendering
- [~] Renderer
    - [x] OpenGL support
    - [x] Blinn-Phong rendering
    - [x] Forward+ light culling
    - [x] Light culling
    - [x] Textures
    - [ ] Shadows
        - [ ] Shadow maps
        - [ ] Cascaded shadow maps
    - [ ] PBR-materials
        - [ ] Metallic/roughness workflow
        - [ ] Image-based lighting
    - [ ] Post-Processing
        - [ ] Bloom
        - [ ] HDR
        - [ ] Tone mapping
        - [ ] Anti-aliasing
    - [ ] Scriptable shaders
    - [ ] \(Vulkan support\)

## Input system
- [~] Input system
    - [x] Key callbacks
    - [~] Mouse input
    - [x] Keyboard axes
    - [x] Custom keytype
    - [x] Keyboard get_event() function
    - [x] Controller support
    - [ ] Input rebinding

## Scheduling system
- [ ] Scheduling system
    - [x] Function scheduling
    - [x] Move engine setup and main loop to scheduler
    - [ ] Multi-threading for some tasks
    - [ ] Fixed update loop
    - [ ] Task dependencies
    - [ ] Async resource loading

## Engine
- [~] Engine
    - [x] Basic, runnable state
    - [x] Rendering subsystem
    - [x] Logging subsystem
    - [x] Scheduling subsystem
    - [~] Input subsystem
    - [ ] Resource manager subsystem
    - [ ] Audio subsystem
    - [ ] Serializer subsystem
    - [ ] Networking subsystem
    - [ ] Scene system
    - [ ] ECS / Module system
    - [ ] Scriptable modules
    - [ ] Configuration system
    - [ ] Built-in physics engine
    - [ ] Documentation
    - [ ] CUTE scripting integration

## Editor
- [ ] Editor
    - [ ] Asset manager
    - [ ] Module system
    - [ ] Built-in compilation
    - [ ] Scene editor
    - [ ] Material editor
    - [ ] Shader editor
    - [ ] Inspector
    - [ ] Debug tools

## Physics
- [ ] Physics engine
    - [ ] Collision detection
    - [ ] Collision resolution
    - [ ] Rigid bodies
    - [ ] Triggers
    - [ ] Constraints
    - [ ] Ray casting

## Audio
- [ ] Audio system
    - [ ] Sound effects
    - [ ] Music playback
    - [ ] 3D spatial audio
    - [ ] Audio mixer
    - [ ] Audio events

## Resource manager
- [~] Resource managing system
    - [x] Resource handles / IDs
    - [x] Resource loading
    - [~] Resource unloading
    - [x] Resource caching
    - [ ] Resource reference counting
    - [ ] Async resource loading
    - [ ] Resource dependency tracking
    - [>] Asset metadata
    - [x] Hot reloading
    - [ ] Asset importing

## ECS / Modules
- [ ] ECS / Module system
    - [ ] Base entity
    - [ ] Modules that inherit the base entity
        - [ ] Transform Module
        - [ ] Rigidbody Module
        - [ ] Meshrenderer Module
            - [ ] Material Module
        - [ ] Collider Module
        - [ ] Audioplayer Module
        - [ ] Input Module
        - [ ] Scriptable Module

## Assets
- [ ] Asset pipeline
    - [ ] Model importing
    - [ ] Texture importing
    - [ ] Material importing
    - [ ] Asset caching
    - [ ] Hot reloading

## Tools
- [ ] Developer tools
    - [ ] Profiler
    - [ ] Debug renderer
    - [ ] Console
    - [ ] Performance graphs
    - [ ] Memory tracking

## Networking
- [ ] Networking
    - [ ] TCP/UDP layer
    - [ ] Client/server architecture
    - [ ] Replication system
