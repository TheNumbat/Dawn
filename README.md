# Dawn

![shot0](https://i.imgur.com/9KcH9sZ.png)
![shot1](https://i.imgur.com/b93N7Xh.png)

Learning about path tracing.

The Visual Studio 2019 compiler optimized build is currently broken, likely due to a compiler bug or unreasonable undefined behavior. I'll fix this eventually, but for now building with clang or MSVC 2017 works fine.

Current features:
- [Ray Tracing in a Weekend](http://www.realtimerendering.com/raytracing/Ray%20Tracing%20in%20a%20Weekend.pdf)
- [Ray Tracing: the Next Week](http://www.realtimerendering.com/raytracing/Ray%20Tracing_%20The%20Next%20Week.pdf)
- Multi-threaded rendering
- SIMD accelerated math + intersection testing
- GUI/CLI interfaces

Future:
- going through [pbrt](http://www.pbr-book.org/3ed-2018/contents.html)
