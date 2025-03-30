# Quagmire Engine

## Overview
This is basically anything I could document about the version of Quagmire used for Turok Evolution.

From what I could tell Turok: Evolution was based on either the 3rd or 4th iteration of Quagmire, interally they referred to it as Quag_3 or Quag_4. The version isn't super important outside of the fact that at this point they were able to target GameCube, Xbox, PC (DX8) and PS2 they no longer targeted legacy platforms like N64, Dreamcast, PS1 like they did in the prior versions of Quagmire.

Attempts to label the Turok Evolution binary have been made using BinDiff, Rizzo and a few other tools and a Redstar xbox binary. I'm not aware of any tools that would allow for architecture independent labeling so that Vexx and 100 Bullets builds could also be used for this purpose but that's where a majority of the symbols in the Turok Evolution IDB here come from (the bindiff and manual research between the PS2 versions of Vexx and 100 Bullets.)

## Source Code

Partial source code for what I refer to as Quagmire one is out there released by Assembler games but, outside of the view object and maybe some of the x_std, x_files, x_math stuff it's not relevant to Turok Evolution at all.

I did run into some more modern source code that was related to All Star Baseball, and this does seem to somewhat directly translate to Turok: Evolution. I do plan to upload this source code to this 'research' archive I have created here just for reference. Either here or an additional repository I'll link to from here.

The source code for Vexx is out there, but I haven't been able to obtain it, otherwise I may have proper code for loading MTFs, and animations etc.

The other issue with the Quagmire 3 Source code is either Quagmire (any iteration) never supported DX9 and the entire work of implementing that support was done by Super Happy Fun Fun when porting to PC, or the version is not comparable to what Turok Evolution utilizes so a lot of the window creation, or rendering routines are not a match for what's found in the binary, at least not directly.

The partial source code has been made available [here](https://github.com/pnill/T4_Research/tree/master/TNT)

## Source Structure
The main portion of Quagmire 3 I've been able to obtain is structured as follows:
```
TNT\
    3rdparty\
        Dolphin\
        PC\
        PS2\
        XBOX\
    Apps\
        AnimTblEd\
        CED\
        CED - for ASB2004\
        DataBlock\
        MemCardWiz\
        RipGeom\
        RipSkin\
        RipStad\
        SkinBlender\
        TBlockTool\
    Auxiliary\
        AnimPlay\
        AnimPlay - For ASB2004\
        ArgList\
        AuxRAM\
        Bitmap\
        BMPColor\
        FlyCam\
        Mesh\
        OrbitCam\
        PS2Prof\
        QRaw\
        skin\
        Tokenizer\
        ZLib\
    NetSpine\
        GameSpy\
        NetGameSDK2\
        NetworkCore\
    Quagmire\
        engine\
            Common\
            GameCube\
            PC\
            PS2\
            XBOX\
            Quag_3.dsp
            Quag_3.vcproj
    Tools\
    x_files\
    x_std\
```    

## x_files and x_std

They have these sets of files which seem to be intended for cross platform compatibility while still using standard functions/routines.

### x_std

An example of this is x_string.cpp where they've effectively made their own version of std::string they refer to as x_std::string.

```
char * x_std::string::New( size_type Capacity ) 
{ 
    Capacity += 10 ; // Always add a little more ... this might prevent some unnecessary reallocations (depending on how the string is used).
    const size_type   NBytes  = sizeof(CPrefix) + Capacity + 1;  // +1 for trailing null.
    CPrefix *   Prefix  = reinterpret_cast<CPrefix *>( new char[NBytes] );

    X_STD_ASSERT( Capacity <= MaxSize );
    Prefix->Length      =   0           ;
    Prefix->Capacity    =   Capacity    ;

    char * Data = reinterpret_cast<char *>( &Prefix[1] );
    Data[0] = 0; // Add null termination.
    return Data;
}
```

The issue with this is either due to optimization or version changes whether Turok Evolution be based on an older version or this is an older version of the engine, it does not translate 1:1 any attempt I've made to map the struct 1:1 has failed despite being able to reference similar naming in symbols from the other games built on the same version of the engine as Turok Evolution, so while the naming is mostly the same the structure of the class has roughly changed between whatever this is and Turok Evo.

### x_files

They've got a few different classes they implement here some for math, some for timing related things, some for I/O, and some for things like colors. The main importance of it all again seems to just be implementing things in a platform independent way so that in the rest of the engine when they use x_Function it automatically handles picking the correct platform to perform the same functionality across the board.


## quagmire/engine

### Q_Engine
Pretty generic exposure of window creation, things to fill Z Buffer, handle rendering etc. 

Not all of it directly translates to the PC port of Turok Evolution, but some of it is useful for naming of functions and etc.

I admittely have not looked too deeply into these because of the fact that I'm reversing the binary and haven't needed things like World 2 Screen functions or if I have they're already labeled and can be called vs me re-implementing them from source.

### Q_View
This seems to mostly directly translate to all versions of Turok Evolution and hasn't changed much. It appears to be directly related to view port and camera setups allowing adjustment of Z limits, FOV, aspect ratio, etc. Most of this in Turok Evolution is reverse engineered and documented.

## Quagmire RE

Most of the reverse engineering I attempt to do is not Quagmire specific but Turok Evolution specific, however because there's so much carry over between games I assume some of this must be related to some piece of Quagmire that was used in the process.

Any source code I've identified or found of Quagmire in the wild however, doesn't implement the same setup. 

The rest of this should be considered TBD while I figure out how I want to document individual classes and what I've determined their usage is, a list of the classes/functions I'd like to at least partially document are below. I'll probably eventually break out interesting classes into sub sections and try to define the functions within them further individually, mostly things like CActor, CLevel, CGame primary classes that allow interacting with the game or engine in a meaningful way.

| Function / Class         | Obj File                                                                                                                      |  Comments                                                                      |
| :----------------------- | :----------------------------------------------------------------------------------------------------------------------------  |  :-----------------------------------------------------------------------------: |
| Anim                     | CTG_Common:Anim.obj, CTG_Common:AnimConst.obj, CTG_Common:DynamicMeshBase.obj, CTG_Common:Mesh.obj, CTG_Common:Morph.obj, GameEngine:Actor.obj, GameEngine:AnimationResourceManager.obj, GameEngine:app.obj, GameEngine:Cinema.obj, GameEngine:CinemaBehavior.obj, GameEngine:GameEngineResources.obj, GameEngine:Geometry.obj, GameEngine:GeometryResourceManager.obj, GameEngine:Pre-CacheManager.obj, GameEngine:ResourceCacheManager.obj, GameEngine:SimpleAnimation.obj, XBOXSystem:StaticMesh.obj | No constructor may be a namespace | 
| AnimationCompressor      | CTG_Common:AnimationCompressor.obj                                                                                              | No constructor may be a namespace |
| AudioDriver              | GameEngine:AAA_Lib.obj and GameEngine:Layer2.obj                                                                                | Constructor in layer2.obj | 
| AudioInstance            | GameEngine:AAA_Lib.obj, GameEngine:AudioInstance.obj, GameEngine:Layer2.obj, GameEngine:Layer3.obj                              | Constructor in AudioInstance.obj |
| CAIAccessory             | Unk                                                                                                                             | Not in redstar so MAP file is irrelevant in finding the object it's defined in, unsure if ps2 debug builds would point to a object| 
| CAIMarker                | Unk                                                                                                                             | Same as the above |
| CActor                   | GameCode:ActorCreator.obj, GameCode:ArcingBullet.obj, GameCode:AttackDamage.obj, GameCode:Beacon.obj, GameCode:Beam.obj, GameCode:Beetle.obj, GameCode:BeetleV2.obj, GameCode:Berserker.obj, GameCode:Bullet.obj, GameCode:BullpupHarrier.obj, GameCode:DeathPlane.obj, GameCode:EliteTrooper.obj, GameCode:EnemyListBox.obj, GameCode:FightMediator.obj, GameCode:GameChannels.obj, GameCode:GameDeity.obj, GameCode:GameHUD.obj, GameCode:Generator.obj, GameCode:Hailer.obj, GameCode:HeavyGuard.obj, GameCode:KarDathra.obj, GameCode:Kyuzo.obj, GameCode:MainMenuItem.obj, GameCode:Makita.obj, GameCode:MarqueeWidget.obj, GameCode:Maya.obj, GameCode:MotionPreviewWidget.obj, GameCode:MovesWidget.obj, GameCode:Mystic.obj, GameCode:NPC.obj, GameCode:ObjectivesWidget.obj, GameCode:OutlineWidget.obj, GameCode:PathMiner.obj, GameCode:PickupHealth.obj, GameCode:Player.obj, GameCode:PlayerPlaceholder.obj, GameCode:RedArmorTrooper.obj, GameCode:RedGunner.obj, GameCode:RedStarCamera.obj, GameCode:RedTrooper.obj, GameCode:RocketArmor.obj, GameCode:Scarab.obj, GameCode:SeekerMine.obj, GameCode:Shooter.obj, GameCode:SliderWidget.obj, GameCode:Spearman.obj, GameCode:SpiritTrooper.obj, GameCode:Swarm.obj, GameCode:TeletypeWidget.obj, GameCode:Tentacle.obj, GameCode:TestVehicle.obj, GameCode:Troika.obj, GameCode:UniqueTextureWidget.obj, GameCode:UpgradeWidget.obj, GameCode:VehicleSideScrolling.obj, GameCode:Widget3D.obj, GameCode:Zealot.obj, GameEngine:Actor.obj, GameEngine:ActorBasicPhysicsBehaviour.obj, GameEngine:ActorChannels.obj, GameEngine:ActorGrid.obj, GameEngine:ActorLinks.obj, GameEngine:ActorModeBehaviour.obj, GameEngine:ActorMover.obj, GameEngine:ActorRegions.obj, GameEngine:ActorShadow.obj, GameEngine:ActorTypeInfo.obj, GameEngine:ActorUpdateGeometryBehaviour.obj, GameEngine:BlendedCamera.obj, GameEngine:BounceBehaviour.obj, GameEngine:Camera.obj, GameEngine:CameraBehavior.obj, GameEngine:ChannelChangeBehaviour.obj, GameEngine:Cinema.obj, GameEngine:CinemaBehavior.obj, GameEngine:CircleBehavior.obj, GameEngine:Collision.obj, GameEngine:CollisionGeometry.obj, GameEngine:Deity.obj, GameEngine:DualPathBehavior.obj, GameEngine:Game.obj, GameEngine:Geometry.obj, GameEngine:Level.obj, GameEngine:LinkEvent.obj, GameEngine:NavData.obj, GameEngine:PathBehavior.obj, GameEngine:Performance.obj, GameEngine:Pickup.obj, GameEngine:PIPCamera.obj, GameEngine:PointAttractBehavior.obj, GameEngine:PolarBehavior.obj, GameEngine:portal.obj, GameEngine:PredefinedLinkCauses.obj, GameEngine:PredefinedLinkConditionals.obj, GameEngine:PredefinedLinkEvents.obj, GameEngine:Region.obj, GameEngine:Sounds.obj, GameEngine:SpringManager.obj, GameEngine:ThirdPersonCamera.obj, GameEngine:ToolsConnection.obj, GameEngine:VectorBehavior.obj, GameEngine:WaterActor.obj, GameEngine:Widget.obj| The primary actor object all AI, player objects, and etc are created out of, massive structure in memory. Obviously references to the "GameCode" project are specific to redstar where the data was obtained from. |
| CActorAssociates         | Unk | Not in redstar map so can't find the relevant object |
| CActorBehaviour          | GameCode:AccentLights.obj, GameCode:ActorFlasher.obj, GameCode:GameChannels.obj, GameCode:GameEvents.obj, GameCode:Player.obj, GameCode:SlidingAttachmentBehaviour.obj, GameEngine:Actor.obj, GameEngine:ActorBasicPhysicsBehaviour.obj, GameEngine:ActorBehaviour.obj, GameEngine:ActorChannels.obj, GameEngine:ActorHeldPhysicsBehaviour.obj, GameEngine:ActorJoystick.obj, GameEngine:ActorKeySequence.obj, GameEngine:ActorLinks.obj, GameEngine:ActorModeBehaviour.obj, GameEngine:ActorMover.obj, GameEngine:ActorUpdateGeometryBehaviour.obj, GameEngine:ActorUpdateTimeEventsBehaviour.obj, GameEngine:AngularPhysics.obj, GameEngine:ChannelChangeBehaviour.obj, GameEngine:Collision.obj, GameEngine:DefaultHistory.obj, GameEngine:Game.obj, GameEngine:Interest.obj, GameEngine:LinkEvent.obj, GameEngine:MoveBehaviour.obj, GameEngine:PredefinedLinkCauses.obj, GameEngine:TurnBehaviour.obj | |
| CActorChannelConnections | GameEngine:ActorChannelConnections.obj | No constructor found |
| CActorCollisionGrids | GameEngine:ActorGrid.obj | No constructor found | 
| CActorDrawInfo | GameEngine:Actor.obj, GameEngine:ActorChannels.obj, GameEngine:ActorDrawInfo.obj, GameEngine:Level.obj | No constructor found |
| CActorGrid | GameEngine:ActorGrid.obj | No constructor found |
| CActorGridSet | GameEngine:ActorGrid.obj, GameEngine:Level.obj | No constructor found |
| CActorHealth | GameCode:GameEvents.obj, GameEngine:Actor.obj, GameEngine:ActorChannels.obj, GameEngine:ActorHealth.obj |  This one I need to research/document further, every actor has a separate health object attached to it that stores 4 values, the only known values are minimum and maximum health, I'm not positive what the other two are. | 
| CActorJoyStick | GameEngine:ActorJoystick.obj, GameEngine:Game.obj, GameEngine:Widget.obj | No constructor found |
| CActorKeySequence | GameEngine:ActorKeySequence.obj, GameEngine:PredefinedLinkEvents.obj  | I believe this closely relates to the "Mode" it controls actor behavior possibly just animations but abstracted further.    | 
| CActorMode | GameEngine:ActorMode.obj | All of the mode stuff is strange, it seems to be animations but also 'actions' there are "handleCause" or "handleModeChange" that will cause the player to fire and it actually commits to the action and the animation, so I refer to the list of "modes" available for actors as "action tables" | 
| CActorSpecailities | No object files found | No constructor found |
| CActorTimers | GameEngine:Actor.obj, GameEngine:ActorChannels.obj, GameEngine:ActorTimers.obj, GameEngine:PredefinedLinkEvents.obj | No constructor found |
| CActorTree | No object files found | No constructor found |
| CActorTypeInfo | GameEngine:Actor.obj, GameEngine:ActorTypeInfo.obj, GameEngine:ActorTypeList.obj, GameEngine:Level.obj, GameEngine:MachinePart.obj | No constructor found |
| CActorTypeList | GameEngine:ActorTypeList.obj | No constructor found |
| CActorUpdateInfo | No object files found | No constructor found |
| CAmbientVolumeManager | GameEngine:AmbientVolumeManager.obj, GameEngine:Light.obj, GameEngine:Lighting.obj, XBOXSystem:StaticMesh.obj | No constructor found |
| CAngularPhysics | GameEngine:Actor.obj, GameEngine:ActorChannels.obj, GameEngine:AngularPhysics.obj | No constructor found |
| CAnimLink | CTG_Common:AnimLink.obj | No constructor found |
| CAnimationResourceManager | GameEngine:AnimationResourceManager.obj | No constructor found |
| CArrivedAtObject | No object files found | No constructor found |
| CArrivedAtObjectCause | No object files found | No constructor found |
| CAutoSpin | GameEngine:ActorChannels.obj, GameEngine:AutoSpin.obj | No constructor found |
| CBaseAI | No object files found | No constructor found |
| CBaseMaterial | CTG_Common:DynamicMeshBase.obj, CTG_Common:Mesh.obj, CTG_Common:RenderManager.obj, GameEngine:MaterialManager.obj, GameEngine:ScreenSprite.obj, GameEngine:VideoActor.obj, XBOXSystem:XBOXMaterialManager.obj, XBOXSystem:XBOXRenderManager.obj | No constructor found |
| CBasePath | GameEngine:Path.obj | No constructor found |
| CBaseTexture | GameEngine:MaterialManager.obj, XBOXSystem:StaticMesh.obj | No constructor found |
| CBasicPhysics | GameCode:Player.obj, GameCode:TestVehicle.obj, GameCode:VehicleSideScrolling.obj, GameEngine:Actor.obj, GameEngine:ActorChannels.obj, GameEngine:BasicPhysics.obj, GameEngine:Region.obj | No constructor found (Note: Found in GameCode project, meaning some of this is game specific) |
| CBehaviour | No object files found | No constructor found |
| CBehaviours | No object files found | No constructor found |
| CBlendedCamera | GameCode:Makita.obj, GameCode:Player.obj, GameCode:RedStarCamera.obj, GameEngine:BlendedCamera.obj, GameEngine:CameraBehavior.obj, GameEngine:DualPathBehavior.obj, GameEngine:PathBehavior.obj, GameEngine:PIPCamera.obj, GameEngine:PredefinedLinkEvents.obj, GameEngine:ThirdPersonCamera.obj, GameEngine:ToolsConnection.obj | No constructor found (Note: Found in GameCode project, meaning some of this is game specific) |
| CBounceBehaviour | GameEngine:Actor.obj, GameEngine:ActorChannels.obj, GameEngine:BounceBehaviour.obj, GameEngine:PredefinedLinkEvents.obj | No constructor found |
| CBoxCollisionPart | GameEngine:BoxCollisionPart.obj, GameEngine:CollisionGeometry.obj | No constructor found |
| CBoxRegion | GameEngine:Game.obj, GameEngine:Region.obj | No constructor found |
| CCalculator | GameEngine:ActorChannels.obj, GameEngine:Calculator.obj, GameEngine:MachinePart.obj, GameEngine:PredefinedLinkEvents.obj | No constructor found |
| CCalculatorLinkConditiional | No object files found | No constructor found |
| CCalculatorLinkEvent | GameEngine:LinkEvent.obj, GameEngine:PredefinedLinkEvents.obj | No constructor found |
| CCamera | GameEngine:ActorChannels.obj, GameEngine:Backdrop.obj, GameEngine:Camera.obj, GameEngine:Level.obj, GameEngine:PIPCamera.obj, GameEngine:portal.obj | No constructor found |
| CCameraBehavior | GameEngine:BlendedCamera.obj, GameEngine:CameraBehavior.obj, GameEngine:CircleBehavior.obj, GameEngine:DualPathBehavior.obj, GameEngine:PathBehavior.obj, GameEngine:PredefinedLinkEvents.obj | No constructor found |
| CCollision | GameCode:AttackDamage.obj, GameEngine:Actor.obj, GameEngine:Collision.obj, GameEngine:CollisionOctree.obj, GameEngine:Level.obj, GameEngine:PredefinedLinkCauses.obj, GameEngine:PredefinedLinkEvents.obj, GameEngine:WaterActor.obj | No constructor found (Note: Found in GameCode project, meaning some of this is game specific) |
| CCollsiionOctree | No object files found | No constructor found |
| CCollisionPart | GameEngine:Actor.obj, GameEngine:ActorTypeInfo.obj, GameEngine:BlendedCamera.obj, GameEngine:Collision.obj, GameEngine:CollisionGeometry.obj, GameEngine:CollisionPart.obj, GameEngine:OctreeCollisionPart.obj | No constructor found |
| CCompiledTextFileParser | GameEngine:TextFileParser.obj | No constructor found |
| CDamageBehaviour | GameEngine:Actor.obj, GameEngine:ActorChannels.obj, GameEngine:DamageBehaviour.obj, GameEngine:PredefinedLinkEvents.obj | No constructor found |
| CDamageInfo | GameCode:AttackDamage.obj, GameCode:Player.obj, GameEngine:Actor.obj, GameEngine:DamageBehaviour.obj, GameEngine:PredefinedLinkEvents.obj | No constructor found (Note: Found in GameCode project, meaning some of this is game specific), CDamageInfo is interesting as when someone is damaged all of the details about it some unknown are passed inside of this object. Such as who performed the damage how much damage was comitted etc. Some Actor OnDamage functions eventually handle whatever is contained within this object. |
| CDeity | GameEngine:Deity.obj, GameEngine:Level.obj | No constructor found |
| CDyanmicMesh | No object files found | Most of the interest in things like DynamicMesh, DyanmicMeshBase etc is to eventually make the ability to modify player models, or weapons in the game. |
| CDynamicMeshBase | CTG_Common:DynamicMeshBase.obj, XBOXSystem:DynamicMesh.obj | No constructor found |
| CEffect | GameEngine:Effect.obj, GameEngine:EffectSys.obj, GameEngine:LoadParticle.obj, GameEngine:ParticleSystem.obj | No constructor found |
| CEmit | GameEngine:Actor.obj, GameEngine:Effect.obj, GameEngine:Emit.obj, GameEngine:EmitSimple.obj, GameEngine:Particle.obj, GameEngine:ParticleSystemMemory.obj | No constructor found |
| CFog | GameEngine:ActorChannels.obj, GameEngine:Fog.obj, GameEngine:Level.obj, GameEngine:PredefinedLinkEvents.obj | The PC version of Turok 4 appears to have issues with fog, but the CFog class is defined and referenced in my IDB. |
| CFont | GameCode:DialogMessage.obj, GameCode:MarqueeWidget.obj, GameCode:Widget3D.obj, GameEngine:Font.obj, GameEngine:Fonts.obj | Not just used for font management but also used to draw text to the screen for pickup text, or etc. Has specific "Draw" functions or "Write" functions to write things to the screen in the game's font. |
| CFrameTime | GameEngine:AudioInstance.obj, GameEngine:Layer3.obj | No constructor found |
| CGame | GameCode:BeetleV2.obj, GameCode:Player.obj, GameCode:RedStarGame.obj, GameEngine:Actor.obj, GameEngine:ActorChannels.obj, GameEngine:ActorTypeInfo.obj, GameEngine:app.obj, GameEngine:Camera.obj, GameEngine:DualPathBehavior.obj, GameEngine:Game.obj, GameEngine:Level.obj, GameEngine:portal.obj, GameEngine:PostEffects.obj, GameEngine:Pre-CacheManager.obj, GameEngine:PredefinedLinkConditionals.obj, GameEngine:PredefinedLinkEvents.obj, GameEngine:ViewportScreenSprite.obj, GameEngine:Widget.obj, GameEngine:XBox_BlurEffect.obj | No constructor found (Note: Found in GameCode project, meaning some of this is game specific). Ultimately CGame is interesting because it and CLevel share a lot of things including keeping track of cameras, and players in some cases, though the Level handles more of the in-depth properties like collision,  CGame keeps track of more things like what level should be loaded next, if it should be restarted, rendering info like the resolution of the game,  or level creation info.|
| CGameControllers | GameEngine:Actor.obj, GameEngine:ActorChannels.obj, GameEngine:Game.obj, GameEngine:GameControllers.obj, GameEngine:PredefinedLinkCauses.obj, GameEngine:PredefinedLinkConditionals.obj | No constructor found |
| CGameMaterial | GameCode:GameMaterial.obj | No constructor found (Note: Found in GameCode project, meaning some of this is game specific) |
| CGamePath | No object files found | No constructor found |
| CGameSettings | GameEngine:GameSettings.obj | No constructor found |
| CGaussianFilter | No object files found | No constructor found |
| CGeneralClock | GameEngine:ActorChannels.obj, GameEngine:GeneralClock.obj, GameEngine:Level.obj, GameEngine:PredefinedLinkCauses.obj | No constructor found |
| CGeometry | GameEngine:Actor.obj, GameEngine:Geometry.obj, GameEngine:portal.obj, GameEngine:PredefinedLinkEvents.obj | No constructor found |
| CGeometryResourceManager | GameEngine:GeometryResourceManager.obj | No constructor found |
| CHUD | GameCode:Player.obj, GameEngine:HUD.obj | No constructor found (Note: Found in GameCode project, meaning some of this is game specific) |
| CHashedString | GameEngine:ActorTypeInfo.obj, GameEngine:ActorTypeList.obj, GameEngine:HashedString.obj | No constructor found |
| CHistories | GameEngine:History.obj, GameEngine:PredefinedLinkCauses.obj | Histories is how the game handles scripted events triggering actions or changing things and persists through into other screens, levels, etc. It stores data on everything from inverted controls to what cheats are enabled, to what map or actor a player selected when starting a multiplayer game. After that Actor Instance files can reference history and determine what's been chosen the way the engine exposes it. |
| CHistory | GameEngine:History.obj | No constructor found |
| CHistoryItem | GameCode:UpgradeWidget.obj, GameEngine:History.obj, GameEngine:PredefinedLinkEvents.obj | No constructor found (Note: Found in GameCode project, meaning some of this is game specific) |
| CHotPoints | GameEngine:Actor.obj, GameEngine:HotPoint.obj | No constructor found, the constructor stuff is wrong there is a HotPoint constructor, hotpoints are like fixed points related to bones where a weapon should attach and such. |
| CIndexedStringGroup | GameEngine:Game.obj, GameEngine:StringGroup.obj | No constructor found |
| CInputManager | GameEngine:ActorChannels.obj, GameEngine:Game.obj, GameEngine:InputManager.obj, GameEngine:PredefinedLinkCauses.obj, GameEngine:Widget.obj, GameEngine:XBOXInputManager.obj | No constructor found |
| CInventory | GameEngine:History.obj, GameEngine:Inventory.obj, GameEngine:Level.obj, GameEngine:ToolsConnection.obj | No constructor found |
| CJoystickHandler | No object files found | No constructor found |
| CLevel | GameCode:ActorFlasher.obj, GameCode:BossHealthMeter.obj, GameCode:GameChannels.obj, GameCode:Indicators.obj, GameCode:RedStarLevel.obj, GameCode:SpawnArea.obj, GameCode:SpawnTrackingInfo.obj, GameEngine:Actor.obj, GameEngine:ActorChannels.obj, GameEngine:ActorJoystick.obj, GameEngine:ActorRegions.obj, GameEngine:BlendedCamera.obj, GameEngine:Font.obj, GameEngine:Game.obj, GameEngine:Level.obj, GameEngine:LightningBolt.obj, GameEngine:LinkEvent.obj, GameEngine:NavData.obj, GameEngine:ParticleContainer.obj, GameEngine:ParticleSystem.obj, GameEngine:PIPCamera.obj, GameEngine:PlayerPathMonitor.obj, GameEngine:portal.obj, GameEngine:Pre-CacheManager.obj, GameEngine:PredefinedLinkCauses.obj, GameEngine:PredefinedLinkEvents.obj, GameEngine:Prop.obj, GameEngine:ScreenFlash.obj, GameEngine:ScreenProxy.obj, GameEngine:ToolsConnection.obj, GameEngine:Widget.obj | No constructor found (Note: Found in GameCode project, meaning some of this is game specific), there is a constructor my automated scripting just sucks. The Level object is as it sounds it contains lists of spawned Cameras, actors in the level from objects to players to AI, and keeps track of various other states of the Level itself. |
| CLight | GameEngine:ActorChannels.obj, GameEngine:Light.obj | No constructor found |
| CLighting | GameEngine:ActorChannels.obj, GameEngine:Lighting.obj | No constructor found |
| CLinkEvent | GameEngine:Link.obj, GameEngine:LinkEvent.obj, GameEngine:PredefinedLinkEvents.obj | No constructor found |
| CMesh | CTG_Common:DynamicMeshBase.obj, CTG_Common:Mesh.obj, GameCode:ActorFlasher.obj, GameCode:Player.obj, GameEngine:Actor.obj, GameEngine:ActorTypeInfo.obj, GameEngine:Particle.obj, GameEngine:PredefinedLinkEvents.obj, GameEngine:WaterActor.obj, XBOXSystem:StaticMesh.obj, XBOXSystem:XBOXRenderManager.obj | No constructor found (Note: Found in GameCode project, meaning some of this is game specific) |
| CMeshLoader | GameEngine:GameEngineResources.obj, GameEngine:GeometryResourceManager.obj, XBOXSystem:MeshLoader.obj | No constructor found |
| CMeshResource | CTG_Common:Mesh.obj, CTG_Common:MeshResource.obj, XBOXSystem:DynamicMesh.obj, XBOXSystem:MeshLoader.obj | No constructor found |
| CPlainTextFileParser | GameEngine:Game.obj, GameEngine:Pre-CacheManager.obj, GameEngine:TextFileParser.obj | No constructor found |
| CPlayer | GameCode:ActorCreator.obj, GameCode:GameChannels.obj, GameCode:GameEvents.obj, GameCode:Kyuzo.obj, GameCode:Makita.obj, GameCode:Maya.obj, GameCode:MotionPreviewWidget.obj, GameCode:Player.obj, GameCode:RedStarCamera.obj, GameCode:VehicleSideScrolling.obj | No constructor found (Note: Found in GameCode project, meaning some of this is game specific) |
| CPlayerObject | No object files found | No constructor found |
| CPostEffects | GameEngine:PostEffects.obj | No constructor found |
| CPostProcessing | No object files found | No constructor found |
| CRadialBlurEffect | GameEngine:PostEffectLinkEvents.obj, XBOXSystem:RadialBlurFullscreenEffectXBOX.obj | No constructor found, on PC these crash the game or cause unintended effects |
| CRadialBlurFullscreenEffect | No object files found | No constructor found, on PC these crash the game or cause unintended effects |
| CRegion | GameEngine:Level.obj, GameEngine:Region.obj | No constructor found, this is how death regions are defined |
| CSCreenSprite | GameCode:AttackStatus.obj, GameCode:DialogMessage.obj, GameCode:MovesWidget.obj, GameCode:PlayerStatus.obj, GameCode:ProtocolCross.obj, GameCode:UIXIndicator.obj, GameCode:UpgradeWidget.obj, GameCode:Widget3D.obj, GameEngine:ScreenSprite.obj, GameEngine:ViewportScreenSprite.obj, GameEngine:Widget.obj | No constructor found (Note: Found in GameCode project, meaning some of this is game specific), this is how HUD elements are drawn |
| CSoundManager | GameEngine:Pre-CacheManager.obj, GameEngine:SoundManager.obj, GameEngine:Sounds.obj | No constructor found |
| CSounds | GameEngine:Sounds.obj | No constructor found, every object has a list of CSounds calls when being constructed if it's intended to play sounds from players to weapons. |
| CStaticMesh | XBOXSystem:StaticMesh.obj | No constructor found |
| CStaticMeshResource | XBOXSystem:StaticMesh.obj | No constructor found |
| CStringGroup | GameEngine:Game.obj, GameEngine:StringGroup.obj | No constructor found |
| CStringHash | GameEngine:HashedString.obj, GameEngine:Level.obj | No constructor found |
| CTextFile | GameEngine:CTextFile.obj, GameEngine:Pre-CacheManager.obj, GameEngine:TextFileParser.obj | No constructor found |
| CTextGroups | GameEngine:Text.obj | No constructor found |
| CThirdPersonCamera | GameCode:RedStarCamera.obj, GameEngine:CircleBehavior.obj, GameEngine:PredefinedLinkEvents.obj, GameEngine:ThirdPersonCamera.obj | No constructor found (Note: Found in GameCode project, meaning some of this is game specific), Turok does have a thirdperson camera for cinematic things but I don't know if it's the same as this. |
| CTreeFileManager | GameEngine:TreeFileManager.obj | No constructor found, tre files are opened/referenced/streamed from utilizing this calss |
| CWidget | GameCode:MarqueeWidget.obj, GameCode:ObjectivesWidget.obj, GameCode:UpgradeWidget.obj, GameCode:Widget3D.obj, GameEngine:Game.obj, GameEngine:PredefinedLinkEvents.obj, GameEngine:Widget.obj | No constructor found (Note: Found in GameCode project, meaning some of this is game specific), UI elements like menus and such are all rendered using these Widgets |
| CollisionOctree | GameEngine:CollisionOctree.obj | No constructor found |
| Iff | CTG_Common:Anim.obj, CTG_Common:Morph.obj, GameEngine:CollisionOctree.obj, GameEngine:Font.obj, GameEngine:GeometryResourceManager.obj, GameEngine:iff.obj, GameEngine:MaterialManager.obj, GameEngine:Visibility.obj, XBOXSystem:XBOXMaterialManager.obj | No constructor found, handles all the loading/parsing of files. |
| Intersection | GameCode:AttackDamage.obj, GameEngine:Actor.obj, GameEngine:ActorGrid.obj, GameEngine:ActorRegions.obj, GameEngine:Collision.obj, GameEngine:CollisionOctree.obj, GameEngine:Intersection.obj, GameEngine:Level.obj, GameEngine:WaterActor.obj, GameEngine:WaterMeshBase.obj, XBOXSystem:WaterMesh.obj | No constructor found (Note: Found in GameCode project, meaning some of this is game specific) |
| [View](https://github.com/pnill/T4_Research/blob/master/TNT/Quagmire/engine/Q_View.hpp) | Quag_XBox:E_View.obj | No constructor found, while this is Xbox specific we do have the source for E_View.cpp in Quag_PC. |