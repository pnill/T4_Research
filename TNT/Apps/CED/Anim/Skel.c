///////////////////////////////////////////////////////////////////////////
//
//  SKEL.C
//
///////////////////////////////////////////////////////////////////////////
#include "x_debug.h"
#include "x_memory.h"
#include "x_math.h"
#include "x_plus.h"

#include "y_token.h"
#include "skel.h"
#include "anim.h"

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//                      BASIC MANIPULATIONS
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void SKEL_InitInstance  (skel* Skel)
{
    ASSERT(Skel);
    Skel->NBones = 0;
    Skel->Bone   = NULL;
}

///////////////////////////////////////////////////////////////////////////

void SKEL_CopySkel      (skel* SkelDst, skel* SkelSrc)
{
    s32 i;

    ASSERT(SkelDst);
    ASSERT(SkelSrc);

    // Allocate new bones
    SkelDst->Bone = (skel_bone*)x_malloc(sizeof(skel_bone)*SkelSrc->NBones);
    ASSERT(SkelDst->Bone);

    // Duplicate bone values
    SkelDst->NBones = SkelSrc->NBones;
    for (i=0; i<SkelSrc->NBones; i++)
    {
        SkelDst->Bone[i] = SkelSrc->Bone[i];
    }
}

///////////////////////////////////////////////////////////////////////////

void SKEL_KillInstance  (skel* Skel)
{
    ASSERT(Skel);
    if (Skel->NBones > 0);
        x_free(Skel->Bone);
    SKEL_InitInstance(Skel);
}

///////////////////////////////////////////////////////////////////////////

s32  SKEL_FindBone           (skel* Skel, const char* BoneName)
{
    s32 i;

    for (i=0; i<Skel->NBones; i++)
    {
        if (x_stricmp(Skel->Bone[i].BoneName,BoneName)==0)
            return i;
    }

    return -1;
}

///////////////////////////////////////////////////////////////////////////

char* SKEL_GetBoneName       (skel* Skel, s32 BoneID)
{
    return Skel->Bone[BoneID].BoneName;
}

///////////////////////////////////////////////////////////////////////////

void CalcGlobalPosFromLocal(skel* Skel)
{
    s32         BStack[100];    // recursion stack...up to 100 bones
    s32         BStackSize;     // num bones in stack
    skel_bone*  B;
    skel_bone*  PB;
    s32         i;

    // Push root bone on stack
    BStack[0]  = 0;
    BStackSize = 1;

    // Process all bones on stack
    while (BStackSize)
    {   
        // Pop bone
        BStackSize--;
        B  = &Skel->Bone[BStack[BStackSize]];

        // Compute OriginToBone
        if (B->ParentID != -1) 
        {
            PB = &Skel->Bone[B->ParentID];
            B->OriginToBone.X = PB->OriginToBone.X + B->ParentToBone.X;
            B->OriginToBone.Y = PB->OriginToBone.Y + B->ParentToBone.Y;
            B->OriginToBone.Z = PB->OriginToBone.Z + B->ParentToBone.Z;
        }

        // Push children
        for (i=0; i<Skel->NBones; i++)
        if (Skel->Bone[i].ParentID == B->BoneID)
        {
            BStack[BStackSize] = i;
            BStackSize++;
        }
    }
}

///////////////////////////////////////////////////////////////////////////

void CalcLocalPosFromGlobal(skel* Skel)
{
    skel_bone* PB;
    skel_bone* B;
    s32 i;

    for (i=1; i<Skel->NBones; i++)
    {
        B  = &Skel->Bone[i];
        PB = &Skel->Bone[B->ParentID];

        B->ParentToBone.X = B->OriginToBone.X - PB->OriginToBone.X;
        B->ParentToBone.Y = B->OriginToBone.Y - PB->OriginToBone.Y;
        B->ParentToBone.Z = B->OriginToBone.Z - PB->OriginToBone.Z;
    }
}

///////////////////////////////////////////////////////////////////////////

void SKEL_DelBone            (skel* Skel, s32 TargetBoneID)
{
    s32 i,j;
    skel_bone* B;

    // Be sure we have global positions of bones
    CalcGlobalPosFromLocal(Skel);

    // Point all children of Target to Target's parent
    for (i=0; i<Skel->NBones; i++)
    if (Skel->Bone[i].ParentID == TargetBoneID)
    {
        x_strcpy(Skel->Bone[i].ParentName,
                 Skel->Bone[TargetBoneID].ParentName);
        Skel->Bone[i].ParentID = Skel->Bone[TargetBoneID].ParentID;
    }

    // Allocate a new bone list with one less bone
    B = (skel_bone*)x_malloc(sizeof(skel_bone)*(Skel->NBones-1));
    ASSERT(B);

    // Copy new bone list without Target and replace in skel
    j = 0;
    for (i=0; i<Skel->NBones; i++)
    if (i != TargetBoneID)
    {
        B[j] = Skel->Bone[i];
        j++;
    }
    x_free(Skel->Bone);
    Skel->Bone = B;
    Skel->NBones--;

    // Decrement BoneID's greater than TargetBoneID since they slid down
    for (i=0; i<Skel->NBones; i++)
    {
        if (Skel->Bone[i].BoneID   > TargetBoneID) Skel->Bone[i].BoneID--;
        if (Skel->Bone[i].ParentID > TargetBoneID) Skel->Bone[i].ParentID--;
    }

    // Recompute local positions of bones
    CalcLocalPosFromGlobal(Skel);

    /////////////////////////////////////////////////////////////
    // Decide on MirrorIDs
    /////////////////////////////////////////////////////////////
    SKEL_DecideMirrorBones (Skel);
}

///////////////////////////////////////////////////////////////////////////

void SKEL_DelDummyBones      (skel* Skel)
{
    s32 i;
    u32 AnyDOF = BONE_DOF_RX | BONE_DOF_RY | BONE_DOF_RZ |
                 BONE_DOF_TX | BONE_DOF_TY | BONE_DOF_TZ;

    // Find any bones with no dofs.  If we find one, delete it then start
    // the process over from the beginning.
    for ( i = 0; (i < Skel->NBones); )
    {
        if ((Skel->Bone[i].Flags & AnyDOF) == 0)
        {
            SKEL_DelBone(Skel,i);
            i = 0;
            continue;
        }

        i++;
    }

    /////////////////////////////////////////////////////////////
    // Decide on MirrorIDs
    /////////////////////////////////////////////////////////////
    SKEL_DecideMirrorBones (Skel);
}

///////////////////////////////////////////////////////////////////////////

void SKEL_ClearAxis (skel* Skel)
{
    s32 i;
    for (i=0; i<Skel->NBones; i++)
    {
        Skel->Bone[i].Axis.X = 0;
        Skel->Bone[i].Axis.Y = 0;
        Skel->Bone[i].Axis.Z = 0;
        Skel->Bone[i].Flags &= ~(BONE_DOF_TX|BONE_DOF_TY|BONE_DOF_TZ);
        Skel->Bone[i].Flags |=  (BONE_DOF_RX|BONE_DOF_RY|BONE_DOF_RZ);
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//  IMPORT ASF
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

err  SKEL_ImportASF     (skel* Skel, const char* FileName)
{
    s32			i,j;
    tokenizer	TOK;
    s32			NBonesRead;
    vector3d	Dir;
    f32			Len;
	err			Success = ERR_FAILURE ;

    /////////////////////////////////////////////////////////////
    // Init skel to valid state, open ASF file with tokenizer
    /////////////////////////////////////////////////////////////
    SKEL_KillInstance(Skel);
    TKN_Clear(&TOK);
    if (TKN_ReadFile(&TOK,FileName))
    {
		goto Error ;
    }

    /////////////////////////////////////////////////////////////
    // Decide how many bones in skeleton by counting instances
    // of 'id'
    /////////////////////////////////////////////////////////////
    TKN_Rewind(&TOK);
    while (TKN_FindToken(&TOK,"ID")) Skel->NBones++;
    Skel->NBones++;
    Skel->Bone = (skel_bone*)x_malloc(sizeof(skel_bone)*Skel->NBones);
    ASSERT(Skel->Bone);
    
    /////////////////////////////////////////////////////////////
    // Clear bones to default values
    /////////////////////////////////////////////////////////////
    x_memset(Skel->Bone,0,sizeof(skel_bone)*Skel->NBones);
    for (i=0; i<Skel->NBones; i++)
    {
        x_strcpy(Skel->Bone[i].BoneName,"NO NAME");
        x_strcpy(Skel->Bone[i].ParentName,"NO NAME");
        for (j=0; j<6; j++)
            Skel->Bone[i].DOFOrder[j] = 0;
    }

    /////////////////////////////////////////////////////////////
    // Build Root Node
    /////////////////////////////////////////////////////////////
    x_strcpy(Skel->Bone[0].BoneName,"ROOT");
    Skel->Bone[0].BoneID    = 0;
    Skel->Bone[0].ParentID  = -1;
    Skel->Bone[0].Flags     = BONE_DOF_RX | BONE_DOF_RY | BONE_DOF_RZ |
                              BONE_DOF_TX | BONE_DOF_TY | BONE_DOF_TZ;
    Dir.X = Dir.Y = Dir.Z = Len = 0.0f;

    /////////////////////////////////////////////////////////////
    // Move to Root Node info, read in order of data
    /////////////////////////////////////////////////////////////
    TKN_Rewind(&TOK);
    TKN_FindToken(&TOK,":root");
    TKN_FindToken(&TOK,"order");
    for (i=0; i<6; i++)
    {
        TKN_ReadToken(&TOK);
        if (TOK.EOLNs!=0) return ERR_FAILURE;
        if (TOK.String[0]=='T')
        {
            if (TOK.String[1]=='X') Skel->Bone[0].DOFOrder[i] = BONE_DOF_TX;
            else
            if (TOK.String[1]=='Y') Skel->Bone[0].DOFOrder[i] = BONE_DOF_TY;
            else
            if (TOK.String[1]=='Z') Skel->Bone[0].DOFOrder[i] = BONE_DOF_TZ;
        }
        else
        {
            if (TOK.String[1]=='X') Skel->Bone[0].DOFOrder[i] = BONE_DOF_RX;
            else
            if (TOK.String[1]=='Y') Skel->Bone[0].DOFOrder[i] = BONE_DOF_RY;
            else
            if (TOK.String[1]=='Z') Skel->Bone[0].DOFOrder[i] = BONE_DOF_RZ;
        }
    }    

    /////////////////////////////////////////////////////////////
    // Check on units
    /////////////////////////////////////////////////////////////
    TKN_Rewind(&TOK);
    TKN_FindToken(&TOK,":UNITS");
    while (1)
    {
        TKN_ReadToken(&TOK);
        if (x_strcmp(TOK.String,"MASS")==0) TKN_ReadToken(&TOK);
        else
        if (x_strcmp(TOK.String,"LENGTH")==0) 
        {
            TKN_ReadToken(&TOK);
            Skel->TransScale = (f32)(1.0f / TOK.Float);
        }
        else
        if (x_strcmp(TOK.String,"ANGLE")==0) 
        {
            TKN_ReadToken(&TOK);
            if (x_strcmp(TOK.String,"DEG")!=0)
                goto Error;
        }
        else break;
    }

    /////////////////////////////////////////////////////////////
    // Read in bone specifics
    /////////////////////////////////////////////////////////////
    if (Skel->NBones > 1)
    {
        TKN_Rewind(&TOK);
        TKN_FindToken(&TOK,":BONEDATA");
        TKN_ReadToken(&TOK);
        NBonesRead = 1;

        for (i=0; i<Skel->NBones-1; i++)
        {
            skel_bone* B = &Skel->Bone[NBonesRead];

            // ID
            TKN_FindToken(&TOK,"ID");   
            TKN_ReadToken(&TOK);
            B->BoneID = NBonesRead;
        
            // NAME
            TKN_FindToken(&TOK,"NAME");   
            TKN_ReadToken(&TOK);
            x_strcpy(B->BoneName,TOK.String);
            x_strtoupper(B->BoneName);

            // DIRECTION
            TKN_FindToken(&TOK,"DIRECTION"); 
            TKN_ReadToken(&TOK); Dir.X = (f32)TOK.Float;
            TKN_ReadToken(&TOK); Dir.Y = (f32)TOK.Float;
            TKN_ReadToken(&TOK); Dir.Z = (f32)TOK.Float;

            // LENGTH
            TKN_FindToken(&TOK,"LENGTH"); 
            TKN_ReadToken(&TOK); Len   = (f32)TOK.Float * Skel->TransScale;
        
            // AXIS
            TKN_FindToken(&TOK,"AXIS"); 
            TKN_ReadToken(&TOK); B->Axis.X = (f32)TOK.Float * R_1;
            TKN_ReadToken(&TOK); B->Axis.Y = (f32)TOK.Float * R_1;
            TKN_ReadToken(&TOK); B->Axis.Z = (f32)TOK.Float * R_1;
            TKN_ReadToken(&TOK);    // xyz

            // Read next token
            TKN_ReadToken(&TOK);
        
            // DOF
            if (x_strcmp(TOK.String,"DOF")==0)
            {
                j=0;
                while (1)
                {
                    TKN_ReadToken(&TOK);
                    if (x_strcmp(TOK.String,"TX")==0) {B->Flags |= BONE_DOF_TX; B->DOFOrder[j] = BONE_DOF_TX;}
				    else
                    if (x_strcmp(TOK.String,"TY")==0) {B->Flags |= BONE_DOF_TY; B->DOFOrder[j] = BONE_DOF_TY;}
				    else
                    if (x_strcmp(TOK.String,"TZ")==0) {B->Flags |= BONE_DOF_TZ; B->DOFOrder[j] = BONE_DOF_TZ;}
				    else
                    if (x_strcmp(TOK.String,"RX")==0) {B->Flags |= BONE_DOF_RX; B->DOFOrder[j] = BONE_DOF_RX;}
				    else
                    if (x_strcmp(TOK.String,"RY")==0) {B->Flags |= BONE_DOF_RY; B->DOFOrder[j] = BONE_DOF_RY;}
				    else
                    if (x_strcmp(TOK.String,"RZ")==0) {B->Flags |= BONE_DOF_RZ; B->DOFOrder[j] = BONE_DOF_RZ;}
				    else break;
                    j++;
                }
            }    


            // SKIP LIMITS
            if (x_strcmp(TOK.String,"LIMITS")==0)
            {
                TKN_FindToken(&TOK,"END");
            }

            // CONFIRM WE ARE AT END OF BONE
            if (x_strcmp(TOK.String,"END")!=0)
                goto Error;

            // Compute ParentToBone ... LocalTranslation
            B->ParentToBone.X = Dir.X * Len;
            B->ParentToBone.Y = Dir.Y * Len;
            B->ParentToBone.Z = Dir.Z * Len;

            B->BoneVector = B->ParentToBone;

            // Increment number of bones
            NBonesRead++;

            // Read next token
            TKN_ReadToken(&TOK);
        }
    }

    /////////////////////////////////////////////////////////////
    // Read in hierarchy
    /////////////////////////////////////////////////////////////
    TKN_Rewind(&TOK);
    TKN_FindToken(&TOK,":HIERARCHY");
    TKN_FindToken(&TOK,"BEGIN");

    // Read Parent bone
    TKN_ReadToken(&TOK);
    while (x_strcmp(TOK.String,"END")!=0)
    {
        s32 PB;
        s32 CB;

        // Find parent bone
        x_strtoupper(TOK.String);
        PB = SKEL_FindBone(Skel,TOK.String);
        if (PB==-1) 
            goto Error ;

        // Loop through children
        TKN_ReadToken(&TOK);
        while (TOK.EOLNs == 0)
        {
            // Find child bone
            x_strtoupper(TOK.String);
            CB = SKEL_FindBone(Skel,TOK.String);
            if (CB==-1) 
                goto Error ;

            // Set ParentID and ParentName
            Skel->Bone[CB].ParentID = Skel->Bone[PB].BoneID;
            x_strcpy(Skel->Bone[CB].ParentName,Skel->Bone[PB].BoneName);

            // Read next child bone
            TKN_ReadToken(&TOK);
        }
    }

    /////////////////////////////////////////////////////////////
    // Sort bones so parents are above (lower indices) than children
    /////////////////////////////////////////////////////////////
    {
        s32 i,j;
        xbool Swaps = TRUE;

        // Swap parent and child until everyone is in order
        while (Swaps)
        {
            Swaps = FALSE;
            for (i=0; i<Skel->NBones; i++)
            if (Skel->Bone[i].ParentID != -1)
            {
                // Find Parent
                for (j=0; j<Skel->NBones; j++)
                    if (Skel->Bone[j].BoneID==Skel->Bone[i].ParentID) break;
                ASSERT(j!=Skel->NBones);

                // Check and swap
                if (j>i)
                {
                    skel_bone TempB;
                    Swaps         = TRUE;
                    TempB         = Skel->Bone[i];
                    Skel->Bone[i] = Skel->Bone[j];
                    Skel->Bone[j] = TempB;
                }
            }
        }

        // Re-index ParentIDs
        for (i=0; i<Skel->NBones; i++)
        if (Skel->Bone[i].ParentID != -1)
        {
            // Find Parent
            for (j=0; j<Skel->NBones; j++)
                if (Skel->Bone[j].BoneID==Skel->Bone[i].ParentID) break;
            ASSERT(j!=Skel->NBones);
            Skel->Bone[i].ParentID = j;
        }

        for (i=0; i<Skel->NBones; i++)
            Skel->Bone[i].BoneID = i;
    }

    /////////////////////////////////////////////////////////////
    // Confirm all bone's parents have lower IDs
    /////////////////////////////////////////////////////////////
    {
        s32 i;
        for (i=0; i<Skel->NBones; i++)
        {
            if (Skel->Bone[i].ParentID >= i) 
                goto Error;
        }
    }

    /////////////////////////////////////////////////////////////
    // Slide Parent to bone down one joint
    /////////////////////////////////////////////////////////////
    {
        s32 i;
        vector3d P2B[100];
        for (i=0; i<Skel->NBones; i++)
            P2B[i] = Skel->Bone[i].ParentToBone;
        for (i=1; i<Skel->NBones; i++)
            Skel->Bone[i].ParentToBone = P2B[Skel->Bone[i].ParentID];
    }

    /////////////////////////////////////////////////////////////
    // Compute Global positions of joints
    /////////////////////////////////////////////////////////////
    CalcGlobalPosFromLocal(Skel);

    /////////////////////////////////////////////////////////////
    // Decide mirror bones
    /////////////////////////////////////////////////////////////
    SKEL_DecideMirrorBones(Skel);

    /////////////////////////////////////////////////////////////
    // Return success code
    /////////////////////////////////////////////////////////////

	Success = ERR_SUCCESS ;
Error:
	TKN_CloseFile(&TOK) ;
	return Success ;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//  EXPORT ASF
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

err  SKEL_ExportASF     (skel* Skel, const char* FileName)
{
    return ERR_FAILURE;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//  MATRICES
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void SKEL_BuildWorldOrient  (matrix4*   M, 
                             f32        Scale, 
                             f32        Tx, 
                             f32        Ty, 
                             f32        Tz,
                             radian     Rx, 
                             radian     Ry, 
                             radian     Rz)
{
    vector3d S = {Scale,Scale,Scale};
    radian3d R = {Rx,Ry,Rz};
    vector3d T = {Tx,Ty,Tz};
    ASSERT(M);
    M4_SetupSRT(M,&S,&R,&T);
}

///////////////////////////////////////////////////////////////////////////

void RecursivelyBuildMatrix (skel*      Skel,
                             anim*      Anim,
                             s32        AnimFrame,
                             matrix4*   DstMatrices,
                             s32        BoneID,
                             matrix4*   ParentMatrix)
{
    s32 i;
    matrix4 M;
    radian3d LR;
    vector3d LT;
    // Get local rotation and translation info
    LT   = Skel->Bone[BoneID].ParentToBone;

    if (Anim)
    {
	    LR.X = ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+0 ) ;
	    LR.Y = ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+1 ) ;
	    LR.Z = ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+2 ) ;
    }
    else
    {
        LR.X = LR.Y = LR.Z = 0;
    }

    // Build matrix local to world matrix bone
    M4_Identity         (&M);
    M4_SetRotationsXYZ  (&M,&LR);
    M4_TranslateOn      (&M,&LT);
    M4_PreMultOn        (&M,ParentMatrix);

    // Call children with L2W matrix as parent matrix
    for (i=0; i<Skel->NBones; i++)
    if (Skel->Bone[i].ParentID == BoneID)
    {
        RecursivelyBuildMatrix(Skel,Anim,AnimFrame,DstMatrices,i,&M);
    }

    // Place L2W matrix in destination array 
    DstMatrices[BoneID] = M;
}

///////////////////////////////////////////////////////////////////////////

void RecursivelyBuildMatrixBlend(	skel*		Skel,
									anim*		Anim,
									s32			AnimFrame0,
									s32			AnimFrame1,
									f32			AnimFrameFrac,
									matrix4*	DstMatrices,
									s32			BoneID,
									matrix4*	ParentMatrix)
{
    s32 i;
    matrix4 M;
	radian DR;
    radian3d LR;
    radian3d LR1;
    vector3d LT;

    // Get local rotation and translation info
    LT   = Skel->Bone[BoneID].ParentToBone;

    if (Anim)
    {
	    LR.X = ANIM_GetRotation( Anim, AnimFrame0, (BoneID*3)+0 ) ;
	    LR.Y = ANIM_GetRotation( Anim, AnimFrame0, (BoneID*3)+1 ) ;
	    LR.Z = ANIM_GetRotation( Anim, AnimFrame0, (BoneID*3)+2 ) ;

		LR1.X = ANIM_GetRotation( Anim, AnimFrame1, (BoneID*3)+0 ) ;
	    LR1.Y = ANIM_GetRotation( Anim, AnimFrame1, (BoneID*3)+1 ) ;
	    LR1.Z = ANIM_GetRotation( Anim, AnimFrame1, (BoneID*3)+2 ) ;

		DR = LR1.X-LR.X;
		while (DR < -R_180) DR += R_360;
		while (DR >  R_180) DR -= R_360;
		LR.X = LR.X + DR*AnimFrameFrac;

		DR = LR1.Y-LR.Y;
		while (DR < -R_180) DR += R_360;
		while (DR >  R_180) DR -= R_360;
		LR.Y = LR.Y + DR*AnimFrameFrac;

		DR = LR1.Z-LR.Z;
		while (DR < -R_180) DR += R_360;
		while (DR >  R_180) DR -= R_360;
		LR.Z = LR.Z + DR*AnimFrameFrac;
    }
    else
    {
        LR.X = LR.Y = LR.Z = 0;
    }

    // Build matrix local to world matrix bone
    M4_Identity         (&M);
    M4_SetRotationsXYZ  (&M,&LR);
    M4_TranslateOn      (&M,&LT);
    M4_PreMultOn        (&M,ParentMatrix);

    // Call children with L2W matrix as parent matrix
    for (i=0; i<Skel->NBones; i++)
    if (Skel->Bone[i].ParentID == BoneID)
    {
        RecursivelyBuildMatrixBlend(Skel,Anim,AnimFrame0,AnimFrame1,AnimFrameFrac,DstMatrices,i,&M);
    }

    // Place L2W matrix in destination array 
    DstMatrices[BoneID] = M;
}

///////////////////////////////////////////////////////////////////////////

void RecursivelyBuildMatrixXZY(skel*      Skel,
                             anim*      Anim,
                             s32        AnimFrame,
                             matrix4*   DstMatrices,
                             s32        BoneID,
                             matrix4*   ParentMatrix)
{
    s32 i;
    matrix4 M;
    radian3d LR;
    vector3d LT;
    // Get local rotation and translation info
    LT   = Skel->Bone[BoneID].ParentToBone;

    if (Anim)
    {
	    LR.X = ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+0 ) ;
	    LR.Y = ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+1 ) ;
	    LR.Z = ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+2 ) ;
    }
    else
    {
        LR.X = LR.Y = LR.Z = 0;
    }

    // Build matrix local to world matrix bone
    M4_Identity         (&M);
    M4_SetRotationsXZY  (&M,&LR);
    M4_TranslateOn      (&M,&LT);
    M4_PreMultOn        (&M,ParentMatrix);

    // Call children with L2W matrix as parent matrix
    for (i=0; i<Skel->NBones; i++)
    if (Skel->Bone[i].ParentID == BoneID)
    {
        RecursivelyBuildMatrix(Skel,Anim,AnimFrame,DstMatrices,i,&M);
    }

    // Place L2W matrix in destination array 
    DstMatrices[BoneID] = M;
}

///////////////////////////////////////////////////////////////////////////

void RecursivelyBuildMatrix2 (skel*      Skel,
                              radian3d*  Rotations,
                              matrix4*   DstMatrices,
                              s32        BoneID,
                              matrix4*   ParentMatrix)
{
    s32 i;
    matrix4 M;
    radian3d LR;
    vector3d LT;

    // Get local rotation and translation info
    LT   = Skel->Bone[BoneID].ParentToBone;
    LR.X = Rotations[BoneID].X;
    LR.Y = Rotations[BoneID].Y;
    LR.Z = Rotations[BoneID].Z;

    // Build matrix local to world matrix bone
    M4_Identity         (&M);
    M4_SetRotationsXYZ  (&M,&LR);
    M4_TranslateOn      (&M,&LT);
    M4_PreMultOn        (&M,ParentMatrix);

    // Call children with L2W matrix as parent matrix
    for (i=0; i<Skel->NBones; i++)
    if (Skel->Bone[i].ParentID == BoneID)
    {
        RecursivelyBuildMatrix2(Skel,Rotations,DstMatrices,i,&M);
    }

    // Place L2W matrix in destination array 
    DstMatrices[BoneID] = M;
}

///////////////////////////////////////////////////////////////////////////

void RecursivelyBuildMatrixMir (skel*      Skel,
							    anim*      Anim,
								s32        AnimFrame,
								matrix4*   DstMatrices,
								s32        BoneID,
								matrix4*   ParentMatrix)
{
    s32 i;
    matrix4 M;
    radian3d LR;
    vector3d LT;
    // Get local rotation and translation info
    LT   = Skel->Bone[ Skel->Bone[BoneID].MirrorID ].ParentToBone;

    if (Anim)
    {
	    LR.X =  ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+0 ) ;
	    LR.Y = -ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+1 ) ;
	    LR.Z = -ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+2 ) ;
    }
    else
    {
        LR.X = LR.Y = LR.Z = 0;
    }

    // Build matrix local to world matrix bone
    M4_Identity         (&M);
    M4_SetRotationsXYZ  (&M,&LR);
    M4_TranslateOn      (&M,&LT);
    M4_PreMultOn        (&M,ParentMatrix);

    // Call children with L2W matrix as parent matrix
    for (i=0; i<Skel->NBones; i++)
    if (Skel->Bone[i].ParentID == BoneID)
    {
        RecursivelyBuildMatrixMir(Skel,Anim,AnimFrame,DstMatrices,i,&M);
    }

    // Place L2W matrix in destination array 
    DstMatrices[BoneID] = M;
}

///////////////////////////////////////////////////////////////////////////

void RecursivelyBuildMatrixMir2 (skel*      Skel,
								 radian3d*  Rotations,
								 matrix4*   DstMatrices,
								 s32        BoneID,
								 matrix4*   ParentMatrix)
{
    s32 i;
    matrix4 M;
    radian3d LR;
    vector3d LT;

    // Get local rotation and translation info
    LT   = Skel->Bone[ Skel->Bone[BoneID].MirrorID ].ParentToBone;
    LR.X = Rotations[BoneID].X;
    LR.Y = -Rotations[BoneID].Y;
    LR.Z = -Rotations[BoneID].Z;

    // Build matrix local to world matrix bone
    M4_Identity         (&M);
    M4_SetRotationsXYZ  (&M,&LR);
    M4_TranslateOn      (&M,&LT);
    M4_PreMultOn        (&M,ParentMatrix);

    // Call children with L2W matrix as parent matrix
    for (i=0; i<Skel->NBones; i++)
    if (Skel->Bone[i].ParentID == BoneID)
    {
        RecursivelyBuildMatrixMir2(Skel,Rotations,DstMatrices,i,&M);
    }

    // Place L2W matrix in destination array 
    DstMatrices[BoneID] = M;
}


///////////////////////////////////////////////////////////////////////////
void SKEL_BuildMatrices     (skel*      Skel,
                             anim*      Anim,
                             s32        AnimFrame,
                             matrix4*   DstMatrices,
                             matrix4*   WorldOrientM)
{
    RecursivelyBuildMatrix (Skel,Anim,AnimFrame,DstMatrices,0,WorldOrientM);
}

///////////////////////////////////////////////////////////////////////////
void SKEL_BuildMatricesBlend(skel*      Skel,
                             anim*      Anim,
                             s32        AnimFrame0,
                             s32        AnimFrame1,
                             f32        AnimFrameFrac,
                             matrix4*   DstMatrices,
                             matrix4*   WorldOrientM)
{
	RecursivelyBuildMatrixBlend( Skel, Anim, AnimFrame0, AnimFrame1, AnimFrameFrac, DstMatrices, 0, WorldOrientM );
}

///////////////////////////////////////////////////////////////////////////
void SKEL_BuildMatricesXZY   (skel*      Skel,
                             anim*      Anim,
                             s32        AnimFrame,
                             matrix4*   DstMatrices,
                             matrix4*   WorldOrientM)
{
    RecursivelyBuildMatrixXZY (Skel,Anim,AnimFrame,DstMatrices,0,WorldOrientM);
}

///////////////////////////////////////////////////////////////////////////

void SKEL_BuildMatrices2    (skel*      Skel,
                             radian3d*  Rotations,
                             matrix4*   DstMatrices,
                             matrix4*   WorldOrientM)
{
    RecursivelyBuildMatrix2 (Skel,Rotations,DstMatrices,0,WorldOrientM);
}

///////////////////////////////////////////////////////////////////////////

void SKEL_GetBoneMatrix     (skel*      Skel,
                             anim*      Anim,
                             s32        AnimFrame,
                             matrix4*   DstMatrix,
                             matrix4*   WorldOrientM,
                             s32        BoneID)
{
    matrix4  LM;
    vector3d LT;
    radian3d LR;

    // Clear DstMatrix
    M4_Identity(DstMatrix);

    // Loop from bone to root along parents in skeleton
    while (BoneID != -1)
    {
        // Build local matrix of this bone
        LT   = Skel->Bone[BoneID].ParentToBone;
		LR.X = ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+0 ) ;
		LR.Y = ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+1 ) ;
		LR.Z = ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+2 ) ;
        M4_SetRotationsXYZ  (&LM,&LR);
        M4_TranslateOn      (&LM,&LT);

        // Since this is a parent's matrix Premult onto dest matrix
        M4_PreMultOn(DstMatrix,&LM);

        // Go to next parent
        BoneID = Skel->Bone[BoneID].ParentID;
    }

    // Use WorldOrientM as final parent matrix
    M4_PreMultOn(DstMatrix,WorldOrientM);
}


///////////////////////////////////////////////////////////////////////////

void SKEL_BuildMatricesMir  (skel*      Skel,
                             anim*      Anim,
                             s32        AnimFrame,
                             matrix4*   DstMatrices,
                             matrix4*   WorldOrientM)
{
    RecursivelyBuildMatrixMir (Skel,Anim,AnimFrame,DstMatrices,0,WorldOrientM);
}

///////////////////////////////////////////////////////////////////////////

void SKEL_BuildMatricesMir2 (skel*      Skel,
                             radian3d*  Rotations,
                             matrix4*   DstMatrices,
                             matrix4*   WorldOrientM)
{
    RecursivelyBuildMatrixMir2 (Skel,Rotations,DstMatrices,0,WorldOrientM);
}

///////////////////////////////////////////////////////////////////////////

void SKEL_GetBoneMatrixMir  (skel*      Skel,
                             anim*      Anim,
                             s32        AnimFrame,
                             matrix4*   DstMatrix,
                             matrix4*   WorldOrientM,
                             s32        BoneID)
{
    matrix4  LM;
    vector3d LT;
    radian3d LR;

    // Clear DstMatrix
    M4_Identity(DstMatrix);

    // Loop from bone to root along parents in skeleton
    while (BoneID != -1)
    {
        // Build local matrix of this bone
        LT   = Skel->Bone[BoneID].ParentToBone;
		LR.X =  ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+0 ) ;
		LR.Y = -ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+1 ) ;
		LR.Z = -ANIM_GetRotation( Anim, AnimFrame, (BoneID*3)+2 ) ;
        M4_SetRotationsXYZ  (&LM,&LR);
        M4_TranslateOn      (&LM,&LT);

        // Since this is a parent's matrix Premult onto dest matrix
        M4_PreMultOn(DstMatrix,&LM);

        // Go to next parent
        BoneID = Skel->Bone[ Skel->Bone[BoneID].MirrorID ].ParentID;
    }

    // Use WorldOrientM as final parent matrix
    M4_PreMultOn(DstMatrix,WorldOrientM);
}

///////////////////////////////////////////////////////////////////////////

void SKEL_DecideMirrorBones (skel*      Skel)
{
    s32 i,j;
    char TestName[32];
    char* SrcName;

    ASSERT(Skel);
    for (i=0; i<Skel->NBones; i++)
    {
        SrcName = Skel->Bone[i].BoneName;
        Skel->Bone[i].MirrorID = i;

        for (j=0; j<Skel->NBones; j++)
        if (i!=j)
        {
            x_strcpy(TestName,Skel->Bone[j].BoneName);
            TestName[0] = SrcName[0];

            if (x_strcmp(TestName,SrcName)==0)
            {
                Skel->Bone[i].MirrorID = j;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////
