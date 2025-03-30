# CTurok4Game

Most of the reverse engineering of this is going to be based on other game's implementations of their own CGameNameGame, mostly 100 Bullets, Vexx, or Redstar.
I don't know a ton of about what's happening here and the current T4MP_V2 code probably references anything to do with this incorrectly.

## VTABLE functions

### CTurok4Game::Begin(CGameSettings* )
#### Address
    0x004830F0

#### Overview

Seems to be responsible for setting up initial game specific properties, selecting the language etc. Calls CGame::Begin within it which would do more engine/generic stuff rather than game specific initialization.

Gets the language utilizing ENG_GetLanguage, which would be Quagmire specific calls out to CGame::SetLanguage using the determined language.

Calls out to initialize/read data from Options.hst and FrontEnd.hst, constructs 3 separate FullScreen effects
- CRadialBlurEffect
- CPrismEdgedBlurEffect
- CWaterWarpFullScreenEffect

Initializes a CScreenSprite instance for the rotating club icon for loading screens.

Finally, calls out to CGame::PushLevelSoon pushing the Begin.atr which would start up the intro scenes.

### CTurok4Game::Update( )

#### Address
    0x004849C0
