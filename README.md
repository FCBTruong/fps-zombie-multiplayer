# CSPUB: Survival & Zombies

Dear Riot Games,  

Today is 27/09/2025 and I am working on my game project **CSPUB: Survival & Zombies**.  
This project is built with **Unreal Engine 5.6.1** using **C++**.  

I hope the recruiter team can see my value through this project.  
Thank you.  


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

  
The out put will  in Binaries/Linux/

## Package docker image
docker build -t fps-demo-server .