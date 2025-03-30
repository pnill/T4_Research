///////////////////////////////////////////////////////////////////////////
//
//  AM_STATS.HPP		
//
///////////////////////////////////////////////////////////////////////////

#ifndef _AM_STATS_HPP_
#define _AM_STATS_HPP_

#include "x_types.hpp"
#include "x_time.hpp"

#if !defined(TARGET_PS2_DVD) && !defined(TARGET_GC_DVD) && !defined(TARGET_XBOX_DVD)
//#define	ANIM_STATS_ACTIVE					// must be defined for the recording of these states to occur
#endif

#define	ANIM_STATS_AVERAGE_NUMFRAMES	60	// number of frames over which averages will be taken

#define	FLT	f64

namespace AMStats
{

///////////////////////////////////////////////////////////////////////////
// Enumeration of different data stored in the stats recorder
enum EStatCatagory
{
	AMSTAT_SETANIM,
	AMSTAT_ADVNFRAMES,
	AMSTAT_JUMPFRAME,
	AMSTAT_DECOMP,
	AMSTAT_INTERP,
	AMSTAT_PROPINTERP,
	AMSTAT_BLEND,
	AMSTAT_PROPBLEND,
	AMSTAT_PREPBLEND,
	AMSTAT_BUILDMATS,
	AMSTAT_BUILDMAT,
	AMSTAT_AM_EVENTS,
	AMSTAT_APP_EVENTS,

	NUM_AMSTATS,

	AMSTATTYPE_FRAME	= 100,
	AMSTATTYPE_TOTAL	= 101,
	AMSTATTYPE_AVERAGE	= 102,

	NUM_AMSTATTYPES = 3
};

///////////////////////////////////////////////////////////////////////////
struct SAnimStat_Float
{
	FLT	Total;
	FLT	History[ANIM_STATS_AVERAGE_NUMFRAMES];
	FLT Highest;
};

///////////////////////////////////////////////////////////////////////////
struct SAnimStat_Int
{
	s32	Total;
	s32	History[ANIM_STATS_AVERAGE_NUMFRAMES];
	s32	Highest;
};

///////////////////////////////////////////////////////////////////////////
struct SAnimStat_Set
{
	SAnimStat_Float	Time;
	SAnimStat_Int	Count;
};

///////////////////////////////////////////////////////////////////////////
//	Animation engine stats structure
///////////////////////////////////////////////////////////////////////////
struct SAnimStatsRecorded
{
	//---	all of the basic stats stored
	SAnimStat_Set	BasicStats[NUM_AMSTATS];

	SAnimStat_Int	DecompFrames;
	SAnimStat_Int	DecompStreams;
};

///////////////////////////////////////////////////////////////////////////
//	MACROS TO PERFORM CODING
///////////////////////////////////////////////////////////////////////////

#ifdef ANIM_STATS_ACTIVE

#define AMSTATS_SETANIM_START				FLT animstats_time_start												= (FLT) x_GetTime();
#define	AMSTATS_SETANIM_END					AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_SETANIM].Time.Total		+= (FLT) x_GetTime() - animstats_time_start;	\
											AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_SETANIM].Count.Total++;
												
#define AMSTATS_ADVNFRAMES_START			FLT animstats_time_start												= (FLT) x_GetTime();
#define	AMSTATS_ADVNFRAMES_END				AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_ADVNFRAMES].Time.Total	+= (FLT) x_GetTime() - animstats_time_start;	\
											AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_ADVNFRAMES].Count.Total++;
												
#define AMSTATS_JUMPFRAME_START				FLT animstats_time_start												= (FLT) x_GetTime();
#define	AMSTATS_JUMPFRAME_END				AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_JUMPFRAME].Time.Total	+= (FLT) x_GetTime() - animstats_time_start;	\
											AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_JUMPFRAME].Count.Total++;

#define AMSTATS_DECOMP_START				FLT animstats_time_start												= (FLT) x_GetTime();
#define	AMSTATS_DECOMP_END(Frames,Streams)	AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_DECOMP].Time.Total		+= (FLT) x_GetTime() - animstats_time_start;	\
											AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_DECOMP].Count.Total++;														\
											AMStats::g_AM_Stats.DecompFrames.Total									+= (Frames);									\
											AMStats::g_AM_Stats.DecompStreams.Total									+= (Streams);

#define AMSTATS_INTERP_START				FLT animstats_time_start												= (FLT) x_GetTime();
#define	AMSTATS_INTERP_END					AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_INTERP].Time.Total		+= (FLT) x_GetTime() - animstats_time_start;	\
											AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_INTERP].Count.Total++;

#define AMSTATS_PROPINTERP_START			FLT animstats_time_start												= (FLT) x_GetTime();
#define	AMSTATS_PROPINTERP_END				AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_PROPINTERP].Time.Total	+= (FLT) x_GetTime() - animstats_time_start;	\
											AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_PROPINTERP].Count.Total++;

#define AMSTATS_BLEND_START					FLT animstats_time_start												= (FLT) x_GetTime();
#define	AMSTATS_BLEND_END					AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_BLEND].Time.Total		+= (FLT) x_GetTime() - animstats_time_start;	\
											AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_BLEND].Count.Total++;
	
#define AMSTATS_PROPBLEND_START				FLT animstats_time_start												= (FLT) x_GetTime();
#define	AMSTATS_PROPBLEND_END				AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_PROPBLEND].Time.Total	+= (FLT) x_GetTime() - animstats_time_start;	\
											AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_PROPBLEND].Count.Total++;

#define AMSTATS_PREPBLEND_START				FLT animstats_time_start												= (FLT) x_GetTime();
#define	AMSTATS_PREPBLEND_END				AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_PREPBLEND].Time.Total	+= (FLT) x_GetTime() - animstats_time_start;	\
											AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_PREPBLEND].Count.Total++;

#define AMSTATS_BUILDMATS_START				FLT animstats_time_start												= (FLT) x_GetTime();
#define	AMSTATS_BUILDMATS_END				AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_BUILDMATS].Time.Total	+= (FLT) x_GetTime() - animstats_time_start;	\
											AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_BUILDMATS].Count.Total++;

#define AMSTATS_BUILDMAT_START				FLT animstats_time_start												= (FLT) x_GetTime();
#define	AMSTATS_BUILDMAT_END				AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_BUILDMAT].Time.Total		+= (FLT) x_GetTime() - animstats_time_start;	\
											AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_BUILDMAT].Count.Total++;

#define AMSTATS_AM_EVENTS_START				FLT animstats_time_start												= (FLT) x_GetTime();
#define	AMSTATS_AM_EVENTS_END(NEvents)		AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_AM_EVENTS].Time.Total	+= (FLT) x_GetTime() - animstats_time_start;	\
											AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_AM_EVENTS].Count.Total	+= NEvents;

#define AMSTATS_APP_EVENTS_START			FLT animstats_time_start												= (FLT) x_GetTime();
#define	AMSTATS_APP_EVENTS_END(NEvents)		AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_APP_EVENTS].Time.Total	+= (FLT) x_GetTime() - animstats_time_start;	\
											AMStats::g_AM_Stats.BasicStats[AMStats::AMSTAT_APP_EVENTS].Count.Total	+= NEvents;
void AMSTATS_Init( void );
void AMSTATS_Update( void );
void AMSTATS_Render( void );
void AMSTATS_Clear( void );
void AM_STATS_DisplayStat( EStatCatagory WhichStat, xbool bSetting );
f64 DisplayStats_WithInput( void );


												
////////////////////////////////////////////////////////////////////////////
//  !ANIM_STATS_ACTIVE
////////////////////////////////////////////////////////////////////////////
#else	// !ANIM_STATS_ACTIVE

#define AMSTATS_SETANIM_START
#define	AMSTATS_SETANIM_END

#define AMSTATS_ADVNFRAMES_START
#define AMSTATS_ADVNFRAMES_END

#define AMSTATS_JUMPFRAME_START
#define	AMSTATS_JUMPFRAME_END

#define AMSTATS_DECOMP_START
#define	AMSTATS_DECOMP_END(Frames,Streams)

#define AMSTATS_INTERP_START
#define	AMSTATS_INTERP_END

#define AMSTATS_PROPINTERP_START
#define	AMSTATS_PROPINTERP_END

#define AMSTATS_BLEND_START
#define	AMSTATS_BLEND_END

#define AMSTATS_PROPBLEND_START
#define	AMSTATS_PROPBLEND_END

#define AMSTATS_PREPBLEND_START
#define	AMSTATS_PREPBLEND_END

#define AMSTATS_BUILDMATS_START
#define	AMSTATS_BUILDMATS_END

#define AMSTATS_BUILDMAT_START
#define	AMSTATS_BUILDMAT_END

#define AMSTATS_AM_EVENTS_START
#define	AMSTATS_AM_EVENTS_END(Count)

#define AMSTATS_APP_EVENTS_START
#define	AMSTATS_APP_EVENTS_END(Count)

inline void AMSTATS_Init( void ){};
inline void AMSTATS_Update( void ){};
inline void AMSTATS_Render( void ){};
inline void AMSTATS_Clear( void ){};
inline void AM_STATS_DisplayStat( EStatCatagory WhichStat, xbool bSetting ){};
inline f64 DisplayStats_WithInput( void ){ return 0.0; }

#endif // ANIM_STATS_ACTIVE


//---	extern to the global animation stats structure.
extern struct SAnimStatsRecorded	g_AM_Stats;
extern xbool						g_AM_DisplayStatCatagory[NUM_AMSTATS];
extern xbool						g_AM_bDisplayFrameStats;
extern xbool						g_AM_bDisplayHighStats;
extern xbool						g_AM_bDisplayAverageStats;
//extern char* AMStats::s_AMSTAT_DisplayNames[NUM_AMSTATS] =


#define	FLT	f64	// float accuracy

};

#endif