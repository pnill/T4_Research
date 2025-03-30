# Quagmire / Turok Evolution Research

## Overview
I'm sort of aiming for this respository to serve as documentation into the research I've done into Turok Evolution and as a result the engine it's based on [Quagmire](https://github.com/pnill/T4_Research/blob/master/Quagmire.md).

## Games Based on the Same Engine
This is not an exhaustive list, but these games are known to at least rely on the same base version of Quagmire.

- Vexx
- Red Star
- 100 bullets

These games are roughly based on a upgraded version of Quagmire from the point where Turok Evolution was built.

- All Star Baseball series

## Debug Builds

While there have been several released/leaked debug/preview/pre-release builds of the game none have ever included symbols unfortunately, the same isn't true of Vexx, Red Star and 100 bullets.

[Hidden Palace](https://hiddenpalace.org/Turok:_Evolution_(Aug_13,_2002_prototype)) does list a number of pre-release builds for Turok Evolution if you're interested in them.

While some of these are interesting due to having a 'debug' menu that allows you to mess with the game in various ways, including enabling some debug rendering options, possess other players, free cam or spawn actors.

I haven't spent a lot of time reverse engineering those, most of what the debug menus are capable of I've already discovered within the game's engine either blindly reversing or matching symbols from the other games (Vexx, Red Star, and 100 bullets) to Turok Evolution itself.

The closest of them appears to be Vexx though 100 Bullets has some Turok specific references namely, those related to pickups the class is specifically named CTurokPickup, suggesting the code was roughly based on whatever version of the engine was being used to develop evolution.

All of the debug builds of the games built on Quagmire I've obtained can be found [here](https://github.com/pnill/T4_Research/tree/master/Other_Builds) though if you're familiar with IDA I recommend just utilizing the provided IDBs I mention later.

### Turok - Evolution (July 25, 2002 prototype) - Xbox - Evolution preview version only

This build of the game specifically was interesting as extracting the TRE files results in discovering that they screwed up and left behind a turok4_debug.xbe file and turok4.xbe. I believe this was unintentional and these files were accidentally copied into some folder being 'archived' by their automated tooling.

It's the only build where this occurred, and the IDA idb related to it is available [here](https://github.com/pnill/T4_Research/tree/master/IDA_IDBS) or the XBE I uploaded [here](https://github.com/pnill/T4_Research/tree/master/T4_XBES)

I've attempted executing the turok4_debug.xbe in emulators without a ton of luck but I haven't tried a ton.

What's interesting about the turok4_debug.xbe is while it's not a debug build containing symbols it is an actual debug compiled build meaning it contains all asserts, non-inlined versions of routines, and almost zero optimization. 

If I were to ever attempt a decompilation of the game this would probably be a good starting point.

## Individual Research

All of the research related to individual topics that somehow relate to Turok Evolution. Most of the research here will be for the PC Version only. I haven't completed all of it but I'd eventually like to document file formats for all of the files the game loads as much as I possibly can.

- [Quagmire Engine Research](https://github.com/pnill/T4_Research/blob/master/Quagmire.md)
- [Turok Evolution Specific Engine Research](https://github.com/pnill/T4_Research/blob/master/Turok_Evolution.md)
- TRE Files TBD
- History Files (HST) TBD
- Actor Files (ATR) TBD
- Actor Instance Files (ATI) TBD
- Model Files (MTF) TBD
- Text Files (TXM) TBD
- Actor Animation Files (ANM) TBD
- Actor Skin Files (ATF) TBD
- Particle Effects (EFF) TBD
- Particle Emitter (EMI) TBD
- Particle Defs (PARX) TBD
- MemCard Export File (MCF) TBD
- Swooshes (SWH) TBD
- Save Data - TBD
- PreCacheList (PCL) - TBD
