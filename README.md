# Software renderer
A toy software implementation of a graphics pipeline with programmable vertex and fragment shaders.

## Prerequisites
 - CPU with AVX
 - OpenGL 4.5
 - CMake
 - C++14 compiler

## TODO
 - add proper culling due to the now limited range of the guard band
 - add subpixel precision to the line rasterizer
 - vectorize
 - parallelize

## References
 - https://habrahabr.ru/post/243011/
 - https://habrahabr.ru/post/248153/
 - https://fgiesen.wordpress.com/2013/02/06/the-barycentric-conspirac/
 - http://forum.devmaster.net/t/advanced-rasterization/6145
 - https://software.intel.com/en-us/articles/rasterization-on-larrabee
 - http://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation
 - http://www.amazon.com/Mathematics-Programming-Computer-Graphics-Third/dp/1435458869/
 - http://www.amazon.com/OpenGL-Superbible-Comprehensive-Tutorial-Reference/dp/0672337479/

## License
[MIT](LICENSE)
