# Vivianite TODO-List

## Rendering
- [~] Forward+ renderer
    - [x] OpenGL support
    - [x] Blinn-Phong rendering
    - [x] Light culling
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
    - [ ] Vulkan support

## Input system
- [~] Input system
    - [~] Key callbacks
    - [ ] Mouse input
    - [ ] Keyboard axes
    - [ ] Custom keytype
    - [ ] Keyboard get_event()
    - [ ] Controller support
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
    - [ ] Scene system
    - [ ] ECS / Module system
    - [ ] Scriptable modules
    - [ ] Resource manager
    - [ ] Serialization
    - [ ] Configuration system
    - [ ] Built-in physics engine
    - [ ] Audio system
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
