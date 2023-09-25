# TreeGen
![forest](https://github.com/caner-milko/TreeGen/assets/13199909/9f2b2c96-d911-4fae-b6b9-4463a08111e2)
## Showcase Video
[![video](https://img.youtube.com/vi/jNKKf2cMzxc/0.jpg)](https://www.youtube.com/watch?v=jNKKf2cMzxc)

A standalone procedural tree generation and rendering application developed for [my bachelor's thesis](./Thesis.pdf).

It uses:
- C++20
- OpenGL 4.6
- ImGui

The tree generation algorithm is an improved real-time version of "Self-organizing tree models for image synthesis", Wojciech Palubicki et al.

The renderer is a raymarching/forward rendering hybrid. Generated trees branches are converted to 3D quadratic bezier curves and rendered using raymarching.
More detailed information can be read from the [thesis paper](./Thesis.pdf).
