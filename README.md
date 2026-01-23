# CSPUB: Survival & Zombies

Dear Riot Games,  

Today is 27/09/2025 and I am working on my game project **CSPUB: Survival & Zombies**.  
This project is built with **Unreal Engine 5.6.1** using **C++**.  

I hope the recruiter team can see my value through this project.  
Thank you.  

## Setup
1. Download gamelift plugi and add it to folder Plugins (Expected: Plugins/GameLiftPlugin/)
https://github.com/amazon-gamelift/amazon-gamelift-plugin-unreal/releases/download/v3.1.0/amazon-gamelift-plugin-unreal-release-3.1.0.zip

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