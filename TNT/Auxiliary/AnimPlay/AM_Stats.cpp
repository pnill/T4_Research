#include "AM_Stats.h"
#include "x_plus.hpp"
#include "x_debug.hpp"
#include "x_stdio.hpp"
#include "Q_Input.hpp"

using namespace AMStats;

#ifdef ANIM_STATS_ACTIVE

static char* s_AMSTAT_DisplayNames[NUM_AMSTATS] =
{
	"SetAnimation()",
	"AdvNFrames()",
	"JumpToFrame()",
	"Decompression",
	"Interpolation",
	"Prop Interp",
	"Bone Blending",
	"Prop Blending",
	"PrepBlending()",
	"Build Matrices",
	"Build Matrix",
	"AM Events",
	"App Events"
};


#define	HIST_LEN	ANIM_STATS_AVERAGE_NUMFRAMES

#define	DISPLAY_TIME_UNITS(t)	((t)*1000.0f)	// convertion to milliseconds
#define	DISPLAY_YRES			25
#define	DISPLAY_XINC			20

SAnimStatsRecorded	AMStats::g_AM_Stats;

xbool				AMStats::g_AM_DisplayStatCatagory[NUM_AMSTATS];
xbool				AMStats::g_AM_bDisplayFrameStats;
xbool				AMStats::g_AM_bDisplayHighStats;
xbool				AMStats::g_AM_bDisplayAverageStats;


//=================================================================================================
//	Foreward declarations
//=================================================================================================

//=================================================================================================
static void UpdateHistoryFloat( FLT* pHistory, FLT Value )
{
	FLT*	pHist	= &pHistory[0];
	FLT*	pEnd	= &pHistory[ANIM_STATS_AVERAGE_NUMFRAMES-1];

	while( pHist!=pEnd )
		*pHist++ = *(pHist+1);

	pHistory[ANIM_STATS_AVERAGE_NUMFRAMES-1] = Value;
}

//=================================================================================================
static void UpdateHistoryInt( s32* pHistory, s32 Value )
{
	s32*	pHist	= &pHistory[0];
	s32*	pEnd	= &pHistory[ANIM_STATS_AVERAGE_NUMFRAMES-1];

	while( pHist!=pEnd )
		*pHist++ = *(pHist+1);

	pHistory[ANIM_STATS_AVERAGE_NUMFRAMES-1] = Value;
}

//=================================================================================================
static FLT CalcHistoryAverageFloat( FLT* pHistory )
{
	FLT*	pHist	= &pHistory[0];
	FLT*	pEnd	= &pHistory[ANIM_STATS_AVERAGE_NUMFRAMES];
	FLT		Total	= 0.0f;

	while( pHist!=pEnd )
		Total += *pHist++;

	return (Total / (FLT)ANIM_STATS_AVERAGE_NUMFRAMES);
}

//=================================================================================================
static s32 CalcHistoryAverageInt( s32* pHistory )
{
	s32*	pHist	= &pHistory[0];
	s32*	pEnd	= &pHistory[ANIM_STATS_AVERAGE_NUMFRAMES];
	s32		Total	= 0;

	while( pHist!=pEnd )
		Total += *pHist++;

	return (s32)(((f32)Total / (f32)ANIM_STATS_AVERAGE_NUMFRAMES)+0.5f);
}

//=================================================================================================
static void UpdateFloatStat( SAnimStat_Float& rStat )
{
	UpdateHistoryFloat( rStat.History, rStat.Total );
	if( rStat.Total > rStat.Highest )
		rStat.Highest = rStat.Total;
}

//=================================================================================================
static void UpdateIntStat( SAnimStat_Int& rStat )
{
	UpdateHistoryInt( rStat.History, rStat.Total );
	if( rStat.Total > rStat.Highest )
		rStat.Highest = rStat.Total;
}

//=================================================================================================
static void UpdateStatSet( SAnimStat_Set& rStat )
{
	UpdateFloatStat( rStat.Time );
	UpdateIntStat( rStat.Count );
}


//=================================================================================================
void AMSTATS_DisplaySection( s32 StatID, s32& X, s32& Y, xbool bFrame, xbool bHigh, xbool bAverage )
{
	SAnimStat_Set* pStatSet = &g_AM_Stats.BasicStats[StatID];

	x_printfxy( X, Y++, "== %s ==", s_AMSTAT_DisplayNames[StatID] );


	if( StatID == AMSTAT_DECOMP )
	{
		if( bFrame )
		{
			if( (Y+6) > DISPLAY_YRES )
			{
				X += DISPLAY_XINC;
				Y = 0;
			}

			FLT	TimePerBlock	= (pStatSet->Count.Total>0) ? pStatSet->Time.Total / pStatSet->Count.Total : 0;
			FLT	TimePerFrame	= (g_AM_Stats.DecompFrames.Total>0) ? pStatSet->Time.Total / g_AM_Stats.DecompFrames.Total : 0;
			FLT	TimePerStream	= (g_AM_Stats.DecompStreams.Total>0) ? pStatSet->Time.Total / g_AM_Stats.DecompStreams.Total : 0;
			x_printfxy( X, Y++, "Per Frame" );
			x_printfxy( X, Y++, " time   =%10.6f", DISPLAY_TIME_UNITS(pStatSet->Time.Total) );
			x_printfxy( X, Y++, " blocks =%3d",	pStatSet->Count.Total );
			x_printfxy( X, Y++, " frames =%3d",	g_AM_Stats.DecompFrames.Total );
			x_printfxy( X, Y++, " streams=%3d",	g_AM_Stats.DecompStreams.Total );
			x_printfxy( X, Y++, " t/b    =%10.6f", DISPLAY_TIME_UNITS(TimePerBlock) );
			x_printfxy( X, Y++, " t/f    =%10.6f", DISPLAY_TIME_UNITS(TimePerFrame) );
			x_printfxy( X, Y++, " t/s    =%10.6f", DISPLAY_TIME_UNITS(TimePerStream) );
		}

		if( bAverage )
		{
			if( (Y+6) > DISPLAY_YRES )
			{
				X += DISPLAY_XINC;
				Y = 0;
			}

			FLT	AverageTime		= CalcHistoryAverageFloat( pStatSet->Time.History );
			s32	AverageBlocks	= CalcHistoryAverageInt( pStatSet->Count.History );
			s32	AverageFrames	= CalcHistoryAverageInt( g_AM_Stats.DecompFrames.History );
			s32	AverageStreams	= CalcHistoryAverageInt( g_AM_Stats.DecompStreams.History );
			FLT TimePerBlock	= (AverageBlocks>0) ? AverageTime / (FLT)AverageBlocks : 0;
			FLT TimePerFrame	= (AverageFrames>0) ? AverageTime / (FLT)AverageFrames : 0;
			FLT TimePerStream	= (AverageStreams>0) ? AverageTime / (FLT)AverageStreams : 0;
			x_printfxy( X, Y++, "Average" );
			x_printfxy( X, Y++, " time   =%10.6f", DISPLAY_TIME_UNITS(AverageTime) );
			x_printfxy( X, Y++, " blocks =%3d",	AverageBlocks );
			x_printfxy( X, Y++, " frames =%3d",	AverageFrames );
			x_printfxy( X, Y++, " streams=%3d",	AverageStreams );
			x_printfxy( X, Y++, " t/b    =%10.6f", DISPLAY_TIME_UNITS(TimePerBlock) );
			x_printfxy( X, Y++, " t/f    =%10.6f", DISPLAY_TIME_UNITS(TimePerFrame) );
			x_printfxy( X, Y++, " t/s    =%10.6f", DISPLAY_TIME_UNITS(TimePerStream) );
		}

		if( bHigh )
		{
			if( (Y+6) > DISPLAY_YRES )
			{
				X += DISPLAY_XINC;
				Y = 0;
			}

			FLT TimePerBlock	= (pStatSet->Count.Highest>0) ? pStatSet->Time.Highest / pStatSet->Count.Highest : 0;
			FLT TimePerFrame	= (g_AM_Stats.DecompFrames.Highest>0) ? pStatSet->Time.Highest / g_AM_Stats.DecompFrames.Highest : 0 ;
			FLT TimePerStream	= (g_AM_Stats.DecompStreams.Highest>0) ? pStatSet->Time.Highest / g_AM_Stats.DecompStreams.Highest : 0;
			x_printfxy( X, Y++, "Max Ever" );
			x_printfxy( X, Y++, " time   =%10.6f", DISPLAY_TIME_UNITS(pStatSet->Time.Highest) );
			x_printfxy( X, Y++, " blocks =%3d",	pStatSet->Count.Highest );
			x_printfxy( X, Y++, " frames =%3d",	g_AM_Stats.DecompFrames.Highest );
			x_printfxy( X, Y++, " streams=%3d",	g_AM_Stats.DecompStreams.Highest );
			x_printfxy( X, Y++, " t/b    =%10.6f", DISPLAY_TIME_UNITS(TimePerBlock) );
			x_printfxy( X, Y++, " t/f    =%10.6f", DISPLAY_TIME_UNITS(TimePerFrame) );
			x_printfxy( X, Y++, " t/s    =%10.6f", DISPLAY_TIME_UNITS(TimePerStream) );
		}
	}
	else
	{
		if( bFrame )
		{
			if( (Y+4) > DISPLAY_YRES )
			{
				X += DISPLAY_XINC;
				Y = 0;
			}

			FLT	TimePerCall	= (pStatSet->Count.Total > 0) ? pStatSet->Time.Total / pStatSet->Count.Total : 0;
			x_printfxy( X, Y++, "Per Frame" );
			x_printfxy( X, Y++, " time   =%10.6f", DISPLAY_TIME_UNITS(pStatSet->Time.Total) );
			x_printfxy( X, Y++, " calls  =%3d", pStatSet->Count.Total );
			x_printfxy( X, Y++, " t/c    =%10.6f", DISPLAY_TIME_UNITS(TimePerCall) );
		}

		if( bAverage )
		{
			if( (Y+4) > DISPLAY_YRES )
			{
				X += DISPLAY_XINC;
				Y = 0;
			}

			FLT	AverageTime		= CalcHistoryAverageFloat( pStatSet->Time.History );
			s32	AverageCount	= CalcHistoryAverageInt( pStatSet->Count.History );
			FLT TimePerCall		= (AverageCount>0) ? AverageTime / (FLT)AverageCount : 0;
			x_printfxy( X, Y++, "Average" );
			x_printfxy( X, Y++, " time   =%10.6f", DISPLAY_TIME_UNITS(AverageTime) );
			x_printfxy( X, Y++, " calls  =%3d",	AverageCount );
			x_printfxy( X, Y++, " t/c    =%10.6f", DISPLAY_TIME_UNITS(TimePerCall) );
		}

		if( bHigh )
		{
			if( (Y+4) > DISPLAY_YRES )
			{
				X += DISPLAY_XINC;
				Y = 0;
			}

			FLT TimePerCall = (pStatSet->Count.Highest>0) ? pStatSet->Time.Highest / pStatSet->Count.Highest : 0;
			x_printfxy( X, Y++, "Max Ever" );
			x_printfxy( X, Y++, " time   =%10.6f", DISPLAY_TIME_UNITS(pStatSet->Time.Highest) );
			x_printfxy( X, Y++, " calls  =%3d",	pStatSet->Count.Highest );
			x_printfxy( X, Y++, " t/c    =%10.6f", DISPLAY_TIME_UNITS(TimePerCall) );
		}
	}
}

//=================================================================================================
void ResetFloatStat( SAnimStat_Float& rStat )
{
	rStat.Total = 0.0f;
}

//=================================================================================================
void ResetIntStat( SAnimStat_Int& rStat )
{
	rStat.Total = 0;
}

//=================================================================================================
void ResetStatSet( SAnimStat_Set& rSet )
{
	ResetFloatStat( rSet.Time );
	ResetIntStat( rSet.Count );
}

//=================================================================================================
void AMStats::AMSTATS_Init( void )
{
	x_memset( &g_AM_Stats, 0, sizeof(SAnimStatsRecorded) );
	x_memset( &g_AM_DisplayStatCatagory, 0, NUM_AMSTATS*sizeof(xbool) );
	g_AM_bDisplayFrameStats		= FALSE;
	g_AM_bDisplayHighStats		= FALSE;
	g_AM_bDisplayAverageStats	= FALSE;

#ifdef ANIM_STATS_ACTIVE
	g_AM_bDisplayFrameStats		= FALSE;
	g_AM_bDisplayHighStats		= FALSE;
	g_AM_bDisplayAverageStats	= TRUE;

	g_AM_DisplayStatCatagory[AMSTAT_DECOMP]    = 1;
	g_AM_DisplayStatCatagory[AMSTAT_BUILDMATS] = 1;
	g_AM_DisplayStatCatagory[AMSTAT_BUILDMAT]  = 1;
	g_AM_DisplayStatCatagory[AMSTAT_AM_EVENTS] = 1;
#endif

}

//=================================================================================================
void AMStats::AMSTATS_Update( void )
{
	s32 i;

	//
	//	update stat data
	//
	for( i=0; i<NUM_AMSTATS; i++ )
		UpdateStatSet( g_AM_Stats.BasicStats[i] );

	UpdateIntStat( g_AM_Stats.DecompFrames );
	UpdateIntStat( g_AM_Stats.DecompStreams );
}

//=================================================================================================
void AMStats::AMSTATS_Render( void )
{
	s32 i;

	//
	//	Display
	//
	s32 X = 3;
	s32 Y = 1;
	for( i=0; i<NUM_AMSTATS; i++ )
		if( g_AM_DisplayStatCatagory[i] )
			AMSTATS_DisplaySection( i, X, Y, g_AM_bDisplayFrameStats, g_AM_bDisplayHighStats, g_AM_bDisplayAverageStats );
}

//=================================================================================================
void AMStats::AMSTATS_Clear( void )
{
	s32 i;
	//
	//	Reset for next frame
	//
	for( i=0; i<NUM_AMSTATS; i++ )
		ResetStatSet( g_AM_Stats.BasicStats[i] );

	ResetIntStat( g_AM_Stats.DecompFrames );
	ResetIntStat( g_AM_Stats.DecompStreams );
}


//=================================================================================================
void AMStats::AM_STATS_DisplayStat( EStatCatagory WhichStat, xbool bSetting )
{
	if( WhichStat < NUM_AMSTATS )
	{
		ASSERT((WhichStat>=0) && (WhichStat<NUM_AMSTATS));
		g_AM_DisplayStatCatagory[WhichStat] = bSetting;
	}
	else if( WhichStat==AMSTATTYPE_FRAME )
		g_AM_bDisplayFrameStats	= bSetting;
	else if( WhichStat==AMSTATTYPE_TOTAL )
		g_AM_bDisplayHighStats	= bSetting;
	else if( WhichStat==AMSTATTYPE_AVERAGE )
		g_AM_bDisplayAverageStats	= bSetting;
}


//=================================================================================================
f64 AMStats::DisplayStats_WithInput( void )
{
#ifdef TARGET_PS2

#define MODE_BUTTON_SELECT  GADGET_PS2_R_STICK

#define	ACTION_BUTTON_0	    GADGET_PS2_TRIANGLE
#define	ACTION_BUTTON_1	    GADGET_PS2_CIRCLE
#define	ACTION_BUTTON_2	    GADGET_PS2_SQUARE
#define CLEAR_STATS_BUTTON  GADGET_PS2_L_STICK

#define JOY_DPAD_U          GADGET_PS2_DPAD_U
#define JOY_DPAD_D          GADGET_PS2_DPAD_D
#define JOY_DPAD_R          GADGET_PS2_DPAD_R
#define JOY_DPAD_L          GADGET_PS2_DPAD_L

#else

#define MODE_BUTTON_SELECT  GADGET_XBOX_R_STICK_BTN

#define	ACTION_BUTTON_0	    GADGET_XBOX_ANALOG_Y
#define	ACTION_BUTTON_1	    GADGET_XBOX_ANALOG_B
#define	ACTION_BUTTON_2	    GADGET_XBOX_ANALOG_A
#define CLEAR_STATS_BUTTON  GADGET_XBOX_L_STICK_BTN

#define JOY_DPAD_U          GADGET_XBOX_DPAD_U
#define JOY_DPAD_D          GADGET_XBOX_DPAD_D
#define JOY_DPAD_R          GADGET_XBOX_DPAD_R
#define JOY_DPAD_L          GADGET_XBOX_DPAD_L

#endif

    xbool bClearStats = FALSE;

    //
	//	Handle controller input
	//
	if (INPUT_GetButton(0, MODE_BUTTON_SELECT) )
	{
		static s32 WhichStat = 0;
		static xbool dpadD = FALSE;
		static xbool dpadU = FALSE;
		static xbool dpadL = FALSE;
		static xbool dpadR = FALSE;
		static xbool bAction1Debounce = FALSE;
		static xbool bAction2Debounce = FALSE;
		static xbool bAction3Debounce = FALSE;
		static xbool frame = FALSE;
		static xbool total = FALSE;
		static xbool average = FALSE;

        // Increase selected stat ID
        //=====================================================================================================================================
		if( INPUT_GetButton(0, JOY_DPAD_D) )
		{
			if( !dpadD )
			{
				if( ++WhichStat==AMStats::NUM_AMSTATS )
					WhichStat = AMStats::NUM_AMSTATS-1;
			}
			dpadD	= TRUE;
		}
		else
			dpadD	= FALSE;

        // Decrease selected stat ID
        //=====================================================================================================================================
		if( INPUT_GetButton(0, JOY_DPAD_U) )
		{
			if( !dpadU )
			{
				if( --WhichStat==-1 )
					WhichStat = 0;
			}
			dpadU	= TRUE;
		}
		else
			dpadU	= FALSE;

        // Disable selected stat
        //=====================================================================================================================================
		if( INPUT_GetButton(0, JOY_DPAD_L) )
		{
			if( !dpadL )
				AM_STATS_DisplayStat( (AMStats::EStatCatagory)WhichStat, FALSE );
			dpadL	= TRUE;
		}
		else
			dpadL	= FALSE;


        // Enable selected stat
        //=====================================================================================================================================
		if( INPUT_GetButton(0, JOY_DPAD_R) )
		{
			if( !dpadR )
				AMStats::AM_STATS_DisplayStat( (AMStats::EStatCatagory)WhichStat, TRUE );
			dpadR	= TRUE;
		}
		else
			dpadR	= FALSE;

        
        // Show Per Frame stats.
        //=====================================================================================================================================
		if( INPUT_GetButton(0, ACTION_BUTTON_1) )
		{
			if( !bAction3Debounce )
				AMStats::AM_STATS_DisplayStat( AMStats::AMSTATTYPE_FRAME, frame = !frame );
			bAction3Debounce = TRUE;
		}
		else
			bAction3Debounce = FALSE;


        // Show Total Stats.
        //=====================================================================================================================================
		if( INPUT_GetButton(0, ACTION_BUTTON_0) )
		{
			if( !bAction1Debounce )
				AMStats::AM_STATS_DisplayStat( AMStats::AMSTATTYPE_TOTAL, total = !total );
			bAction1Debounce = TRUE;
		}
		else
			bAction1Debounce = FALSE;


        // Show average stats.
        //=====================================================================================================================================
		if( INPUT_GetButton(0, ACTION_BUTTON_2) )
		{
			if( !bAction2Debounce )
				AMStats::AM_STATS_DisplayStat( AMStats::AMSTATTYPE_AVERAGE, average = !average );
			bAction2Debounce = TRUE;
		}
		else
			bAction2Debounce = FALSE;


        // Clear stats.
        //=====================================================================================================================================
        if( INPUT_GetButton(0, CLEAR_STATS_BUTTON) )
			bClearStats = TRUE;
	}

	//
	//	update the stats display
	//
	static s32 amstat = 1;
	if( amstat )
	{
		AMStats::AMSTATS_Init();
		amstat = 0;
	}

    AMStats::AMSTATS_Update();
	AMStats::AMSTATS_Render();

    if( bClearStats )
        AMStats::AMSTATS_Clear();

    return 0.0f;
}

#endif

