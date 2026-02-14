## Build Dedicated Server
To build dedicated server, you need to pull source code ue5
download here https://github.com/EpicGames/UnrealEngine/releases/tag/5.6.1-release
Run Setup.bat Genenerate file

Step 0 — Build Windows editor (required for cooking on Windows)
cd /d "C:\Workspace\GameStudio\UE5.6.1\Engine\Build\BatchFiles"
Build.bat FPSDemoEditor Win64 Development -Project="C:\Workspace\GameStudio\FPSDemo\FPSDemo.uproject" -WaitMutex

Step 1 — Build Linux server binary
cd /d "C:\Workspace\GameStudio\UE5.6.1\Engine\Build\BatchFiles"
Build.bat FPSDemoServer Linux Development -Project="C:\Workspace\GameStudio\FPSDemo\FPSDemo.uproject" -WaitMutex

Step 2 — Cook + stage + pak + archive server (also build if needed)
cd /d "C:\Workspace\GameStudio\UE5.6.1\Engine\Build\BatchFiles"
RunUAT.bat BuildCookRun ^
 -project="C:\Workspace\GameStudio\FPSDemo\FPSDemo.uproject" ^
 -server -noclient ^
 -serverplatform=Linux ^
 -serverconfig=Development ^
 -build ^
 -cook -stage -pak ^
 -archive -archivedirectory="C:\Workspace\GameStudio\FPSDemo\ArchivedBuilds"

## Build windows dedicated server
cd /d "C:\Workspace\GameStudio\UE5.6.1\Engine\Build\BatchFiles"
RunUAT.bat BuildCookRun ^
 -project="C:\Workspace\GameStudio\FPSDemo\FPSDemo.uproject" ^
 -server -noclient ^
 -serverplatform=Win64 ^
 -serverconfig=Development ^
 -build ^
 -cook -stage -pak ^
 -archive -archivedirectory="C:\Workspace\GameStudio\FPSDemo\ArchivedBuilds"

 # Build windows game client
 cd /d "C:\Workspace\GameStudio\UE5.6.1\Engine\Build\BatchFiles"
RunUAT.bat BuildCookRun ^
 -project="C:\Workspace\GameStudio\FPSDemo\FPSDemo.uproject" ^
 -platform=Win64 ^
 -clientconfig=Development ^
 -build ^
 -cook -stage -pak ^
 -archive -archivedirectory="C:\Workspace\GameStudio\FPSDemo\ArchivedBuilds"

The out put will  in Binaries/Linux/

## Package docker image
docker build -t fps-demo-server .

## Run
docker compose up -d
docker compose ps
docker logs -f fps-demo-server