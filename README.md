# Synthesizer

This is a small project I'm working on to build a synthesizer from scratch in C++.  
No audio libraries, no standard formats — my goal is to learn how audio processing works at a low level by building everything from the ground up.

Since this is a learning project, it may not conform to industry norms.  
I'm focusing on design where it makes sense but intentionally working things out on my own wherever possible to deepen my understanding.

Real-time audio processing and device access currently rely on the Windows API.  
As a result, versions of this repository that support real-time playback or input will be **Windows-only** for now. Cross-platform support may be added in the future.

---

## 🔧 Build Instructions

From the project root:

```sh
mkdir build
cd build
```

### ✅ Recommended: Ninja

```sh
cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### 🪟 Alternative: Visual Studio (Windows)

```sh
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

