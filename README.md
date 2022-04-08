# Yggdrasil
A personal Vulkan 1.3 render / game engine.

# Focus and Goals
The focus of this project is to create a game / render engine for open-world type games and potentially
an editor for creating such worlds. The terrain system planned is heavily inspired by the GDC talks
about the terrain and procedural techniques used in Far Cry 5 and Horizon Zero Dawn.

# Design
The core design of this engine should be lightweight, reusable abstractions that can be used in stand-alone
sample applications that do not require pulling the entire engine. The amount of interdependencies is kept
as small as possible to allow for this which should make the engine easier to extend overall.

The engine itself should work more akin to a framework than an engine like Unity or Unreal.
That means it isn't editor-focused, instead it should be as tools-focused as possible.
