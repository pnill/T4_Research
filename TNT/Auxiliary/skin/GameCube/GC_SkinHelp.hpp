////////////////////////////////////////////////////////////////////////////
//
// GC_SkinHelp.hpp
//
////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////
// INCLUDES
////////////////////////////////////////////////////////////////////////////

#include "Skin.hpp"


////////////////////////////////////////////////////////////////////////////
// MACROS
////////////////////////////////////////////////////////////////////////////

#define IS_ALIGN32( x )     ( ((u32)(x) & 0x1F) == 0 )


////////////////////////////////////////////////////////////////////////////
// CONSTANTS
////////////////////////////////////////////////////////////////////////////

#define DMA_BUFFER_COUNT            4       //3
#define NUM_VTX_PER_BUFFER          144     //192
#define NUM_SRC_BLOCKS_PER_BUFFER   ( ALIGN_32(NUM_VTX_PER_BUFFER * sizeof(GCSkinVert_CPU)) >> 5 )
#define NUM_DST_BLOCKS_PER_BUFFER   ( ALIGN_32(NUM_VTX_PER_BUFFER * sizeof(GCSkinVert_PN)) >> 5 )


////////////////////////////////////////////////////////////////////////////
// SKIN HELPERS
////////////////////////////////////////////////////////////////////////////

namespace GQR
{
    // Macro to create a value for a GQR register
    #define GQR_MAKE( LDScl, LDType, STScl, STType )    \
    (                                                   \
      ( ((STType) & 0x07)       ) |                     \
      ( ((STScl)  & 0x3F) <<  8 ) |                     \
      ( ((LDType) & 0x07) << 16 ) |                     \
      ( ((LDScl)  & 0x3F) << 24 )                       \
    )

    // GQR conversion type values
    enum GQR_TYPE
    {
        TYPE_F32    = 0,
        TYPE_U8     = 4,
        TYPE_U16    = 5,
        TYPE_S8     = 6,
        TYPE_S16    = 7
    };

    // Useful GQR values for loading/storing to same type
    const u32 LD_F32_ST_F32     = GQR_MAKE( 0, GQR::TYPE_F32, 0, GQR::TYPE_F32 );
    const u32 LD_U8_ST_U8       = GQR_MAKE( 0, GQR::TYPE_U8,  0, GQR::TYPE_U8  );
    const u32 LD_U16_ST_U16     = GQR_MAKE( 0, GQR::TYPE_U16, 0, GQR::TYPE_U16 );
    const u32 LD_S8_ST_S8       = GQR_MAKE( 0, GQR::TYPE_S8,  0, GQR::TYPE_S8  );
    const u32 LD_S16_ST_S16     = GQR_MAKE( 0, GQR::TYPE_S16, 0, GQR::TYPE_S16 );

    // Set/Get functions for working with GQR registers
    inline void SetGQR0( register u32 QVal )    { asm __volatile__ ( "mtspr GQR0, (%0)" : : "r" (QVal) ); }
    inline void SetGQR1( register u32 QVal )    { asm __volatile__ ( "mtspr GQR1, (%0)" : : "r" (QVal) ); }
    inline void SetGQR2( register u32 QVal )    { asm __volatile__ ( "mtspr GQR2, (%0)" : : "r" (QVal) ); }
    inline void SetGQR3( register u32 QVal )    { asm __volatile__ ( "mtspr GQR3, (%0)" : : "r" (QVal) ); }
    inline void SetGQR4( register u32 QVal )    { asm __volatile__ ( "mtspr GQR4, (%0)" : : "r" (QVal) ); }
    inline void SetGQR5( register u32 QVal )    { asm __volatile__ ( "mtspr GQR5, (%0)" : : "r" (QVal) ); }
    inline void SetGQR6( register u32 QVal )    { asm __volatile__ ( "mtspr GQR6, (%0)" : : "r" (QVal) ); }
    inline void SetGQR7( register u32 QVal )    { asm __volatile__ ( "mtspr GQR7, (%0)" : : "r" (QVal) ); }

    inline u32  GetGQR0( void )                 { register u32 QVal;  asm __volatile__ ( "mfspr (%0), GQR0" : "=r" (QVal) );  return QVal; }
    inline u32  GetGQR1( void )                 { register u32 QVal;  asm __volatile__ ( "mfspr (%0), GQR1" : "=r" (QVal) );  return QVal; }
    inline u32  GetGQR2( void )                 { register u32 QVal;  asm __volatile__ ( "mfspr (%0), GQR2" : "=r" (QVal) );  return QVal; }
    inline u32  GetGQR3( void )                 { register u32 QVal;  asm __volatile__ ( "mfspr (%0), GQR3" : "=r" (QVal) );  return QVal; }
    inline u32  GetGQR4( void )                 { register u32 QVal;  asm __volatile__ ( "mfspr (%0), GQR4" : "=r" (QVal) );  return QVal; }
    inline u32  GetGQR5( void )                 { register u32 QVal;  asm __volatile__ ( "mfspr (%0), GQR5" : "=r" (QVal) );  return QVal; }
    inline u32  GetGQR6( void )                 { register u32 QVal;  asm __volatile__ ( "mfspr (%0), GQR6" : "=r" (QVal) );  return QVal; }
    inline u32  GetGQR7( void )                 { register u32 QVal;  asm __volatile__ ( "mfspr (%0), GQR7" : "=r" (QVal) );  return QVal; }

} // namespace GQR


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


class GCSKIN
{
  public:
    /////////////////////////////////////////////////////////////////
    // GCSKIN DATA
    /////////////////////////////////////////////////////////////////

    struct SDMABuf
    {
        byte*   pSrcTag;
        byte*   pDstTag;
    };


    static s32              s_NMtxGroups;
    static s32              s_NSubMeshes;

    static s32              s_MaxMtxGroups;
    static s32              s_MaxSubMeshes;
    static byte*            s_pSkinBuffer;

    static GCMtxGroup*      s_pMtxGroups;
    static GCSubMeshGroup*  s_pSubmeshGroups;

    static s32              s_FixedAlpha;

    static SDMABuf          s_DMABuf[DMA_BUFFER_COUNT];

    static u32              s_SaveGQR4;
    static u32              s_SaveGQR5;


    /////////////////////////////////////////////////////////////////
    // GCSKIN FUNCTIONS
    /////////////////////////////////////////////////////////////////

    static inline GCMtxGroup* AddMtxGroup( void )
    {
        ASSERT( s_pMtxGroups != NULL );
        ASSERT( s_NMtxGroups < s_MaxMtxGroups );
        s_NMtxGroups++;
        return &s_pMtxGroups[ s_NMtxGroups-1 ];
    }

    static inline GCSubMeshGroup* AddSubMeshGroup( void )
    {
        ASSERT( s_pSubmeshGroups != NULL );
        ASSERT( s_NSubMeshes < s_MaxSubMeshes );
        s_NSubMeshes++;
        return &s_pSubmeshGroups[ s_NSubMeshes-1 ];
    }

    static inline void CloseBuffer( void )
    {
        s_NMtxGroups     = 0;
        s_NSubMeshes     = 0;
        s_MaxMtxGroups   = 0;
        s_MaxSubMeshes   = 0;
        s_pSkinBuffer    = NULL;
        s_pMtxGroups     = NULL;
        s_pSubmeshGroups = NULL;
        s_FixedAlpha     = 0;
    }

    static inline GCMtxGroup*     GetMtxGroupPtr    ( void )   { return s_pMtxGroups; }
    static inline GCSubMeshGroup* GetSubMeshGroupPtr( void )   { return s_pSubmeshGroups; }
    static inline s32             GetMtxGroupCount  ( void )   { return s_NMtxGroups; }
    static inline s32             GetSubMeshCount   ( void )   { return s_NSubMeshes; }

    static inline xbool IsBufferOpen( void )   { return (s_pSkinBuffer != NULL) ? TRUE : FALSE; }
    static inline xbool IsBufferEmpty( void )  { return (s_NSubMeshes == 0) ? TRUE : FALSE; }


    /////////////////////////////////////////////////////////////////
    // GCSKIN VERTEX SETUP FUNCTIONS
    /////////////////////////////////////////////////////////////////

    static inline void SetupVtxFmt( u32 SubMeshFlags, xbool CPUList )
    {
        if( SubMeshFlags & SKIN_SUBMESH_FLAG_SHADOW )
        {
            GXClearVtxDesc();
            GXSetVtxDesc  ( GX_VA_PNMTXIDX, GX_DIRECT  );
            GXSetVtxDesc  ( GX_VA_POS,      GX_INDEX16 );
            GXSetVtxDesc  ( GX_VA_CLR0,     GX_INDEX8  );

            GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_POS,  GX_POS_XYZ,  GX_S16,   GC_FRAC_POS );
            GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA4, 0 );
        }
        else
        {
            GXClearVtxDesc();
            GXSetVtxDesc  ( GX_VA_PNMTXIDX,   (CPUList ? GX_NONE : GX_DIRECT) );

            if( SubMeshFlags & SKIN_SUBMESH_FLAG_ENVPASS )
                GXSetVtxDesc( GX_VA_TEX1MTXIDX, (CPUList ? GX_NONE : GX_DIRECT) );

            GXSetVtxDesc  ( GX_VA_POS,        GX_INDEX16 );
            GXSetVtxDesc  ( GX_VA_NRM,        GX_INDEX16 );
            GXSetVtxDesc  ( GX_VA_TEX0,       GX_INDEX16 );

            GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_POS,  GX_POS_XYZ, GX_S16, GC_FRAC_POS );
            GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_NRM,  GX_NRM_XYZ, GX_S16, GC_FRAC_NRM );
            GXSetVtxAttrFmt( GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST,  GX_S16, GC_FRAC_UV  );
        }
    }

    static inline void SetVtxArray( GCSkinVert_PN* pVtx )
    {
        GXSetArray( GX_VA_POS, &pVtx[0].PX, sizeof(GCSkinVert_PN) );
        GXSetArray( GX_VA_NRM, &pVtx[0].NX, sizeof(GCSkinVert_PN) );
    }

    static inline void SetVtxArray( GCSkinVert_P* pVtx )
    {
        GXSetArray( GX_VA_POS,  &pVtx[0].PX, sizeof(GCSkinVert_P) );
        GXSetArray( GX_VA_CLR0, &pVtx[0].PX, sizeof(u16) );
    }

    static inline void SetVtxArray( GCSkinVert_UV* pUVs )
    {
        GXSetArray( GX_VA_TEX0, &pUVs[0].U, sizeof(GCSkinVert_UV) );
    }

    static inline void SetMtxArray( GCMtxGroup* pBoneMtx )
    {
        GXSetArray( GX_POS_MTX_ARRAY, &pBoneMtx->L2V[0],    sizeof(GCPosMtx) );
        GXSetArray( GX_NRM_MTX_ARRAY, &pBoneMtx->NL2V[0],   sizeof(GCNrmMtx) );
        GXSetArray( GX_TEX_MTX_ARRAY, &pBoneMtx->NLAnim[0], sizeof(GCPosMtx) );
    }

    static inline void SetMtxArray( GCPosMtxArray* pShadowBoneMtx )
    {
        GXSetArray( GX_POS_MTX_ARRAY, &(*pShadowBoneMtx)[0], sizeof(GCPosMtx) );
    }


    /////////////////////////////////////////////////////////////////
    // GCSKIN GQR SETUP FUNCTIONS
    /////////////////////////////////////////////////////////////////

    static inline void SetupGQR( void )
    {
        s_SaveGQR4 = GQR::GetGQR4();
        s_SaveGQR5 = GQR::GetGQR5();

        GQR::SetGQR4( GQR_MAKE( GC_FRAC_DELTA,  GQR::TYPE_S16, GC_FRAC_DELTA,  GQR::TYPE_S16 ) );
        GQR::SetGQR5( GQR_MAKE( GC_FRAC_POS,    GQR::TYPE_S16, GC_FRAC_POS,    GQR::TYPE_S16 ) );
        GQR::SetGQR6( GQR_MAKE( GC_FRAC_NRM,    GQR::TYPE_S16, GC_FRAC_NRM,    GQR::TYPE_S16 ) );
        GQR::SetGQR7( GQR_MAKE( GC_FRAC_WEIGHT, GQR::TYPE_U8,  GC_FRAC_WEIGHT, GQR::TYPE_U8  ) );
    }

    static inline void RestoreGQR( void )
    {
        GQR::SetGQR4( s_SaveGQR4 );
        GQR::SetGQR5( s_SaveGQR5 );
    }

    /////////////////////////////////////////////////////////////////
    // GCSKIN LOCKED CACHE FUNCTIONS
    /////////////////////////////////////////////////////////////////

    static inline void SetupLockedCache( void )
    {
        LCEnable();
        //--- offset by 32 bytes for padding
        byte* pLCache = (byte*)((u32)LCGetBase() + 32);

        for( s32 i = 0; i < DMA_BUFFER_COUNT; i++ )
        {
            s_DMABuf[i].pSrcTag = pLCache + (32 * i * (NUM_SRC_BLOCKS_PER_BUFFER + NUM_DST_BLOCKS_PER_BUFFER));
            s_DMABuf[i].pDstTag = s_DMABuf[i].pSrcTag + (32 * NUM_SRC_BLOCKS_PER_BUFFER);
        }
    }

    static inline void RestoreL1Cache( void )
    {
        for( s32 i = 0; i < DMA_BUFFER_COUNT; i++ )
        {
            s_DMABuf[i].pSrcTag = NULL;
            s_DMABuf[i].pDstTag = NULL;
        }

        LCDisable();
    }

    static inline GCSkinVert_CPU* GetDMASrcBuf( s32 BufIdx )
    {
        return (GCSkinVert_CPU*)s_DMABuf[BufIdx].pSrcTag;
    }

    static inline GCSkinVert_PN* GetDMADstBuf( s32 BufIdx )
    {
        return (GCSkinVert_PN*)s_DMABuf[BufIdx].pDstTag;
    }

};

//==========================================================================

// This function should never be called.  It is only here to let the
// assembler know the specified names of the Gekko registers.
static void SN_Init_ASM_Register_Names( void ) __attribute__((naked));
static void SN_Init_ASM_Register_Names( void )
{
    asm(".equr r0,  %r0");      asm(".equr f0,  %f0");
    asm(".equr r1,  %r1");      asm(".equr f1,  %f1");
    asm(".equr r2,  %r2");      asm(".equr f2,  %f2");
    asm(".equr r3,  %r3");      asm(".equr f3,  %f3");
    asm(".equr r4,  %r4");      asm(".equr f4,  %f4");
    asm(".equr r5,  %r5");      asm(".equr f5,  %f5");
    asm(".equr r6,  %r6");      asm(".equr f6,  %f6");
    asm(".equr r7,  %r7");      asm(".equr f7,  %f7");
    asm(".equr r8,  %r8");      asm(".equr f8,  %f8");
    asm(".equr r9,  %r9");      asm(".equr f9,  %f9");
    asm(".equr r10, %r10");     asm(".equr f10, %f10");
    asm(".equr r11, %r11");     asm(".equr f11, %f11");
    asm(".equr r12, %r12");     asm(".equr f12, %f12");
    asm(".equr r13, %r13");     asm(".equr f13, %f13");
    asm(".equr r14, %r14");     asm(".equr f14, %f14");
    asm(".equr r15, %r15");     asm(".equr f15, %f15");
    asm(".equr r16, %r16");     asm(".equr f16, %f16");
    asm(".equr r17, %r17");     asm(".equr f17, %f17");
    asm(".equr r18, %r18");     asm(".equr f18, %f18");
    asm(".equr r19, %r19");     asm(".equr f19, %f19");
    asm(".equr r20, %r20");     asm(".equr f20, %f20");
    asm(".equr r21, %r21");     asm(".equr f21, %f21");
    asm(".equr r22, %r22");     asm(".equr f22, %f22");
    asm(".equr r23, %r23");     asm(".equr f23, %f23");
    asm(".equr r24, %r24");     asm(".equr f24, %f24");
    asm(".equr r25, %r25");     asm(".equr f25, %f25");
    asm(".equr r26, %r26");     asm(".equr f26, %f26");
    asm(".equr r27, %r27");     asm(".equr f27, %f27");
    asm(".equr r28, %r28");     asm(".equr f28, %f28");
    asm(".equr r29, %r29");     asm(".equr f29, %f29");
    asm(".equr r30, %r30");     asm(".equr f30, %f30");
    asm(".equr r31, %r31");     asm(".equr f31, %f31");

    asm(".equr cr0, %cr0");
    asm(".equr cr1, %cr1");
    asm(".equr cr2, %cr2");
    asm(".equr cr3, %cr3");
    asm(".equr cr4, %cr4");
    asm(".equr cr5, %cr5");
    asm(".equr cr6, %cr6");
    asm(".equr cr7, %cr7");
}

//==========================================================================
