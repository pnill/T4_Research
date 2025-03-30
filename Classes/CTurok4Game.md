# CTurok4Game

Most of the reverse engineering of this is going to be based on other game's implementations of their own CGameNameGame, mostly 100 Bullets, Vexx, or Redstar.
I don't know a ton of about what's happening here and the current T4MP_V2 code probably references anything to do with this incorrectly.

## Constructor
The CTurok4Game object is created using a `MakeGame` call called within `QuagMain`. The game allocates a total of 0x1258 bytes for it, and assigns the result of the constructor/initialization to `g_Turok4Game` which is later referenced all throughout the engine.

```cpp
CTurok4Game *__cdecl MakeGame(CIndexedStringGroup *a1)
{
  CTurok4Game *v1; // eax
  CTurok4Game *result; // eax

  g_GameAllocator[0] = MakeGameAllocator();
  v1 = (CTurok4Game *)__nw(0x1258);
  if ( v1 )
  {
    result = CTurok4Game::CTurok4Game(v1, a1);
    g_Turok4Game = result;
  }
  else
  {
    result = 0;
    g_Turok4Game = 0;
  }
  return result;
}
```

Additionally, when it returns from `MakeGame` QuagMain assigns it to `g_Game`:
```cpp
  g_Game = MakeGame((CIndexedStringGroup *)(this + 71606));
  if ( !g_Game )
    return 0x80004005;
```

My guess here is `g_Game` is intended to be `CGame` where `g_Turok4Game` is the game specific instance of the class that inherits everything from `CGame`.

In the xbox debug build of the game the class is significantly smaller, only allocating a totoal of `0x338` bytes:
```cpp
  g_Game = (int)MakeGame();
  if ( !g_Game && debugStr(0, "C:\\Projects\\NewTech\\Shared\\World\\app.cpp", 195, "g_Game != 0", 0) )
    __debugbreak();
```

```cpp
_DWORD *MakeGame()
{
  _DWORD *v1; // [esp+0h] [ebp-1Ch]
  _DWORD *v2; // [esp+4h] [ebp-18h]

  if ( g_GameAllocator
    && debugStr(0, "C:\\Projects\\NewTech\\Turok4\\Code\\TurokGame.cpp", 1399, "g_GameAllocator == 0", 0) )
  {
    __debugbreak();
  }
  g_GameAllocator = MakeGameAllocator();
  v2 = (_DWORD *)__nw(0x338);
  if ( v2 )
    v1 = CTurok4Game::CTurok4Game(v2);
  else
    v1 = 0;
  g_TurokGame = (int)v1;
  return v1;
}
```

Most of the known members pointers to specific things like CSoundManager, ActorTypeList etc were discovered by reversing the `CGame` constructor vs the `CTurok4Game` constructor, a lot of what I currently have in my IDBs is seemingly inaccurate also in terms of all of the members of this. Some of this is due to false positives when comparing to other databases/games with debug symbols. It's not always a 1:1 process matching the data up on a routine or what's being called in a similar chain.


## Members / Structure
I use "int" as a place holder for anything that's 4 bytes, even if it's not actually represented as an integer. Where possible or if I know it's a pointer I've updated the table to void*.

| m_Type               | m_Name              | offset | Notes                                                                                                                                                   |
|:-------------------- | :------------------ | :----- | :------------------------------------------------------------------------------------------------------------------------------------------------------ |
| int                  | field_0x04          |  0x04  | Manipulating it seemed to have no imapct on it, not sure what it is or used for.                                                                        |
| CLevel*              | m_pLevel            |  0x08  | This might acutally be something like loading level, it's set in CLevel:Update and zero'd out later.                                                    | 
| Font**               | m_pFontList         |  0x0C  | "Font**" probably isn't the appropriate type here but you get the idea, it's a list of CFont object pointers.                                           |
| CIndexedStringGroup* | m_pStringGroup      |  0x10  | All of the strings current loaded into the game in some sorted array.                                                                                   | 
| CLevel**             | m_levelListBegin    |  0x14  | Technically this is a vector and there's something like .begin() happening here which this points to.                                                   |
| CLevel**             | m_levelListEnd      |  0x18  | Like the above, it's the pointer to the vector but .end().                                                                                              |
| int                  | field_0x1C          |  0x1C  | Unknown, appeared to be set to a pointer but not sure.                                                                                                  |
| void*                | m_StandbyLevelBegin |  0x20  | Accordding to old notes I didn't know what it was, but I seem to have labeled it as the beginning of a standby level array, like the level list vector. |
| void*                | m_StandbyLevelEnd   |  0x24  | Like the above, technically neither of them are void* but I don't know what datatype they're supposed to have likely CLevel**                           | 
| int                  | field_0x28          |  0x28  | Uknown, setting it to 1-255 had no impact on the game.                                                                                                  |
| char*                | dataPath            |  0x2C  | A non-translated path to where the data folder is, e.x. "$\Data\"                                                                                       |
| CLevelCreationInfo*  | m_LevelCreationInfo |  0x30  | An object indiciating what information is passed to create/push a level. I assume this contains data about the last level pushed.                       |
| int                  | field_0x34          |  0x34  | Unknown but initialized to 1 during MP modes, resets itself to 1 if you update it to 0. Breakpointing it could be useful in discovering what's going on.|
| int                  | field_0x38          |  0x38  | Same as the above at 0x34.                                                                                                                              |
| int                  | field_0x3C          |  0x3C  | Unknown have no other details.                                                                                                                          |
| int                  | field_0x40          |  0x40  | Unknown.                                                                                                                                                |
| int                  | field_0x44          |  0x44  | Unknown.                                                                                                                                                |
| void*                | field_0x48          |  0x48  | Pointer of some sort, but unknown.                                                                                                                      |
| char*                | m_LevelPath         |  0x4C  | Utilized in CGame::PushLevelSoon.                                                                                                                       |
| char*                | m_ClassName         |  0x50  | Statically assigned to "Turok4".                                                                                                                        |
| CLevel**             | m_PendingLevels     |  0x54  | Array of pending levels, x_std_vector most likely.                                                                                                      |
| CLevel**             | m_PendingLevelsEnd  |  0x58  | Seems to be the end of the array. Likely the result of vector.end().                                                                                    |
| CLevel**             | m_PendingLevelsBegin|  0x5C  | Likely the result of vector.begin()                                                                                                                     |
| int                  | field_0x60          |  0x60  | Unknown, incremented actively.                                                                                                                          |
| float                | field_0x64          |  0x64  | Unknown, incremented actively in MP, appears to be audio tick related.                                                                                  |
| float                | field_0x68          |  0x68  | unknown copies the previous field.                                                                                                                      |
| float                | field_0x6C          |  0x6C  | unknown - updateCount, incremented on CGame::Update.                                                                                                    |
| int                  | field_0x70          |  0x70  | unknown.                                                                                                                                                |
| void*                | m_pHostInfo         |  0x74  | Actually don't know what CHostInfo is responsible for, but this is what it points at according to notes.                                                |
| int                  | field_0x78          |  0x78  | unknown.                                                                                                                                                |
| int                  | field_0x7C          |  0x7C  | unknown.                                                                                                                                                |
| CHistories*          | m_pHistories        |  0x80  | Where the game stores all the data about histories that are loaded/accessed.                                                                            |
| float                | field_0x84          |  0x84  | Unknown, modifying it manipulated the camera. Values over 200 resulted in the camera being upside down.                                                 |
| int                  | globalRenderFlags   |  0x88  | Uknown what render flags are specified, originally had notes stating it modified an update timer.                                                       |
| float                | field_0x8C          |  0x8C  | Uknown. Most likely rendering related based on previous values.                                                                                         |
| int                  | field_0x90          |  0x90  | Uknown.                                                                                                                                                 |
| CTextGroups*         | m_pTextGroups       |  0x94  | Haven't reverse engineered "CTextGroups" yet but according to notes it points at an instance of it.                                                     |
| CTreeFileManager*    | m_pTreeFileManager  |  0x98  | Haven't reverse engineered "CTreeFIleManager" but tree files are mounted/streamed from and act like archives for game data.                             |
| CMemoryUnit*         | m_pMemoryUnit       |  0x9C  | Haven't reverse engineered "CMemoryUnit" or "CMemoryUnitManager" but this appears to point to an instance of it.                                        |
| CSoundManager*       | m_pSoundManager     |  0xA0  | Haven't reverse engineered "CSoundManager" but this points to an instance of it.                                                                        |
| CGameControllers*    | m_pGameControllers  |  0xA4  | Like the above haven't reverse engineered "CGameControllers" but this has a pointer to an instance of it.                                               |
| int                  | m_LangIndex         |  0xA8  | Set by CGame::SetLanguage                                                                                                                               |
| CActorTypeList*      | m_pActorTypeList    |  0xAC  | Pointer to array of Actor Types.                                                                                                                        |
| bool                 | field_0xB0          |  0xB0  | Unknown, checked when determining if specific sounds should be loaded.                                                                                  |
| char                 | field_0xB1          |  0xB1  | Unknown.                                                                                                                                                |
| bool                 | field_0xB2          |  0xB2  | Checked before loading font list and sounds.                                                                                                            |
| char                 | field_0xB3          |  0xB3  | Unknown.                                                                                                                                                |
| int                  | field_0xB4          |  0xB4  | Unknown.                                                                                                                                                |
| int                  | field_0xB8          |  0xB8  | Unknown.                                                                                                                                                |

## Custom Functions
I created some custom functions to handle things that were inlined by the engine, so basically re-implementations of functions that no longer exist based on reverse engineering. Plan to list some of them here.

### levelCount()
The game inlined every reference to this in the Turok4_Debug.xbe and some of the other debug builds for other games you'll see it be referenced so I re-implemented it in my own instance of the class.
```cpp
	int levelCount()
	{
		return (((int)this->levelListEnd - (int)this->levelListBegin) >> 2);
	}
```

## VTABLE functions
| Class / Function                                                                  | vtblAddress | Notes                                                                                                                                                                                                                               |
| :-----------------------------------------------------                            | :---------- | :---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| CTurok4Game::~CTurok4Game                                                         | 0x0063C778  | Self explanatory.                                                                                                                                                                                                                   |
| CTurok4Game::Begin(CGameSettings *)                                               | 0x0063C77C  | Almost every class in the game implements a "Begin", "End", "Update", and "Render", Begin acts as some form of initialization.                                                                                                      |
| CTurok4Game::Update(float)                                                        | 0x0063C780  | Called every tick, used to update physics or things unrelated to rendering calls CGame::Update which then begins to loop arrays of levels an actors and work it's way down calling everything else's update function as well.       |
| CTurok4Game::Render(void)                                                         | 0x0063C784  | Sort of self explanatory based on the above descriptions.                                                                                                                                                                           |
| CTurok4Game::Name(void)                                                           | 0x0063C788  | Just returns "Turok4" statically.                                                                                                                                                                                                   |
| CGame::Version(const(void))                                                       | 0x0063C78C  | Date/Time when the build was produced, I'm guessing generated by a compiler macro.                                                                                                                                                  |
| CGame::LevelSelectFontName(void)                                                  | 0x0063C790  | Seems to return a static font, got the name from other builds unsure if it's accurate.                                                                                                                                              |
| Optimize_Ret_Float0                                                               | 0x0063C794  | This is some custom naming I came up with, basically the compiler decided anything that returned 0.0f would use the routine pointed to here.                                                                                        |
| sub_482AE0                                                                        | 0x0063C798  | Unknown, returns a globally defined float value 5000.0f could be related to sounds, render distance etc.                                                                                                                            |
| Optimize_Ret_Float0                                                               | 0x0063C79C  | Same as previous instance of this, means it returns 0.0f.                                                                                                                                                                           |
| CTurok4Game::DefaultSoundUpdateDistance(const(void))                              | 0x0063C7A0  | 100 bullets is the only one that also returns 0x1E/30                                                                                                                                                                               |
| CTurok4Game::InitializeLoadingScreen((void))                                      | 0x0063C7A4  | Seems to create all of textures, CHud instances and CScreenSprite for loading screen related stuff.                                                                                                                                 |
| CTuork4Game::UpdateLoadingScreen((void))                                          | 0x0063C7A8  | Same as other "Update" routines but specific to the loadgin screen setup.                                                                                                                                                           |
| CTurok4Game::RenderLoadingScreen(void)                                            | 0x0063C7AC  | Mostly self explanatory.                                                                                                                                                                                                            |
| CTurok4Game::DestroyLoadingScreen(void)                                           | 0x0063C7B0  | Again mostly self explanatory.                                                                                                                                                                                                      |
| CTurok4Game::InitializeFileSystem(void)                                           | 0x0063C7B4  | Related to tre initialization stuff.                                                                                                                                                                                                |
| CGame::ShadowMaterial(int)                                                        | 0x0063C7B8  | Not actually used in PC version, maybe shadows or this specific type of shadow is disabled in T4 PC? Xbox debug build also just returns -1                                                                                          |
| CGame::ShadowMesh(int)                                                            | 0x0063C7BC  | Like the above, not implemented just some generic return.                                                                                                                                                                           |
| ret0                                                                              | 0x0063C7C0  | Custom name for some optimized routine that just returns 0 and gets referenced everywhere.                                                                                                                                          |
| sub_4836B0                                                                        | 0x0063C7C4  | Uknown, tried to compare to same area in the vtable of 100 Bullets, Red Star and Vexx, can't seem to match it to anything going on there.                                                                                           |
| CTurok4Game::CreateLevel((char const *))                                          | 0x0063C7C8  | Somewhat self explanatory.                                                                                                                                                                                                          |
| CTurok4Game::CreateActor((CLevel *,char const *,char const *,char const *))       | 0x0063C7CC  | Somewhat self explanatory.                                                                                                                                                                                                          |
| CTurok4Game::CreateLinkCause(char const *)                                        | 0x0063C7D0  | Creates a link cause component, likely part of the game's event system.                                                                                                                                                             |
| CTurok4Game::CreateLinkEvent((char const *))                                      | 0x0063C7D4  | Creates a link event, complementary to the link cause functionality.                                                                                                                                                                |
| CGame::CreateLinkConditional((char const *))                                      | 0x0063C7D8  | Creates a conditional component for the link system.                                                                                                                                                                                |
| ret_pop4_1                                                                        | 0x0063C7DC  | Custom named function that returns 1 and pops 4 bytes from the stack.                                                                                                                                                               |
| CTurok4Game::CreateMaterial((char const *))                                       | 0x0063C7E0  | Creates material instances based on string identifier.                                                                                                                                                                              |
| CBulletsGame::AdjustMaterialType(const(uint))                                     | 0x0063C7E4  | Appears to be inherited from the Bullets Game engine, adjusts material types.                                                                                                                                                       |
| CGame::CreateActorModeDetails(const(void))                                        | 0x0063C7E8  | Creates actor mode details, likely used for actor states or behavior modes.                                                                                                                                                         |
| CBulletsGame::CreatePath(const(void))                                             | 0x0063C7EC  | Creates a path object, likely for AI navigation or scripted movement.                                                                                                                                                               |
| CTurok4GameC::CreatePathNode((void))                                              | 0x0063C7F0  | Creates a path node, part of the path system.                                                                                                                                                                                       |
| CTurok4Game::CreateNavigationNode(void)                                           | 0x0063C7F4  | Creates a navigation node for the AI navigation system.                                                                                                                                                                             |
| CTurokGame::CreateNavigationLink(const(void))                                     | 0x0063C7F8  | Creates links between navigation nodes.                                                                                                                                                                                             |
| CTurok4Game::CreateNavigationData(void)                                           | 0x0063C7FC  | Creates navigation data structure for the game's pathfinding system.                                                                                                                                                                |
| CGame::HandleNewLevel((CLevel &,int))                                             | 0x0063C800  | Handles logic when transitioning to a new level.                                                                                                                                                                                    |
| CTurokGameF::HandleChangeInConnectedControllers(uint,uint)                        | 0x0063C804  | Handles changes in connected controllers, likely for supporting hot-swapping controllers.                                                                                                                                           |
| ret_no_pop                                                                        | 0x0063C808  | Custom named function that returns without popping anything from the stack.                                                                                                                                                         |
| CTurokGame::AdjustedUpdateTime(const(float))                                      | 0x0063C80C  | Adjusts the update time, possibly for frame rate independence.                                                                                                                                                                      |
| CTurokGameR::AssignMemoryUnitData(CMemoryUnitData)                                | 0x0063C810  | Handles memory unit data assignment, likely for save games.                                                                                                                                                                         |
| ret_pop4_1                                                                        | 0x0063C814  | Another instance of the function that returns 1 and pops 4 bytes from the stack.                                                                                                                                                    |
| CTurokGame::HandleLoadDataComplete((CMemoryUnitData &))                           | 0x0063C818  | Handles completion of data loading, part of the save/load system.                                                                                                                                                                   |
| ret_pop4_1                                                                        | 0x0063C81C  | Another instance of the function that returns 1 and pops 4 bytes from the stack.                                                                                                                                                    |
| CTurokGame::HandleCheckAvailableSpaceComplete((CMemoryUnitData &))                | 0x0063C820  | Handles completion of checking available space, part of the save system.                                                                                                                                                            |
| ret_pop4_1                                                                        | 0x0063C824  | Another instance of the function that returns 1 and pops 4 bytes from the stack.                                                                                                                                                    |
| ret_pop4_1                                                                        | 0x0063C828  | Another instance of the function that returns 1 and pops 4 bytes from the stack.                                                                                                                                                    |
| ret_pop12                                                                         | 0x0063C82C  | Custom named function that pops 12 bytes from the stack and returns.                                                                                                                                                                |
| ret_no_pop                                                                        | 0x0063C830  | Another instance of the function that returns without popping anything from the stack.                                                                                                                                              |
| CGame::RestartLevel((char const *))                                               | 0x0063C834  | Handles restarting the current level.                                                                                                                                                                                               |
| ProportionalFont_Name                                                             | 0x0063C838  | Reference to a proportional font name string.                                                                                                                                                                                       |
| DebugFont_Name                                                                    | 0x0063C83C  | Reference to a debug font name string.                                                                                                                                                                                              |

