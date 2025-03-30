///////////////////////////////////////////////////////////////////////////
//  SKEL.CPP
///////////////////////////////////////////////////////////////////////////

#include "x_debug.hpp"
#include "x_memory.hpp"
#include "x_math.hpp"
#include "x_plus.hpp"

#include "Tokenizer.hpp"
#include "skel.h"

//==========================================================================
// Implementation
//==========================================================================

QSkel::QSkel( void )
{
    m_NBones    = 0;
    m_pBone     = NULL;
}

//==========================================================================

QSkel::~QSkel( void )
{
    if ( m_NBones > 0 )
        x_free( m_pBone );
}

//==========================================================================

void QSkel::CopySkel( QSkel* SkelSrc )
{
    s32 i;

    ASSERT(SkelSrc);

    //---   free up previous bones
    if ( m_NBones > 0 )
    {
        x_free( m_pBone );
        m_pBone = NULL;
        m_NBones = 0;
    }
    
    //---   copy the src skeleton
    if ( SkelSrc->m_NBones )
    {
        m_pBone = (t_SkelBone*)x_malloc(sizeof(t_SkelBone)*SkelSrc->m_NBones);
        ASSERT(m_pBone);

        //---   duplicate bone value
        m_NBones = SkelSrc->m_NBones;
        for ( i = 0; i < SkelSrc->m_NBones; i++ )
        {
            m_pBone[i] = SkelSrc->m_pBone[i];
        }
    }
}

//==========================================================================

xbool QSkel::ImportASF( const char* FileName )
{
    s32			i, j;
    tokenizer	TOK;
    s32			NBonesRead;
    vector3 	Dir;
    f32			Len;
	xbool		Success = FALSE;
    xbool       Swaps = TRUE;
    vector3     P2B[100];

    //---   Init skel to valid state
    if ( m_NBones > 0 )
        x_free( m_pBone );
    m_NBones    = 0;
    m_pBone     = NULL;
    TKN_Clear( &TOK );
    if ( !TKN_ReadFile( &TOK, FileName ) )
    {
		goto Error;
    }

    //---   decide how many bones in skeleton by counting instances of 'id'
    TKN_Rewind( &TOK );
    while ( TKN_FindToken( &TOK, "ID" ) )
        m_NBones++;
    m_NBones++; // for the root node
    m_pBone = (t_SkelBone*)x_malloc( sizeof(t_SkelBone) * m_NBones );
    ASSERT( m_pBone );

    //---   clear bones to default values
    x_memset( m_pBone,0, sizeof(t_SkelBone) * m_NBones );
    for ( i=0; i < m_NBones; i++ )
    {
        x_strcpy( m_pBone[i].BoneName, "NO NAME" );
        x_strcpy( m_pBone[i].ParentName, "NO NAME" );
        for ( j=0; j < 6; j++ )
            m_pBone[i].DOFOrder[j] = 0;
    }

    //---   Build a root node
    x_strcpy( m_pBone[0].BoneName, "ROOT" );
    m_pBone[0].BoneID    = 0;
    m_pBone[0].ParentID  = -1;
    m_pBone[0].Flags     = BONE_DOF_RX | BONE_DOF_RY | BONE_DOF_RZ |
                           BONE_DOF_TX | BONE_DOF_TY | BONE_DOF_TZ;
    Dir.X = Dir.Y = Dir.Z = Len = 0.0f;

    //---   move to root node info, read in order of data
    TKN_Rewind( &TOK );
    TKN_FindToken( &TOK, ":root" );
    TKN_FindToken( &TOK, "order" );
    for ( i = 0; i < 6; i++ )
    {
        TKN_ReadToken( &TOK );
        if ( TOK.EOLNs!=0 )
            return FALSE;
        if ( TOK.String[0] == 'T' )
        {
            if ( TOK.String[1] == 'X' )
                m_pBone[0].DOFOrder[i] = BONE_DOF_TX;
            else
            if ( TOK.String[1] == 'Y' )
                m_pBone[0].DOFOrder[i] = BONE_DOF_TY;
            else
            if ( TOK.String[1] == 'Z' )
                m_pBone[0].DOFOrder[i] = BONE_DOF_TZ;
        }
        else
        {
            if ( TOK.String[1] == 'X' )
                m_pBone[0].DOFOrder[i] = BONE_DOF_RX;
            else
            if ( TOK.String[1] == 'Y' )
                m_pBone[0].DOFOrder[i] = BONE_DOF_RY;
            else
            if ( TOK.String[1] == 'Z' )
                m_pBone[0].DOFOrder[i] = BONE_DOF_RZ;
        }
    }    

    //---   check on units
    TKN_Rewind( &TOK );
    TKN_FindToken( &TOK, ":UNITS" );
    while ( 1 )
    {
        TKN_ReadToken( &TOK );
        if ( x_strcmp( TOK.String, "MASS" ) == 0 )
        {
            TKN_ReadToken( &TOK );
        }
        else
        if ( x_strcmp( TOK.String, "LENGTH" ) == 0 ) 
        {
            TKN_ReadToken( &TOK );
            m_TransScale = (f32)(1.0f / TOK.Float);
        }
        else
        if ( x_strcmp( TOK.String, "ANGLE" ) == 0 ) 
        {
            TKN_ReadToken( &TOK );
            if ( x_strcmp( TOK.String, "DEG" ) != 0 )
                goto Error;
        }
        else
        {
            break;
        }
    }

    //---   Read in bone specifics
    if ( m_NBones > 1 )
    {
        TKN_Rewind( &TOK );
        TKN_FindToken( &TOK, ":BONEDATA" );
        TKN_ReadToken( &TOK );
        NBonesRead = 1;

        for ( i = 0; i < m_NBones - 1; i++ )
        {
            t_SkelBone* pBone = &m_pBone[NBonesRead];

            //---   read the id
            TKN_FindToken( &TOK, "ID" );
            TKN_ReadToken( &TOK );
            pBone->BoneID = NBonesRead;
        
            //---   read the name
            TKN_FindToken( &TOK, "NAME" );
            TKN_ReadToken( &TOK );
            x_strcpy( pBone->BoneName, TOK.String );
            x_strtoupper( pBone->BoneName );

            //---   read the direction
            TKN_FindToken( &TOK, "DIRECTION" );
            TKN_ReadToken( &TOK );
            Dir.X = (f32)TOK.Float;
            TKN_ReadToken( &TOK );
            Dir.Y = (f32)TOK.Float;
            TKN_ReadToken( &TOK );
            Dir.Z = (f32)TOK.Float;

            //---   read the length
            TKN_FindToken( &TOK, "LENGTH" ); 
            TKN_ReadToken( &TOK );
            Len   = (f32)TOK.Float * m_TransScale;
        
            //---   read the axis
            TKN_FindToken( &TOK, "AXIS" ); 
            TKN_ReadToken( &TOK );
            pBone->Axis.Pitch = (f32)TOK.Float * R_1;
            TKN_ReadToken( &TOK );
            pBone->Axis.Yaw = (f32)TOK.Float * R_1;
            TKN_ReadToken( &TOK );
            pBone->Axis.Roll = (f32)TOK.Float * R_1;
            TKN_ReadToken( &TOK );    // xyz

            //---   Read next token
            TKN_ReadToken( &TOK );
        
            //---   read the DOFs
            if ( x_strcmp( TOK.String, "DOF" ) == 0 )
            {
                j = 0;
                while (1)
                {
                    TKN_ReadToken( &TOK );
                    if ( x_strcmp( TOK.String, "TX" ) == 0 )
                    {
                        pBone->Flags |= BONE_DOF_TX;
                        pBone->DOFOrder[j] = BONE_DOF_TX;
                    }
				    else
                    if ( x_strcmp( TOK.String, "TY" )== 0 )
                    {
                        pBone->Flags |= BONE_DOF_TY;
                        pBone->DOFOrder[j] = BONE_DOF_TY;
                    }
				    else
                    if ( x_strcmp( TOK.String, "TZ" ) == 0 )
                    {
                        pBone->Flags |= BONE_DOF_TZ;
                        pBone->DOFOrder[j] = BONE_DOF_TZ;
                    }
				    else
                    if ( x_strcmp( TOK.String, "RX" ) == 0 )
                    {
                        pBone->Flags |= BONE_DOF_RX;
                        pBone->DOFOrder[j] = BONE_DOF_RX;
                    }
				    else
                    if ( x_strcmp( TOK.String, "RY" ) == 0 )
                    {
                        pBone->Flags |= BONE_DOF_RY;
                        pBone->DOFOrder[j] = BONE_DOF_RY;
                    }
				    else
                    if ( x_strcmp( TOK.String, "RZ" ) == 0 )
                    {
                        pBone->Flags |= BONE_DOF_RZ;
                        pBone->DOFOrder[j] = BONE_DOF_RZ;
                    }
				    else
                    {
                        break;
                    }
                    j++;
                }
            }    

            //---   Skip limits
            if ( x_strcmp( TOK.String, "LIMITS" ) == 0 )
            {
                TKN_FindToken( &TOK, "END" );
            }

            //---   confirm that we are at the end of the bone
            if ( x_strcmp( TOK.String, "END" ) != 0 )
                goto Error;

            //---   Compute ParentToBone ... LocalTranslation
            pBone->ParentToBone.X = Dir.X * Len;
            pBone->ParentToBone.Y = Dir.Y * Len;
            pBone->ParentToBone.Z = Dir.Z * Len;

            pBone->BoneVector = pBone->ParentToBone;

            //---   Increment number of bones read
            NBonesRead++;

            //---   Read next token
            TKN_ReadToken( &TOK );
        }
    }

    //---   read in hierarchy
    TKN_Rewind( &TOK );
    TKN_FindToken( &TOK, ":HIERARCHY" );
    TKN_FindToken( &TOK, "BEGIN" );

    //---   Read Parent bone
    TKN_ReadToken( &TOK );
    while ( x_strcmp( TOK.String, "END" ) != 0 )
    {
        s32 ParentIndex;
        s32 ChildIndex;

        //---   Find parent bone
        x_strtoupper( TOK.String );
        ParentIndex = FindBone( TOK.String );
        if ( ParentIndex == -1 )
            goto Error;

        //---   Loop through children
        TKN_ReadToken( &TOK );
        while ( TOK.EOLNs == 0 )
        {
            //---   Find child bone
            x_strtoupper( TOK.String );
            ChildIndex = FindBone( TOK.String );
            if ( ChildIndex == -1 ) 
                goto Error;

            //---   Set ParentID and ParentName
            m_pBone[ChildIndex].ParentID = m_pBone[ParentIndex].BoneID;
            x_strcpy( m_pBone[ChildIndex].ParentName, m_pBone[ParentIndex].BoneName );

            //---   Read next child bone
            TKN_ReadToken( &TOK );
        }
    }

    //---   Sort the bones so that parents are above (have lower indices)
    //      than children

    //---   Swap the parent and child until everyone is in order
    while ( Swaps )
    {
        Swaps = FALSE;
        for ( i = 0; i < m_NBones; i++ )
        {
            if ( m_pBone[i].ParentID != -1 )
            {
                //---   Find Parent
                for ( j = 0; j < m_NBones; j++ )
                {
                    if ( m_pBone[j].BoneID == m_pBone[i].ParentID )
                        break;
                }
                ASSERT( j != m_NBones );

                //---   Check and swap
                if ( j > i )
                {
                    t_SkelBone TempB;
                    Swaps       = TRUE;
                    TempB       = m_pBone[i];
                    m_pBone[i]  = m_pBone[j];
                    m_pBone[j]  = TempB;
                }
            }
        }
    }

    //---   Re-index ParentIDs
    for ( i = 0; i < m_NBones; i++ )
    {
        if ( m_pBone[i].ParentID != -1 )
        {
            //---   Find Parent
            for ( j = 0; j < m_NBones; j++ )
            {
                if ( m_pBone[j].BoneID == m_pBone[i].ParentID )
                    break;
            }
            ASSERT( j != m_NBones );
            m_pBone[i].ParentID = j;
        }
    }

    for ( i = 0; i < m_NBones; i++ )
        m_pBone[i].BoneID = i;

    //---   Confirm all bone's parents have lower IDs
    for ( i = 0; i < m_NBones; i++ )
    {
        if ( m_pBone[i].ParentID >= i ) 
            goto Error;
    }

    //---   We had only half-calculated the parent-to-bone, because
    //      the ParentToBone actually only contains the bone without
    //      regards to the parent
    for ( i = 0; i < m_NBones; i++ )
        P2B[i] = m_pBone[i].ParentToBone;
    for ( i = 1; i < m_NBones; i++ )
        m_pBone[i].ParentToBone = P2B[m_pBone[i].ParentID];

    //---   Compute Global positions of joints
    CalcGlobalPosFromLocal();

    //---   Decide mirror bones
    DecideMirrorBones();

    //---   Return success code
	Success = TRUE;

Error:
    //---   we're outta here!
	TKN_CloseFile( &TOK );
	return Success;
}

//==========================================================================

s32 QSkel::FindBone( const char* BoneName )
{
    s32 i;

    for ( i = 0; i < m_NBones; i++ )
    {
        if ( x_stricmp( m_pBone[i].BoneName, BoneName ) == 0 )
            return i;
    }

    return -1;
}

//==========================================================================

void QSkel::CalcGlobalPosFromLocal( void )
{
    s32             BStack[100];    // recursion stack...up to 100 bones
    s32             BStackSize;     // num bones in stack
    t_SkelBone*     pBone;
    t_SkelBone*     pParent;
    s32             i;

    //---   Push root bone on stack
    BStack[0]  = 0;
    BStackSize = 1;

    //---   Process all bones on stack
    while ( BStackSize )
    {   
        //---   Pop bone
        BStackSize--;
        pBone = &m_pBone[BStack[BStackSize]];

        //---   Compute OriginToBone
        if ( pBone->ParentID != -1 ) 
        {
            pParent = &m_pBone[pBone->ParentID];
            pBone->OriginToBone = pParent->OriginToBone + pBone->ParentToBone;
        }

        //---   Push children
        for ( i = 0; i < m_NBones; i++ )
        {
            if ( m_pBone[i].ParentID == pBone->BoneID )
            {
                BStack[BStackSize] = i;
                BStackSize++;
            }
        }
    }
}

//==========================================================================

void QSkel::DecideMirrorBones( void )
{
    s32 i,j;
    char TestName[32];
    char* SrcName;

    for ( i = 0; i < m_NBones; i++ )
    {
        SrcName = m_pBone[i].BoneName;
        m_pBone[i].MirrorID = i;

        for ( j = 0; j < m_NBones; j++ )
        {
            if ( i != j )
            {
                //---   mirrored bones differ only by the first letter,
                //      so check that...
                x_strcpy( TestName, m_pBone[j].BoneName );
                TestName[0] = SrcName[0];

                if ( x_strcmp( TestName, SrcName ) == 0 )
                {
                    m_pBone[i].MirrorID = j;
                }
            }
        }
    }
}

//==========================================================================

const char* QSkel::GetBoneName( s32 BoneID )
{
    return m_pBone[BoneID].BoneName;
}

//==========================================================================

void QSkel::CalcLocalPosFromGlobal( void )
{
    t_SkelBone* pParent;
    t_SkelBone* pBone;
    s32         i;

    for ( i = 1; i < m_NBones; i++ )
    {
        pBone = &m_pBone[i];
        pParent = &m_pBone[pBone->ParentID];
        
        pBone->ParentToBone = pBone->OriginToBone - pParent->OriginToBone;
    }
}

//==========================================================================

void QSkel::DelBone( s32 TargetBoneID )
{
    s32         i,j;
    t_SkelBone* pBone;

    //---   Be sure we have global positions of bones
    CalcGlobalPosFromLocal();

    //---   Point all children of Target to Target's parent
    for ( i = 0; i < m_NBones; i++ )
    {
        if ( m_pBone[i].ParentID == TargetBoneID )
        {
            x_strcpy( m_pBone[i].ParentName,
                      m_pBone[TargetBoneID].ParentName );
            m_pBone[i].ParentID = m_pBone[TargetBoneID].ParentID;
        }
    }

    //---   Allocate a new bone list with one less bone
    pBone = (t_SkelBone*)x_malloc( sizeof(t_SkelBone) * (m_NBones - 1) );
    ASSERT(pBone);

    //---   Copy new bone list without Target and replace in skel
    j = 0;
    for ( i = 0; i < m_NBones; i++ )
    {
        if ( i != TargetBoneID )
        {
            pBone[j] = m_pBone[i];
            j++;
        }
    }
    x_free( m_pBone );
    m_pBone = pBone;
    m_NBones--;

    //---   Decrement BoneID's greater than TargetBoneID since they slid down
    for ( i = 0; i < m_NBones; i++ )
    {
        if ( m_pBone[i].BoneID > TargetBoneID )
            m_pBone[i].BoneID--;
        if ( m_pBone[i].ParentID > TargetBoneID )
            m_pBone[i].ParentID--;
    }

    //---   Recompute local positions of bones
    CalcLocalPosFromGlobal();

    //--- Decide on MirrorIDs
    DecideMirrorBones();
}

//==========================================================================

void QSkel::DelDummyBones( void )
{
    s32 i;
    u32 AnyDOF = BONE_DOF_RX | BONE_DOF_RY | BONE_DOF_RZ |
                 BONE_DOF_TX | BONE_DOF_TY | BONE_DOF_TZ;

    //---   Find any bones with no dofs. If we find one, delete it then
    //      start the process over from the beginning.
    i = 0;
    while ( i < m_NBones )
    {
        if ( (m_pBone[i].Flags & AnyDOF) == 0 )
        {
            DelBone( i );
            i = 0;
        }
        else
        {
            i++;
        }
    }

    //---   Decide on MirrorIDs
    DecideMirrorBones();
}

//==========================================================================

void QSkel::ClearAxis( void )
{
    s32 i;

    for ( i = 0; i < m_NBones; i++ )
    {
        m_pBone[i].Axis.Set( R_0, R_0, R_0 );
        m_pBone[i].Flags &= ~(BONE_DOF_TX|BONE_DOF_TY|BONE_DOF_TZ);
        m_pBone[i].Flags |=  (BONE_DOF_RX|BONE_DOF_RY|BONE_DOF_RZ);
    }
}
