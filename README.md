# Yggdrasil
A personal Vulkan 1.3 render / game engine.

# Focus and Goals
The focus of this project is to create a game / render engine for open-world type games and potentially
an editor for creating such worlds. The terrain system planned is heavily inspired by the GDC talks
about the terrain and procedural techniques used in Far Cry 5 and Horizon Zero Dawn.

# Vulkan
## Mindset on abstractions
The abstractions should be as lightweight as possible. Considering that this engine is only using Vulkan
and will only use Vulkan it isn't sensible to hide all of Vulkan away in an RHI as of now.
