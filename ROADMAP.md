# Roadmap

## Wishlist
- Transform as a matrix/rotation as a quaternion.
- Registry diffs. Need to maintain a diff between current scene and scene on disk, and also between simulating scene vs current scene.
  - Will be beneficial when I pass modified values across threads.
- Component diffs are also required to pass changes across physics systems
- Undo/Redo system for UI
- Serialization. For effective saving/loading scenes, binding components to editor panels etc etc
- UI markup? Have a "ImGUILang" parser on the editor side that reads an ImGUILang file for each module that has UI elements, maybe autogenerated by CMake.
  - Can be used for custom editor gizmos
- Rethink module structure. Plugins vs modules? Try [cr](https://github.com/fungos/cr) for hot reloading? 
  - EnTT 3.6 makes breaking changes. Module and a central `type_context` in the engine is important before I bump the version.
  - EnTT 3.6 (3.7 now) will make diffing and serialization simpler I think. Need more research.
- Engine modules for rendering, resources, physics etc. These would each be separate DLLs. The game itself would be a module (see Godot)
- Engine play button for testing the game in editor
- Split simulation and rendering threads
  - Single tick or independent ticks? I would prefer to go for single tick, but then we'll have to synchronize each frame.
  - FixedUpdate vs Update for physics and rendering
- Low-level render library to interface with various graphics APIs like Vulkan, OpenGL etc. (or just use Vulkan everywhere?)
- Deferred rendering
- Shadows and reflections
- Importer tool to import an asset into the engine and store it with all required metadata in an optimized format
- Build tool to create no-editor builds of the game. Have a couple of stub files that implement the runtime and use CMake scripts
to build and link it with the engine and the game DLL
