# Gfx

A graphics library for rapid prototyping. Shader code and pipeline states are declared in JSON files that are loaded at runtime to create pipelines. Shaders can be hotreloaded at the press of a button. This approach is very much based on [a blog post by Tobias Persson from Our Machinery](https://ourmachinery.com/post/the-machinery-shader-system-part-2/).

![screenshot](https://user-images.githubusercontent.com/3328360/54489599-3c054b00-48ae-11e9-96ec-2d46ad5aed0a.png)

## Requirements

* Vulkan SDK
* CMake
* Visual Studio 2017 x64 (currently the only supported build configuration)