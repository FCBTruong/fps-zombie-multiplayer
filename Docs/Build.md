## Build Dedicated Server

To build a dedicated server, you need the **Unreal Engine 5 source code** (not the launcher version).

- Download UE source (5.6.1):  
  https://github.com/EpicGames/UnrealEngine/releases/tag/5.6.1-release

After downloading the source, run:

- `Setup.bat`
- Generate project files

---

### Step 0 — Build Windows Editor (required for cooking on Windows)

```bat
cd /d "C:\Workspace\GameStudio\UE5.6.1\Engine\Build\BatchFiles"
Build.bat FPSDemoEditor Win64 Development -Project="C:\Workspace\GameStudio\FPSDemo\FPSDemo.uproject" -WaitMutex
````

---

### Step 1 — Build Linux Server Binary

```bat
cd /d "C:\Workspace\GameStudio\UE5.6.1\Engine\Build\BatchFiles"
Build.bat FPSDemoServer Linux Development -Project="C:\Workspace\GameStudio\FPSDemo\FPSDemo.uproject" -WaitMutex
```

---

### Step 2 — Cook, Stage, Pak, and Archive Linux Server

(Also builds if needed)

```bat
cd /d "C:\Workspace\GameStudio\UE5.6.1\Engine\Build\BatchFiles"
RunUAT.bat BuildCookRun ^
 -project="C:\Workspace\GameStudio\FPSDemo\FPSDemo.uproject" ^
 -server -noclient ^
 -serverplatform=Linux ^
 -serverconfig=Development ^
 -build ^
 -cook -stage -pak ^
 -archive -archivedirectory="C:\Workspace\GameStudio\FPSDemo\ArchivedBuilds"
```

---

## Build Windows Dedicated Server

```bat
cd /d "C:\Workspace\GameStudio\UE5.6.1\Engine\Build\BatchFiles"
RunUAT.bat BuildCookRun ^
 -project="C:\Workspace\GameStudio\FPSDemo\FPSDemo.uproject" ^
 -server -noclient ^
 -serverplatform=Win64 ^
 -serverconfig=Development ^
 -build ^
 -cook -stage -pak ^
 -archive -archivedirectory="C:\Workspace\GameStudio\FPSDemo\ArchivedBuilds"
```

---

## Build Windows Game Client

```bat
cd /d "C:\Workspace\GameStudio\UE5.6.1\Engine\Build\BatchFiles"
RunUAT.bat BuildCookRun ^
 -project="C:\Workspace\GameStudio\FPSDemo\FPSDemo.uproject" ^
 -platform=Win64 ^
 -clientconfig=Development ^
 -build ^
 -cook -stage -pak ^
 -archive -archivedirectory="C:\Workspace\GameStudio\FPSDemo\ArchivedBuilds"
```

---

## Output

The Linux server output will be generated in:

```text
Binaries/Linux/
```

Packaged builds will also be archived to:

```text
C:\Workspace\GameStudio\FPSDemo\ArchivedBuilds
```

---

## Package Docker Image

```bash
docker build -t fps-demo-server .
```

---

## Run

```bash
docker compose up -d
docker compose ps
docker logs -f fps-demo-server
```

```

Small fixes made:
- Cleaned grammar and wording
- Kept your commands unchanged
- Standardized section titles and step formatting
- Fixed “The out put will in” → clearer output section
```
