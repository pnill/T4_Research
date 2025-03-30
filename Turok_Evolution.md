# Turok Evolution (PC) Reverse Engineering

## Overview

This is anything that I deem non-engine specific related to Evolution, most of what I've researched I've done to achieve the Multiplayer modification I created. Each of these defined are likely to be found due to an actor specifically calling out a class by type. When an actor file is loaded it contains the type which gets associated to a class and the game calls a class constructor based on that which is how these classes were able to be identified or discovered.

There's a high posibility what I've defined as the CBaseAI constructor is actually some other generic routine, as it's referenced often in places I wouldn't expect. This is not a exhaustive list of potential classes/objects in Turok Evo, just the things I've been able to identify or have taken specific interest in.

### Classes / Functions

I hope to eventually create individual pages for each of the classes where I have members, structuctures, or virtual functions even partially documented or reverse engineered right now the only one defined is the CTurok4Game and others simply point to their object definitions that would've been used in ActorEd or their World Builder.

| Class / Function | Notes  |
| :--------------- | :----- |
| [CTurok4Game](https://github.com/T4_Research/Classes/CTurok4Game.md)      | From what I can tell this is a child of CGame/inherits it, each game built on Quagmire has it's own implementation of this which ultimately calls out to CGame in it's sub-functions/vtable functions. |
| SpawnPoint | Kind of as it sounds, handles spawning players or objects into a map, appears to be initialized by very specific ATR files. |
| FlagSpawnPoint | It may not be it's own class but it is loaded differently, however still uses the exactly same constructor as SpawnPoint |
| MPPickup | Seems to be related to only very specific pickups in multiplayer. Powerups more than likely but requires more research. |
| DMPlayer | I believe it's short for "DeathMatch Player" and it's only initialized/used during Multiplayer. It doesn't exist in the same form or name in any of the other games built on the engine that I've looked at. |
| CompyPlayer | Inherits DMPlayer, and defines it's vtable at 0x1a90 or a vtable related to DMPlayer there not exactly It's vtable. Seems to be primarily related to multiplayer modes where the player controls a 'compy' dinosaur vs a normal actor/human player. |
| BagActor | I believe this is related to specific game modes where a monkey is in a bag, this ultimately acts like a pickup |
| [Player](#player) | Likely the single player version of a player object vs DMPlayer, this is technically implemented in other games but obviously the structure would change or become turok specific. |
| DeadPlayer | As it sounds, this object/class gets spawned or created when a player dies they have a specific object for the 'dead' version of a player |
| EnemyAI | I'm assuming as it sounds, it controls any EnemyAI objects |
| TestEnemyAI | Needs further research, but probably similar to the above but some testing they were doing internally |
| HumanAI | I'm assuming this is for the 'friendly' AI that gets spawned throughout the game |
| RiderAI | This would possibly be any AI which is riding a dino or in a flight mode |
| AnimalAI | Very specific AI control for animals vs dinos or humans subclasses EnemyAI |
| GroupAnimalAI | Assuming this is constructing a "Group" of the AnimalAI objects, most likely creates several of them based on an array or such. |
| MountAI | Don't know much about it, inherits from EnemyAI |
| IndigenousAI | Inherits from AnimalAI |
| AquaticAI |  Inherits from AnimalAI |
| AquaticIndigenousAI | Inherits from AnimalAI |
| FlyingIndigenousAI | Inherits from IndigenousAI |
| PlesiosaurAI | Inherits from CBaseAI |
| Steracosaur | Inherits from EnemyAI |
| TrexAI | Inherits from EnemyAI |
| DeviceAI | Inherits from CBaseAI |
| BulletAI | Inherits from CBaseAI |
| CoverObject | Inherits from CBaseAI |
| [LevelExitObject](#levelexitobject) | Inherits from CBaseAI | 
| GeneratorObject | I believe this is used to respawn items at their spawn points, respawn AIs etc. Inherits from CBaseAI | 
| AlarmBox | Inherits from CBaseAI |
| TurrentObject | Inherits from CBaseAI, is not only turok specific I've seen it defined in the other games and was able to document some of it's vtable. The actual class is CTurret |
| AITarget | |
| RCDevice | Inherits from BaseAI, I'm under the impression this is related to SpiderMine object or SpiderMine may inherit it |
| EnemyWeapon | Not entirely sure how it's used, but I assume it's to assign weapons to AI |
| EnemyAccessory | As with the above, not entirely sure how it's used |
| AIMarker | |
| FallDeathRegion | Inherits CRegion |
| WarClubObject | Inherits WeaponObject or CWeapon, I never named it appropriately, has it's own VTable of actions. Would be the actual "Weapon" object the player interacts with when firing or etc. |
| BowObject | Like the above but for the Bow |
| [Weapon Wheel](#weaponwheel) | Acts like an inventory for weapons when it comes to players, defines what weapon they're holding, how much ammo it currently has, what upgrades are obtained, and what fire state a weapon is in. |
| Flag | Probably related to the capture the flag game mode, seems to inherit Pickup related classes but also has it's own VTable |
| Door | |
| Shotgun | |
| SuperShotgun | Appears to be a cut weapon that is no longer used, main Shotgun's actor refers to effects with a similar name. |
| DarkMatterCube | |
| RocketLauncher | |
| MiniGun | |
| TekBow | |
| CrossBow | |
| DinoBite | |
| GuidedDevice | |
| SpikedMine | |
| SniperPistol || 
| FlyingEnemyAI ||
| SwayingTreeAI ||
| StackManager ||
| Stackable ||
| FlameThrower ||
| [CTurokPickup](#turokpickup) |  Seems to act as a parent class to CPickUp which is not game specific, what's odd is 100 Bullets implements a CTurokPickup suggesting it may've been based on the same code base as Turok's version of quagmire. Not a lot of other code appears to be shared between the two. There are references to "Turok4" in Vexx but none of them are as specific as this where it's using the name of one of the classes. There are zero references like this in the RedStar build with a map. |
| SpikedMineObject ||
| DarkMatterObject ||
| GuidedDeviceObject ||
| NapalmGelObject || 
| [Rocket3Actor](#rocket3actor) ||
| SmartBullet ||
| RocketPteranadon ||
| Sky ||
| Lock2D ||
| TekWeapon ||
| GravityDirupter || 
| SwarmBore || 
| GameModeInfo ||
| CinemaCameraAttractor ||
| EmpathyBlast ||
| RidingRaptorAI ||
| MPGeneratorObject || 

### Linked Events
In addition to the above classes/objects and the fact that the spawning routine creates them there's also a large set of linked events that will end up being useful for RE later and are likely triggered by actor files, almost like a scripted event in some way.

| Linked Event       | Notes  |
| :----------------- | :----- |
| SetEnemyTurretType | |
| Change State | |
| SetPathFollowMode ||
| OpenCloseDoor ||
| LockUnlockDoor||
| GotoObject || 
| SetAIMarker ||
| SetAlertLevel ||
| ShootObject || 
| SetFollowTarget ||
| MakeAISound ||
| EnemyAIFireWeapon ||
| EnemyAIDropWeapon ||
| EnemyAIPickupWeapon ||
| EnemyAIWantsCover ||
| FlashPlayerView ||
| LightFlash ||
| AlarmBoxSet ||
| StackMngr ||
| EnemyAIAlwaysRun ||
| WarclubHitFront || 
| Nuke ||
| Stick ||
| WarclubHitBack ||
| EnemyAIAlwaysDetectPlayer ||
| Turret ||
| CreateStickyMesh ||
| TekWeaponCollide ||
| TurokSpawnParticle ||
| FE:TI ||
| FE:VCC ||
| FE:SMP ||
| EnemyAIInvul || 
| EnemyAIJabber ||
| DM:T ||
| EnemyAIIngoreTargets ||
| StartCinema ||
| EndCinema ||
| DecrementAmmo ||
| WaterFog ||
| EnemyAIRemoveBodyPart ||
| EnemyAIAlwaysWalk ||
| EnemyAIAIAvoid ||
| EnemyAISetLeash ||
| LoadingScreen || 
| DM:GM ||
| XBOX:DASH | Yes, this is in the PC version... |
| FE:InGame ||
| Display message ||
| QuitSinglePlayer ||
| StartSinglePlayer ||
| ResetSinglePlayerHealth ||
| EnemyAISetLookAt ||
| Teleport ||
| SupressDeathcam ||
| SwarmBoreParticle ||
| FE:StartLoadedGame ||
| FE:SaveWeaponState ||


### Turok4_debug.xbe

Figured this deserves it's own section in here as there are interesting things it's asserts expose, for instance within one routine it's possible to see how an actor would be referenced within an routine as it shows the debug string `pActor != 0`.

I haven't relied on this xbe as much as I should to determine specific things, the asserts only seem to relate to things I may already have symbols for via other game builds, but there's still some stuff to be learned from it like file paths as well as what I mentioned above about argument names. An example of that would be where CLevel.cpp lives `C:\\Projects\\NewTech\\Shared\\World\\Level.cpp`.

Some other things that can potentially be gleaned from this would be things such finding where functions have been inlined in the release PC build I don't believe they would've been inlined in this specific build due to otpimization being turned way down.

Another reference to a file which relates to Quagmire specifically would be the following path and error found in the game xbe:
```
int sub_1B042()
{
  int result; // eax

  result = debugStr(
             1,
             "C:\\Projects\\NewTech\\TnT\\Quagmire\\Engine\\XBox\\XBox_Engine.cpp",
             444,
             "FALSE",
             "not implemented on xbox !");
  if ( result )
    __debugbreak();
  return result;
}
```

The source that's leaked online contained the `TnT` folder but not `NewTech` so this gives us an idea at least of code we may have from online leaks of Quagmire that directly relate to Turok if we can find them referencing the `TnT` folder in a specific routine with their asserts.

Other potential things if another game or source ever leaked or was obtained look to be anything in the `NewTech\Shared\` folder which also has never leaked, this appears to contain quite a few of the base objects shared amongst the games built on Quagmire some examples are:'

##### World
* `World\Actor.cpp`
* `World\ActorMode.cpp`
* `World\ActorTree.cpp`
* `World\ActorTypeInfo.cpp`
* `World\ActorTypeList.cpp`
* `World\AlphaManager.cpp`
* `World\AmbientVolumemanager.cpp`
* `World\AmbientVolumeOctree.cpp`
* `World\Anim.cpp`
* `World\AnimLink.cpp`
* `World\AnimTimer.cpp`
* `World\AnimationCompressor.cpp`
* `World\Behaviour.cpp`
* `World\BounceBehaviour.cpp`
* `World\CGeometry.cpp`
* `World\Camera.cpp`
* `World\Collision.cpp`
* `World\CollisionGeometry.cpp`
* `World\Counters.cpp`
* `World\DynamicMeshBase.cpp`
* `World\EdgeGrabber.cpp`
* `World\Fonts.cpp`
* `World\Game.cpp`
* `World\GroundStepper.cpp`
* `World\HUD.cpp`
* `World\History.cpp`
* `World\HotPoint.cpp`
* `World\Inventory.cpp`
* `World\Level.cpp`
* `World\Light.cpp`
* `World\MachinePart.cpp`
* `World\MaterialManager.cpp`
* `World\MenuWidgets.cpp`
* `World\Mesh.cpp`
* `World\Morph.cpp`
* `World\NavData.cpp`
* `World\NavLink.cpp`
* `World\NavNode.cpp`
* `World\ParticleFollow.cpp`
* `World\Path.cpp`
* `World\PathFollower.cpp`
* `World\PathNode.cpp`
* `World\Performance.cpp`
* `World\PerformanceGroup.cpp`
* `World\Platform.cpp`
* `World\Region.cpp`
* `World\ScreenSprite.cpp`
* `World\ShadowManager.cpp`
* `World\SpecializedSpatialProbes.cpp`
* `World\SplittingPlanes.cpp`
* `World\SpringManager.cpp`
* `World\SpringSystem.cpp`
* `World\TextMessage.cpp`
* `World\Visability.cpp`
* `World\WaterMeshBase.cpp`
* `World\Widget.cpp`
* `World\app.cpp`
* `World\portal.cpp`

#### World\Video
* `World\Video\StreamingVideomanager.cpp`

#### World\Text

* `World\Text\Text.cpp`
* `World\Text\TextKeyValues.cpp`
* `World\Text\TextSubstitutions.cpp`

#### World\Sounds
* `World\Sounds\SoundInfo.cpp`
* `World\Sounds\SoundManager.cpp`

#### World\Post
* `World\Post\ChameleonEffect.cpp`
* `World\Post\PostProcessing.cpp`
* `World\Post\ShimmerEffect.cpp`
* `World\Post\SnapshotEffect.cpp`
* `World\Post\SuckHoleFullscreenEffect.cpp`
* `World\Post\WaterWarpFullScreenEffect.cpp`

#### World\MemoryUnit
* `World\MemoryUnit\MemoryUnitManager.cpp`

#### World\Materials
* `World\Materials\Material.cpp`
* `World\Materials\MaterialList.cpp`

#### World\Links
* `World\Links\Link.cpp`
* `World\Links\LinkCause.cpp`
* `World\Links\LinkConditional.cpp`
* `World\Links\LinkEvent.cpp`
* `World\Links\PostEffectLinkEvents.cpp`
* `World\Links\PredefinedLinkCauses.cpp`
* `World\Links\PredefinedLinkConditionals.cpp`
* `World\Links\PredefinedLinkEvents.cpp`

#### World\Levels
* `World\Levels\InstancePopulationTool.cpp`
* `World\Levels\LevelFragment.cpp`
* `World\Levels\ParticleContainer.cpp`
* `World\Levels\Pre-CacheManager.cpp`

#### World\Languages
* `World\Languages\Languages.cpp`

#### World\Game
* `World\Game\GameAllocator.cpp`

#### World\Fog
* `World\Fog\Fog.cpp`

#### World\Debugging
* `World\Debugging\ActorDebugMenu.cpp`
* `World\Debugging\CommonDebugMenu.cpp`
* `World\Debugging\ControllerTestDebugMenu.cpp`
* `World\Debugging\DebugDemon.cpp`
* `World\Debugging\DebugMarkers.cpp`
* `World\Debugging\DebugMessages.cpp`
* `World\Debugging\DemonDebugMenu.cpp`
* `World\Debugging\FakeJoystickInput.cpp`
* `World\Debugging\HistoryDebugMenu.cpp`
* `World\Debugging\LevelSelectDebugMenu.cpp`
* `World\Debugging\MemoryDebugMenu.cpp`
* `World\Debugging\ParticleDebugMenu.cpp`
* `World\Debugging\PerformanceDebugMenu.cpp`
* `World\Debugging\ProfileDebugMenu.cpp`
* `World\Debugging\Profiler.cpp`
* `World\Debugging\RenderDebugMenu.cpp`
* `World\Debugging\RunTimeWarningsDebugMenu.cpp`
* `World\Debugging\TimedDebugMessage.cpp`
* `World\Debugging\VariableEditWidgets.cpp`

#### World\Collision

* `World\Collision\CollisionFaceGrid.cpp`
* `World\Collision\CollisionOctree.cpp`
* `World\Collision\CollisionOptimizer.cpp`
* `World\Collision\CollisionPart.cpp`
* `World\Collision\SphereCollisionPart.cpp`

#### World\Cameras

* `World\Cameras\BlendedCamera.cpp`
* `World\Cameras\CameraBehaviour.cpp`
* `World\Cameras\CameraSupport.cpp`
* `World\Cameras\CircleBehavior.cpp`
* `World\Cameras\FreeFollowBehavior.cpp`
* `World\Cameras\PathBehavior.cpp`
* `World\Cameras\PointAttractBehavior.cpp`
* `World\Cameras\TerrainAvoidanceCone.cpp`
* `World\Cameras\ThirdPersonCamera.cpp`
* `World\Cameras\VectorBehavior.cpp`

#### World\Behaviours
* `World\Behaviours\ActorDrawInfo.cpp`
* `World\Behaviours\ActorKeySequence.cpp`
* `World\Behaviours\ActorSpecialities.cpp`
* `World\Behaviours\AutoSpin.cpp`
* `World\Behaviours\ChannelChangeBehaviour.cpp`
* `World\Behaviours\MoveBehaviour.cpp`
* `World\Behaviours\TurnBehaviour.cpp`

#### World\Actors
* `World\Actors\ActorCreation.cpp`
* `World\Actors\LightningBolt.cpp`
* `World\Actors\VideoActor.cpp`
* `World\Actors\WaterActor.cpp`

#### AAA_Lib

* `AAA_Lib\AudioInstance.cpp`
* `AAA_Lib\IIP_Manager.cpp`
* `AAA_Lib\ItemObject.cpp`
* `AAA_Lib\Layer2.cpp`
* `AAA_Lib\Layer3.cpp`
* `AAA_Lib\SampleObject.cpp`

#### Input

* `Input\InputManager.cpp`

#### Particle

* `Particle\Effect.cpp`
* `Particle\EffecSys.cpp`
* `Particle\Emit.cpp`
* `Particle\EmitSimple.cpp`
* `Particle\LoadParticle.cpp`
* `Particle\PartDef.cpp`
* `Particle\Particle.cpp`
* `Particle\ParticleFormulaHandler.cpp`
* `Particle\ParticleSystem.cpp`
* `Particle\ParticleSystemMemory.cpp`
* `Particle\RenderParticleTable.cpp`
* `Particle\SwooshManager.cpp`
* `Particle\SwooshSystem.cpp`

#### Support

* `Support\BitSttream.cpp`
* `Support\CTextFile.cpp`
* `Support\Calculator.cpp`
* `Support\CompiledTextFile.cpp`

This one specifically is interesting because I believe ATR, MTF, etc files are considered a CompiledTextFile.

* `Support\DebugI.cpp`
* `Support\EventTimer.cpp`
* `Support\Font.cpp`
* `Support\GeneralClock.cpp`
* `Support\Intersection.cpp`
* `Support\LoadedFileList.cpp`
* `Support\MemberProperties.cpp`
* `Support\Memory\MemoryBuckets.cpp`
* `Support\Parser.cpp`
* `Support\RealTimeProfiler.cpp`
* `Support\ResourceSystem\AnimationResourceManager.cpp`
* `Support\ResourceSystem\GeometryResourceManager.cpp`
* `Support\ResourceSystem\ResourceCacheManager.cpp`
* `Support\ResourceSystem\TextureResourceManager.cpp`
* `Support\StringPair.cpp`
* `Support\StringSearchTree.cpp`
* `Support\Strings\StringGroup.cpp`
* `Support\TextFileParser.cpp`
* `Support\Trajectory.cpp`
* `Support\chitarray.cpp`
* `Support\Iff.cpp`

#### System\XBOX
* `System\XBOX\first.cpp`
* `System\XBOX\system.cpp`

#### System\XBOX\Materials

* `System\XBOX\Materials\XBOXAmbientVolumeManager.cpp`
* `System\XBOX\Materials\XBOXMaterialManager.cpp`
* `System\XBOX\Materials\XBOXShadowManager.cpp`

#### System\XBOX\Render

* `System\XBOX\Render\DynamicMesh.cpp`
* `System\XBOX\Render\MeshLoader.cpp`
* `System\XBOX\Render\ParticleRender.cpp`
* `System\XBOX\Render\StaticMesh.cpp`
* `System\XBOX\Render\WaterMesh.cpp`
* `System\XBOX\Render\XBOXFogManager.cpp`
* `System\XBOX\Render\XboxAlphaManager.cpp`
* `System\XBOX\Render\XboxMorph.cpp`
* `System\XBOX\Render\XboxStreamingVideoManager.cpp`

#### System\XBOX\Render\Post

* `System\XBOX\Render\Post\MotionBlurFullScreenEffectXBOX.cpp`
* `System\XBOX\Render\Post\PrismEdgedBlurFullScreenEffectXBOX.cpp`
* `System\XBOX\Render\Post\RadialBlurFullScreenEffectXBOX.cpp`
* `System\XBOX\Render\Post\ShimmerEffectXBOX.cpp`
* `System\XBOX\Render\Post\SnapshotEffectXBOX.cpp`
* `System\XBOX\Render\Post\WaterWarpFullscreenEffectXBOX.cpp`


There are also references to the `TNT\x_files\` folder but this has already been leaked/obtained as it's part of the `TNT` folder vs being a part of the above mentioned `Shared` folder, the same goes for `TnT\Quagmire`.

Some references to Turok 4 specific code which is not shared are also found within the xbe referencing the path `NewTech\Turok4\Code\`

#### Code\
* `CheatMenu.cpp`
* `DebugControllerOptionMenu.cpp`
* `GameCause.cpp`
* `GameEvent.cpp`
* `TurokAllocator.cpp`
* `TurokGame.cpp`

#### Code\Objects
* `Objects\StickyMesh.cpp`
* `Objects\TurokCameraObject.cpp`
* `Objects\TurokCams.cpp`

#### Code\Objects\HUD
* `Objects\HUD\MultiplayerHUD.cpp`
* `Objects\HUD\ScrollingTextMessage.cpp`
* `Objects\HUD\SinglePlayerHUD.cpp`

#### Code\Objects\AI
* `Objects\AI\PlayerObject.cpp`
* `Objects\Ai\PathFollower.cpp`
* `Objects\Ai\Pickup.cpp`
* `Objects\Ai\TurokAIObject.cpp`

#### Code\Objects\AI\Weapon
* `Objects\AI\Weapon\CrossBow.cpp`
* `Objects\AI\Weapon\PteranadonGun.cpp`
* `Objects\AI\Weapon\RocketLauncher.cpp`
* `Objects\AI\Weapon\RocketLauncher.cpp`
* `Objects\AI\Weapon\SniperPistol.cpp`
* `Objects\AI\Weapon\TekBow.cpp`
* `Objects\AI\Weapon\TekWeapon.cpp`
* `Objects\AI\Weapon\WeaponManager.cpp`
* `Objects\AI\Weapon\WeaponObject.cpp`
* `Objects\AI\Weapon\WeaponWheelObject.cpp`

#### Code\Objects\AI\Weapon\Object
* `Objects\AI\Weapon\Object\GuidedDeviceObject.cpp`

#### Code\Objects\AI\Device
* `Objects\Ai\Device\Alarmbox.cpp`
* `Objects\Ai\Device\Generator.cpp`

#### Code\Objects\AI\Enemy
* `Objects\Ai\Enemy\EnemyAIObject.cpp`

#### Code\Objects\AI\Enemy\Weapons
* `Objects\AI\Enemy\Weapons\EnemyWeapon.cpp`

#### Code\Objects\AI\Enemy\Organic
* `Objects\Ai\Enemy\Organic\Stackable.cpp`
* `Objects\Ai\Enemy\Organic\SwayingTreeAI.cpp`

#### Code\Objects\AI\Enemy\WaterBased
* `Objects\Ai\Enemy\WaterBased\Plesiosaur.cpp`

#### Code\Objects\AI\Enemy\Humanoid
* `Objects\Ai\Enemy\Humanoid\HumanAI.cpp`

#### Code\Objects\AI\Enemy\Animal
* `Objects\Ai\Enemy\Animal\FlyingEnemyAI.cpp`
* `Objects\Ai\Enemy\Animal\GroupAnimalAI.cpp`

#### Code\Multiplayer
* `Multiplayer\DeathCameraBehavior.cpp`
* `Multiplayer\JoystickMonitor.cpp`
* `Multiplayer\MPGameManager.cpp`
* `Multiplayer\MPOptionMenu.cpp`
* `Mutliplayer\MPPickup.cpp`
* `Multiplayer\MPViewController.cpp`
* `Multiplayer\ScoreBoard.cpp`
* `Multiplayer\SpawnPolicy.cpp`
* `Multiplayer\TableWidget.cpp`
* `Multiplayer\TurokThirdPersonCam.cpp`

#### Code\Multiplayer\Modes
* `Mutliplayer\Modes\DeadPlayer.cpp`
* `Multiplayer\Modes\MPGameMode.cpp`
* `Multiplayer\Modes\MPPlayerObject.cpp`
* `Multiplayer\Modes\Team.cpp`

#### Code\Multiplayer\Modes\BagTag
* `Multiplayer\Modes\BagTag\BagTagMode.cpp`

#### Code\Multiplayer\Modes\CTF
* `Multiplayer\Modes\CTF\CTFGameMode.cpp`
* `Multiplayer\Modes\CTF\CTFTeam.cpp`
* `Multiplayer\Modes\CTF\Flag.cpp`

#### Code\Multiplayer\Modes\DM
* `Multiplayer\Modes\DM\DMGameMode.cpp`

#### Code\Multiplayer\Modes\DinoHunt
* `Multiplayer\Modes\DinoHunt\DinoHuntMode.cpp`
* `Multiplayer\Modes\DinoHunt\HuntResultsWidget.cpp`

#### Code\Multiplayer\Modes\LastMan
* `Multiplayer\Modes\LastMan\CompyPlayer.cpp`
* `Multiplayer\Modes\LastMan\LastManMode.cpp`

#### Codes\Multiplayer\Modes\Predator
* `Multiplayer\Modes\Predator\Predator.cpp`

#### Code\Multiplayer\Modes\SingleFlag
* `Multiplayer\Modes\SingleFlag\SingleFlagMode.cpp`

#### Code\Events
* `Events\GameEvents.cpp`

#### FrontEnd
* `FrontEnd\DebugMenu.cpp`



 ### Object Properties and Modes
Every object seems to have a list of properties that define specific things related to it's object, such as a follow distance, throw distance, max throw distance etc. As well as "Modes" which I sometimes refer to as "Actions".

These actions/modes can be anything from the way a weapon fires to a player crouching, using ChangeMode typically does impact both the animation and do the actual action meaning these are not just an animation table but some kind of control system for every actor instance.

### God Files

The ".god" files appear to be related to their "WorldBuilder" system for Quagmire and are utilized to compile the actual .ATR/.ATI files down the line I assume, one was seemingly accidentally included in the PC build. It only appears to define generic actors related to the game and has the following in it's header:
```
;------------------------------------------------------------------------------
;                       The .god file for T4
;------------------------------------------------------------------------------
;
;   Refer to Main.god for an overview of the .god files.
;   
;------------------------------------------------------------------------------

#include "Gods\Path.god"
#include "Gods\PathNode.god"

#include "Gods\NavigationLink.god"
#include "Gods\NavigationNode.god"

#include "Gods\Materials.god"

#include    "Gods\Shared\Causes.god"            ;   The shared causes
#include    "Gods\Shared\Events.god"            ;   The shared events
#include    "Gods\Shared\LinkConditionals.god"  ;   The shared link conditionals.
#include    "Gods\Causes.god"                   ;   The game-specific causes
#include    "Gods\Events.god"                   ;   The game-specific events
#include    "Gods\Conditionals.god"             ;   The game-specific conditionals

#include    "Gods\Collision.god"

#include "Gods\Mode.god"

#include "Gods\Shared\Actors.god"
#include "Gods\Shared\Cameras.god"

#include "Gods\FaceFlags.god"
```

It's unfortunate that we don't have any of these other "god" files as they seem like they would've given a ton more information around events, conditional events, and etc that were more game specific, we can still see where the game loads some of those things so we have a basic idea of what was defined in the .god file anyway.

It's possible these were utilized as actual header files in some way as a lot of the data does roughly translate to the object loading code specifically when loading properties/values and modes. Meaning the data we do have is extremely useful for it's comments or information related to types and such when it comes to each object defined.

A few examples of defined data in the included .god file can be seen below, I've tried searching Xbox and PS2 and GameCube .tre files to see if anything similar was accidentally leaked in other builds but that doesn't seem to be the case in any instance. I'm not going to cover every object defined in the `gameobjectdata.txt` but I will cover a few that are interesting or in the beginning of the file. The file itself if you have the PC version of the game installed can be found in `\data\levels\u_jcowlishaw\screens\VideoOptions\gameobjectdata.txt`

#### TurokAIObject
There are no references to this in the game's code that I can identify, there is however one single reference in an actor file located at `data\actors\users\mkyle\FakeGun\fakegun.atr`

`FakeGun.atr` appears to be a `Weapon Wheel` based on it's defined `ACTOR_CODE`.

```
;------------------------------------------------------------------------------
; Base AI
;------------------------------------------------------------------------------
*OBJECT = "TurokAIObject"
{
	*NAME	= "TurokAI"
	*INHERITS = "Object"

	*VARIABLES = 
	{
		*Combo = "TurokAIType"
		{
			*Usage			= "Protected"
			*DisplayName	=	"Turok AI Type"
			*Value			=	0
			*Comment		=	"The Turok AI type."

			*ComboList = 
			{
				"0 - None",
				"1 - Enemy",
				"2 - Player",
				"3 - Ally",
				"4 - Neutral Aggressive",
				"5 - Passive Aggressive",
				"6 - Passive Neutral"				
			}
		}

		*FLOAT = "VisionEyeHeight"
		{
			*Usage			= "Protected"
			*VALUE	= 1
			*DISPLAYNAME = "Vision Eye Height"
			*Comment = "Eye Height for vision test"
		}

		*FLOAT = "ShallowWaterHeight"
		{
			*Usage			= "Protected"
			*VALUE	= 1
			*DISPLAYNAME = "Shallow Water Height"
			*Comment = "The height at which ai is considered in shallow water"
		}

		*FLOAT = "AimAtHeight"
		{
			*Usage			= "Protected"
			*VALUE	= 1.0f
			*DISPLAYNAME = "Aim At Height"
			*Comment = "The height at which to aim at this AI"
		}

		*FLOAT = "AIRadius"
		{
			*Usage			= "Protected"
			*VALUE			= 0.5f
			*DISPLAYNAME	= "AI Radius"
			*Comment		= "The radius size of this AI"
		}

		*FLOAT = "FDMult"
		{
			*VALUE	= 1.0f
			*DISPLAYNAME = "Fire Damage Multiplier"
			*Min		 = 0.0
			*Comment = "1.0 is normal"
			*Comment = "0.0 is impervious to fire"
		}

		*Combo = "LTarg"
		{
			*DisplayName	=	"Lockable target?"
			*Value		=	0 
			*ComboList	=
			{
				"Nope",
				"Regular",
				"Primary"
			}
		}

		*FLOAT = "PointValue"
		{
			*Usage			= "Public"
			*VALUE			= 0.0f
			*DISPLAYNAME	= "Point Value"
			*Comment		= "The value of this Object. Can be used for scoring etc."
		}

		*BOOL = "IgnoreNav"
		{
			*Usage			= "Protected"
			*VALUE	= 0
			*DISPLAYNAME = "Ignore Nav Data"
			*Comment = "Ignore Nav Data"
		}


		*Group = "Multiplayer Scaling"
		{
			*BOOL = "MPSOnOff"
			{
				*VALUE	= "False"
				*DISPLAYNAME = "Enable"
				*Comment = "Use multiplayer scaling on this actor?"
			}
		
			*FLOAT = "MPSFar"
			{
				*VALUE	= 100.0f
				*DISPLAYNAME = "Far distance"
				*Comment = "The distance away from the view"
				*Comment = "that the scale will start."
			}

			*FLOAT = "MPSMid"
			{
				*VALUE	= 50
				*DISPLAYNAME = "Mid distance"
				*Comment = "The distance away from the view"
				*Comment = "that the scale will be biggest."
			}

			*FLOAT = "MPSNea"
			{
				*VALUE	= 10
				*DISPLAYNAME = "Near distance"
				*Comment = "The distance away from the view"
				*Comment = "that the scale will be back to normal."
			}

			*FLOAT = "MPSSca"
			{
				*VALUE	= 10.0f
				*DISPLAYNAME = "Scale multiplier"
				*Comment = "Mid distance scale."
			}
			
		}

		*Group = "Accessories"
		{
			*Comment = "Attached items"

			*FilePick = "AccessoryName1"
			{
				*DISPLAYNAME = "Accessory1"
				*Value			=	""
				*Directory      =   "$\Data\Actors"
				*Extension      =   "atr"
			}
			*String = "AccessoryPoint1"
			{
				*DisplayName	=	"Hotpoint1"
				*Value			=	""
				*Comment		=	"The hotpoint to attach the accessory to"
			}

			*FilePick = "AccessoryName2"
			{
				*DISPLAYNAME = "Accessory2"
				*Value			=	""
				*Directory      =   "$\Data\Actors"
				*Extension      =   "atr"
			}
			*String = "AccessoryPoint2"
			{
				*DisplayName	=	"Hotpoint2"
				*Value			=	""
				*Comment		=	"The hotpoint to attach the accessory to"
			}

			*FilePick = "AccessoryName3"
			{
				*DISPLAYNAME = "Accessory3"
				*Value			=	""
				*Directory      =   "$\Data\Actors"
				*Extension      =   "atr"
			}
			*String = "AccessoryPoint3"
			{
				*DisplayName	=	"Hotpoint3"
				*Value			=	""
				*Comment		=	"The hotpoint to attach the accessory to"
			}

		}

		*Group = "Path Speed Match"
		{
			*Comment = "Path Speed Match"

			*BOOL = "PSMONOFF"
			{
				*VALUE	= "False"
				*DISPLAYNAME = "Enable"
				*Comment = "Should this actor speed match with the player?"
			}

			*FLOAT = "PSMPOS"
			{
				*VALUE	= 30.0f
				*DISPLAYNAME = "Infont/Behind position (meters)"
				*Comment = "Offset that this actor will stay compared to the player. "
				*Comment = "i.e."
				*Comment = "If this value is 10.0f then this actor will try and stay 10 meters ahead of the player."
				*Comment = "If this value is -20.0f then this actor will try and stay 20 meters behind the player."
			}

			*FLOAT = "PSMACCEL"
			{
				*VALUE	= 0.15f
				*MIN	= 0.0f
				*DISPLAYNAME = "Accel"
				*Comment = "Rate at which this actor will accelerate to match get into position."
			}

			*FLOAT = "PSMDECEL"
			{
				*VALUE	= 0.15f
				*MIN	= 0.0f
				*DISPLAYNAME = "Decel"
				*Comment = "Rate at which this actor will Decelerate to match get into position."
			}
		}
	}
}
```

#### Rocket1Actor
The interesting thing about this one is I can't seem to find it defined in the binary anywhere meaning it must've not been compiled in, there is however a actor file in the game files which references this `ACTOR_CODE` located at `data\actors\users\brandg\weapons\ROCKET1\rocket1.atr`, this file however is not referenced in any other actor or actor instance file meaning it's never intended to be loaded into the game and is some cut projectile or weapon. I'm guessing projectile.

```
*OBJECT = "Rocket1Actor"
{
	*NAME = "Rocket1Actor"
	*INHERITS = "Actor"
	*TYPE = "OBJECT"
	*UseInActorEd = "True"
	*Variables	= 
	{
	        *FilePick           =   "ShellName"
        	{
	            *DisplayName    =   "Shell Name"
        	    *Value          =   ""
	            *Directory      =   "$\Data\Particle\"
        	    *Extension      =   "par"
	            *Extension      =   "eff"
			*Comment		=	"The particle used for the basic ordinance shell."
	        }
	        *FilePick           =   "ExplosionName"
        	{
	            *DisplayName    =   "Explosion Name"
        	    *Value          =   ""
	            *Directory      =   "$\Data\Particle\"
	            *Extension      =   "par"
	            *Extension      =   "eff"
			*Comment		=	"The particle used for the collision explosion."
	        }
	        *FilePick           =   "BallisticName"
        	{
			*DisplayName    =   "Ballistic Name"
			*Value          =   ""
			*Directory      =   "$\Data\Particle\"
			*Extension      =   "par"
			*Extension      =   "eff"
			*Comment		=	"The particle used for the travelling rocket."
	        }
		*Float		= "Speed"
		{
			*Value 		= 0.3f
			*DISPLAYNAME	= "Rocket Speed"
			*Comment	= "Velocity that the Rocket uses."
		}
		*Float		= "MaxTurn"
		{
			*Value 		= 0.2f
			*DISPLAYNAME	= "Max Turn Rate"
			*Comment	= "Maximum angle percentage a rocket can turn per update."
			*Comment	= "(1.0 == aim right at enemy . . . 0.0 == never turn his way)"
		}
	}
}
```

There's also Rocket2Actor, but there's nothing extremely notable about it.
```
*OBJECT = "Rocket2Actor"
{
	*NAME = "Rocket2Actor"
	*INHERITS = "Actor"
	*TYPE = "OBJECT"
	*UseInActorEd = "True"
}
```

#### Rocket3Actor

This one is interesting because it shares a lot of the same properties as Rocket1Actor except this one is actually referenced in the game code and an object available via the game's CreateActor functions, meaning the game code will recognize the actor code associated to it if a actor file defines it as it's ACTOR_CODE.

The actor file which defines it via it's `ACTOR_CODE` is `data\actors\users\brandg\weapons\rocket3\rocket3.atr`.

It does appear to get referenced by game files, specifically `precachelists` (PCL files) for serveral levels, but to my knowledge would have to be referenced in either an Actor Instance File (ATI) or Actor File (ATR) to actually be spawned or utilized in a level. Meaning it's either referenced/spawned directly via code somewhere vs a actor based file, or I'm unaware of some other method of loading it into the game if it's not an unused object.

```
*OBJECT = "Rocket3Actor"
{
	*NAME = "Rocket3Actor"
	*INHERITS = "Actor"
	*TYPE = "OBJECT"
	*UseInActorEd = "True"

	*Variables	= 
	{
	        *FilePick           =   "ExplosionName"
        	{
			*DisplayName    =   "Explosion Name"
			*Value          =   ""
			*Directory      =   "$\Data\Particle\"
			*Extension      =   "par"
			*Extension      =   "eff"
			*Comment		=	"The particle used for the SwarmBore Explosion."
	        }
	        *FilePick           =   "BoreActorName"
        	{
			*DisplayName    =   "Bore Actor Name"
			*Value          =   ""
			*Directory      =   "$\Data\Actors\"
			*Extension      =   "atr"
			*Comment		=	"The actor used to control the swarm bore."
	        }
		*Float		= "Speed"
		{
			*Value 		= 0.3f
			*DISPLAYNAME	= "Rocket Speed"
			*Comment	= "Velocity that the Rocket uses."
		}
		*Float		= "MaxTurn"
		{
			*Value 		= 0.2f
			*DISPLAYNAME	= "Max Turn Rate"
			*Comment	= "Maximum angle percentage a rocket can turn per update."
			*Comment	= "(1.0 == aim right at enemy . . . 0.0 == never turn his way)"
		}
	}
}
```

#### EnemyAIObject
As we know the game does define an object for this, and what's interesting is it appears to inherit TurokAIObject which I haven't found used in the game itself. Just documenting this may lead to me attempting to research the constructor for this further to see if something is mis-labeled and there's a constructor for TurokAIObject beneath it.

This provides a ton of information into how the AI operates as well as mode definitions and how the game might load this from a binary file such as an actor file. A lot of the properties, groups, modes, and etc. Are going to be loaded by the engine referencing a string in a table, meaning we should be able to reverse engineer how the engine handles each one of these properties and assigns it to the EnemyAIObject in code.

Ultimately, leading to getting a better defined idea of structures/members for both Actor and EnemyAIObjects.
```
;------------------------------------------------------------------------------
; Enemy AI
;------------------------------------------------------------------------------
*OBJECT = "EnemyAIObject"
{
	*NAME	= "EnemyAI"
	*INHERITS = "TurokAIObject"
	*TYPE	= "OBJECT"

	*AIMODES = 
	{
		Idle,
		AlertIdle,
		CrouchIdle,
		ProneIdle,
		RotateLeft,
		RotateRight,
		AlertRotateLeft,
		AlertRotateRight,
		StandingSleep,
		Walk,
		AlertWalk,
		CrouchWalk,
		ProneCrawl,
		Run,
		AlertRun,
		FireRun,
		FireStarter,
		FireDeath,
		Fall,
		ExaggeratedFall,
		FallDeath,
		Jump,
		Climb,
		ClimbEnd,
		HangClimb,
		Damaged,
		Death,
		PoisonDamage,
		PoisonDeath,
		ExplosionDeath,
		ExplosionDamage,
		ImplodeDeath,
		GravityHeld,
		Attack,
		SecondAttack,
		ThirdAttack,
		FourthAttack,
		FifthAttack,
		CrouchAttack,
		ProneAttack,
		UnderwaterIdle,
		UnderwaterSwim,
		WatersurfaceIdle,
		WatersurfaceSwim,
		SwimmingAttack,
		SquatIdle,
		Taunt,
		Talk,
		Special1,
		Special2,
		Special3,
		Special4
	}

	*VARIABLES = 
	{

		*Group = "Detection"
		{
			*Comment = "Sight, sound and smell detection parameters."

			*BOOL = "IGNOREPLAYER"
			{
				*VALUE	= "False"
				*DISPLAYNAME = "Ignore Player?"
				*Comment = "No detection.  Ignores player."
			}

			*BOOL = "ProvokeOnly"
			{
				*VALUE	= "False"
				*DISPLAYNAME = "Respond Only To Provoked"
				*Comment = "AI only selects a target when attacked by it."
			}

			*BOOL = "CONEVISION"
			{
				*VALUE	= "True"
				*DISPLAYNAME = "Cone Vision?"
				*Comment = "True for forward cone sight."
				*Comment = "False for 360 degree sight."
			}

		
			*FLOAT = "SIGHTRADIUS"
			{
				*VALUE	= 20
				*MIN	= 0
				*DISPLAYNAME = "Sight Radius"
				*DISPLAYASDISC = "255,0,0"
				*Comment = "Distance the AI can see."
			}

			*FLOAT = "SXZAngle"
			{
				*VALUE	= 90
				*MIN	= 0
				*DISPLAYNAME = "Cone XZ Angle"
				*Comment = "XZ Angle for cone vision."
			}

			*FLOAT = "SYAngle"
			{
				*VALUE	= 45
				*MIN	= 0
				*DISPLAYNAME = "Cone Y Angle"
				*Comment = "Y Angle for cone vision."
			}

			*FLOAT = "SOUNDRADIUS"
			{
				*VALUE	= 30
				*MIN	= 0
				*DISPLAYNAME = "Sound Radius"
				*DISPLAYASDISC = "205,120,30"
				*Comment = "Distance the AI can hear."
			}

			*FLOAT = "LostContactResetTime"
			{
				*VALUE			= -1.0
				*DISPLAYNAME	= "Lost Contact Reset Time"
			}

			*BOOL = "IgnAlBox"
			{
				*VALUE	= "false"
				*DISPLAYNAME = "Ignore Alarm Boxes?"
				*Comment = "True - ignores alarms"
				*Comment = "False - responds to alarms."
			}

			*BOOL = "OnlyPTarget"
			{
				*VALUE	= "false"
				*DISPLAYNAME = "Only Player Target"
				*Comment = "True - The player is the only actor that will be considered for targetting"
				*Comment = "False - Or not."
			}


			*BOOL = "HasPTarget"
			{
				*VALUE	= "false"
				*DISPLAYNAME = "Always Detects Player"
				*Comment = "True - Always detects player as a target, even if it can't normally detect him"
				*Comment = "False - Or not."
			}

			*BOOL = "HasSTarget"
			{
				*VALUE	= "false"
				*DISPLAYNAME = "Shooter is Target"
				*Comment = "True - Has shooter as a target, even if it can't normally detect him"
				*Comment = "False - Or not."
			}

		}

		*Group = "Movement"
		{
			*Comment = "Movement parameters."

			*FLOAT = "MovementSpeed"
			{
				*VALUE	= 2.2
				*MIN	= 0
				*DISPLAYNAME = "Movement Speed"
				*Comment = "Running Movement speed in m/s."
			}

			*FLOAT = "SlowMovementSpeed"
			{
				*VALUE	= 1.0
				*MIN	= 0
				*DISPLAYNAME = "Slow Movement Speed"
				*Comment = "Walking Movement speed in m/s."
			}

			*FLOAT = "TurningSpeed"
			{
				*VALUE	= 180
				*MIN	= 0
				*DISPLAYNAME = "Turning Speed"
				*Comment = "Turning speed in degrees/s."
			}

			*FLOAT = "PitchSpeed"
			{
				*VALUE	= 180
				*MIN	= 0
				*DISPLAYNAME = "Pitch Speed"
				*Comment = "Pitch speed in degrees/s."
			}

			*BOOL = "SlowTurn"
			{
				*VALUE	= "False"
				*DISPLAYNAME = "Slow Moving Turn?"
			}

			*BOOL = "AlwaysRun"
			{
				*VALUE	= "False"
				*DISPLAYNAME = "Always Run?"
			}

			*BOOL = "AlwaysWalk"
			{
				*VALUE	= "False"
				*DISPLAYNAME = "Always Walk?"
			}

			*BOOL = "CanCrawl"
			{
				*Usage			= "Protected"
				*VALUE	= "False"
				*DISPLAYNAME = "Can Crawl?"
				*Comment = "True if AI can use crawl links."
			}

			*BOOL = "DFTurrets"
			{
				*VALUE	= "false"
				*DISPLAYNAME = "Drop for Turrets?"
				*Comment = "True - drops to avoid turrets"
				*Comment = "False - dont drop for turrets."
			}

			*BOOL = "AIAvoid"
			{
				*VALUE	= "True"
				*DISPLAYNAME = "AI Avoidance?"
			}

			*BOOL = "CantBePushed"
			{
				*VALUE	= "false"
				*DISPLAYNAME = "Cant Be Pushed"
				*Comment = "True - AI cant be Force pushed"
				*Comment = "False - AI can be pushed."
			}

			*Group = "Climbing"
			{
				*BOOL = "CanClimb"
				{
					*Usage			= "Protected"
					*VALUE	= "False"
					*DISPLAYNAME = "Can Climb?"
					*Comment = "True if AI can scale climbable surfaces."
				}

				*BOOL = "CanMonkeyClimb"
				{
					*VALUE	= "False"
					*DISPLAYNAME = "Can Monkey Climb?"
					*Comment = "True if AI can use monkeybar climbable surfaces."
				}

				*FLOAT = "ClimbEndHeight"
				{
					*Usage			= "Protected"
					*VALUE			= 1.0
					*DISPLAYNAME	= "ClimbEndHeight"
				}
			}

			*Group = "Swimming"
			{
				*BOOL = "CanSwim"
				{
					*Usage			= "Protected"
					*VALUE	= "False"
					*DISPLAYNAME = "Can Swim?"
					*Comment = "True if AI can swim in water."
				}

				*FLOAT = "FastSwimmingSpeed"
				{
					*VALUE	= 3.0
					*MIN	= 0
					*DISPLAYNAME = "Fast Swimming Speed"
					*Comment = "Fast Swimming speed in m/s."
				}

				*FLOAT = "SlowSwimmingSpeed"
				{
					*VALUE	= 1.0
					*MIN	= 0
					*DISPLAYNAME = "Slow Swimming Speed"
					*Comment = "Slow Swimming speed in m/s."
				}
			}

		}

		*Group = "Leash"
		{
			*BOOL = "LEASHED"
			{
				*VALUE	= "False"
				*DISPLAYNAME = "Leashed to point?"
				*Comment = "True if AI stays within the leash radius"
				*Comment = "of its creation spot."
			}

			*FLOAT = "LEASHRADIUS"
			{
				*VALUE	= 50
				*MIN	= 0
				*DISPLAYNAME = "Leash Radius"
				*DISPLAYASDISC = "155,30,140"
			}

			*BOOL = "LeashedToRegion"
			{
				*VALUE	= "False"
				*DISPLAYNAME = "Leashed to region?"
				*Comment = "True if AI stays within the smallest region "
				*Comment = "surrounding it at creation."
			}

			*BOOL = "RegionTargets"
			{
				*VALUE	= "False"
				*DISPLAYNAME = "Ignore Targets?"
				*Comment = "True if AI ignores targets outside its leash"
			}

			*FLOAT = "FleeR"
			{
				*VALUE	= 20
				*MIN	= 0
				*DISPLAYNAME = "Flee Radius"
			}

		}

		*Group = "Cover"
		{

			*FLOAT = "COVERRADIUS"
			{
				*VALUE	= 40
				*MIN	= 0
				*DISPLAYNAME = "Cover Radius"
;				 *DISPLAYASDISC = "155,30,140"
				*Comment = "The max distance the AI will seek out a cover position."
			}

			*Int = "MinCoverShots"
			{
				*VALUE	= 1
				*MIN	= 1
				*DISPLAYNAME = "Min Shots"
			}

			*Int = "MaxCoverShots"
			{
				*VALUE	= 1
				*MIN	= 1
				*DISPLAYNAME = "Max Shots"
			}

			*BOOL = "CUCCover"
			{
				*VALUE	= "true"
				*DISPLAYNAME = "Use Circle Cover"
			}

			*BOOL = "CUUCover"
			{
				*VALUE	= "true"
				*DISPLAYNAME = "Use PopUp Cover"
			}

			*BOOL = "CUOCover"
			{
				*VALUE	= "true"
				*DISPLAYNAME = "Use PopOut Cover"
			}

		}


		*Group = "Logic"
		{
			*Comment = "AI Logic Parameters"

			*INT = "LogicInitialAttackChance"
			{
				*VALUE			= 1
				*MIN			= 0
				*DISPLAYNAME	= "Intial Attack Chance"
			}

			*INT = "LogicAttackChance"
			{
				*VALUE			= 1
				*MIN			= 0
				*DISPLAYNAME	= "Attack Chance"
			}

			*BOOL = "WANTSCOVER"
			{
				*VALUE	= "False"
				*DISPLAYNAME = "Seeks Cover?"
				*Comment = "True if AI seeks out cover when threatened."
			}

			*INT = "LogicInitialCoverChance"
			{
				*VALUE			= 0
				*MIN			= 0
				*DISPLAYNAME	= "Initial Cover Chance"
			}

			*INT = "LogicCoverChance"
			{
				*VALUE			= 0
				*MIN			= 0
				*DISPLAYNAME	= "Cover Chance"
			}

			*FLOAT = "BreachRadius"
			{
				*VALUE			= -1.0
				*DISPLAYNAME	= "Breach Radius"
				*Comment		= "Use -1 for no breach"
			}

			*BOOL = "AttackOnBreach"
			{
				*VALUE	= "False"
				*DISPLAYNAME = "Attack On Breach"
				*Comment = "True if AI switches to attack when player within the breach radius"
			}

			*FLOAT = "AttackResetTime"
			{
				*VALUE			= -1.0
				*DISPLAYNAME	= "Attack Reset Time"
				*Comment		= "The time in attack mode before a new attack logic is selected."
				*Comment		= "Use -1 for never resetting."
			}

			*FLOAT = "DamageLogicChangeAmount"
			{
				*VALUE			= -1.0
				*DISPLAYNAME	= "Damage Logic Change"
			}

			*FLOAT = "FearfulThreshhold"
			{
				*VALUE			= -1.0
				*DISPLAYNAME	= "Fearful Threshhold"
				*Comment		= "The health level below which the AI will be fearful."
			}

			*FLOAT = "CriticalThreshhold"
			{
				*VALUE			= -1.0
				*DISPLAYNAME	= "Critical Threshhold"
				*Comment		= "The health level below which the AI will sellect critical damage logic."
			}

			*Group = "Critical Logic"
			{
				*INT = "CritNoChange"
				{
					*VALUE			= 1
					*MIN			= 0
					*DISPLAYNAME	= "No Change Chance"
				}

				*INT = "CritFlee"
				{
					*VALUE			= 0
					*MIN			= 0
					*DISPLAYNAME	= "Flee Chance"
				}

				*INT = "CritRetreat"
				{
					*VALUE			= 0
					*MIN			= 0
					*DISPLAYNAME	= "Retreat Chance"
				}

				*INT = "CritKamikaze"
				{
					*VALUE			= 0
					*MIN			= 0
					*DISPLAYNAME	= "Kamikaze Chance"
				}

				*INT = "CritSurrender"
				{
					*VALUE			= 0
					*MIN			= 0
					*DISPLAYNAME	= "Surrender Chance"
				}

				*INT = "CritCover"
				{
					*VALUE			= 0
					*MIN			= 0
					*DISPLAYNAME	= "Cover Chance"
				}

				*INT = "CritProne"
				{
					*VALUE			= 0
					*MIN			= 0
					*DISPLAYNAME	= "Prone Chance"
				}

			}

			*Combo = "SGR"
			{
				*Usage			= "Protected"
				*DisplayName	=	"Sticky Grenade"
				*Value			=	0
				*ComboList		=
				{
					"No Change",
					"Flee",
					"Kamikaze"
				}
			}
		}


		*Group = "Attack"
		{
			*Comment = "Attacking parameters."
						
			*Group = "Primary Attack"
			{
				*FilePick = "WeaponName1"
				{
					*Usage			= "Protected"
					*DISPLAYNAME = "Weapon"
					*Value			=	""
					*Directory      =   "$\Data\Actors\EnemyWeapons"
					*Extension      =   "atr"
				}

				*BOOL = "UsePrimary1"
				{
					*Usage			= "Protected"
					*VALUE	= "True"
					*DISPLAYNAME = "Uses Primary Weapon"
				}

				*String = "WeaponPoint1"
				{
					*Usage			= "Protected"
					*DisplayName	=	"Hotpoint"
					*Value			=	""
					*Comment		=	"The hotpoint to attach the weapon to"
				}

				*FLOAT = "WeaponRange1"
				{
					*Usage			= "Protected"
					*VALUE	= 20
					*MIN	= 0
					*DISPLAYNAME = "Attack Range"
				}
			}

			*Group = "Second Attack"
			{
				*FilePick = "WeaponName2"
				{
					*Usage			= "Protected"
					*DISPLAYNAME = "Weapon"
					*Value			=	""
					*Directory      =   "$\Data\Actors\EnemyWeapons"
					*Extension      =   "atr"
				}

				*BOOL = "UsePrimary2"
				{
					*Usage			= "Protected"
					*VALUE	= "False"
					*DISPLAYNAME = "Uses Primary Weapon"
				}

				*String = "WeaponPoint2"
				{
					*Usage			= "Protected"
					*DisplayName	=	"Hotpoint"
					*Value			=	""
					*Comment		=	"The hotpoint to attach the weapon to"
				}

				*FLOAT = "WeaponRange2"
				{
					*Usage			= "Protected"
					*VALUE	= 20
					*MIN	= 0
					*DISPLAYNAME = "Attack Range"
				}
			}

			*Group = "Third Attack"
			{
				*FilePick = "WeaponName3"
				{
					*Usage			= "Protected"
					*DISPLAYNAME = "Weapon"
					*Value			=	""
					*Directory      =   "$\Data\Actors\EnemyWeapons"
					*Extension      =   "atr"
				}

				*BOOL = "UsePrimary3"
				{
					*Usage			= "Protected"
					*VALUE	= "False"
					*DISPLAYNAME = "Uses Primary Weapon"
				}

				*String = "WeaponPoint3"
				{
					*Usage			= "Protected"
					*DisplayName	=	"Hotpoint"
					*Value			=	""
					*Comment		=	"The hotpoint to attach the weapon to"
				}

				*FLOAT = "WeaponRange3"
				{
					*Usage			= "Protected"
					*VALUE	= 20
					*MIN	= 0
					*DISPLAYNAME = "Attack Range"
				}
			}

			*Group = "Fourth Attack"
			{
				*FilePick = "WeaponName4"
				{
					*Usage			= "Protected"
					*DISPLAYNAME = "Weapon"
					*Value			=	""
					*Directory      =   "$\Data\Actors\EnemyWeapons"
					*Extension      =   "atr"
				}

				*BOOL = "UsePrimary4"
				{
					*Usage			= "Protected"
					*VALUE	= "False"
					*DISPLAYNAME = "Uses Primary Weapon"
				}

				*String = "WeaponPoint4"
				{
					*Usage			= "Protected"
					*DisplayName	=	"Hotpoint"
					*Value			=	""
					*Comment		=	"The hotpoint to attach the weapon to"
				}

				*FLOAT = "WeaponRange4"
				{
					*Usage			= "Protected"
					*VALUE	= 20
					*MIN	= 0
					*DISPLAYNAME = "Attack Range"
				}

			}

			*Group = "Fifth Attack"
			{
				*FilePick = "WeaponName5"
				{
					*Usage			= "Protected"
					*DISPLAYNAME = "Weapon"
					*Value			=	""
					*Directory      =   "$\Data\Actors\EnemyWeapons"
					*Extension      =   "atr"
				}

				*BOOL = "UsePrimary5"
				{
					*Usage			= "Protected"
					*VALUE	= "False"
					*DISPLAYNAME = "Uses Primary Weapon"
				}

				*String = "WeaponPoint5"
				{
					*Usage			= "Protected"
					*DisplayName	=	"Hotpoint"
					*Value			=	""
					*Comment		=	"The hotpoint to attach the weapon to"
				}

				*FLOAT = "WeaponRange5"
				{
					*Usage			= "Protected"
					*VALUE	= 20
					*MIN	= 0
					*DISPLAYNAME = "Attack Range"
				}

			}

			*Group = "Close Range"
			{
				*Comment = "Close Range Attack"

				*INT = "CloseRangeChance"
				{
					*VALUE	= 25
					*MIN	= 0
					*DISPLAYNAME = "Chance"
					*Comment = "The chance the AI will select close range attack"
				}
				
				*FLOAT = "CloseRangeDist"
				{
					*VALUE	= 5
					*MIN	= 0
					*DISPLAYNAME = "Distance"
					*Comment = "The distance from target in close range attack"
				}
			}

			*Group = "Medium Range"
			{
				*Comment = "Medium Range Attack"

				*INT = "MediumRangeChance"
				{
					*VALUE	= 50
					*MIN	= 0
					*DISPLAYNAME = "Chance"
					*Comment = "The chance the AI will select medium range attack"
				}
				
				*FLOAT = "MediumRangeDist"
				{
					*VALUE	= 15
					*MIN	= 0
					*DISPLAYNAME = "Distance"
					*Comment = "The distance from target in medium range attack"
				}
			}

			*Group = "Long Range"
			{
				*Comment = "Long Range Attack"

				*INT = "LongRangeChance"
				{
					*VALUE	= 25
					*MIN	= 0
					*DISPLAYNAME = "Chance"
					*Comment = "The chance the AI will select long range attack"
				}
				
				*FLOAT = "LongRangeDist"
				{
					*VALUE	= 30
					*MIN	= 0
					*DISPLAYNAME = "Distance"
					*Comment = "The distance from target in long range attack"
				}
			}

			*BOOL = "IsSniper"
			{
				*VALUE	= "False"
				*DISPLAYNAME = "Sniper"
				*Comment = "The AI will not move from placed spot"
			}

			*INT = "CrouchAttackChance"
			{
				*VALUE	= 0
				*MIN	= 0
				*DISPLAYNAME = "Crouch Attack Chance"
				*Comment = "The chance out of 100 the AI will crouch when attacking"
			}

			*INT = "ProneAttackChance"
			{
				*VALUE	= 0
				*MIN	= 0
				*DISPLAYNAME = "Prone Attack Chance"
				*Comment = "The chance out of 100 the AI will go prone when attacking"
			}

			*BOOL = "KeepADis"
			{
				*VALUE			= "True"
				*DISPLAYNAME	= "Keep Attack Distance"
				*Comment		= "Keep distance away from target when in medium or long range attack"
			}

		}

		*Group = "Head Tracking"
		{
			*BOOL = "UseHeadTracking"
			{
				*VALUE	= "False"
				*DISPLAYNAME = "Head Tracking"
			}

			*Group = "Head"
			{
				*int = "HeadBoneIndex"
				{
					*Usage			= "Protected"
					*VALUE			= -1
					*DISPLAYNAME	= "BoneIndex"
				}

				*Combo = "HeadPitchAxis"
				{
					*Usage			= "Protected"
					*DisplayName	=	"Pitch Axis"
					*Value			=	2 
					*ComboList		=
					{
						"X+",
						"X-",
						"Y+",
						"Y-",
						"Z+",
						"Z-"
					}
				}

				*Combo = "HeadYawAxis"
				{
					*Usage			= "Protected"
					*DisplayName	=	"Yaw Axis"
					*Value			=	4
					*ComboList		=
					{
						"X+",
						"X-",
						"Y+",
						"Y-",
						"Z+",
						"Z-"
					}
				}

				*FLOAT = "HeadXFix"
				{
					*Usage		= "Protected"
					*VALUE	= 0
					*DISPLAYNAME = "X Fix"
				}

				*FLOAT = "HeadYFix"
				{
					*Usage		= "Protected"
					*VALUE	= 0
					*DISPLAYNAME = "Y Fix"
				}

				*FLOAT = "HeadZFix"
				{
					*Usage		= "Protected"
					*VALUE	= 0
					*DISPLAYNAME = "Z Fix"
				}

				*FLOAT = "HeadPitchMax"
				{
					*Usage		= "Protected"
					*VALUE	= 0
					*DISPLAYNAME = "Pitch Max"
				}

				*FLOAT = "HeadPitchMin"
				{
					*Usage		= "Protected"
					*VALUE	= 0
					*DISPLAYNAME = "Pitch Min"
				}

				*FLOAT = "HeadYawMax"
				{
					*Usage		= "Protected"
					*VALUE	= 0
					*DISPLAYNAME = "Yaw Max"
				}

				*FLOAT = "HeadYawMin"
				{
					*Usage		= "Protected"
					*VALUE	= 0
					*DISPLAYNAME = "Yaw Min"
				}

				*FLOAT = "HeadPitchCorr"
				{
					*Usage		= "Protected"
					*VALUE	= 0
					*DISPLAYNAME = "Pitch Correction"
				}

				*FLOAT = "HeadPPCorr"
				{
					*Usage		= "Protected"
					*VALUE	= 0
					*DISPLAYNAME = "Parent Pitch Correction"
				}

				*FLOAT = "HeadPYCorr"
				{
					*Usage		= "Protected"
					*VALUE	= 90
					*DISPLAYNAME = "Parent Yaw Correction"
				}

			}


			*Group = "Spine"
			{
				*int = "BackBoneIndex"
				{
					*Usage			= "Protected"
					*VALUE			= -1
					*DISPLAYNAME	= "BoneIndex"
				}

				*Combo = "BackPitchAxis"
				{
					*Usage			= "Protected"
					*DisplayName	=	"Pitch Axis"
					*Value			=	2 
					*ComboList		=
					{
						"X+",
						"X-",
						"Y+",
						"Y-",
						"Z+",
						"Z-"
					}
				}

				*Combo = "BackYawAxis"
				{
					*Usage			= "Protected"
					*DisplayName	=	"Yaw Axis"
					*Value			=	4
					*ComboList		=
					{
						"X+",
						"X-",
						"Y+",
						"Y-",
						"Z+",
						"Z-"
					}
				}

				*FLOAT = "BackXFix"
				{
					*Usage		= "Protected"
					*VALUE	= 0
					*DISPLAYNAME = "X Fix"
				}

				*FLOAT = "BackYFix"
				{
					*Usage		= "Protected"
					*VALUE	= 0
					*DISPLAYNAME = "Y Fix"
				}

				*FLOAT = "BackZFix"
				{
					*Usage		= "Protected"
					*VALUE	= 0
					*DISPLAYNAME = "Z Fix"
				}

				*FLOAT = "BackPitchMax"
				{
					*Usage		= "Protected"
					*VALUE	= 0
					*DISPLAYNAME = "Pitch Max"
				}

				*FLOAT = "BackPitchMin"
				{
					*Usage		= "Protected"
					*VALUE	= 0
					*DISPLAYNAME = "Pitch Min"
				}

				*FLOAT = "BackYawMax"
				{
					*Usage		= "Protected"
					*VALUE	= 0
					*DISPLAYNAME = "Yaw Max"
				}

				*FLOAT = "BackYawMin"
				{
					*Usage		= "Protected"
					*VALUE	= 0
					*DISPLAYNAME = "Yaw Min"
				}

				*FLOAT = "BackPitchCorr"
				{
					*Usage		= "Protected"
					*VALUE	= 0
					*DISPLAYNAME = "Pitch Correction"
				}

				*FLOAT = "BackPPCorr"
				{
					*Usage		= "Protected"
					*VALUE	= 0
					*DISPLAYNAME = "Parent Pitch Correction"
				}

				*FLOAT = "BackPYCorr"
				{
					*Usage		= "Protected"
					*VALUE	= 90
					*DISPLAYNAME = "Parent Yaw Correction"
				}

			}



		}

		*Group = "Tail Tracking"
		{
			*BOOL = "UseTailTracking"
			{
				*Usage			= "Protected"
				*VALUE			= "False"
				*DISPLAYNAME	= "Tail Tracking"
			}

			*Group = "Base"
			{
				*int = "BaseBoneIndex"
				{
					*Usage			= "Protected"
					*VALUE			= -1
					*DISPLAYNAME	= "BoneIndex"
				}

				*Combo = "BasePitchAxis"
				{
					*Usage			= "Protected"
					*DisplayName	= "Pitch Axis"
					*Value			= 3 
					*ComboList		=
					{
						"X+",
						"X-",
						"Y+",
						"Y-",
						"Z+",
						"Z-"
					}
				}

				*Combo = "BaseYawAxis"
				{
					*Usage			= "Protected"
					*DisplayName	= "Yaw Axis"
					*Value			= 5
					*ComboList		=
					{
						"X+",
						"X-",
						"Y+",
						"Y-",
						"Z+",
						"Z-"
					}
				}

				*FLOAT = "BaseXFix"
				{
					*Usage			= "Protected"
					*VALUE			= -90
					*DISPLAYNAME	= "X Fix"
				}

				*FLOAT = "BaseYFix"
				{
					*Usage			= "Protected"
					*VALUE			= 0
					*DISPLAYNAME	= "Y Fix"
				}

				*FLOAT = "BaseZFix"
				{
					*Usage			= "Protected"
					*VALUE			= -90
					*DISPLAYNAME	= "Z Fix"
				}

				*FLOAT = "BasePitchMax"
				{
					*Usage			= "Protected"
					*VALUE			= 90
					*DISPLAYNAME	= "Pitch Max"
				}

				*FLOAT = "BasePitchMin"
				{
					*Usage			= "Protected"
					*VALUE			= -90
					*DISPLAYNAME	= "Pitch Min"
				}

				*FLOAT = "BaseYawMax"
				{
					*Usage			= "Protected"
					*VALUE			= 180
					*DISPLAYNAME	= "Yaw Max"
				}

				*FLOAT = "BaseYawMin"
				{
					*Usage			= "Protected"
					*VALUE			= -180
					*DISPLAYNAME	= "Yaw Min"
				}

				*FLOAT = "BasePitchCorr"
				{
					*Usage			= "Protected"
					*VALUE			= -20
					*DISPLAYNAME	= "Pitch Correction"
				}
			}


			*Group = "NearBase"
			{
				*int = "NearBaseBoneIndex"
				{
					*Usage			= "Protected"
					*VALUE			= -1
					*DISPLAYNAME	= "BoneIndex"
				}

				*Combo = "NearBasePitchAxis"
				{
					*Usage			= "Protected"
					*DisplayName	= "Pitch Axis"
					*Value			= 3 
					*ComboList		=
					{
						"X+",
						"X-",
						"Y+",
						"Y-",
						"Z+",
						"Z-"
					}
				}

				*Combo = "NearBaseYawAxis"
				{
					*Usage			= "Protected"
					*DisplayName	= "Yaw Axis"
					*Value			= 5
					*ComboList		=
					{
						"X+",
						"X-",
						"Y+",
						"Y-",
						"Z+",
						"Z-"
					}
				}

				*FLOAT = "NearBaseXFix"
				{
					*Usage			= "Protected"
					*VALUE			= -90
					*DISPLAYNAME	= "X Fix"
				}

				*FLOAT = "NearBaseYFix"
				{
					*Usage			= "Protected"
					*VALUE			= 0
					*DISPLAYNAME	= "Y Fix"
				}

				*FLOAT = "NearBaseZFix"
				{
					*Usage			= "Protected"
					*VALUE			= -90
					*DISPLAYNAME	= "Z Fix"
				}

				*FLOAT = "NearBasePitchMax"
				{
					*Usage			= "Protected"
					*VALUE			= 20
					*DISPLAYNAME	= "Pitch Max"
				}

				*FLOAT = "NearBasePitchMin"
				{
					*Usage			= "Protected"
					*VALUE			= -20
					*DISPLAYNAME	= "Pitch Min"
				}

				*FLOAT = "NearBaseYawMax"
				{
					*Usage			= "Protected"
					*VALUE			= 30
					*DISPLAYNAME	= "Yaw Max"
				}

				*FLOAT = "NearBaseYawMin"
				{
					*Usage			= "Protected"
					*VALUE			= -30
					*DISPLAYNAME	= "Yaw Min"
				}

				*FLOAT = "NearBasePitchCorr"
				{
					*Usage			= "Protected"
					*VALUE			= 0
					*DISPLAYNAME	= "Pitch Correction"
				}
			}

			*Group = "NearTip"
			{
				*int = "NearTipBoneIndex"
				{
					*Usage			= "Protected"
					*VALUE			= -1
					*DISPLAYNAME	= "BoneIndex"
				}

				*Combo = "NearTipPitchAxis"
				{
					*Usage			= "Protected"
					*DisplayName	= "Pitch Axis"
					*Value			= 3 
					*ComboList		=
					{
						"X+",
						"X-",
						"Y+",
						"Y-",
						"Z+",
						"Z-"
					}
				}

				*Combo = "NearTipYawAxis"
				{
					*Usage			= "Protected"
					*DisplayName	= "Yaw Axis"
					*Value			= 5
					*ComboList		=
					{
						"X+",
						"X-",
						"Y+",
						"Y-",
						"Z+",
						"Z-"
					}
				}

				*FLOAT = "NearTipXFix"
				{
					*Usage			= "Protected"
					*VALUE			= -90
					*DISPLAYNAME	= "X Fix"
				}

				*FLOAT = "NearTipYFix"
				{
					*Usage			= "Protected"
					*VALUE			= 0
					*DISPLAYNAME	= "Y Fix"
				}

				*FLOAT = "NearTipZFix"
				{
					*Usage			= "Protected"
					*VALUE			= -90
					*DISPLAYNAME	= "Z Fix"
				}

				*FLOAT = "NearTipPitchMax"
				{
					*Usage			= "Protected"
					*VALUE			= 30
					*DISPLAYNAME	= "Pitch Max"
				}

				*FLOAT = "NearTipPitchMin"
				{
					*Usage			= "Protected"
					*VALUE			= -30
					*DISPLAYNAME	= "Pitch Min"
				}

				*FLOAT = "NearTipYawMax"
				{
					*Usage			= "Protected"
					*VALUE			= 30
					*DISPLAYNAME	= "Yaw Max"
				}

				*FLOAT = "NearTipYawMin"
				{
					*Usage			= "Protected"
					*VALUE			= -30
					*DISPLAYNAME	= "Yaw Min"
				}

				*FLOAT = "NearTipPitchCorr"
				{
					*Usage			= "Protected"
					*VALUE			= 0
					*DISPLAYNAME	= "Pitch Correction"
				}
			}

			*Group = "Tip"
			{
				*int = "TipBoneIndex"
				{
					*Usage			= "Protected"
					*VALUE			= -1
					*DISPLAYNAME	= "BoneIndex"
				}

				*Combo = "TipPitchAxis"
				{
					*Usage			= "Protected"
					*DisplayName	= "Pitch Axis"
					*Value			= 3 
					*ComboList		=
					{
						"X+",
						"X-",
						"Y+",
						"Y-",
						"Z+",
						"Z-"
					}
				}

				*Combo = "TipYawAxis"
				{
					*Usage			= "Protected"
					*DisplayName	= "Yaw Axis"
					*Value			= 5
					*ComboList		=
					{
						"X+",
						"X-",
						"Y+",
						"Y-",
						"Z+",
						"Z-"
					}
				}

				*FLOAT = "TipXFix"
				{
					*Usage			= "Protected"
					*VALUE			= -90
					*DISPLAYNAME	= "X Fix"
				}

				*FLOAT = "TipYFix"
				{
					*Usage			= "Protected"
					*VALUE			= 0
					*DISPLAYNAME	= "Y Fix"
				}

				*FLOAT = "TipZFix"
				{
					*Usage			= "Protected"
					*VALUE			= -90
					*DISPLAYNAME	= "Z Fix"
				}

				*FLOAT = "TipPitchMax"
				{
					*Usage			= "Protected"
					*VALUE			= 30
					*DISPLAYNAME	= "Pitch Max"
				}

				*FLOAT = "TipPitchMin"
				{
					*Usage			= "Protected"
					*VALUE			= -30
					*DISPLAYNAME	= "Pitch Min"
				}

				*FLOAT = "TipYawMax"
				{
					*Usage			= "Protected"
					*VALUE			= 30
					*DISPLAYNAME	= "Yaw Max"
				}

				*FLOAT = "TipYawMin"
				{
					*Usage			= "Protected"
					*VALUE			= -30
					*DISPLAYNAME	= "Yaw Min"
				}

				*FLOAT = "TipPitchCorr"
				{
					*Usage			= "Protected"
					*VALUE			= 0
					*DISPLAYNAME	= "Pitch Correction"
				}
			}
		}


		*String = "InitialState"
		{
			*DisplayName	=	"Initial State"
			*Value			=	""
			*Comment		=	"The initial state"
		}

		*String = "InitialMode"
		{
			*DisplayName	=	"Initial Mode"
			*Value			=	""
			*Comment		=	"The initial Mode"
		}

		*Combo = "InitialAlert"
		{
			*DisplayName	=	"Initial Alert Level"
			*Value			=	0 
			*ComboList		=
			{
				"None",
				"Low",
				"Medium",
				"High"
			}
		}

		*FLOAT = "DefTransBlend"
		{
			*VALUE			=	0.266667
			*DISPLAYNAME	=	"Default Blend"
			*Comment		=	"Default transition blend time"
		}

		*Group = "Wander/Search"
		{
			*FLOAT = "WTime"
			{
				*VALUE			=	-1.0
				*DISPLAYNAME	=	"Wander Time"
				*Comment		=	"The time to stay in wander before switching to patrol"
				*Comment		=	"Use -1 for never switching"
			}

			*FLOAT = "WPTime"
			{
				*VALUE			=	-1.0
				*DISPLAYNAME	=	"Wander Pause Time"
				*Comment		=	"The time to pause at wander destination before moving to the next one"
				*Comment		=	"Use -1 for never switching"
			}

		}

		*Group = "Death"
		{

			*BOOL = "ResetWhenDie"
			{
				*VALUE	= "False"
				*DISPLAYNAME = "Reset When Die"
				*Comment = "True if resets after dying."
			}

			*FLOAT = "RDelay"
			{
				*VALUE			=	-1.0
				*DISPLAYNAME	=	"Reset Delay"
				*Comment		=	"The time to delay before resetting"
				*Comment		=	"Use -1 for no delay"
			}

			*BOOL = "GibDeath"
			{
				*VALUE	= "False"
				*DISPLAYNAME = "Gib When Die"
				*Comment = "True if gibs on death."
			}

			*FilePick = "GibPart"
			{
				*DisplayName	=	"Gib Particle"
				*Value			=	""
				*Directory      =   "$\Data\Particle\"
				*Extension      =   "par"
				*Extension      =   "eff"
			}
		}

        *Group = "Sound"
        {
            *Sound = "LDSound"
            {
                *DisplayName    =   "Light Damage Sound"
            }
            *Sound = "MDSound"
            {
                *DisplayName    =   "Medium Damage Sound"
            }
            *Sound = "HDSound"
            {
                *DisplayName    =   "Heavy Damage Sound"
            }
            
        }



		*BOOL = "UseGroundTracking"
		{
			*Usage			= "Protected"
			*VALUE			= "False"
			*DISPLAYNAME	= "Ground Tracking"
			*COMMENT		= "AI aligns itself to the slope of the ground."
		}

		*BOOL = "CanBeSwarmBored"
		{
			*Usage			= "Protected"
			*VALUE	= "True"
			*DISPLAYNAME = "Can Be Swarm Bored?"
		}

		*Group = "Spawn Actors"
		{
			*Comment = "Actors to spawn when this enemy dies"

			*FilePick = "SpawnActor1"
			{
				*DISPLAYNAME = "SpawnActor1"
				*Value		 = ""
				*Directory   = "$\Data\Actors"
				*Extension   = "atr"
			}

			*FilePick = "SpawnActor2"
			{
				*DISPLAYNAME = "SpawnActor2"
				*Value		 = ""
				*Directory   = "$\Data\Actors"
				*Extension   = "atr"
			}

			*FilePick = "SpawnActor3"
			{
				*DISPLAYNAME = "SpawnActor3"
				*Value		 = ""
				*Directory   = "$\Data\Actors"
				*Extension   = "atr"
			}

			*FilePick = "SpawnActor4"
			{
				*DISPLAYNAME = "SpawnActor4"
				*Value		 = ""
				*Directory   = "$\Data\Actors"
				*Extension   = "atr"
			}

			*FilePick = "SpawnActor5"
			{
				*DISPLAYNAME = "SpawnActor5"
				*Value		 = ""
				*Directory   = "$\Data\Actors"
				*Extension   = "atr"
			}

			*FLOAT	  = "SpawnActor1Velocity"
			{
				*VALUE	= 3.0f
				*DISPLAYNAME = "Actor 1 Velocity"
				*Comment = "Speed that the first spawned actor moves away from the enemy"
			}

			*FLOAT	  = "SpawnActor2Velocity"
			{
				*VALUE	= 3.0f
				*DISPLAYNAME = "Actor 2 Velocity"
				*Comment = "Speed that the second spawned actor moves away from the enemy"
			}

			*FLOAT	  = "SpawnActor3Velocity"
			{
				*VALUE	= 3.0f
				*DISPLAYNAME = "Actor 3 Velocity"
				*Comment = "Speed that the third spawned actor moves away from the enemy"
			}

			*FLOAT	  = "SpawnActor4Velocity"
			{
				*VALUE	= 3.0f
				*DISPLAYNAME = "Actor 4 Velocity"
				*Comment = "Speed that the fourth spawned actor moves away from the enemy"
			}

			*FLOAT	  = "SpawnActor5Velocity"
			{
				*VALUE	= 3.0f
				*DISPLAYNAME = "Actor 5 Velocity"
				*Comment = "Speed that the fifth spawned actor moves away from the enemy"
			}


			*INT	  = "SpawnActor1Chance"
			{
				*VALUE	= 100
				*MAX	= 100
				*MIN	= 0
				*DISPLAYNAME = "Actor 1 Chance"
				*Comment = "The chance of the first spawned actor actually appearing"
			}

			*INT	  = "SpawnActor2Chance"
			{
				*VALUE	= 100
				*MAX	= 100
				*MIN	= 0
				*DISPLAYNAME = "Actor 2 Chance"
				*Comment = "The chance of the second spawned actor actually appearing"
			}

			*INT	  = "SpawnActor3Chance"
			{
				*VALUE	= 100
				*MAX	= 100
				*MIN	= 0
				*DISPLAYNAME = "Actor 3 Chance"
				*Comment = "The chance of the third spawned actor actually appearing"
			}

			*INT	  = "SpawnActor4Chance"
			{
				*VALUE	= 100
				*MAX	= 100
				*MIN	= 0
				*DISPLAYNAME = "Actor 4 Chance"
				*Comment = "The chance of the fourth spawned actor actually appearing"
			}

			*INT	  = "SpawnActor5Chance"
			{
				*VALUE	= 100
				*MAX	= 100
				*MIN	= 0
				*DISPLAYNAME = "Actor 5 Chance"
				*Comment = "The chance of the fifth spawned actor actually appearing"
			}
		}
	}

	*Protected	=	"Mass"
	*Protected	=	"Gravity"
	*Protected	=	"Density"
	*Protected	=	"Drag"
	*Protected	=	"Type"
	*Protected	=	"Collides"
	*Protected	=	"Touches"
	*Protected	=	"Regions"
	*Protected	=	"Platforms"
	*Protected	=	"smStep"
	*Protected	=	"smWall"
}
```

#### Player
`Player` is an interesting one as the comments in the file sort of refer to it as being an AI `PlayerAI` specifically, and the fact it inherits `TurokAIObject`.

Several interesting things are defined in the `AIMODES` and `VARIABLES` of this one, one example is `CheatFly` I've never attempted changing an DMPlayer or Player's object mode to this to determine if it does allow them to fly, I'm not sure if there is a 'fly' cheat in the retail game at all.

Some `VARIABLES` entries that are interesting are the `PLAYERTYPE` `COMBO` which lists the following:
* `NormalPlayer`
* `Raptor Ride`
* `Steracosaur ride`
* `Pterandon ride`

Meaning it seems to still utilize the same player object even in flight based modes.

Another interesting variable is the `Inventory Capacity` `Group` definition, I've always been under the impression that the Weapon Wheel determines this information, but there appears to be definitions within here like `MaxBowAmmo` and it tells us this is an integer and has the comment `Maximum number of arrows player can take` while setting a value of `20`.

Anything defined in the `VARIABLES` for a player that is prefixed with `PT_` seems to be flight mode specific including camera FOV, distance etc.

```
;------------------------------------------------------------------------------
; Player
;
; PlayerAI
;------------------------------------------------------------------------------
*OBJECT = "Player"
{
	*NAME	= "Player"
	*INHERITS = "TurokAIObject"
	*TYPE	= "OBJECT"
	*UseInActorEd = "True"

	*AIMODES = 
	{
		"Idle"					,
		"IdleNoControl"			,
		"Run"					,
		"Jump" 					,
		"JumpBack"				,
		"Fall" 					,
		"CheatFly" 				,
		"Shoved" 				,
									
		"LedgeHang" 			,
		"LedgeShimmy"			,
									
		"Ride" 					,
		"RideSteracosaurIdle"	,
		"RideSteracosaurMove"	,
		"RideSteracosaurDash"	,
								
		"Climb" 				,
		"HangClimb"				,
									 
		"WaterSurfaceIdle"		,
		"WaterSurfaceMove"		,
		"UnderwaterIdle"		,
		"UnderwaterMove"		,
		"SwimBurst"				,
		"LeapToShore"			,
									 
		"UsingTurret"			,
		"UsingRC"				,
									 
		"PteranadonSetup"	,
		"PteranadonIdle"	,
		"PteranadonFlyUp"	,
		"PteranadonFlyUpMed" 	,
		"PteranadonFlyUpFull"	,
		"PteranadonFlyDown"  	,
		"PteranadonFlyDownMed"	,
		"PteranadonFlyDownFull"	,
		"PteranadonFlyLeft"	,
		"PteranadonFlyRight"	,
		"PteranadonEdgeLeft"	,
		"PteranadonEdgeRight"	,
		"PteranadonFireGun"	,
		"PteranadonFireMissile"	,
		"PteranadonWorldCollsion",
		"PteranadonActorCollsion",

		"Dead",

		"RaptorRideIdle",
		"RaptorRideIdleNoControl",
		"RaptorRideRun",
		"RaptorRideJump",
		"RaptorRideFall"
	}

	*VARIABLES = 
	{
		*Combo = "PLAYERTYPE"
		{
			*DisplayName	=	"Player Type"
			*Value			=	0 
			*ComboList		=
			{
				"Normal player",
				"Raptor ride",
				"Steracosaur ride",
				"Pteranadon ride"
			}
		}
		*Group = "Inventory Capacity"
		{
			*Int = "MaxBowAmmo"
			{
				*DisplayName = "Max Bow Ammo"
				*Value = 20;
				*Comment = "Maximum number of arrows player can take"
			}
			*Int = "MaxTekBowAmmo"
			{
				*DisplayName = "Max TekBow Ammo"
				*Value = 20;
				*Comment = "Maximum number of tek arrows player can take"
			}
			*Int = "MaxPistolAmmo"
			{
				*DisplayName = "Max Pistol Ammo"
				*Value = 5;
				*Comment = "Maximum number of CLIPS of pistol slugs the player can take"
			}					
			*Int = "MaxShotgunAmmo"
			{
				*DisplayName = "Max Shotgun Ammo"
				*Value = 3;
				*Comment = "Maximum number of shotgun CLIPS the player can take"
			}
			*Int = "MaxMinigunAmmo"
			{
				*DisplayName = "Max Minigun Ammo"
				*Value = 5;
				*Comment = "Maximum number of minigun CLIPS the player can take"
			}					
			*Int = "MaxLauncherAmmo"
			{
				*DisplayName = "Max Rockets Ammo"
				*Value = 4;
				*Comment = "Maximum number of rocket CLIPS the player can take"
			}	
			*Int = "MaxGrenadeAmmo"
			{
				*DisplayName = "Max Grenades"
				*Value = 10;
				*Comment = "Maximum number of grenades the player can take"
			}					
			*Int = "MaxGuidedDeviceAmmo"
			{
				*DisplayName = "Max Guided Device Ammo (DON'T CHANGE)"
				*Value = 1;
				*Comment = "Leave this as 1, please"
			}					
			*Int = "MaxTechWeaponAmmo"
			{
				*DisplayName = "Max Tech Weapon Ammo"
				*Value = 4;
				*Comment = "Maximum number of Tech Weapon CLIPS the player can take"
			}		
			*Int = "MaxDarkMatterCubeAmmo"
			{
				*DisplayName = "Max Dark Matter cube Ammo (DON'T CHANGE)"
				*Value = 1;
				*Comment = "Leave this as 1, please"
			}					
		}
		*Group = "Standard Movement"
		{
			*FLOAT = "EYEHEIGHT"
			{
				*VALUE	= 1.5
				*MIN	= 0.1
				*MAX	= 50
				*DISPLAYNAME = "Eye Height"
			}
	
			*FLOAT = "MAXRUNSPEED"
			{
				*VALUE	= 8
				*MIN	= 1
				*MAX	= 50
				*DISPLAYNAME = "Run Speed"
			}
	
			*FLOAT = "MAXSTRAFESPEED"
			{
				*VALUE	= 8
				*MIN	= 1
				*MAX	= 50
				*DISPLAYNAME = "Strafe Speed"
			}
	
			*FLOAT = "JUMPVELOCITY"
			{
				*VALUE	= 12
				*MIN	= 0.1
				*MAX	= 50
				*DISPLAYNAME = "Jump Velocity"
			}

			*FLOAT = "TURNSPEED"
			{
				*VALUE	= 240
				*MIN	= 1
				*MAX	= 720
				*DISPLAYNAME = "Turn Speed"
			}

			*FLOAT = "LEDGESHIMMYSPEED"
			{
				*VALUE	= 0.25
				*MIN	= 0.01
				*MAX	= 10
				*DISPLAYNAME = "Ledge Shimmy Speed"
			}
		}

		*Group = "Crouch Movement"
		{
			*FLOAT = "CROUCHEYEHEIGHT"
			{
				*VALUE	= 1.2
				*MIN	= 0.1
				*MAX	= 50
				*DISPLAYNAME = "Crouch Eye Height"
			}
	
			*FLOAT = "MAXCROUCHRUNSPEED"
			{
				*VALUE	= 3
				*MIN	= 1
				*MAX	= 20
				*DISPLAYNAME = "Crouch Run Speed"
			}
	
			*FLOAT = "MAXCROUCHSTRAFESPEED"
			{
				*VALUE	= 3
				*MIN	= 1
				*MAX	= 20
				*DISPLAYNAME = "Crouch Strafe Speed"
			}
		}

		*Group = "Fall damage"
		{
			*FLOAT = "FALLDAMAGEMINSPEED"
			{
				*VALUE	= 30
				*MIN	= 1
				*MAX	= 200
				*DISPLAYNAME = "Min speed for damage"
			}

			*FLOAT = "FALLDAMAGEMAXSPEED"
			{
				*VALUE	= 80
				*MIN	= 1
				*MAX	= 200
				*DISPLAYNAME = "Max (100 dmg) speed"
			}
		}
	
		*Group = "Water interaction"
		{
			*FLOAT = "SWIMSPEED"
			{
				*VALUE	= 5
				*MIN	= 1
				*MAX	= 50
				*DISPLAYNAME = "Swim Speed"
			}

			*FLOAT = "DROWNINITIALTIME"
			{
				*VALUE	= 10.0
				*MIN	= 1.0
				*MAX	= 100.0
				*DISPLAYNAME = "Time to start drowning"
			}

			*FLOAT = "DROWNINTERVAL"
			{
				*VALUE	= 2.0
				*MIN	= 0.1
				*MAX	= 100.0
				*DISPLAYNAME = "Drown damage time interval"
			}

			*FLOAT = "DROWNDAMAGE"
			{
				*VALUE	= 5
				*MIN	= 1
				*MAX	= 500
				*DISPLAYNAME = "Drown damage"
			}
		}

		*Group = "Climbing"
		{
			*FLOAT = "CLIMB_SPEED"
			{
				*VALUE	= 8
				*MIN	= 1
				*MAX	= 20
				*DISPLAYNAME = "Climb Speed"
			}

			*FLOAT = "CLIMB_ACCEL"
			{
				*VALUE	= 0.8
				*MIN	= 0.1
				*MAX	= 5
				*DISPLAYNAME = "Acceleration"
			}

			*FLOAT = "CLIMB_DECEL"
			{
				*VALUE	= 0.2
				*MIN	= 0.1
				*MAX	= 5
				*DISPLAYNAME = "Deceleration"
			}

			*FLOAT = "CLIMB_HORIZVIEWANGLE"
			{
				*VALUE	= 120
				*MIN	= 0
				*MAX	= 180
				*DISPLAYNAME = "Horizontal View Angle"
			}
		}

		*Group = "Raptor-riding"
		{
			*FLOAT = "RAPTOR_EYEHEIGHT"
			{
				*VALUE	= 2.0
				*MIN	= 1
				*MAX	= 10
				*DISPLAYNAME = "Eye Height"
			}

			*FLOAT = "RAPTOR_MAXRUNSPEED"
			{
				*VALUE	= 14
				*MIN	= 1
				*MAX	= 50
				*DISPLAYNAME = "Run Speed"
			}
	
			*FLOAT = "RAPTOR_MAXSTRAFESPEED"
			{
				*VALUE	= 11
				*MIN	= 1
				*MAX	= 50
				*DISPLAYNAME = "Strafe Speed"
			}
	
			*FLOAT = "RAPTOR_JUMPVELOCITY"
			{
				*VALUE	= 15
				*MIN	= 0.1
				*MAX	= 50
				*DISPLAYNAME = "Jump Velocity"
			}

			*FLOAT = "RAPTOR_EYEZOFFSET"
			{
				*VALUE	= 0.0
				*MIN	= -10
				*MAX	= 10
				*DISPLAYNAME = "Eye Offset (Z axis)"
			}

			*FLOAT = "RAPTOR_TURNLAG"
			{
				*VALUE	= 0.035
				*MIN	= 0
				*MAX	= 10
				*DISPLAYNAME = "Raptor Turn Lag"
			}
		}

		*Group = "Steracosaur riding"
		{
			*FLOAT = "STERACOSAUR_CAMERAHEIGHT"
			{
				*VALUE	= 5.0
				*MIN	= 0
				*MAX	= 100
				*DISPLAYNAME = "Camera Height"
			}

			*FLOAT = "STERACOSAUR_CAMERADISTANCE"
			{
				*VALUE	= 10.0
				*MIN	= 0
				*MAX	= 100
				*DISPLAYNAME = "Camera - Distance behind Steracosaur"
			}

			*FLOAT = "STERACOSAUR_MAXRUNSPEED"
			{
				*VALUE	= 14
				*MIN	= 1
				*MAX	= 50
				*DISPLAYNAME = "Run Speed"
			}
	
			*FLOAT = "STERACOSAUR_MAXSTRAFESPEED"
			{
				*VALUE	= 11
				*MIN	= 1
				*MAX	= 50
				*DISPLAYNAME = "Strafe Speed"
			}

			*FLOAT = "STERACOSAUR_TURNSPEED"
			{
				*VALUE	= 60
				*MIN	= 1
				*MAX	= 720
				*DISPLAYNAME = "Turn Speed"
			}

			*FLOAT = "STERACOSAUR_DASHSPEED"
			{
				*VALUE	= 18
				*MIN	= 1
				*MAX	= 50
				*DISPLAYNAME = "Dash Speed"
			}

			*FLOAT = "STERACOSAUR_DASHTIME"
			{
				*VALUE	= 3
				*MIN	= 1
				*MAX	= 120
				*DISPLAYNAME = "Dash Time"
			}

			*FLOAT = "STERACOSAUR_DASHRECHARGETIME"
			{
				*VALUE	= 5
				*MIN	= 1
				*MAX	= 120
				*DISPLAYNAME = "Dash Recharge Time"
			}
		}
		
		*Group = "Pteranadon-riding"
		{

			*FLOAT = "PT_WODAM"
			{
					*VALUE	= 25.0
					*MIN	= 0.0
					*DISPLAYNAME = "World collision damage"
					*Comment = "How much damage will the player receive if there's a collision with the world."
			}

			*FLOAT = "PT_ACDAM"
			{
					*VALUE	= 10.0
					*MIN	= 0.0
					*DISPLAYNAME = "Actor collision damage "
					*Comment = "How much damage will the player receive if there's a collision with the actor."
			}

			*FLOAT = "PT_WOSTN"
			{
					*VALUE	= 30.0
					*MIN	= 0.0
					*DISPLAYNAME = "World collision stun"
					*Comment = "How much stun delay the player receives if there's a collision with the world."
			}

			*FLOAT = "PT_ACSTN"
			{
					*VALUE	= 20.0
					*MIN	= 0.0
					*DISPLAYNAME = "Actor collision stun"
					*Comment = "How much stun delay the player receives if there's a collision with another actor."
			}

			*FLOAT = "PT_FRSDACC"
				{
					*VALUE	= 2.0
					*MIN	= 0.5
					*MAX	= 10
					*DISPLAYNAME = "Fly forward acceleration"
			}

			*FLOAT = "PT_FRSDMAX"
				{
					*VALUE	= 8.0
					*MIN	= 0
					*MAX	= 20
					*DISPLAYNAME = "Fly forward max speed"
			}

			*FLOAT = "PT_FRSDMIN"
				{
					*VALUE	= 2.0
					*MIN	= 0
					*MAX	= 20
					*DISPLAYNAME = "Fly forward min speed"
			}

			*FLOAT = "PT_FRSDSTART"
				{
					*VALUE	= 4.5
					*MIN	= 0
					*MAX	= 20
					*DISPLAYNAME = "Fly forward start speed"
			}


			*FLOAT = "PT_FRHACC"
				{
					*VALUE	= 3.0
					*MIN	= 0.0
					*MAX	= 10
					*DISPLAYNAME = "Move horizontal accel"
			}

			*FLOAT = "PT_FRDACC"
				{
					*VALUE	= 3.0
					*MIN	= 0.0
					*MAX	= 10
					*DISPLAYNAME = "Move down accel"
			}

			*FLOAT = "PT_FRUACC"
				{
					*VALUE	= 3.0
					*MIN	= 0.0
					*MAX	= 10
					*DISPLAYNAME = "Move up accel"
			}

			*FLOAT = "PT_FRHDMP"
				{
					*VALUE	= 0.95
					*MIN	= 0.1
					*MAX	= 10
					*DISPLAYNAME = "Horizontal movement dampen"
			}

			*FLOAT = "PT_FRVDMP"
				{
					*VALUE	= 0.95
					*MIN	= 0.1
					*MAX	= 10
					*DISPLAYNAME = "Vertical movement dampen"
			}



			*FLOAT = "PT_FRFGUPSPD"
			{
					*VALUE	= 2.5
					*MIN	= 0.0
					*MAX	= 255
					*DISPLAYNAME = "Faux gravity up deceleration"
					*Comment = "This is the speed that the gravity will be applied in against an upward motion"
			}

			*FLOAT = "PT_FRFGDNSPD"
			{
					*VALUE	= 2.5
					*MIN	= 0.0
					*MAX	= 255
					*DISPLAYNAME = "Faux gravity down acceleration"
					*Comment = "This is the speed that the gravity will be applied towards a downward motion"
			}


			*FLOAT = "PT_FRFGUPAMNT"
			{
					*VALUE	= 10.0
					*MIN	= 0.0
					*MAX	= 255
					*DISPLAYNAME = "Faux gravity amount Up"
					*Comment = "Amount of faux gravity added when in trying to fly up"
			}

			*FLOAT = "PT_FRFGDNAMNT"
			{
					*VALUE	= 10.0
					*MIN	= 0.0
					*MAX	= 255
					*DISPLAYNAME = "Faux gravity amount Down"
					*Comment = "Amount of faux gravity added when in trying to fly up"
			}

			*FLOAT = "PT_FRFGUPBLN"
			{
					*VALUE	= 2.5
					*MIN	= 0.0
					*MAX	= 100
					*DISPLAYNAME = "Faux gravity blend Up to Down"
					*Comment = "Speed that faux gravity effect is blended from Up to Down"
			}

			*FLOAT = "PT_FRFGDNBLN"
			{
					*VALUE	= 2.5
					*MIN	= 0.0
					*MAX	= 100
					*DISPLAYNAME = "Faux gravity blend Down to Up"
					*Comment = "Speed that faux gravity effect is blended from Down to Up"
			}

			*FLOAT = "PT_FREDHACC"
			{
					*VALUE	= 6.0
					*MIN	= 0.0
					*MAX	= 10
					*DISPLAYNAME = "Edge mode horizontal accelerate"
			}

			*FLOAT = "PT_FREDVACC"
			{
					*VALUE	= 3.0
					*MIN	= 0.0
					*MAX	= 10
					*DISPLAYNAME = "Edge mode vertical accelerate"
			}

			*FLOAT = "PT_FRGR"
				{
					*VALUE	= 0.05
					*MIN	= 0.0
					*MAX	= 10
					*DISPLAYNAME = "Gravity value"
			}

			*FLOAT = "PT_FREDGR"
				{
					*VALUE	= 0.1
					*MIN	= 0.0
					*MAX	= 10
					*DISPLAYNAME = "Edge mode gravity value"
			}


		
			*FLOAT = "PT_LPWAV"
		        {
		            *VALUE  = 55
					*MIN	= 0
					*MAX	= 90
		            *DISPLAYNAME = "Degrees player can drift to sides."
			}

			*Group = "Weapon settings"
			{
				*FLOAT = "PT_WEAMGD"
				{
					*VALUE	= 0.25
					*MIN	= 0.0
					*DISPLAYNAME = "Machine Gun Delay"
					*Comment = "Delay between shots."
				}			
				*FLOAT = "PT_WEAMSD"
				{
					*VALUE	= 1.0
					*MIN	= 0.0
					*DISPLAYNAME = "Missile Delay"
					*Comment = "Delay between shots."
				}			
				*FLOAT = "PT_WEAMSSPD"
				{
					*VALUE	= 200.0
					*MIN	= 0.0
					*DISPLAYNAME = "Missile Speed"
					*Comment = "Speed at which missiles travel"
				}
				*FLOAT = "PT_WEAMGDMG"
				{
					*VALUE = 100.0
					*MIN = 0.0
					*DISPLAYNAME = "Machine Gun Damage Modifier" 
					*Comment = "Set this to modify the amount of damage the machine gun does. "
					*Comment = "E.g., 200% means double the damage"
				}
				*FLOAT = "PT_WEAMSDMG"
				{
					*VALUE = 100.0
					*MIN = 0.0
					*DISPLAYNAME = "Missile Damage Modifier" 
					*Comment = "Set this to modify the amount of damage the missiles do."
					*Comment = "E.g., 200% means double the damage, 50% halves the damage"
				}
					
			}

			*Group = "Rider settings"
			{
				*FilePick = "PT_RiderName"
				{
					*Usage			=	"Protected"
					*DISPLAYNAME	=	"Rider Actor"
					*Value			=	""
					*Directory      =   "$\Data\Actors\Players\Pteranadon\Riders"
					*Extension      =   "atr"
				}

				*String = "PT_RiderPoint"
				{
					*Usage			=	"Protected"
					*DisplayName	=	"Rider HotPoint"
					*Value			=	""
					*Comment		=	"The hotpoint to attach the rider to"
				}
			}
	
			*Group = "Camera settings"
			{
				*Group = "1st person"
				{
					*FLOAT = "PT_CAM1FOV"
					{
						*VALUE	= 50.0
						*MIN	= 0.0
						*DISPLAYNAME = "FOV"
						*Comment = "Field of View, (60degrees is about normal)."
					}
				
					*FLOAT = "PT_CAM1YPB"
					{
						*VALUE	= 1.0
						*MIN	= 0.0
						*DISPLAYNAME = "Y pos on bird"
						*Comment = "Y offset of the camera."
						*Comment = "i.e How high/low you want the camera to be sat on the bird."
					}
										
					*FLOAT = "PT_CAM1ZPB"
					{
						*VALUE	= 0.0
						*DISPLAYNAME = "Z pos on bird"
						*Comment = "Z offset of the camera."
						*Comment = "i.e How far along the bird you want the camera to be positioned."
					}
					
					*FLOAT = "PT_CAM1CROLL"
					{
						*VALUE	= 0.25
						*MIN	= 0.0
						*MAX	= 1
						*DISPLAYNAME = "Camera roll with turn effect"
					}					
					
				}

				*Group = "3rd person"
				{
					*FLOAT = "PT_CAM3FOV"
					{
						*VALUE	= 60.0
						*DISPLAYNAME = "FOV"
						*Comment = "Field of View, (60degrees is about normal)."
					}
				
					*FLOAT = "PT_CAM3ETB"
					{
						*VALUE	= 8.0
						*MIN	= 0.0
						*DISPLAYNAME = "Eye to bird distance"
						*Comment = "How far back the camera eye is placed to the bird."
					}

					*FLOAT = "PT_CAM3EL"
					{
						*VALUE	= 10.0
						*MIN	= 0.0
						*MAX	= 10.0
						*DISPLAYNAME = "Eye lag time"
					}
	
					*FLOAT = "PT_CAM3LAL"
					{
						*VALUE	= 10.0
						*MIN	= 0.0
						*MAX	= 10.0
						*DISPLAYNAME = "LookAt lag time"
					}


					*FLOAT = "PT_CAM3EAC"
					{
						*VALUE	= 10.0
						*MIN	= 0.0
						*DISPLAYNAME = "Eye corrdior pos adjust"
						*Comment = "Based on the players vertical position within the corridor,"
						*Comment = "the camera eye pos will be adjust by X amount."
					}
	
					*FLOAT = "PT_CAM3LAC"
					{
						*VALUE	= 10.0
						*MIN	= 0.0
						*DISPLAYNAME = "LookAt corrdior pos adjust"
						*Comment = "Based on the players vertical position within the corridor,"
						*Comment = "the camera LookAt pos will be adjust by X amount."
					}
					
					*FLOAT = "PT_CAM3CROLL"
					{
						*VALUE	= 0.25
						*MIN	= 0.0
						*MAX	= 1
						*DISPLAYNAME = "Camera roll with turn effect"
					}						
				}
				*Group = "Levy Cam"
				{
				
					*FLOAT = "PT_CAMLFOV"
					{
						*VALUE	= 60.0
						*DISPLAYNAME = "FOV"
						*Comment = "Field of View, (60degrees is about normal)."
					}
				
					*FLOAT = "PT_CAMLETB"
					{
						*VALUE	= 8.0
						*MIN	= 0.0
						*DISPLAYNAME = "Eye to bird distance"
						*Comment = "How far back the camera eye is placed to the bird."
					}

					*FLOAT = "PT_CAMLCLA"
					{
						*VALUE	= 2.0
						*MIN	= 0.0
						*DISPLAYNAME = "LookAt in front of Pteranadon"
						*Comment = "How many meters to look ahead of the player/pteranadon."
					}

					*FLOAT = "PT_CAMLEL"
					{
						*VALUE	= 2.5
						*MIN	= 0.0
						*MAX	= 10.0
						*DISPLAYNAME = "Eye lag time"
						*Comment = "Delay Eye has catching up when player moves.."
					}
	
					*FLOAT = "PT_CAMLLAL"
					{
						*VALUE	= 10.0
						*MIN	= 0.0
						*MAX	= 10.0
						*DISPLAYNAME = "LookAt lag time"
						*Comment = "Delay LookAt has catching up when player moves.."
					}

					*FLOAT = "Pt_Camlylaoff"
					{
						*VALUE	= 4.0
						*DISPLAYNAME = "Y offset on LookAt"
						*Comment = "LookAt Y offset, you can position the bird vertically on screen with this value"
					}

					*FLOAT = "PT_CAMLYEYOFF"
					{
						*VALUE	= 2.0
						*DISPLAYNAME = "Y offset on Eye"
						*Comment = "Eye Y offset, you can control the vertical angle the bird is viewed from with this value"
					}

					*FLOAT = "PT_CAMLYLASW"
					{
						*VALUE	= 1.0
						*DISPLAYNAME = "LookAt X swing"
						*Comment = "When the player moves horizontally this is the amount the LookAt will swing horizonatlly."
					}

					*FLOAT = "PT_CAMLYEYSW"
					{
						*VALUE	= 4.0
						*DISPLAYNAME = "Eye X swing"
						*Comment = "When the player moves horizontally this is the amount the Eye will swing horizonatlly."
					}
					
					*FLOAT = "PT_CAMLCROLL"
					{
						*VALUE	= 0.65
						*MIN	= 0.0
						*MAX	= 1
						*DISPLAYNAME = "Camera roll with turn"
					}				
					
					*FLOAT = "PT_CAMLPROLL"
					{
						*VALUE	= 1.0
						*MIN	= 0.0
						*MAX	= 1
						*DISPLAYNAME = "Player roll with turn"
					}				
					
				
					*INT   = "PT_CAMLCHLAG"
					{
						*VALUE	= 10
						*MIN	= 0
						*MAX	= 255
						*DISPLAYNAME = "CrossHair lag frames"
						*Comment = "How many frames lag on the crosshair position."
						
					}
				}
			}
            *Group = "Sound"
            {
                *Sound = "FltLeftSnd"
                {
                    *DisplayName    =   "Flight Whoosh Left"
                }
                *Sound = "FltRightSnd"
                {
                    *DisplayName    =   "Flight Whoosh Right"
                }
                *Sound = "FltSpeedSnd"
                {
                    *DisplayName    =   "Speed whoosh sound"
                }
                
                *Sound = "FltLockOnSnd"
                {
                    *DisplayName    =   "Missile lock-on sound"
                }
                
                *Sound = "FltHitWorldSnd"
                {
                    *DisplayName    =   "Environment collision sound"
                }
                
                *Sound = "FltWaterSkimSnd"
                {
                    *DisplayName    =   "Water skimming sound"
                }
                
                *Sound = "FltNoMissileSnd"
                {
                    *DisplayName    =   "Out-of-missiles sound"
                }
                
                *Sound = "FltMissileFireSnd"
                {
                    *DisplayName    =   "Fire missile sound"
                }
                
            }

		}		

		*Group = "Sound"
        {
            *Sound = "FootSnd"
            {
                *DisplayName    =   "Footstep"
            }
            *Sound = "JmpSnd"
            {
                *DisplayName    =   "Jump"
            }
            *Sound = "JmpLndSnd"
            {
                *DisplayName    =   "Jump Land"
            }
            *Sound = "TurnSnd"
            {
                *DisplayName    =   "Turn"
            }
            *Sound = "SurfaceSwimSnd"
            {
                *DisplayName    =   "Surface Swim"
            }
            *Sound = "UnderwaterSwimSnd"
            {
                *DisplayName    =   "Underwater Swim"
            }
            *Sound = "EnterWaterSnd"
            {
                *DisplayName    =   "Enter Water"
            }
            *Sound = "ExitWaterSnd"
            {
                *DisplayName    =   "Exit Water"
            }
            *Sound = "WaterSurfaceGoodSnd"
            {
                *DisplayName    =   "Water Surface (Good)"
            }
            *Sound = "WaterSurfaceBadSnd"
            {
                *DisplayName    =   "Water Surface (Bad)"
            }
            *Sound = "DrownSnd"
            {
                *DisplayName    =   "Drown"
            }
            *Sound = "DrownDeathSnd"
            {
                *DisplayName    =   "Drown Death"
            }
            *Sound = "FallingSnd"
            {
                *DisplayName    =   "Fall"
            }
            *Sound = "ClimbSnd"
            {
                *DisplayName    =   "Climb"
            }
            *Sound = "WadeSnd"
            {
                *DisplayName    =   "Wade thru water"
            }
            *Sound = "PainLightSnd"
            {
                *DisplayName    =   "Pain (light)"
            }
            *Sound = "PainMediumSnd"
            {
                *DisplayName    =   "Pain (medium)"
            }
            *Sound = "PainHeavySnd"
            {
                *DisplayName    =   "Pain (heavy)"
            }
            *Sound = "PainPoisonChokeSnd"
            {
                *DisplayName    =   "Pain (poisoned choke)"
            }
            *Sound = "PainPoisonGroanSnd"
            {
                *DisplayName    =   "Pain (poisoned groan)"
            }
            *Sound = "DeathShortSnd"
            {
                *DisplayName    =   "Death (short)"
            }
            *Sound = "DeathLongSnd"
            {
                *DisplayName    =   "Death (long)"
            }
            *Sound = "DeathSwarmBoredSnd"
            {
                *DisplayName    =   "Death (swarmbored)"
            }
            *Sound = "JumpLandHeavySnd"
            {
                *DisplayName    =   "Jump Land (heavy)"
            }
            *Sound = "FallingSplatSnd"
            {
                *DisplayName    =   "Falling splat"
            }
        }
		*Group = "Cheats"
		{
			*FLOAT = "EasyAimRadius"
			{
				*VALUE	= 10
				*MIN	= 0
				*MAX	= 90
				*DISPLAYNAME = "Easy Aim Radius"
				*COMMENT = "Angle (in degrees) of the Easy Aim Radius"
			}

		}
	}

	; Don't let certain variables be changed on an instance basis.
	*Protected	=	"Mass"
	*Protected	=	"Health"
	*Protected	=	"MaxHealth"
	*Protected	=	"Density"
	*Protected	=	"Drag"
	*Protected	=	"Type"
	*Protected	=	"Collides"
	*Protected	=	"Touches"
	*Protected	=	"Regions"
	*Protected	=	"Platforms"
    *Protected  =   "Sound"
}
```

#### LevelExitObject
This one is simply interesting because one it inherits `DeviceAIObject` and two it appears to handle not only defining a region of a LevelExit but also which level the game should change to after the fact, the way it's defined in the object data file seems to suggest that the Values are able to be overwritten by the actor instance files as they're compiled or as an object is instanced.

This is noted by the fact there's a entry in the `VARIABLES` section for `LevelExitDestinationFilename` and there's a `Value` entry that is not populated. Though with this being defined as `FilePick` it seems to suggest that this would cause their World Builder tool to have the user pick a file to populate the `Value` field. This is likely what happened is the object file provided a basic definition and adding it to a map created an instance of this to later be compiled into an individual ATR file.

Meaning each of these entries were only for the world builder to reference and create instances of the objects around, to later turn into actor files.

```
;------------------------------------------------------------------------------
; Level Exit
;------------------------------------------------------------------------------
*OBJECT = "LevelExitObject"
{
	*NAME	= "LevelExitObject"
	*INHERITS = "DeviceAIObject"
	*TYPE	= "OBJECT"
	*UseInActorEd = "True"

	*VARIABLES = 
	{
		*FLOAT = "LevelExitRadius"
		{
			*VALUE = 8.0
			*DISPLAYNAME = "Radius"
			*Comment = "How close the player must be to the exit, in the XZ plane, to leave the level"
		}

		*FLOAT = "LevelExitHeight"
		{
			*VALUE = 2.0
			*DISPLAYNAME = "Height"
			*Comment = "The height of the exit cylinder"
		}

	    *FilePick           =   "LevelExitDestinationLevelName"
	    {
			*DISPLAYNAME = "Destination Level Name"
	        *Comment = "The name of the level to go to (don't include full path)"
	        *Value          =   ""
	        *Directory      =   "$/Data/Levels/"
	        *Extension      =   "atr"
	    }
	}
}
```

#### TurokPickup

Another interesting one inherits `Pickup` which isn't directly related to any one of the games and would've been a more generic definition, likely not defined at all in this god file due to that. However, we do have Turok specific data in relation to the `TurokPickup` object.

Starting with a couple of the `AIMODES` it defines.
- `Enter`
- `Exit`
- `Idle`
- `WaitToSpawn`
- `Respawn`

The modes suggest that these `AIMODES` definitions are certainly used for more of an 'action' or 'state' than just animations which I previously believed while reading them, I'm unsure how specific actions like `WaitToSpawn` would work unless the `TurokPickup` object spawns itself or has some spawn function within it's vtable.

There's also another entry in `VARIABLES` for the `PickupType` which is a `Group` definition, this seems to have a list of all potential pickups in the game for both Single Player and Multiplayer, and finally a  `RespawnTimer` further indicating that as expected with `WaitToSpawn` the vtable for `CTurokPickup` likely has some form of a 'spawn' routine that's called while it's being spawned into the game.

```
*OBJECT = "TurokPickup"
{
	*NAME	= "TurokPickup"
	*INHERITS = "Pickup"
	*TYPE	= "OBJECT"
	*UseInActorEd = "True"

	*AIMODES = 
	{
		Enter,
		Exit,
		Idle,
		WaitToSpawn,
		Respawn
	}
	*VARIABLES =
	{
		*Combo = "PickupType"
		{
			*DisplayName	=	"Pickup Type"
			*Value			=	0
			*ComboList		=
			{
				"Dark Matter Cube Ammo",
				"Flame Thrower Ammo",
				"Grenade Ammo",
				"Guided Device Ammo",
				"Minigun Ammo",
				"Pistol Ammo",
				"Rocket Ammo",
				"Shotgun Ammo",
				"Tech Weapon Ammo",
				"Spiked Mine Ammo",
				"Flachette Ammo",
				"Nuke Ammo",
				"Arrow Ammo",
				"Explosive Arrow Ammo",
				"Poison Arrow Ammo",
				"Gravity Gun Ammo",
				"Swarm Bore Ammo",
				"Baloon",
				"Proxy",
				"Healing Crystal",
				"Healing Herb",
				"Large MedKit",
				"Small MedKit",
				"Warclub",
				"Bow",
				"Crossbow",
				"Dark Matter Cube",
				"Flame Thrower",
				"Remote Device",
				"MiniGun",
				"Sniper Pistol",
				"Rocket Launcher",
				"Shotgun",
				"Tech Weapon",
				"Spiked Mine",
				"TekBow",
				"Gravity Gun",
				"Minigun Upgrade",
				"Sniper Pistol Upgrade",
				"RL Swarm Bore Upgrade"
				"Shotgun Upgrade",
				"Nuke Upgrade",
				"Tech Weapon Beam Upgrade",
				"Tech Weapon Smart Upgrade",
				"Jump Boots",
				"Cloaking Belt",
				"Shield Belt",
				"Empathy Chest plate",
				"Death Helm",
				"Speed",
				"Max Health",
				"Damage",
				"Regeneration",
				"Predator",
				"FlightHealth",
				"FlightOverdrive",
				"FlightRocketsAmmo",
				"Key"
			}
			*Usage		 = "Protected"
			*Comment		=	"What Kind of Pickup this is"
		}
		*Int = "AmmoAmount"
		{
			*DisplayName	=	"Ammo Ammount"
			*Value			=	0
			*Comment		=	"The amount of ammunition contained in this pickup. Use a value of 0 if not applicable, e.g., this is not an ammo pickup."
			*Usage			=	"Protected"
		}
		*Float = "HealthAmount"
		{
			*DisplayName	=	"Health Ammount"
			*Value			=	0.0000
			*Comment		=	"The amount of health contained in this pickup. Use a value of 0.0 if not applicable, e.g., this is not an ammo pickup."
			*Usage			=	"Protected"
		}
		*Group = "Dancing"
		{
			*Bool = "Dances"
			{
				*DisplayName	=	"Dances?"
				*Value			=	false
				*Comment		=	"If true, this pickup will sorta dance. "
			}
			*Float = "DanceAmount"
			{
				*DisplayName	=	"Dance Amount"
				*Value			=	1.0
				*Comment		=	"The higher the value the less subtle the dance"
			}
			*Float = "DanceSpeed"
			{
				*DisplayName	=	"Dance Speed"
				*Value			=	1.0
				*Comment		=	"The higher the value the faster the dance."
			}
		}
		
		*Float = "RSpwnt"
		{
			*DisplayName	=	"Respawn time"
			*Value			=	-1
			*Comment		=	"This is the amount of time in seconds it takes the pickup to reappear."
			*Comment		=	"If the time is -1 then the pickup will never respawn."
		}
	}
}
```

#### WeaponWheel
This gives us more insight into how the WeaponWheel.atr operates, and already in the beginning of the `Variables` definitions there's an interesting comment left behind referring to a `WeaponInfo.h` and suggesting the list of items must be kept in sync with it. This sort of lets us know that the game code was not directly referencing these files or settings and that they were indeed only intended for use by their world builder tool.

The `WeaponWheel` object appears to inherit no other objects, including an Actor which most do as a base at least. 

There's not a ton to say about the variables within the `WeaponWheel` as they seem to mostly be self explanatory, but may be helpful in modifying the actual `WeaponWheel.atr` contained within the game or further deciphering the object in memory.

```
;------------------------------------------------------------------------------
; Weapon Wheel
;------------------------------------------------------------------------------
*Object = "WeaponWheel"
{
	*Name			=	"Weapon Wheel"
	*Access 		=	"Protected"
	*Type			=	"Object"
	*UseInActorEd = "True"

	;-----------------------------------
	; Variables
	;-----------------------------------
	*Variables	= 
	{
		*Group = "Slot 0"
		{
			*Combo = "WeaponSlot0"
			{
				*DisplayName	=	"Weapon"
				*Value			=	0
				*ComboList		=
				{
					; this list must be kept synchronized with WeaponInfo.h!
					"War Club",
					"Bow",
					"TekBow",
					"Grenade",
					"Pistol",
					"Shotgun",
					"Machine Gun",
					"Launcher",
					"Tek Weapon",
					"Flame Thrower",
					"Guided Device",
					"Gravity Disrupter",
					"Dark Matter Cube",
					"Crossbow"
				}
				*Usage		 = "Public"
				*Comment		=	"The weapon that is to be used in this weapon wheel slot."
			}
			*BOOL = "Upgrade1Slot0"
			{
				*Value			=	0
				*DisplayName	=	"Upgrade 1"
				*Comment	 = "Does this weapon have the first upgrade?"
				*Usage		 = "Public"
			}
			*BOOL = "Upgrade2Slot0"
			{
				*Value			=	0
				*DisplayName	=	"Upgrade 2"
				*Comment	 = "Does this weapon have the second upgrade?"
				*Usage		 = "Public"
			}
			*BOOL = "StartsOffWithWeapon0"
			{
				*VALUE	= "True"
				*DISPLAYNAME = "Starts Off With Weapon?"
				*Comment = "Does the player start out with the weapon in this slot in his inventory?"
				*Usage		 = "Public"
			}
			*Int = "AmmoUPG0Slot0"
			{
				*DisplayName	=	"Base Ammo"
				*Value			=	0
				*Comment		=	"If the player starts out with the weapon in this slot, how much ammo comes along with it?"
				*Usage		 = "Public"
			}
			*Int = "AmmoUPG1Slot0"
			{
				*DisplayName	=	"Ammo for Upgrade 1"
				*Value			=	0
				*Comment		=	"If the player starts out with this upgrade, how much ammo comes along with it?"
				*Usage		 = "Public"
			}
			*Int = "AmmoUPG2Slot0"
			{
				*DisplayName	=	"Ammo for Upgrade 2"
				*Value			=	0
				*Comment		=	"If the player starts out with this upgrade, how much ammo comes along with it?"
				*Usage		 = "Public"
			}
		}


		*Group = "Slot 1"
		{
			*Combo = "WeaponSlot1"
			{
				*DisplayName	=	"Weapon"
				*Value			=	0
				*ComboList		=
				{
					; this list must be kept synchronized with WeaponInfo.h!
					"War Club",
					"Bow",
					"TekBow",
					"Grenade",
					"Pistol",
					"Shotgun",
					"Machine Gun",
					"Launcher",
					"Tek Weapon",
					"Flame Thrower",
					"Guided Device",
					"Gravity Disrupter",
					"Dark Matter Cube",
					"Crossbow"
				}
				*Usage		 = "Public"
				*Comment		=	"The weapon that is to be used in this weapon wheel slot."
			}
			*BOOL = "Upgrade1Slot1"
			{
				*Value			=	0
				*DisplayName	=	"Upgrade 1"
				*Comment	 = "Does this weapon have the first upgrade?"
				*Usage		 = "Public"
			}
			*BOOL = "Upgrade2Slot1"
			{
				*Value			=	0
				*DisplayName	=	"Upgrade 2"
				*Comment	 = "Does this weapon have the second upgrade?"
				*Usage		 = "Public"
			}

			*BOOL = "StartsOffWithWeapon1"
			{
				*VALUE	= "False"
				*DISPLAYNAME = "Starts Off With Weapon?"
				*Comment = "Does the player start out with the weapon in this slot in his inventory?"
				*Usage		 = "Public"
			}
			*Int = "AmmoUPG0Slot1"
			{
				*DisplayName	=	"Base Ammo"
				*Value			=	0
				*Comment		=	"If the player starts out with the weapon in this slot, how much ammo comes along with it?"
				*Usage		 = "Public"
			}
			*Int = "AmmoUPG1Slot1"
			{
				*DisplayName	=	"Ammo for Upgrade 1"
				*Value			=	0
				*Comment		=	"If the player starts out with this upgrade, how much ammo comes along with it?"
				*Usage		 = "Public"
			}
			*Int = "AmmoUPG2Slot1"
			{
				*DisplayName	=	"Ammo for Upgrade 2"
				*Value			=	0
				*Comment		=	"If the player starts out with this upgrade, how much ammo comes along with it?"
				*Usage		 = "Public"
			}
		}

		*Group = "Slot 2"
		{
			*Combo = "WeaponSlot2"
			{
				*DisplayName	=	"Weapon"
				*Value			=	0
				*ComboList		=
				{
					; this list must be kept synchronized with WeaponInfo.h!
					"War Club",
					"Bow",
					"TekBow",
					"Grenade",
					"Pistol",
					"Shotgun",
					"Machine Gun",
					"Launcher",
					"Tek Weapon",
					"Flame Thrower",
					"Guided Device",
					"Gravity Disrupter",
					"Dark Matter Cube",
					"Crossbow"
				}
				*Usage		 = "Public"
				*Comment		=	"The weapon that is to be used in this weapon wheel slot."
			}
			*BOOL = "Upgrade1Slot2"
			{
				*Value			=	0
				*DisplayName	=	"Upgrade 1"
				*Comment	 = "Does this weapon have the first upgrade?"
				*Usage		 = "Public"
			}
			*BOOL = "Upgrade2Slot2"
			{
				*Value			=	0
				*DisplayName	=	"Upgrade 2"
				*Comment	 = "Does this weapon have the second upgrade?"
				*Usage		 = "Public"
			}
			*BOOL = "StartsOffWithWeapon2"
			{
				*VALUE	= "False"
				*DISPLAYNAME = "Starts Off With Weapon?"
				*Comment = "Does the player start out with the weapon in this slot in his inventory?"
				*Usage		 = "Public"
			}
			*Int = "AmmoUPG0Slot2"
			{
				*DisplayName	=	"Base Ammo"
				*Value			=	0
				*Comment		=	"If the player starts out with the weapon in this slot, how much ammo comes along with it?"
				*Usage		 = "Public"
			}
			*Int = "AmmoUPG1Slot2"
			{
				*DisplayName	=	"Ammo for Upgrade 1"
				*Value			=	0
				*Comment		=	"If the player starts out with this upgrade, how much ammo comes along with it?"
				*Usage		 = "Public"
			}
			*Int = "AmmoUPG2Slot2"
			{
				*DisplayName	=	"Ammo for Upgrade 2"
				*Value			=	0
				*Comment		=	"If the player starts out with this upgrade, how much ammo comes along with it?"
				*Usage		 = "Public"
			}
		}

		*Group = "Slot 3"
		{
			*Combo = "WeaponSlot3"
			{
				*DisplayName	=	"Weapon"
				*Value			=	0
				*ComboList		=
				{
					; this list must be kept synchronized with WeaponInfo.h!
					"War Club",
					"Bow",
					"TekBow",
					"Grenade",
					"Pistol",
					"Shotgun",
					"Machine Gun",
					"Launcher",
					"Tek Weapon",
					"Flame Thrower",
					"Guided Device",
					"Gravity Disrupter",
					"Dark Matter Cube",
					"Crossbow"
				}
				*Usage		 = "Public"
				*Comment		=	"The weapon that is to be used in this weapon wheel slot."
			}
			*BOOL = "Upgrade1Slot3"
			{
				*Value			=	0
				*DisplayName	=	"Upgrade 1"
				*Comment	 = "Does this weapon have the first upgrade?"
				*Usage		 = "Public"
			}
			*BOOL = "Upgrade2Slot3"
			{
				*Value			=	0
				*DisplayName	=	"Upgrade 2"
				*Comment	 = "Does this weapon have the second upgrade?"
				*Usage		 = "Public"
			}
			*BOOL = "StartsOffWithWeapon3"
			{
				*VALUE	= "False"
				*DISPLAYNAME = "Starts Off With Weapon?"
				*Comment = "Does the player start out with the weapon in this slot in his inventory?"
				*Usage		 = "Public"
			}
			*Int = "AmmoUPG0Slot3"
			{
				*DisplayName	=	"Base Ammo"
				*Value			=	0
				*Comment		=	"If the player starts out with the weapon in this slot, how much ammo comes along with it?"
				*Usage		 = "Public"
			}
			*Int = "AmmoUPG1Slot3"
			{
				*DisplayName	=	"Ammo for Upgrade 1"
				*Value			=	0
				*Comment		=	"If the player starts out with this upgrade, how much ammo comes along with it?"
				*Usage		 = "Public"
			}
			*Int = "AmmoUPG2Slot3"
			{
				*DisplayName	=	"Ammo for Upgrade 2"
				*Value			=	0
				*Comment		=	"If the player starts out with this upgrade, how much ammo comes along with it?"
				*Usage		 = "Public"
			}
		}

		*Group = "Slot 4"
		{
			*Combo = "WeaponSlot4"
			{
				*DisplayName	=	"Weapon"
				*Value			=	0
				*ComboList		=
				{
					; this list must be kept synchronized with WeaponInfo.h!
					"War Club",
					"Bow",
					"TekBow",
					"Grenade",
					"Pistol",
					"Shotgun",
					"Machine Gun",
					"Launcher",
					"Tek Weapon",
					"Flame Thrower",
					"Guided Device",
					"Gravity Disrupter",
					"Dark Matter Cube",
					"Crossbow"
				}
				*Usage		 = "Public"
				*Comment		=	"The weapon that is to be used in this weapon wheel slot."
			}
			*BOOL = "Upgrade1Slot4"
			{
				*Value			=	0
				*DisplayName	=	"Upgrade 1"
				*Comment	 = "Does this weapon have the first upgrade?"
				*Usage		 = "Public"
			}
			*BOOL = "Upgrade2Slot4"
			{
				*Value			=	0
				*DisplayName	=	"Upgrade 2"
				*Comment	 = "Does this weapon have the second upgrade?"
				*Usage		 = "Public"
			}
			*BOOL = "StartsOffWithWeapon4"
			{
				*VALUE	= "False"
				*DISPLAYNAME = "Starts Off With Weapon?"
				*Comment = "Does the player start out with the weapon in this slot in his inventory?"
				*Usage		 = "Public"
			}
			*Int = "AmmoUPG0Slot4"
			{
				*DisplayName	=	"Base Ammo"
				*Value			=	0
				*Comment		=	"If the player starts out with the weapon in this slot, how much ammo comes along with it?"
				*Usage		 = "Public"
			}
			*Int = "AmmoUPG1Slot4"
			{
				*DisplayName	=	"Ammo for Upgrade 1"
				*Value			=	0
				*Comment		=	"If the player starts out with this upgrade, how much ammo comes along with it?"
				*Usage		 = "Public"
			}
			*Int = "AmmoUPG2Slot4"
			{
				*DisplayName	=	"Ammo for Upgrade 2"
				*Value			=	0
				*Comment		=	"If the player starts out with this upgrade, how much ammo comes along with it?"
				*Usage		 = "Public"
			}
		}

		*Group = "Slot 5"
		{
			*Combo = "WeaponSlot5"
			{
				*DisplayName	=	"Weapon"
				*Value			=	0
				*ComboList		=
				{
					; this list must be kept synchronized with WeaponInfo.h!
					"War Club",
					"Bow",
					"TekBow",
					"Grenade",
					"Pistol",
					"Shotgun",
					"Machine Gun",
					"Launcher",
					"Tek Weapon",
					"Flame Thrower",
					"Guided Device",
					"Gravity Disrupter",
					"Dark Matter Cube",
					"Crossbow"
				}
				*Usage		 = "Public"
				*Comment		=	"The weapon that is to be used in this weapon wheel slot."
			}
			*BOOL = "Upgrade1Slot5"
			{
				*Value			=	0
				*DisplayName	=	"Upgrade 1"
				*Comment	 = "Does this weapon have the first upgrade?"
				*Usage		 = "Public"
			}
			*BOOL = "Upgrade2Slot5"
			{
				*Value			=	0
				*DisplayName	=	"Upgrade 2"
				*Comment	 = "Does this weapon have the second upgrade?"
				*Usage		 = "Public"
			}
			*BOOL = "StartsOffWithWeapon5"
			{
				*VALUE	= "False"
				*DISPLAYNAME = "Starts Off With Weapon?"
				*Comment = "Does the player start out with the weapon in this slot in his inventory?"
				*Usage		 = "Public"
			}
			*Int = "AmmoUPG0Slot5"
			{
				*DisplayName	=	"Base Ammo"
				*Value			=	0
				*Comment		=	"If the player starts out with the weapon in this slot, how much ammo comes along with it?"
				*Usage		 = "Public"
			}
			*Int = "AmmoUPG1Slot5"
			{
				*DisplayName	=	"Ammo for Upgrade 1"
				*Value			=	0
				*Comment		=	"If the player starts out with this upgrade, how much ammo comes along with it?"
				*Usage		 = "Public"
			}
			*Int = "AmmoUPG2Slot5"
			{
				*DisplayName	=	"Ammo for Upgrade 2"
				*Value			=	0
				*Comment		=	"If the player starts out with this upgrade, how much ammo comes along with it?"
				*Usage		 = "Public"
			}
		}
		*Group = "Slot 6"
		{
			*Combo = "WeaponSlot6"
			{
				*DisplayName	=	"Weapon"
				*Value			=	0
				*ComboList		=
				{
					; this list must be kept synchronized with WeaponInfo.h!
					"War Club",
					"Bow",
					"TekBow",
					"Grenade",
					"Pistol",
					"Shotgun",
					"Machine Gun",
					"Launcher",
					"Tek Weapon",
					"Flame Thrower",
					"Guided Device",
					"Gravity Disrupter",
					"Dark Matter Cube",
					"Crossbow"
				}
				*Usage		 = "Public"
				*Comment		=	"The weapon that is to be used in this weapon wheel slot."
			}
			*BOOL = "Upgrade1Slot6"
			{
				*Value			=	0
				*DisplayName	=	"Upgrade 1"
				*Comment	 = "Does this weapon have the first upgrade?"
				*Usage		 = "Public"
			}
			*BOOL = "Upgrade2Slot6"
			{
				*Value			=	0
				*DisplayName	=	"Upgrade 2"
				*Comment	 = "Does this weapon have the second upgrade?"
				*Usage		 = "Public"
			}
			*BOOL = "StartsOffWithWeapon6"
			{
				*VALUE	= "False"
				*DISPLAYNAME = "Starts Off With Weapon?"
				*Comment = "Does the player start out with the weapon in this slot in his inventory?"
				*Usage		 = "Public"
			}
			*Int = "AmmoUPG0Slot6"
			{
				*DisplayName	=	"Base Ammo"
				*Value			=	0
				*Comment		=	"If the player starts out with the weapon in this slot, how much ammo comes along with it?"
				*Usage		 = "Public"
			}
			*Int = "AmmoUPG1Slot6"
			{
				*DisplayName	=	"Ammo for Upgrade 1"
				*Value			=	0
				*Comment		=	"If the player starts out with this upgrade, how much ammo comes along with it?"
				*Usage		 = "Public"
			}
			*Int = "AmmoUPG2Slot6"
			{
				*DisplayName	=	"Ammo for Upgrade 2"
				*Value			=	0
				*Comment		=	"If the player starts out with this upgrade, how much ammo comes along with it?"
				*Usage		 = "Public"
			}
		}

		*Group = "Slot 7"
		{
			*Combo = "WeaponSlot7"
			{
				*DisplayName	=	"Weapon"
				*Value			=	0
				*ComboList		=
				{
					; this list must be kept synchronized with WeaponInfo.h!
					"War Club",
					"Bow",
					"TekBow",
					"Grenade",
					"Pistol",
					"Shotgun",
					"Machine Gun",
					"Launcher",
					"Tek Weapon",
					"Flame Thrower",
					"Guided Device",
					"Gravity Disrupter",
					"Dark Matter Cube",
					"Crossbow"
				}
				*Usage		 = "Public"
				*Comment		=	"The weapon that is to be used in this weapon wheel slot."
			}
			*BOOL = "Upgrade1Slot7"
			{
				*Value			=	0
				*DisplayName	=	"Upgrade 1"
				*Comment	 = "Does this weapon have the first upgrade?"
				*Usage		 = "Public"
			}
			*BOOL = "Upgrade2Slot7"
			{
				*Value			=	0
				*DisplayName	=	"Upgrade 2"
				*Comment	 = "Does this weapon have the second upgrade?"
				*Usage		 = "Public"
			}
			*BOOL = "StartsOffWithWeapon7"
			{
				*VALUE	= "False"
				*DISPLAYNAME = "Starts Off With Weapon?"
				*Comment = "Does the player start out with the weapon in this slot in his inventory?"
				*Usage		 = "Public"
			}
			*Int = "AmmoUPG0Slot7"
			{
				*DisplayName	=	"Base Ammo"
				*Value			=	0
				*Comment		=	"If the player starts out with the weapon in this slot, how much ammo comes along with it?"
				*Usage		 = "Public"
			}
			*Int = "AmmoUPG1Slot7"
			{
				*DisplayName	=	"Ammo for Upgrade 1"
				*Value			=	0
				*Comment		=	"If the player starts out with this upgrade, how much ammo comes along with it?"
				*Usage		 = "Public"
			}
			*Int = "AmmoUPG2Slot7"
			{
				*DisplayName	=	"Ammo for Upgrade 2"
				*Value			=	0
				*Comment		=	"If the player starts out with this upgrade, how much ammo comes along with it?"
				*Usage		 = "Public"
			}
		}


;			 *Combo = "WeaponSlot1"
;			 {
;				 *DisplayName	 =	 "WeaponSlot1"
;				 *Value 		 =	 0
;				 *ComboList 	 =
;				 {
;					 ; this list must be kept synchronized with WeaponInfo.h!
;					 "War Club",
;					 "Bow",
;					 "TekBow",
;					 "Grenade",
;					 "Pistol",
;					 "Sniper Pistol",
;					 "Shotgun",
;					 "Shotgun MultiLoad",
;					 "Machine Gun",
;					 "Machine Gun Flechette",
;					 "Rocket Launcher",
;					 "Cluster Launcher",
;					 "Swarm Launcher",
;					 "Tech Weapon Pulse",
;					 "Tech Weapon Beam",
;					 "Tech Weapon Full",
;					 "Flame Thrower",
;					 "Guided Device",
;					 "Gravity Disrupter",
;					 "Gravity Disrupter Grapple",
;					 "Dark Matter Cube"
;				 }
;				 *Comment		 =	 "The weapon that is to be used in this weapon wheel slot."
;				 *Usage 	  = "Public"
;			 }
;			 *BOOL = "HasWeaponInSlot1"
;			 {
;				 *VALUE  = "False"
;				 *DISPLAYNAME = "Has Weapon in Slot 1?"
;				 *Comment = "Does the player start out with the weapon in this slot in his inventory?"
;				 *Usage 	  = "Public"
;			 }
;			 *Int = "ClipsForSlot1"
;			 {
;				 *DisplayName	 =	 "Ammo Clips For Slot 1"
;				 *Value 		 =	 1
;				 *Comment		 =	 "If the player starts out with the weapon in this slot, how many clips of ammo comes along with it."
;				 *Usage 	  = "Public"
;			 }
;
;
	}

}
```

#### WeaponAmmo
This one is only interesting because it's entirely commented and I've never seen it referenced by the game at all, it appears to have been intended to be a separate object but the ammo in memory is typically stored inside of the weaponwheel itself.

```
;*Object = "WeaponAmmo"
;{
;	 *Name			 =	 "Weapon Ammo"
;	 *Access		 =	 "Public"
;	 *Type			 =	 "Object"
;
;	 ;-----------------------------------
;	 ; Variables
;	 ;-----------------------------------
;	 *Variables  = 
;	 {
;		 *Int = "AmmoPerClip"
;		 {
;			 *DisplayName	 =	 "Ammo Per Clip"
;			 *Value 		 =	 5
;			 *Min			 =	 1
;			 *Max			 =	 100
;			 *Comment		 =	 "The number of rounds of ammo can each clip hold."
;		 }
;
;		 *Int = "AmmoUsedPerShot"
;		 {
;			 *DisplayName	 =	 "Ammo Used Per Shot"
;			 *Value 		 =	 1
;			 *Min			 =	 0
;			 *Max			 =	 100
;			 *Comment		 =	 "The number of rounds of ammo that is used each time the weapon is fired."
;		 }
;
;		 *Float = "MuzzleRidingXFactor"
;		 {
;			 *DisplayName	 =	 "Muzzle Riding X Factor"
;			 *Value 		 =	 0.1
;			 *Min			 =	 0.0
;			 *Max			 =	 10.0
;			 *Comment		 =	 "The amount by which the weapon will move left/right while firing."
;		 }
;
;		 *Float = "MuzzleRidingYFactor"
;		 {
;			 *DisplayName	 =	 "Muzzle Riding Y Factor"
;			 *Value 		 =	 0.1
;			 *Min			 =	 0.0
;			 *Max			 =	 10.0
;			 *Comment		 =	 "The amount by which the weapon will move up/down while firing."
;		 }
;	 }
;}
```

#### WeaponObject
It used to inherit `WeaponAmmo` which was then commented and instead inherits `TurokAIObject` it has a `Combo` in `VARIABLES` called `Weapon` which seems similar to the `Weapon Wheel` list of weapons, there's a comment indicating that this is the weapon type.

Again we see a `PICKFROMFILE` option indicating a user would 'import' or 'clone' this to create a instance of it in their actor file then be shown a file picker to populate this specific data, in this case it's `WeaponNameBase` which appears to be them choosing a specific text entry for the name of the weapon.

Few other options that are interesting here as well and seem to be defined as "BOOL" values:
* `UsableUnderWaterBase`
* `TwoHandedWeaponBase`
* `UsableWhenClimbingBase`

There's a `WeaponTypeBase` that appears to only have a small list of options and the list of options is referred to as a `ComboList` I'm guessing this told the WorldBuilder to list these in a combobox UI and force the user to pick one when creating an instance of it, the options are as follows:

* Melee
* Primitive Projectile
* Thrown
* Ballistic

Each weapon appears to define it's own `Ammo` including the max ammo possible, and also individual `Upgrade` slots as well.

Something interesting is under ConstructActor which was used to define the table of classes/objects in this document, "Weapon" is never referenced. Individual weapons which inherit it probably are but not "Weapon itself.

#### WarClubObject

This seems to inherit `WeaponObject` specifically, which is something I documented previously when defining the class for this. `Variables` are interesting for it as it only has one,  `RElaxDelay` the coments of this state that it's the amount of time before it will enter an "IdleMode".

Possible modes for the object are as follows:
* IdleRelax
* IdleRelaxEnter
* IdleRelaxExit
* Enter
* Exit
* Idle
* Fire
* FireReturn
* Combo1 
* Combo1Return
* Combo2
* Combo2Return
* Combo3
* Combo3Return
* Charge
* ChargeReturn

There's nothing else defined for the object, it's a pretty short one but I'm assuming there's a reliance on the `WeaponObject` to handle most base variables.

#### BowObject

Inherits from `WeaponObject` like the `WarClubObject` however, this one has some other `Variables` entries which seem associated to the upgrades for it. The interesting thing is these are `FilePick` entries but they specifically refer to the extenions `par` and `eff` which are particle effects. Meaning the upgrades aren't associated to actor files in these variable entries, additionally they reference the path `$\Data\Particle` as a prefix.

Some other interesting entires within `Variables` are:
* `Snipe FOV`
* `Hold Modifier`
* `BaseWobble`
* `RandWobble`

Some of these are fairly interesting for multiplayer development and current bugs being encountered. Some of those bugs have to do with projectiles being delivered to the incorrect location despite view angles being synchronized. The wobble of the ewapon and the fact it's being randomized could play a factor in this and is something that will help with further investigation into why projectiles are offset.

The weapon doesn't define any interesting `AIMODES` a complete list is here:
* `Enter`
* `Exit`
* `Idle`
* `PullBack`
* `Hold`
* `Release`
* `ReleaseDbl`
* `Reload`
* `DefaultIdle`
* `SniperEnter`
* `SniperExit`
* `ShortFire`
* `SwitchToExplosive`
* `SwitchToNormal`
* `ShortExplodeFire`
* `EmptyToLoaded`
* `FireToEmpty`
* `IdleEmpty`
* `ExitEmpty`


#### Shotgun
Inherits from `WeaponObject` first defines some `AIMODES`:
* `Enter`
* `Exit`
* `Idle`
* `Fire`
* `Fire2`
* `Fire3`
* `Fire4`
* `Reload`
* `Switch`
* `DefaultIdle`

The only other thing defined for the `Shotgun` specifically is a single sound entry for `DryClick` with the `DisplayName` "No Ammo Click Sound".

#### CrossBow
Inherits `WeaponObject`, defines similar `Variables` to a `BowObject` but has some other `Variables` entires also, one of which being `SPO1Name`. The comments for this seem to suggest it's related to an overlay which would be picked when defining an instance of it via a `FilePick`. The exact comment assosciated is "This is the Snipe mode screen overlay."

Additionally this does define a `BaseWobble`, and `RandWobble` which further tells me it could be related to the offset projectile delivery in the current version of T4MP_V2.

Not a ton of interesting `AIMODES` defined for this one:
* `Enter`
* `ExitA`
* `ExitB`
* `IdleA`
* `IdleB`
* `PullBack`
* `Hold`
* `Release`
* `Reload`
* `DefaultIdle`
* `SnipeEnterA`
* `SnipeEnterB`
* `SnipeExitA`
* `SnipeExitB`
* `SnipeFire`
* `SwitchToExplosive`
* `SwitchToPoison`
* `SwitchToBroadHead`

#### DarkMatterCube
Inherits `WeaponObject` not a lot going on here. The `AIMODES` seem to lean more on the animation side of things:
* `Enter`
* `Exit`
* `Idle`
* `DefaultIdle`
* `Fire`
* `ImplodeTransform`
* `ExplodeTransform`
* `Pullback`

`Variables` entries aren't too interesting and define throw distances mostly the following is the list of all potential variables
* `MaxHoldTime` 
* `Max Throw Distance`
* `MinThrowDist`

#### DarkMatterObject
Inherits `TurokAIObject` this is what's spawned when the dark matter cube weapon is actually thrown/fired. It references a particle via a `FilePick` property meaning someone in the world builder would've choosen the explosion particle but it also defines a number of other variables.

* `DMTmOut`

`DisplayName =  "Time Out"`

`Comment = "Time in seconds to wait before the Dark Matter explodes on its own."`

* `DMExActRad`

`Display Name = "Explosion Activation Radius"`

`Comment = "Distance in feet that activaties the explosion cube."`

* `DmExDamRad`

`Display Name = "Explosion Damage Radius"`

`Comment = "Distance in feet from the cube that enemies will be instantly killed"`

* `DmImActRad`

`Display Name = "Implode Activation Radius"`

`Comment = "Distance in feet that activates the Implosion cube."`

* `DMImDamRad`

`Display Name = "Implode Damage Radius"`

`Comment = "Distance in feet from the cube that enemies will be instantly killed."`

* `DMImTm`

`Display Name = "Implode Time"`

`Comment = "Time in seconds it takes an actor to get sucked into the center of the implosion."`

* `DMImpTimeOut`

`Display Name = "Implode Timeout"`

`Comment = "Time in seconds that the DMC will stay in Implode mode."`

#### DMPlayer

Inherits `Player`, this is the `Deathmatch Player` object which the comments above the definition for the object actually state. There a number of interesting `Variables` entries on this one starting with a `FilePick` for `DeadPlayerActor`. 

The comment here indicates "This is the actor that wil be spawned whena  player is killed.", meaning this is responsible for creating a "DeadPlayer" object, there's a static reference to `"\Data\Actors\Multiplayer\DeadPlayer\DeadPlayer.atr` pre-assigned to the variable entry.

Additional variables are:
* `GreenTeamMesh`

"Stone Team Mesh" 

* `YellowTeamMesh`

"Fire Team Mesh"

* `Snapshot`

"Picture of the player"

* `IsRaptor`

"Set this to true if the player is a raptor"

Other than the above there's really nothing else defined for the DMPlayer which is interesting considering how much data is stored in memory or how many entries there are in it's vtable. There's a wide range of things defined for it's `AIMODES` as well meaning it overrides what it inherits from `Player`

```
	*AIMODES = 
	{
		"Idle",
		"AlertIdle",
		"Run",
		"Walk",
		"Jump",
		"MoveForward"
		"MoveBackward",
		"StrafeLeft",
		"StrafeRight",
		"MoveLeft",
		"MoveRight",
		"TurnLeft",
		"TurnRight",
		"BackWalk",
		"BackRun",
		"Damaged",
		"Attack",
		"LoopAttack",
		"Fall",
		"CrouchIdle",
		"CrouchForward",
		"CrouchBackward",
		"CrouchLeft",
		"CrouchRight",
		"CrouchTurnRight",
		"CrouchTurnLeft",
		"Respawn",
		"LeftDamage",
		"ClimbIdle",
		"ClimbUp",
		"ClimbDown",
		"ClimbLeft",
		"ClimbRight",
		"EndClimb",
		"AttackForward",
		"AttackBackward",
		"AttackIdle",
		"AttackStrafeLeft",
		"AttackStrafeRight",
		"AttackJump",
		"AttackTurnLeft",
		"AttackTurnRight",
		"CrouchAttackLeft",
		"CrouchAttackRight",
		"CrouchAttackForward",
		"CrouchAttackBackward",
		"CrouchAttackIdle",
		"SnipeAttack",
		"SwimForward",
		"SwimBackward",
		"SwimUp",
		"SwimDown",
		"SwimLeft",
		"SwimRight",
		"SwimBurst",
		"WaterSurfaceIdle",
		"UnderwaterIdle",
		"SwimTurnLeft",
		"SwimTurnRight",
		"LeapToShore",
		"SwimAttack",
		"DiveIntoWater",
		"UsingGuidedDevice",
		"OneHandIdle",
		"OneHandAlertIdle",
		"OneHandMoveForward",
		"OneHandMoveBackward",
		"OneHandStrafeLeft",
		"OneHandStrafeRight",
		"OneHandTurnLeft",
		"OneHandTurnRight",
		"OneHandAttackIdle",
		"OneHandAttackForward",
		"OneHandAttackBackward",
		"OneHandAttackStrafeLeft",
		"OneHandAttackStrafeRight",
		"OneHandAttackJump",
		"OneHandAttackTurnLeft",
		"OneHandAttackTurnRight",
		"OneHandAttackSnipe",
		"OneHandCrouchAttackStrafeLeft",
		"OneHandCrouchAttackStrafeRight",
		"OneHandCrouchAttackForward",
		"OneHandCrouchAttackBackward",
		"OneHandCrouchAttackIdle",
		"Celebrate",
		"MonkeyClimbLeft",
		"MonkeyClimbRight",
		"MonkeyClimbForward",
		"MonkeyClimbBackward",
		"MonkeyClimbIdle",
		"MonkeyClimbBegin",
		"MonkeyClimbEnd",
		"WarclubAttack",
		"WarclubCombo1",
		"WarclubCombo2",
		"WarclubCombo3",
		"ThrowIdle",
		"ThrowRunForward",
		"ThrowRunBackward",
		"ThrowStrafeRight",
		"ThrowStrafeLeft",
		"ThrowGuidedDevice"
	}
```

#### CompyPlayer

Inherits from `DMPlayer` which is interesting because it tells you that it inherits all of the data from that object, and the player object. The only overrides it seems to have are on `AIMODES` and the introduction of a single `Sound` entry indicating it's a "Call out sound".

#### SpawnPoint

Inherits from  not other place and has some interesting `Variables` defined within it.

There is a `Group` for `Team` that has a `TEAMNAME` combo, with a `ComboList` meaning the person using the world builder or actor editor would choose from one of these options, of which there are two:
* `Fire`
* `Stone`

Additionally, there appears to be a `BitField` entry under variables for `ModeFlags` and it has a display name of `ExcludeModes`. This also defines a `BitList` and appears to determine what modes a spawn point will not be used in based on what fields are set.

```
		*BitList =
			{
				"Predator",			; 1
				"Flight",			; 1 << 1
				"Deathmatch",		; 1 << 2
				"DinoRider",		; 1 << 3
				"MixedRiding",		; 1 << 4
				"TRex",				; 1 << 5
				"DinoCatcher",		; 1 << 6
				"CTF",				; 1 << 7
				"OneFlagCTF",		; 1 << 8
				"LastManStanding",	; 1 << 9
				"Coop",				; 1 << 10
				"BagTag",			; 1 << 11
				"Assault",			; 1 << 12
				"HumansVsDinos"		; 1 << 13
			}
```

Some other interesting things seen when observing these modes is that there are certainly some modes which didn't make it into the final game like:
* DinoCatcher
* TRex
* MIxedRiding
* DinoRider
* Assault
* HumansVsDinos
* Coop

#### DMBot

There's no defined object/class in the game code for `DMBot` so it's not usable, however there is an actor file associated. Trying to spawn it in the game results in crashing, the Object defined in the gameobjectdata file though is still interesting.

It shows that it inherits from `DMPlayer` and appears to be some concept of a bot that would play along with others in the multiplayer mode. It shares many of the same `AIMODES` and has a `Weapon Selection` variable definition where preferred weapons are set for the bot. The bot would've been given a preference for specific weapons in different slots. With a total of 12 potential slots.

Some other variable entries were a group called `Detection` with variables like `IGNOREPLAYER`, `CONEVISION`, `SIGHTRADIUS`, `SXZAngle`, `SYAngle`, `SOUNDRADIUS`, `LostContactResetTime` and another group called `Attack`, containing `CloseRangeChance`, and `CloseRangeDist`.

#### End of the GameObjectData
While there are other things defined I didn't document here the rest of the file just contains with no other definitions:
```

;	+------------------------------------------------------------------+
;	|																   |
;	|				End of Turok4-specific section. 				   |
;	|																   |
;	+------------------------------------------------------------------+
```



