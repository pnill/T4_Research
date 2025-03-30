///////////////////////////////////////////////////////////////////////////////////////
// PS2Perf.cpp		PS2 Performance Measuring and Display
// 4-1-03 - jmX

///////////////////////////////////////////////////////////////////////////////////////


#include "x_files.hpp"
#include "x_types.hpp"
#include "x_debug.hpp"
#include "x_stdio.hpp"
#include "x_plus.hpp"
#include "x_time.hpp"

#include <eeregs.h>
#include <eekernel.h>
#include <stdio.h>
#include <sifdev.h>
#include <string.h>
#include <assert.h>
#include <libpc.h>

#include "Q_Engine.hpp"
#include "Q_Input.hpp"
#include "Q_View.hpp"
#include "Q_Draw.hpp"
#include "Q_PS2.hpp"
#include "Q_Perf.hpp"
#include "PS2Perf.h"
#include "ps2_font.hpp"



//uncomment this for some basic function summary stats at the top of the screen
//#define RENDER_FUNCTION_SUMMARY

#ifdef ENABLE_PERF

/////////////////////////////////////
// DEFINES
/////////////////////////////////////
#define PS2_CLOCK						147456000L	
#define PS2PERF_SAMPLES_PER_FRAME		256	/* Change this to control sampling resolution, Hz=SAMPLES_PER_FRAME*60 */
#define PS2PERF_CACHE_MOD				0x1	/* This will be or'd with the interupt counter, and when the result is 0, a new CACHE sample is stored*/
#define PS2PERF_CACHE_SPF				(PS2PERF_SAMPLES_PER_FRAME / (PS2PERF_CACHE_MOD+1))
#define PS2PERF_MAX_FRAMES				8	/* PS2PERF_MAX_FRAMES is the max number of frames (1/60th of a sec) that the perf storage holds*/
#define PS2PERF_MAX_SAMPLES				(PS2PERF_SAMPLES_PER_FRAME * PS2PERF_MAX_FRAMES)
#define PS2PERF_MAX_CACHE_SAMPLES		(PS2PERF_CACHE_SPF * PS2PERF_MAX_FRAMES)

#define FRAMES_OF_REGISTERS

color MarkerColors[12] =
{
	color(255,  0,  0,255),
	color(192,128,  0,255),
	color(255,255,  0,255),
	color(  0,255,  0,255),
	color(  0,255,128,255),
	color(  0,255,255,255),
	color(  0,128,255,255),
	color(  0,  0,255,255),
	color(128,  0,255,255),
	color(255,  0,128,255),
	color(128,128,128,255),
	color(255,255,255,255),
};



struct Marker
{
	color m_Color;
	char m_Description[128];
};




/////////////////////////////////////
// Variables
/////////////////////////////////////
struct TimerSample
{
	s32		mVif1_Stat;		// vif1 stat register
	s32		mVpu_Stat;		// vpu stat register (from VU0 or COP2 register vi29)
	s32		mGif_Stat;		// gif stat register
	s16		mVif1_Mark;		// mark register
	s16		mDma_Stats;		// bit 0 = vif0, bit1 = vif1, bit2 = gif

	// Labeling info
	s16		MarkerID;		// lookup id into marker table

};

enum RecordMode
{
	PS2PERF_RECORD_CPU = 0,
	PS2PERF_RECORD_CACHE = 1
};

struct PerfData
{
	RecordMode m_RecordMode;

	TimerSample m_Samples[PS2PERF_MAX_SAMPLES];
	u32 m_ICacheMissFrames[PS2PERF_MAX_CACHE_SAMPLES];
	u32 m_DCacheMissFrames[PS2PERF_MAX_CACHE_SAMPLES];

	u32 m_SingleCPUFrames[PS2PERF_MAX_CACHE_SAMPLES];
	u32 m_DoubleCPUFrames[PS2PERF_MAX_CACHE_SAMPLES];

	u32 m_count;
	u32 m_cacheframesStored;
	u32 m_cacheframestoDisplay;


	Marker m_Markers[128];
	u32	   m_NumMarkers;
};

//Double buffer the recording
PerfData g_PS2PERF_PerfData1;
PerfData g_PS2PERF_PerfData2;
PerfData *g_PS2PERF_pPERFDataRecord		= NULL;
PerfData *g_PS2PERF_pPERFDataDisplay	= NULL;

static xbool g_PS2PERF_Enabled		= FALSE;
static f32	 g_PercentHighlighter	= 0.00f;
static xbool g_PS2PERF_bInstalled	= FALSE;

static s32 g_CurrentMarkerID = -1;



//////////////////////////////////////////////////////////////////////////////////
// PERF interface code
//////////////////////////////////////////////////////////////////////////////////

void PERF_Begin(char *Description,color Color)
{
	s32 MarkerID = -1;

	if (g_PS2PERF_bInstalled)
	{
		//Search and see if this one is already registered
		for(u32 i = 0; i < g_PS2PERF_pPERFDataRecord->m_NumMarkers; i++ )
		{
			if (x_strcmp(Description,g_PS2PERF_pPERFDataRecord->m_Markers[i].m_Description)==0)
			{
				MarkerID = i;
			}
		}

		if (MarkerID == -1)
		{
			MarkerID = g_PS2PERF_pPERFDataRecord->m_NumMarkers;
			g_PS2PERF_pPERFDataRecord->m_NumMarkers++;

			x_strcpy(g_PS2PERF_pPERFDataRecord->m_Markers[MarkerID].m_Description,Description);

			if (Color == color(0,1,2,3))
			{
				g_PS2PERF_pPERFDataRecord->m_Markers[MarkerID].m_Color = MarkerColors[MarkerID%12];

			} else
				g_PS2PERF_pPERFDataRecord->m_Markers[MarkerID].m_Color = Color;

		}

//	u32	*pData = (u32 *)DList.IncrementBy(sizeof(sceDmaTag) + 16);
//	DList.BuildDmaCont((sceDmaTag *)pData, 16);
//	pData[4] = VIFHELP_SET_MARK(MarkerID, 0);
//	pData[5] = pData[6] = pData[7] = 0;

		g_CurrentMarkerID = MarkerID;
	}
}

void PERF_End(void)
{
	if (g_PS2PERF_bInstalled)
	{
		g_CurrentMarkerID = -1;
	}
}

void PERF_Toggle(void)
{
	if (g_PS2PERF_bInstalled)
		g_PS2PERF_Enabled = !g_PS2PERF_Enabled;
}




//////////////////////////////////////////////////////////////////////////////////
// PS2 Specific PERF code
//////////////////////////////////////////////////////////////////////////////////
void PS2PERF_SetPointerPosition(f32 PercentPosition)
{
	g_PercentHighlighter = PercentPosition;
}



static int PS2PERF_interruptHanlder(int Cause)
{
//	static bool bInitialized = false;
	static s32	PerfCounter			= SCE_PC_CTE;

	s32 *pC0FramesUncached = NULL;
	s32 *pC1FramesUncached = NULL;

	if (g_PS2PERF_Enabled)
	{
		if (g_PS2PERF_pPERFDataRecord->m_RecordMode == PS2PERF_RECORD_CPU)
		{
			pC0FramesUncached = (s32 *)((s32)g_PS2PERF_pPERFDataRecord->m_SingleCPUFrames | 0x00000000);
			pC1FramesUncached = (s32 *)((s32)g_PS2PERF_pPERFDataRecord->m_DoubleCPUFrames | 0x00000000);
		} else
		{
			pC0FramesUncached = (s32 *)((s32)g_PS2PERF_pPERFDataRecord->m_ICacheMissFrames | 0x00000000);
			pC1FramesUncached = (s32 *)((s32)g_PS2PERF_pPERFDataRecord->m_DCacheMissFrames | 0x00000000);
		}


		if (g_PS2PERF_pPERFDataRecord->m_count == 0)	//first frame of sample window
		{
			if (g_PS2PERF_pPERFDataRecord->m_RecordMode == PS2PERF_RECORD_CPU)
			{
//				x_memset(g_PS2PERF_pPERFDataRecord->m_SingleCPUFrames,0,PS2PERF_MAX_CACHE_SAMPLES*sizeof(u32));
//				x_memset(g_PS2PERF_pPERFDataRecord->m_DoubleCPUFrames,0,PS2PERF_MAX_CACHE_SAMPLES*sizeof(u32));
				
				PerfCounter			= SCE_PC_CTE;
				PerfCounter |= (SCE_PC_U0 | SCE_PC0_SINGLE_ISSUE);
				PerfCounter |= (SCE_PC_U1 | SCE_PC1_DUAL_ISSUE);
			}
			else
			{
//				x_memset(g_PS2PERF_pPERFDataRecord->m_ICacheMissFrames,0,PS2PERF_MAX_CACHE_SAMPLES*sizeof(u32));
//				x_memset(g_PS2PERF_pPERFDataRecord->m_DCacheMissFrames,0,PS2PERF_MAX_CACHE_SAMPLES*sizeof(u32));

				PerfCounter			= SCE_PC_CTE;
				PerfCounter |= (SCE_PC_U0 | SCE_PC0_ICACHE_MISS);
				PerfCounter |= (SCE_PC_U1 | SCE_PC1_DCACHE_MISS);
			}


			scePcStart(PerfCounter, 0, 0);
		} else
		{
			if (g_PS2PERF_pPERFDataRecord->m_count<PS2PERF_MAX_SAMPLES)
			{
				TimerSample *pSamp = &g_PS2PERF_pPERFDataRecord->m_Samples[g_PS2PERF_pPERFDataRecord->m_count];




				pSamp->mVif1_Stat		= *VIF1_STAT;
				asm ("cfc2.ni	%0, vi29": "=r" (pSamp->mVpu_Stat)  );
				pSamp->mGif_Stat		= *GIF_STAT;
				pSamp->mVif1_Mark		= (s16)*VIF1_MARK;
				register s32 dmaStat; 
				dmaStat 				= ( (D_CHCR_STR_M&(*D0_CHCR))>>8  ) |	// vif0 channel in bit 0
										  ( (D_CHCR_STR_M&(*D1_CHCR))>>7  ) |	// vif1 channel in bit 1
										  ( (D_CHCR_STR_M&(*D2_CHCR))>>6  );	// gif channel in bit 2
				pSamp->mDma_Stats		= (s16)dmaStat;
				
				pSamp->MarkerID = g_CurrentMarkerID;

				if (((g_PS2PERF_pPERFDataRecord->m_count & PS2PERF_CACHE_MOD) == 0) && 
					( g_PS2PERF_pPERFDataRecord->m_cacheframesStored < PS2PERF_MAX_CACHE_SAMPLES))
				{
					pC0FramesUncached[g_PS2PERF_pPERFDataRecord->m_cacheframesStored] = scePcGetCounter0();
					pC1FramesUncached[g_PS2PERF_pPERFDataRecord->m_cacheframesStored] = scePcGetCounter1();


					scePcStop();
					scePcStart(PerfCounter, 0, 0);
					g_PS2PERF_pPERFDataRecord->m_cacheframesStored++;
				}
			}

		}


	}

	if (g_PS2PERF_pPERFDataRecord->m_count<PS2PERF_MAX_SAMPLES)
		g_PS2PERF_pPERFDataRecord->m_count++;

	// Clear Overflow Interrupt
	*T1_MODE  = *T1_MODE | T_MODE_EQUF_M;

	return(1);
}



void PS2PERF_Install()
{
    if (g_PS2PERF_bInstalled)
        return;


	AddIntcHandler( INTC_TIM1, PS2PERF_interruptHanlder, 0 );
	
	x_memset(&g_PS2PERF_PerfData1,0,sizeof(PerfData));
	x_memset(&g_PS2PERF_PerfData2,0,sizeof(PerfData));

	g_PS2PERF_pPERFDataRecord = &g_PS2PERF_PerfData1;
	g_PS2PERF_pPERFDataDisplay = &g_PS2PERF_PerfData2;

	g_PS2PERF_pPERFDataRecord->m_RecordMode = PS2PERF_RECORD_CPU;
	g_PS2PERF_pPERFDataDisplay->m_RecordMode = PS2PERF_RECORD_CACHE;


    // Setup Timer1
    *T1_COUNT = 0; // Set up our count
    *T1_COMP = (PS2_CLOCK / (PS2PERF_SAMPLES_PER_FRAME*60));

    *T1_MODE =	(int)0x00 |		// Bus sampling rate
		        T_MODE_EQUF_M |			// Clear the equal flag
		        T_MODE_CMPE_M |			// Enable compare interrupt
		        T_MODE_CUE_M |			// Restart count
		        T_MODE_ZRET_M;			// Clear counter to 0 when counter == reference value


    // Enable Interrupt VBlankON
    EnableIntc( INTC_TIM1 );

    g_PS2PERF_bInstalled = true;
}



void PS2PERF_Pageflip(void)
{
	if (g_PS2PERF_bInstalled)
	{

		if (g_PS2PERF_pPERFDataRecord == &g_PS2PERF_PerfData1)
		{
			g_PS2PERF_pPERFDataRecord = &g_PS2PERF_PerfData2;
			g_PS2PERF_pPERFDataDisplay = &g_PS2PERF_PerfData1;
		}
		else
		{
			g_PS2PERF_pPERFDataRecord = &g_PS2PERF_PerfData1;
			g_PS2PERF_pPERFDataDisplay = &g_PS2PERF_PerfData2;
		}

		//Toggle what we're recording this frame
		if (g_PS2PERF_pPERFDataRecord->m_RecordMode == PS2PERF_RECORD_CPU)
			g_PS2PERF_pPERFDataRecord->m_RecordMode = PS2PERF_RECORD_CACHE;
		else
			g_PS2PERF_pPERFDataRecord->m_RecordMode = PS2PERF_RECORD_CPU;


		//Begin new recording
		g_PS2PERF_pPERFDataRecord->m_count=0;
		g_PS2PERF_pPERFDataRecord->m_cacheframesStored = 0;

		g_PS2PERF_pPERFDataRecord->m_NumMarkers = 0;

		g_CurrentMarkerID = -1;
	}

}

void PS2PERF_DrawStats(void)
{

	static s32 frame = 0;
	frame++;
	if (g_PS2PERF_bInstalled && g_PS2PERF_Enabled)
	{

	PERF_Begin("PS2PERF",color(255,(frame%2) ? 0:255,0,255));

	color PointerColors[3] = {color(255,255,0,255),color(255,255,0,255),color(255,255,0,255)};

	f32 xPos;
	f32 yPos;
	f32 BarWidth;
	f32 cacheFrames = 0;
//	f32 CPUFrames = 0;
	f32 VUFrames = 0;
	u32 localCount = g_PS2PERF_pPERFDataDisplay->m_count;
	u32 localCacheCount = g_PS2PERF_pPERFDataDisplay->m_cacheframestoDisplay;
	s32 numFrames;
	s32 PS2PERFCacheSamples = 0;


		//NOTE: SAMPLES_PER_BAR can be set to a constant (1 for instance) rather than have graph granularity change
		//		with framerate, however, it means more bars will get drawn during slow frames and could cause the 
		//		display list buffer to overflow! I set it to numFrames to constantly scale the resolution of the 
		//		graph bars to avoid this issue. The lower this number is, the thinner the bars on the graph will be -jmX 
		#define SAMPLES_PER_BAR				numFrames
		#define NUMBER_OF_BARS_PER_FRAME	(PS2PERF_CACHE_SPF / SAMPLES_PER_BAR)
		#define NUMBER_OF_BARS				(localCacheCount / SAMPLES_PER_BAR)

		//How many cache frames of data did we get? (note, frame here means 1/60th of a second)
		cacheFrames = (f32)localCacheCount / (f32)PS2PERF_CACHE_SPF;

		//How many frames did the CPU take? (note, frame here means 1/60th of a second)
		f32 MaxFrames = cacheFrames;
		if (VUFrames > MaxFrames)	MaxFrames = VUFrames;

		if (MaxFrames > 5)
			return;

		//Figure out what was slowest, and scale our display to be that big
		numFrames = (s32)x_ceil(MaxFrames);
		if (numFrames <= 0) numFrames = 1;
		if (numFrames > 4) numFrames = 4;



		/////////////////////////////////////////////////////
		// Begin drawing
		/////////////////////////////////////////////////////

		ENG_BeginRenderMode();
		DRAW_SetMode( DRAW_2D |
					  DRAW_FILL |
					  DRAW_NO_TEXTURE |
					  DRAW_ALPHA |
					  DRAW_NO_ZBUFFER |
					  DRAW_NO_CLIP |
					  DRAW_NO_LIGHT );


	/////////////////////////////////////////////////////
	/////////////////////////////////////////////////////
	// DRAW BIG CPU/CACHE GRAPH!
	/////////////////////////////////////////////////////
	/////////////////////////////////////////////////////
	
		/////////////////////////////////////////////////////
		// Draw CPU/CACHE graph background
		/////////////////////////////////////////////////////
		color CacheBackgroundColor(0,0,0,190);
		DRAW_Rectangle( (f32)20, (f32)(320.0f - 0), 0.0f, (f32)(600), (f32)105, CacheBackgroundColor, CacheBackgroundColor, CacheBackgroundColor, CacheBackgroundColor );


		/////////////////////////////////////////////////////
		// Draw Milisecond bars
		/////////////////////////////////////////////////////
		color MSColor				(255,255,0,127);
		f32 Miliseconds = numFrames*16.7;
		for (int i=0;i<Miliseconds;i++)
			DRAW_Rectangle( (f32)25.0f+i*(590.0f/Miliseconds), (f32)(302.0f), 0.0f, (f32)(1), (f32)120.0f, MSColor,MSColor,MSColor,MSColor);


		/////////////////////////////////////////////////////
		// Draw vertical cache stats bar graph
		/////////////////////////////////////////////////////

		color DCacheMissColor(255,127,0,255);
		color ICacheMissColor(255,230,40,255);

		color SingleCPUColor(255,255,63,255);
		color DoubleCPUColor(0,255,0,255);



		xPos = 25.0f;	//left of graph
		yPos = 420.0f;	//bottom of graph

		BarWidth = (590.0f/(f32)NUMBER_OF_BARS_PER_FRAME) / numFrames;	//width of each



		xPos = 25.0f;	//left of graph
		yPos = 420.0f;	//bottom of graph
		s32 CachedMarkerID=-777;
		for (u32 i=0; i<NUMBER_OF_BARS; i++)
		{
			xPos+=BarWidth;
			f32 DCacheMissHeight = 0.0f;
			f32 ICacheMissHeight = 0.0f;

			f32 SingleCPUHeight = 0.0f;
			f32 DoubleCPUHeight = 0.0f;

			for (int n=0;n<SAMPLES_PER_BAR;n++)
			{
				DCacheMissHeight += g_PS2PERF_pPERFDataDisplay->m_DCacheMissFrames[i*SAMPLES_PER_BAR+n];
				ICacheMissHeight += g_PS2PERF_pPERFDataDisplay->m_ICacheMissFrames[i*SAMPLES_PER_BAR+n];

				SingleCPUHeight += g_PS2PERF_pPERFDataDisplay->m_SingleCPUFrames[i*SAMPLES_PER_BAR+n];
				DoubleCPUHeight += g_PS2PERF_pPERFDataDisplay->m_DoubleCPUFrames[i*SAMPLES_PER_BAR+n];

			}

			DCacheMissHeight /= (25.0f * SAMPLES_PER_BAR);
			ICacheMissHeight /= (25.0f * SAMPLES_PER_BAR);
			SingleCPUHeight /= (500.0f * SAMPLES_PER_BAR);
			DoubleCPUHeight /= (500.0f * SAMPLES_PER_BAR);

			TimerSample *pStat = &g_PS2PERF_pPERFDataDisplay->m_Samples[(i*SAMPLES_PER_BAR)*(PS2PERF_CACHE_MOD+1)];

			color CDMiss = DCacheMissColor;
			color CIMiss = ICacheMissColor;
			color CSingle = SingleCPUColor;
			color CDouble = DoubleCPUColor;

			if ((pStat->MarkerID == CachedMarkerID) || (x_strcmp(g_PS2PERF_pPERFDataDisplay->m_Markers[pStat->MarkerID].m_Description,"PS2PERF")==0))
			{
				CDMiss.A /= 4;
				CIMiss.A /= 4;
				CSingle.A /= 4;
				CDouble.A /= 4;
				CachedMarkerID = pStat->MarkerID;
				PS2PERFCacheSamples += SAMPLES_PER_BAR;
			}

				DRAW_Rectangle( (f32)xPos, (f32)(320.0f), 0.0f, (f32)(BarWidth), (f32)DCacheMissHeight, CDMiss, CDMiss, CDMiss, CDMiss );
				DRAW_Rectangle( (f32)xPos, (f32)(320.0f + DCacheMissHeight), 0.0f, (f32)(BarWidth), (f32)ICacheMissHeight, CIMiss, CIMiss, CIMiss, CIMiss );

				//Draw Pipeline Bars
				DRAW_Rectangle( (f32)xPos, (f32)(420.0f - DoubleCPUHeight), 0.0f, (f32)(BarWidth), (f32)DoubleCPUHeight, CDouble, CDouble, CDouble, CDouble );
				DRAW_Rectangle( (f32)xPos, (f32)(420.0f - DoubleCPUHeight - SingleCPUHeight), 0.0f, (f32)(BarWidth), (f32)SingleCPUHeight, CSingle, CSingle, CSingle, CSingle );
		}

	/////////////////////////////////////////////////////
	/////////////////////////////////////////////////////
	// DONE DRAWING BIG CPU/CACHE GRAPH
	/////////////////////////////////////////////////////
	/////////////////////////////////////////////////////
	
		
		////////////////////////////////////////////////////////
		// Draw VU status bars
		// NOTE: VU0 stuff commented out as it doesnt seem to work right now -jmX
		// NOTE2: Added what may be a VU1 blocking graph. Not sure yet -jmX
		////////////////////////////////////////////////////////

		color VU0Color				(255,0,0,255);
		color VU0ColorBG			(0,0,0,190);
		color VU1Color				(0,0,255,255);
		color VU1ColorBG			(0,0,0,190);

	
		xPos = 25.0f;
		yPos = 265.0f;
		BarWidth = (590.0f/(PS2PERF_SAMPLES_PER_FRAME*numFrames));

		DRAW_Rectangle( (f32)20, (f32)(222.0f), 0.0f, (f32)(600), (f32)16, VU0ColorBG, VU0ColorBG, VU0ColorBG, VU0ColorBG );
		DRAW_Rectangle( (f32)20, (f32)(242.0f), 0.0f, (f32)(600), (f32)16, VU1ColorBG, VU1ColorBG, VU1ColorBG, VU1ColorBG );
		
		for (u32 i=0;i<localCount;i++)
		{
			TimerSample *pStat = &g_PS2PERF_pPERFDataDisplay->m_Samples[i];
			
//			if ((pStat->mVpu_Stat & (0x1)) || (pStat->mVpu_Stat & (0xFF)))  //VU0 USAGE
			if (pStat->mVif1_Stat & (1<<3))									//VU1 Blocking?
				DRAW_Rectangle( (f32)xPos, (f32)(225), 0.0f, (f32)(BarWidth), (f32)10.0f, VU0Color,VU0Color,VU0Color,VU0Color);
 
			if (pStat->mVpu_Stat & (0x01<<8))								//VU1 USAGE
				DRAW_Rectangle( (f32)xPos, (f32)(245), 0.0f, (f32)(BarWidth), (f32)10.0f, VU1Color,VU1Color,VU1Color,VU1Color);

			xPos+=BarWidth;
		}


		////////////////////////////////////////////////////////
		// Draw horizontal VIF1 STAT BAR
		////////////////////////////////////////////////////////
		color PATH1Color			(255,0,0,255);
		color PATH2Color			(200,200,0,255);
		color PATH3Color			(0,240,0,255);
		color PATHColorBG			(0,0,0,190);

		
		xPos = 25.0f;
		yPos = 265.0f;
		BarWidth = (590.0f/(PS2PERF_SAMPLES_PER_FRAME*numFrames));

		DRAW_Rectangle( (f32)20, (f32)(262.0f), 0.0f, (f32)(600), (f32)16, PATHColorBG, PATHColorBG, PATHColorBG, PATHColorBG );
		
		for (u32 i=0;i<localCount;i++)
		{
			TimerSample *pStat = &g_PS2PERF_pPERFDataDisplay->m_Samples[i];
		
			if (pStat->mGif_Stat & (0x0001 << 10)) //PATH 1 USAGE
				DRAW_Rectangle( (f32)xPos, (f32)(265), 0.0f, (f32)(BarWidth), (f32)10.0f, PATH1Color,PATH1Color,PATH1Color,PATH1Color);
 
			if (pStat->mGif_Stat & (0x0001 << 11) ) //PATH 2 USAGE
				DRAW_Rectangle( (f32)xPos, (f32)(265), 0.0f, (f32)(BarWidth), (f32)10.0f, PATH2Color,PATH2Color,PATH2Color,PATH2Color);
 
			if ((pStat->mGif_Stat & (0x0003 << 10)) == 0x3 ) //PATH 3 USAGE
				DRAW_Rectangle( (f32)xPos, (f32)(265), 0.0f, (f32)(BarWidth), (f32)10.0f, PATH3Color,PATH3Color,PATH3Color,PATH3Color);



			xPos+=BarWidth;
		}





		////////////////////////////////////////////////////////
		// Render Function Marker bars
		////////////////////////////////////////////////////////
		xPos = 25.0f;
		yPos = 305.0f;
		BarWidth = (590.0f/(PS2PERF_SAMPLES_PER_FRAME*numFrames));
		DRAW_Rectangle( (f32)20, (f32)(yPos-3), 0.0f, (f32)(600), (f32)16, CacheBackgroundColor, CacheBackgroundColor, CacheBackgroundColor, CacheBackgroundColor );
		
		for (u32 i=0;i<localCount;i++)
		{
			TimerSample *pStat = &g_PS2PERF_pPERFDataDisplay->m_Samples[i];
			
			if (pStat->MarkerID != -1)
			{
				color c = g_PS2PERF_pPERFDataDisplay->m_Markers[pStat->MarkerID].m_Color;
				DRAW_Rectangle( (f32)xPos, (f32)(yPos), 0.0f, (f32)(BarWidth), (f32)10.0f, c,c,c,c);
			}

			xPos+=BarWidth;
		}

		////////////////////////////////////////////////////////
		// Draw pointer and text
		////////////////////////////////////////////////////////
		BarWidth = (590.0f/(PS2PERF_SAMPLES_PER_FRAME*numFrames));
		s32 SampleHighlighted = (s32)(g_PercentHighlighter * localCount);

		f32 PointerXPosition = 25.0f+((SampleHighlighted - 1) * BarWidth) - (BarWidth / 2.0f);
		
		TimerSample *pStat = &g_PS2PERF_pPERFDataDisplay->m_Samples[SampleHighlighted];
		char TempString[1024];
		
		DRAW_Rectangle( (f32)PointerXPosition, (f32)(298), 0.0f, (f32)(3), (f32)24, PointerColors[frame%3], PointerColors[frame%3], PointerColors[frame%3], PointerColors[frame%3] );
//		DRAW_Rectangle( (f32)PointerXPosition, (f32)(218), 0.0f, (f32)(3), (f32)44, PointerColors[frame%3], PointerColors[frame%3], PointerColors[frame%3], PointerColors[frame%3] );

		{
			s32 nSamples = 0;

			for (u32 l=0;l<localCount;l++)
			{
				TimerSample *pStat2 = &g_PS2PERF_pPERFDataDisplay->m_Samples[l];
				if (pStat2->MarkerID==pStat->MarkerID)
					nSamples++;
			}


			x_sprintf(TempString,"%s (%3.1f%%)",g_PS2PERF_pPERFDataDisplay->m_Markers[pStat->MarkerID].m_Description,(f32)(100.0f*nSamples)/(f32)(localCacheCount*(PS2PERF_CACHE_MOD+1)));
			FONT_BeginRender();
			if (pStat->MarkerID!=-1)
				FONT_Render( TempString, (s32)PointerXPosition, 285, color(255,255,255,255) );

//			x_sprintf(TempString,"%s",g_PS2PERF_pPERFDataDisplay->m_Markers[pStat->mVif1_Mark].m_Description);
//			if (pStat->MarkerID!=0)
//				FONT_Render( TempString, PointerXPosition, 205, color(255,255,255,255) );

				FONT_EndRender();
		}

#ifdef RENDER_FUNCTION_SUMMARY
		///////////////////////////////////
		//RENDER FUNCTION SUMMARY UP TOP
		///////////////////////////////////

		FONT_BeginRender();

		for (int i=0;i<g_PS2PERF_pPERFDataDisplay->m_NumMarkers;i++)
		{
			f32 xPos = 20.0f;
			if (i>10)
				xPos = 320.0f;
			
			s32 nSamples = 0;

			for (int l=0;l<localCount;l++)
			{
				TimerSample *pStat = &g_PS2PERF_pPERFDataDisplay->m_Samples[l];
				if (pStat->MarkerID==i)
					nSamples++;
			}
			
			char tempStats[256];


			f32 TaskTime = (f32)(nSamples) / (f32)PS2PERF_SAMPLES_PER_FRAME * 16.7f;

			x_sprintf(tempStats,"%s - %3.1f %% (%3.2fms)",g_PS2PERF_pPERFDataDisplay->m_Markers[i].m_Description,(f32)(100.0f*nSamples)/(f32)(localCacheCount*(PS2PERF_CACHE_MOD+1)),TaskTime);
			FONT_Render( tempStats, xPos+2, 12+i*15, color(0,0,0,192) );
			
			FONT_Render( tempStats, xPos, 10+i*15, g_PS2PERF_pPERFDataDisplay->m_Markers[i].m_Color );
		}

		FONT_EndRender();
#endif

		////////////////////////////////////////////////////////
		// Render CPU Time taken
		////////////////////////////////////////////////////////
		
		f32 CPUTime = (f32)(localCacheCount-PS2PERFCacheSamples) / (f32)PS2PERF_CACHE_SPF;

		FONT_BeginRender();
			xPos = 25+590.0f*CPUTime/(numFrames);
			f32 MS = (f32)16.7f*CPUTime;
			char tempStats[128];
			x_sprintf(tempStats,"%2.1fms",MS);
			FONT_Render( tempStats, (s32)xPos, 320, color(255,255,255,180) );
		FONT_EndRender();

		color CPUDoneBarColor	(255,255,0,255);
		xPos = 25+590.0f*CPUTime/(numFrames);
		DRAW_Rectangle( (f32)xPos, (f32)(320), 0.0f, (f32)(1), (f32)105.0f, CPUDoneBarColor,CPUDoneBarColor,CPUDoneBarColor,CPUDoneBarColor);


		////////////////////////////////////////////////////////
		// Draw vertical frame divider bars if needed (FPS<60)
		////////////////////////////////////////////////////////
		color DividerBarColor	(255,255,255,255);

		if (numFrames>1)
		{
			s32 barsToDraw = numFrames - 1;
			xPos = 25.0f;
			f32 xStep = 590.0f / numFrames;

			for (s32 i=0;i<barsToDraw;i++)
			{
				xPos += xStep;
				yPos = 282;
			
				DRAW_Rectangle( (f32)xPos, (f32)(yPos), 0.0f, (f32)(1), (f32)140.0f, DividerBarColor,DividerBarColor,DividerBarColor,DividerBarColor);
			}
		}

		////////////////////////////////////////////////////////
		// Done
		////////////////////////////////////////////////////////
		ENG_EndRenderMode();

	g_PS2PERF_pPERFDataRecord->m_cacheframestoDisplay = g_PS2PERF_pPERFDataRecord->m_cacheframesStored;
	PERF_End();
	
	} //if enabled



}

#else

void PERF_Begin( char* Description, color Color )
{
}
void PERF_End( void )
{
}
void PERF_Toggle( void )
{
}
void PS2PERF_Install( void )
{
}
void PS2PERF_Uninstall( void )
{
}
void PS2PERF_Pageflip( void )
{
}
void PS2PERF_DrawStats( void )
{
}
void PS2PERF_SetPointerPosition( f32 p )
{
}

#endif

