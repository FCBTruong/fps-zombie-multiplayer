# Multiplayer Shooting Prototype

> **Note**
> This project is a prototype and includes free and AI-generated assets. If you believe any asset infringes copyright, please let me know and I will remove it immediately.

## Demo

https://github.com/user-attachments/assets/07b79cbf-c5cf-44ad-b8af-0cb58e6d97df

<p align="center">
  <a href="https://youtu.be/O9IfP6dzKvo" target="_blank">
    <img src="https://img.shields.io/badge/Watch%20Full%20Video-YouTube-red?style=for-the-badge&logo=youtube" />
  </a>
</p>

**Download the game here:** [FpsGame.zip](https://github.com/FCBTruong/fps-zombie-multiplayer/releases/download/Play/FpsGame.zip)

## About the project
An online multiplayer FPS prototype inspired by **CrossFire**, **VALORANT**, and **Counter-Strike**.

The project focuses on the **technical side** of **shooter development**, including:

- **Game architecture**
- **Server-authoritative multiplayer**
- **Client prediction**
- **Lag compensation**
- **Anti-cheat foundations**
- **AI bots**

The goal is to build a **reliable**, **scalable foundation** for a **competitive shooting game**.

### Gameplay Features
- **Item System**: Equip and use different weapon types during matches.
- **Spike Plant Mode**: Objective-based mode focused on planting or defending the spike.
- **Zombie Mode**: A CrossFire-style infection mode where zombies hunt and infect human players.
- **Deathmatch Mode**: Fast-paced free-for-all combat focused on eliminations.

## Dedicated backend service 
This is for matchmaking/session/game services

[FCBTruong/fps-zombie-multiplayer-backend](https://github.com/FCBTruong/fps-zombie-multiplayer-backend)

---
## Table of Contents

1. [Technical Documentation](#technical-documentation)
2. [Technical Challenges Solved](#technical-challenges-solved)
3. [Limitations](#limitations)
4. [Quick Start](#quick-start)
5. [Setup](#setup)
6. [Build Dedicated Server](#build-dedicated-server)

---
## Technical Documentation

### System Architecture

![Architecture Class Diagram](./Docs/Asset/ClassCommon.drawio.svg)

[View detailed class diagram (Draw.io)](https://drive.google.com/file/d/1zdQQeo8DQTNyPRmSnwcuFQPTmwAZ9PAv/view?usp=sharing)

### Gameplay Flow
1. Pickup Item
2. Equip Item
3. Primary Action Flow (Shoot/Melee/Throw/Plant Spike)
4. Reload
5. Damage/Death
6. Movement (Walk/Jump/Crouch)
7. Become Hero
8. Match Flow
9. Spectator
10. Reconnect

#### Shooting Sequence Diagram
![Primary Action](./Docs/Asset/PrimaryAction.drawio.svg)

[View all sequence diagrams (Draw.io)](https://drive.google.com/file/d/1zdQQeo8DQTNyPRmSnwcuFQPTmwAZ9PAv/view?usp=sharing)

### Project Structure
![Folders diagram](./Docs/Asset/Folders.drawio.svg)

---
## Technical Challenges Solved
### 1. Recoil Prediction (Client-Side)
![Recoil Prediction](./Docs/Asset/RecoilPrediction.png)
- The client starts firing with a recoil seed and sends it to the server.
- Both client and server use the same seed and shot index to generate the same deterministic recoil/spread values.
- The shot index is incremented on both client and server for each shot.
- The client plays immediate local firing feedback (muzzle flash, recoil, tracer) without waiting for a server response.
- The server performs authoritative raycast/hit validation, applies damage, and replicates the hit result to other clients.
### 2. Latency
#### Lag Compensation (Hit Registration)

```cpp
// Test
NetEmulation.PktLagMin 100
NetEmulation.PktLagMax 100
NetEmulation.PktIncomingLagMin 100
NetEmulation.PktIncomingLagMax 100
```

* Server keeps ~200ms of hitbox history.
* Hit checks are server-side (client doesn't decide hits).
* On shot, server checks against a rewound target state based on shot time / latency.
* Helps shots register more consistently under ping.

Reference https://www.youtube.com/watch?v=6EwaW2iz4iA&t=6s

#### Interpolation (Remote Player Movement)

* Remote players are rendered with interpolation, not raw snapshots.
* The client buffers position/rotation snapshots and renders slightly behind real time.
* Movement is blended between two snapshots to reduce jitter from packet delay/jitter.
* This makes remote movement look smoother and more stable.

  #### Result

* Remote movement looks smoother.
* Fewer jitter spikes when packets arrive unevenly.
* Hit registration stays consistent with server-side validation.
### 3. Disconnect / Reconnect Handling

* When a player disconnects, their pawn is not destroyed and remains in the player slot.
* When the player reconnects, the server should find the corresponding pawn:

  * If the pawn is still alive, allow the controller to possess it again.
  * Otherwise, switch the player to Spectator Mode.

### 4. Late Join Synchronization

* The game provides a **15-second pre-match phase** for players to join before the match starts.
* If all players are ready before the timer ends, the match can start early.

### 5. Edge Case Handling

* Joining an in-progress match
* Race conditions (e.g., reload + shoot, plant spike + shoot, etc.)
* Duplicate actions caused by packet retries
* Out-of-order RPCs
* If a zombie is permanently killed by a hero, a reconnecting player should join as a spectator
---
## Limitations
- Some classes still have too much responsibility and need further separation/refactoring.
- Some configuration values are still hardcoded for testing purposes.
- Some folder contents are still mixed (different types of assets/files in the same place) and need better organization.
- The project has only been tested in small-scale sessions and has not been stress-tested for larger player counts.
- UI/UX polish is still in progress (feedback, transitions, effects, settings).
- Some systems still need better modularization and clearer interfaces for easier maintenance and feature expansion.
## Quick Start

1. Install **Unreal Engine 5.6.1** or later from the **Epic Games Launcher**.
2. Clone this repository (branch: `main`):

```bash
git clone -b main https://github.com/FCBTruong/fps-zombie-multiplayer.git
````

3. Open the project in Unreal Engine.

---

## Setup

1. Download the **Amazon GameLift plugin** and extract it into the `Plugins/` folder.

   Expected path:

```text
Plugins/GameLiftPlugin/
```

2. Plugin release (v3.1.0):

```text
https://github.com/amazon-gamelift/amazon-gamelift-plugin-unreal/releases/download/v3.1.0/amazon-gamelift-plugin-unreal-release-3.1.0.zip
```

3. Regenerate project files (if needed), then build the project.

---

## Production Deployment

### Build
Linux/Windows, Development/Shipping

See the detailed guide here:
* [Build Dedicated Server](./Docs/Build.md)


### Deploy AWS

1. Upload build to AWS

```text
aws gamelift upload-build `
  --name "FPSDemo-LinuxServer" `
  --build-version "1.0.0" `
  --build-root "C:\Workspace\GameStudio\FPSDemo\ArchivedBuilds\LinuxServer" `
  --operating-system AMAZON_LINUX_2023 `
  --server-sdk-version "5.4.0" `
  --region ap-southeast-1
```
