# ACCheat — AssaultCube 1.3.0.2

<p align="center">
  <b>Internal cheat for AssaultCube 1.3.0.2</b><br>
  ESP · Aimbot · God Mode · Infinite Ammo · No Recoil
</p>

---

## 🎯 Features

| Feature | Description |
|---------|-------------|
| **ESP** | 2D boxes, player names, health bars, snaplines, distance — color-coded by team |
| **Aimbot** | Aim-lock on enemy head while holding RMB, configurable FOV |
| **God Mode** | Infinite health & armor |
| **Infinite Ammo** | All weapons + reserves maxed |
| **No Recoil** | Completely removes weapon recoil |
| **In-Game Menu** | Toggle features on/off with a clean overlay menu |

## 🎮 Controls

| Key | Action |
|-----|--------|
| `INSERT` | Toggle cheat menu |
| `↑` / `↓` | Navigate menu items |
| `←` / `→` / `Enter` | Toggle or adjust values |
| `Right Mouse Button` | Hold to activate aimbot lock |
| `END` | Eject cheat (unload DLL) |

## 🔧 Build

### Requirements
- **CMake** ≥ 3.16
- **Visual Studio** (2019 / 2022 / 2025) with **C++ Desktop** workload
- Target: **x86 (Win32)** — AssaultCube is 32-bit

### Steps

```powershell
cd AssaultCubeCheat
cmake -A Win32 -B build
cmake --build build --config Release
```

Output:
```
build/Release/ACCheat.dll
build/Release/Injector.exe
```

## 🚀 Usage

1. Launch **AssaultCube 1.3.0.2**
2. Start a bot match
3. Inject `ACCheat.dll` using one of:
   - **Process Hacker 2** → Right-click `ac_client.exe` → Miscellaneous → Inject DLL
   - **Injector.exe** (place it next to `ACCheat.dll` and run)
4. Press **INSERT** in-game to open the cheat menu
5. Press **END** to eject when done

## 📁 Project Structure

```
AssaultCubeCheat/
├── CMakeLists.txt          Build system
├── cheat/
│   ├── pch.h               Common includes
│   ├── game.h              Offsets, Entity struct, W2S, angle math
│   ├── globals.h            Cheat settings & runtime state
│   ├── hooks.h / hooks.cpp  wglSwapBuffers trampoline hook
│   ├── render.h / render.cpp OpenGL 2D overlay rendering
│   ├── features.h / features.cpp  ESP, Aimbot, Misc features
│   ├── menu.h / menu.cpp    In-game toggle menu
│   └── dllmain.cpp          DLL entry point
└── injector/
    └── main.cpp             CreateRemoteThread DLL injector
```

## ⚙️ How It Works

- **Hook**: Trampoline hook on `wglSwapBuffers` (OpenGL) — renders the overlay every frame
- **ESP**: Projects 3D entity positions to 2D screen coordinates using the game's view matrix
- **Aimbot**: Calculates angles from local player head to target head, writes directly to view angles
- **Misc**: Writes health / ammo / recoil values in memory each frame

## 📜 License

This project is provided as-is for research and learning purposes.
