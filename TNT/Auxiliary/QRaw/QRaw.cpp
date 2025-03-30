///////////////////////////////////////////////////////////////////////////
//
//  QRAW.CPP
//
///////////////////////////////////////////////////////////////////////////

#include "QRaw.hpp"

#include "x_stdio.hpp"
#include "x_debug.hpp"
#include "x_memory.hpp"
#include "x_plus.hpp"
#include "Tokenizer.hpp"

////////////////////////////////////////////////////////////////
// MODULE OPTIONS
////////////////////////////////////////////////////////////////

#define M_ASSERT(A)  ASSERT(A)
#define M_VERBOSE(A) // A

///////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////

//========================================================================
// Simplify3DSMax
//========================================================================
static
void Simplify3DSMax( qraw* QRaw )
{
    qtri*       QT;
    s32         i,j,k;
    f32         U,V;
    qmaterial*  QM;
    f32         S,C,NU,NV;

    // Apply all material attributes to UVs
    for (i=0; i<QRaw->NQTris; i++)
    {
        QT = &(QRaw->QTri[i]);
        QM = &(QRaw->Material[QT->MaterialID]);

        for (j=0; j<3; j++)
        {
            ASSERT((QT->XYZ[j].X <= 30000.0f) && (QT->XYZ[j].X >= -30000.0f));
            ASSERT((QT->XYZ[j].Y <= 30000.0f) && (QT->XYZ[j].Y >= -30000.0f));
            ASSERT((QT->XYZ[j].Z <= 30000.0f) && (QT->XYZ[j].Z >= -30000.0f));

            ///////////////////////////////////////////////////////////////////
            U = QT->UV[j].X;
            V = QT->UV[j].Y;

            U += -0.5f;
            V += -0.5f;

            U += -QM->UOffset;
            V += -QM->VOffset;

            x_sincos(QM->Angle, S, C );
            NU = C*U - S*V;
            NV = C*V + S*U;
            U = NU;
            V = NV;

            U *= QM->UTile;
            V *= QM->VTile;

            U += 0.5f;
            V += 0.5f;

            if (QM->UMirror!=0) U = U*2;
            else                U = U;

            if (QM->VMirror!=0) V = (0.5f - V)*2;
            else                V = 1.0f - V;

            ///////////////////////////////////////////////////////////////////

            QT->UV[j].X = U;
            QT->UV[j].Y = V;
            QT->UV[j].Z = 0.0f;
        }
    }

    // Simplify material attributes
    for (i=0; i<QRaw->NMaterials; i++)
    {
        QM = &QRaw->Material[i];
        QM->UOffset = 0;
        QM->VOffset = 0;
        QM->Angle   = 0;
        QM->UTile   = 1.0f;
        QM->VTile   = 1.0f;
    }

    ///////////////////////////////////////////////////////////////////////
    // REMOVE DUPLICATE MATERIALS
    ///////////////////////////////////////////////////////////////////////
    if (1)
    {
        qmaterial  NewMat[MAX_RAW_MATERIALS];
        s32        NNewMats;
        s32        OldToNew[MAX_RAW_MATERIALS];
        qmaterial* NQM;

        ///////////////////////////////////////////////////////////////////
        // Generate new unduplicated materials and construct remapping list
        ///////////////////////////////////////////////////////////////////
        NNewMats = 0;
        for (i=0; i<QRaw->NMaterials; i++)
        {
            QM = &QRaw->Material[i];
            for (j=0; j<NNewMats; j++)
            {
                NQM = &NewMat[j];
                if (
                    (NQM->UMirror       == QM->UMirror)         &&
                    (NQM->VMirror       == QM->VMirror)         &&
                    (NQM->TextureIndex  == QM->TextureIndex)    &&
                    (NQM->UOffset       == QM->UOffset)         &&
                    (NQM->VOffset       == QM->VOffset)         &&
                    (NQM->Angle         == QM->Angle  )         &&
                    (NQM->UTile         == QM->UTile  )         &&
                    (NQM->VTile         == QM->VTile  )
                   )
                break;
            }

            if (j==NNewMats)
            {
                NewMat[NNewMats] = *QM;
                NNewMats++;
            }

            OldToNew[i] = j;
        }

        ///////////////////////////////////////////////////////////////////
        // Copy new materials back into qraw
        ///////////////////////////////////////////////////////////////////
        QRaw->NMaterials = NNewMats;
        for (i=0; i<NNewMats; i++)
            QRaw->Material[i] = NewMat[i];


        ///////////////////////////////////////////////////////////////////
        // Remap tris to the new materials
        ///////////////////////////////////////////////////////////////////
        for (i=0; i<QRaw->NQTris; i++)
        {
            QT = &(QRaw->QTri[i]);
            QT->MaterialID = OldToNew[QT->MaterialID];
        }
    }

    // Bring all triangles closest to origin
    for (i=0; i<QRaw->NQTris; i++)
    {
        s32 MinU;
        s32 MaxU;
        f32 USlide;
        s32 MinV;
        s32 MaxV;
        f32 VSlide;

        QT = &(QRaw->QTri[i]);
        QM = &(QRaw->Material[QT->MaterialID]);

        ///////////////////////////////////////////////////////////////////
        // find minimum U
        MinU = 0;
        if (QT->UV[1].X < QT->UV[MinU].X) MinU = 1;
        if (QT->UV[2].X < QT->UV[MinU].X) MinU = 2;

        MaxU = 0;
        if (QT->UV[1].X > QT->UV[MinU].X) MaxU = 1;
        if (QT->UV[2].X > QT->UV[MinU].X) MaxU = 2;

        ////////////////////////////////////////
        // find slide factor
        if (QT->UV[MinU].X>0) USlide = -1;
        else                  USlide =  1;

        if (QM->UMirror)      USlide*=2;

        ////////////////////////////////////////
        // slide U values over
        if (USlide > 0)
        {
            while ( QT->UV[MinU].X < 0 )
            {
                QT->UV[0].X += USlide;
                QT->UV[1].X += USlide;
                QT->UV[2].X += USlide;
            }
        }
        else
        {
            while ( (QT->UV[MinU].X >= 1.0f) && (QT->UV[MinU].X+USlide >= 0.0f))
            {
                QT->UV[0].X += USlide;
                QT->UV[1].X += USlide;
                QT->UV[2].X += USlide;
            }
        }

        ///////////////////////////////////////////////////////////////////
        // find minimum V
        MinV = 0;
        if (QT->UV[1].Y < QT->UV[MinV].Y) MinV = 1;
        if (QT->UV[2].Y < QT->UV[MinV].Y) MinV = 2;

        MaxV = 0;
        if (QT->UV[1].Y > QT->UV[MinV].Y) MaxV = 1;
        if (QT->UV[2].Y > QT->UV[MinV].Y) MaxV = 2;

        ////////////////////////////////////////
        // find slide factor
        if (QT->UV[MinV].Y>0) VSlide = -1;
        else                  VSlide =  1;

        if (QM->VMirror)      VSlide*=2;

        ////////////////////////////////////////
        // slide V values over
        if (VSlide > 0)
        {
            while ( QT->UV[MinV].Y < 0 )
            {
                QT->UV[0].Y += VSlide;
                QT->UV[1].Y += VSlide;
                QT->UV[2].Y += VSlide;
            }
        }
        else
        {
            while ( (QT->UV[MinV].Y >= 1.0f) && (QT->UV[MinV].Y+VSlide >= 0.0f))
            {
                QT->UV[0].Y += VSlide;
                QT->UV[1].Y += VSlide;
                QT->UV[2].Y += VSlide;
            }
        }

    }

    // confirm UVs are positive
    for (j=0; j<3; j++)
    {
        ASSERT(QT->UV[j].Y >= 0.0f);
        ASSERT(QT->UV[j].X >= 0.0f);
    }

    // Turn on mirroring if at all possible
    for (i=0; i<QRaw->NMaterials; i++)
    {
        vector3 UVMin = vector3( 1000, 1000,0);
        vector3 UVMax = vector3(-1000,-1000,0);

        QM = &QRaw->Material[i];

        for (j=0; j<QRaw->NQTris; j++)
        if (QRaw->QTri[j].MaterialID==i)
        {
            QT = &QRaw->QTri[j];

            for (k=0; k<3; k++)
            {
                if (QT->UV[k].X > UVMax.X) UVMax.X = QT->UV[k].X;
                if (QT->UV[k].Y > UVMax.Y) UVMax.Y = QT->UV[k].Y;
                if (QT->UV[k].X < UVMin.X) UVMin.X = QT->UV[k].X;
                if (QT->UV[k].Y < UVMin.Y) UVMin.Y = QT->UV[k].Y;
            }
        }
    }
}

//========================================================================
// ReadNumber
//========================================================================
static
f32 ReadNumber(tokenizer* Text)
{
    s32 i;
    f32 f;
    if (Text->TokenType==TOKEN_NUMBER) {f = (f32)Text->Float; i=2;}
    else                               {f = 1.0f;             i=1;}
    TKN_ReadTokens(Text,i);
    return f;
}

//========================================================================
// ReadMangledNumber
//========================================================================
static
f32 ReadMangledNumber(tokenizer* Text)
{
    s32 i,n;
    f32 f;
    if (Text->TokenType==TOKEN_NUMBER) {f = (f32)Text->Float; i=2;}
    else                               {f = 1.0f;             i=1;}

    //-- Now we check for a mangled number.
    //   The old exporter used in ASB2K is exporting -1.#QNB for some of
    //   the R,G,B,U, or V values, and jumping 2 tokens just doesn't work.
    for (n=0;n<i;n++)
    {
        TKN_ReadToken(Text);
        if (Text->TokenType == TOKEN_SYMBOL)
        {
            if ( (x_strcmp(Text->String,"#QNB")==0) ||
                 (x_strcmp(Text->String,"#INF")==0) ||
                 (x_strcmp(Text->String,"#NAN")==0) )
            {
                n--;
                f=0;
            }
        }
    }
    
    //TKN_ReadTokens(Text,i);
    return f;
}

//========================================================================
// QRAW_CleanQRaw
//========================================================================
xbool QRAW_CleanQRaw( qraw* QRaw )
{
    qraw*     QRaw1;
    qraw*     QRaw2;
    s32       i;

    ASSERT( QRaw );

    //---------------------------------------------------------------
    // Set QRaw Pointers
    //---------------------------------------------------------------
    QRaw1 = QRaw;

    QRaw2 = (qraw*)x_malloc(sizeof(qraw));
    if( QRaw2 == NULL ) 
        return FALSE;
    QRAW_InitInstance( QRaw2 );

    ///////////////////////////////////////////////////////////////////////
    // Eliminate unused materials.
    {
        s32 i;

        // Start copying stuff from QRaw1 to QRaw2.

        QRaw2->NMeshes    = QRaw1->NMeshes;
        QRaw2->NTextures  = QRaw1->NTextures;
        x_memcpy( QRaw2->MeshName, QRaw1->MeshName, MAX_RAW_MESHES*32 );
        QRaw2->NMaterials = 0;

        // If a material is used, copy it to QRaw2.
        // Store its new index in the old material's reference count.
        // Store a -1 for unused materials.

        for( i = 0; i < QRaw1->NMaterials; ++i )
        {
            if( QRaw1->Material[i].References > 0 )
            {
                x_memcpy( &(QRaw2->Material[QRaw2->NMaterials]),
                          &(QRaw1->Material[i]) ,
                          sizeof( qmaterial ) );
                QRaw1->Material[i].References = QRaw2->NMaterials;
                QRaw2->NMaterials += 1;
            }
            else
            {
                M_VERBOSE( printf( "Removed unused or duplicated material '%s'\n%s\n",
                    QRaw1->Material[i].MaterialName,
                    QRaw1->Texture[QRaw1->Material[i].TextureIndex].TextureName ) );
                QRaw1->Material[i].References = -1;
            }
        }

        // Hand the tris over to QRaw2.

        QRaw2->NQTris = QRaw1->NQTris;
        QRaw2->QTri   = QRaw1->QTri;

        // Now reindex the tris.

        for( i = 0; i < QRaw2->NQTris; ++i )
        {
            s32 OldIndex = QRaw2->QTri[i].MaterialID;
            s32 NewIndex = QRaw1->Material[OldIndex].References;
            QRaw2->QTri[i].MaterialID = NewIndex;
            ASSERT( NewIndex != -1 );
        }

        //-----------------------------------------------------------------
        // Don't forget to copy texture info
        //-----------------------------------------------------------------
        x_memcpy( QRaw2->Texture, 
                  QRaw1->Texture, 
                  sizeof(qtexture) * MAX_RAW_MATERIALS );

        //-----------------------------------------------------------------
        // Leave the QRaw1 as the prime info.  Make sure that QRaw1 always 
        // have the parameter pointer because we don't know if it is static 
        // data or dynamic.
        //-----------------------------------------------------------------
        {
            qraw T;

             T     = *QRaw1;
            *QRaw1 = *QRaw2;
            *QRaw2 = T;
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // Clean up the texture duplicates.
    {
        s32 NTextures = 0;

        //-------------------------------------
        // Merge the textures
        //-------------------------------------

        for ( i = 0; i < QRaw1->NMaterials; i++)
        {
            qtexture*   Tex;
            s32         j;

            // Get the texture for this material
            Tex = &QRaw1->Texture[ QRaw1->Material[i].TextureIndex ];

            // Search for the texture in our temporary buffer
            for ( j = 0; j < NTextures; j++ )
            {
                if( x_stricmp( Tex->TextureName, QRaw2->Texture[j].TextureName ) == 0 ) 
                    break;
            }

            // Fix up the material reference
            QRaw1->Material[i].TextureIndex = j;

            // Check whether we found it
            if ( j == NTextures )
            {
                // Copy the data
                QRaw2->Texture[j] = *Tex;

                // Increase the Texture Count
                NTextures++;
            }
        }

        //----------------------------------
        // Update QRaw1 Texture Info
        //----------------------------------
        QRaw1->NTextures = NTextures;
        x_memcpy( QRaw1->Texture, QRaw2->Texture, NTextures * sizeof(qtexture) );
    }

    //-------------------------------------------------
    // Clean after our selves
    //-------------------------------------------------

    // Delete the QRaw2.
    x_free( QRaw2 );

    // Reset the the Mesh Flags
/*
    for( i = 0; i < QRaw1->NMeshes; ++i )
    {
        QRaw1->MeshFlags[i] = 0;
    }
*/
    return TRUE;
}

//========================================================================
// QRAW_LoadQRaw
//========================================================================
xbool QRAW_LoadQRaw( qraw* QRaw, char* FileName )
{
    tokenizer Text;
    s32       i;

    ASSERT( FileName );
    ASSERT( QRaw     );

    ///////////////////////////////////////////////////////////////////////
    // Clear the qraw
    QRAW_InitInstance( QRaw );

    ///////////////////////////////////////////////////////////////////////
    // Open text file as input
    TKN_Clear( &Text );
    if( TKN_ReadFile( &Text, FileName ) == FALSE ) 
        return FALSE;
 
    ///////////////////////////////////////////////////////////////////////
    // First pass, gather material and mesh info
    {
        TKN_Rewind( &Text );
        TKN_ReadToken( &Text );
        while( Text.TokenType != TOKEN_EOF )
        {
            ASSERT( Text.TokenType == TOKEN_DELIMITER );

            ///////////////////////////////////////////////////////////////
            // If it's a facet, gather mesh name
            if (Text.Delimiter == '[')
            {
                QRaw->NQTris++;

                while (Text.TokenType != TOKEN_STRING)
                    TKN_ReadToken(&Text);

                // See if it's a new mesh
                for (i=0; i<QRaw->NMeshes; i++)
                    if (x_stricmp(QRaw->MeshName[i],Text.String)==0)
                        break;

                if (i==QRaw->NMeshes)
                {
                    x_strcpy(QRaw->MeshName[i],Text.String);
                    QRaw->NMeshes++;
                }

                while (Text.Delimiter != ']')
                    TKN_ReadToken(&Text);
            }

            ///////////////////////////////////////////////////////////////
            // If it's a material, gather info
            //;:==W==:==H==:=UMIRROR=:=VMIRROR=:=UTILE=:=VTILE=:=UOFFSET=:=VOFFSET=:=ROTATION=:===MATERIAL===:=============TEX MAP================:
            //(   64,   64,       0,        0,    1.00,    1.00,   0.00,     0.00,      0   ,  " # 45",  "\\BIG-IGUANA\USER\PROJECTS\BASEBALL\art\n64\stadiums\comiskey\field\cp_fbox4.gif"   )
            if( Text.Delimiter == '(' )
            {
                f32 UMirror, VMirror, UTile, VTile;

                //----------------------------------------------
                // Set the textures info
                //----------------------------------------------

                TKN_ReadToken(&Text);
                QRaw->Texture[ QRaw->NTextures ].TextureWidth = Text.Int;

                TKN_ReadTokens(&Text,2);
                QRaw->Texture[ QRaw->NTextures ].TextureHeight = Text.Int;

                //----------------------------------------------
                // Set the material info
                //----------------------------------------------

                QRaw->Material[QRaw->NMaterials].References   = 0;
                QRaw->Material[QRaw->NMaterials].TextureIndex = QRaw->NTextures;

                // Handle mirror and tile info
                TKN_ReadTokens(&Text,2);
                UMirror = (f32)Text.Float;

                TKN_ReadTokens(&Text,2);
                VMirror = (f32)Text.Float;

                TKN_ReadTokens(&Text,2);
                UTile   = (f32)Text.Float;

                TKN_ReadTokens(&Text,2);
                VTile   = (f32)Text.Float;

                if (ABS(UMirror) > ABS(UTile))
                {
                    QRaw->Material[QRaw->NMaterials].UTile   = ABS(UMirror);
                    QRaw->Material[QRaw->NMaterials].UMirror = 1;
                }
                else
                {
                    QRaw->Material[QRaw->NMaterials].UTile   = ABS(UTile);
                    QRaw->Material[QRaw->NMaterials].UMirror = 0;
                }

                if (ABS(VMirror) > ABS(VTile)) 
                {
                    QRaw->Material[QRaw->NMaterials].VTile   = ABS(VMirror);
                    QRaw->Material[QRaw->NMaterials].VMirror = 1;
                }
                else
                {
                    QRaw->Material[QRaw->NMaterials].VTile   = ABS(VTile);
                    QRaw->Material[QRaw->NMaterials].VMirror = 0;
                }

                TKN_ReadTokens(&Text,2);
                QRaw->Material[QRaw->NMaterials].UOffset = (f32)Text.Float;

                TKN_ReadTokens(&Text,2);
                QRaw->Material[QRaw->NMaterials].VOffset = (f32)Text.Float;

                TKN_ReadTokens(&Text,2);
                QRaw->Material[QRaw->NMaterials].Angle = DEG_TO_RAD((f32)Text.Float);

                TKN_ReadTokens(&Text,2);
                x_strncpy(QRaw->Material[QRaw->NMaterials].MaterialName,Text.String,128);

                //----------------------------------------------
                // Set the texture name
                //----------------------------------------------
                TKN_ReadTokens(&Text,2);
                x_strncpy( QRaw->Texture[QRaw->NTextures].TextureName, Text.String,128);

                //----------------------------------------------
                // Increment the counters
                //----------------------------------------------
                QRaw->NMaterials++;
                QRaw->NTextures++;

                while (Text.Delimiter != ')')
                    TKN_ReadToken(&Text);
            }

            TKN_ReadToken(&Text);
        }
        TKN_Rewind(&Text);
    }

    ///////////////////////////////////////////////////////////////////////
    // Second pass, gather facet info
    {
        s32 i,j;
        qtri* QT;

        QRaw->QTri = (qtri*)x_malloc(sizeof(qtri)*QRaw->NQTris);
        ASSERT(QRaw->QTri);

        i = 0;
        TKN_Rewind(&Text);
        TKN_ReadToken(&Text);
        while (Text.TokenType != TOKEN_EOF)
        {
            if ((Text.TokenType == TOKEN_DELIMITER) && (Text.Delimiter == '['))
            {
                QT = &(QRaw->QTri[i]);
                i++;

                TKN_ReadToken(&Text);

                QT->XYZ[0].X    = ReadMangledNumber(&Text);
                QT->XYZ[0].Y    = ReadMangledNumber(&Text);
                QT->XYZ[0].Z    = ReadMangledNumber(&Text);
                QT->UV[0].X     = ReadMangledNumber(&Text);
                QT->UV[0].Y     = ReadMangledNumber(&Text);
                QT->Normal[0].X = ReadMangledNumber(&Text);
                QT->Normal[0].Y = ReadMangledNumber(&Text);
                QT->Normal[0].Z = ReadMangledNumber(&Text);
                QT->Color[0].X  = ReadMangledNumber(&Text);
                QT->Color[0].Y  = ReadMangledNumber(&Text);
                QT->Color[0].Z  = ReadMangledNumber(&Text);
                QT->Alpha[0]    = ReadMangledNumber(&Text);
                QT->MatrixID[0] = (s32)ReadMangledNumber(&Text);

                // Find Mesh
                ASSERT(Text.TokenType == TOKEN_STRING);
                for (j=0; j<QRaw->NMeshes; j++)
                    if (x_stricmp(QRaw->MeshName[j],Text.String)==0) 
                        break;
                ASSERT(j<QRaw->NMeshes);
                QT->MeshID = j;
                TKN_ReadTokens(&Text,2);

                QT->XYZ[1].X    = ReadMangledNumber(&Text);
                QT->XYZ[1].Y    = ReadMangledNumber(&Text);
                QT->XYZ[1].Z    = ReadMangledNumber(&Text);
                QT->UV[1].X     = ReadMangledNumber(&Text);
                QT->UV[1].Y     = ReadMangledNumber(&Text);
                QT->Normal[1].X = ReadMangledNumber(&Text);
                QT->Normal[1].Y = ReadMangledNumber(&Text);
                QT->Normal[1].Z = ReadMangledNumber(&Text);
                QT->Color[1].X  = ReadMangledNumber(&Text);
                QT->Color[1].Y  = ReadMangledNumber(&Text);
                QT->Color[1].Z  = ReadMangledNumber(&Text);
                QT->Alpha[1]    = ReadMangledNumber(&Text);
                QT->MatrixID[1] = (s32)ReadMangledNumber(&Text);
                
                // Find Material
                ASSERT(Text.TokenType == TOKEN_STRING);
                for (j=0; j<QRaw->NMaterials; j++)
                    if (x_stricmp(QRaw->Material[j].MaterialName,Text.String)==0) 
                        break;
                ASSERT(j<QRaw->NMaterials);
                QRaw->Material[j].References++;     // Material is referenced!
                QT->MaterialID = j;
                TKN_ReadTokens(&Text,2);

                QT->XYZ[2].X    = ReadMangledNumber(&Text);
                QT->XYZ[2].Y    = ReadMangledNumber(&Text);
                QT->XYZ[2].Z    = ReadMangledNumber(&Text);
                QT->UV[2].X     = ReadMangledNumber(&Text);
                QT->UV[2].Y     = ReadMangledNumber(&Text);
                QT->Normal[2].X = ReadMangledNumber(&Text);
                QT->Normal[2].Y = ReadMangledNumber(&Text);
                QT->Normal[2].Z = ReadMangledNumber(&Text);
                QT->Color[2].X  = ReadMangledNumber(&Text);
                QT->Color[2].Y  = ReadMangledNumber(&Text);
                QT->Color[2].Z  = ReadMangledNumber(&Text);
                QT->Alpha[2]    = ReadMangledNumber(&Text);

                if ( Text.TokenType == TOKEN_DELIMITER )
                {
                    QT->MatrixID[2] = 0;
                    break;
                }

                QT->MatrixID[2] = Text.Int;

                TKN_ReadTokens(&Text,1);

                ASSERT((Text.TokenType==TOKEN_DELIMITER) && (Text.Delimiter==']'));

                ///////////////////////////////////////////////////////////
                // IF IT IS A COLLAPSED FACET ROLL BACK ONE TRIANGLE
                if (1)
                {
                    f32 D0,D1,D2,T;

                    D0 = (QT->XYZ[0]-QT->XYZ[1]).Length();
                    D1 = (QT->XYZ[1]-QT->XYZ[2]).Length();
                    D2 = (QT->XYZ[2]-QT->XYZ[0]).Length();

                    if (D2 > D0) {T=D2; D2=D0; D0=T;}
                    if (D1 > D0) {T=D1; D1=D0; D0=T;}
                    if (D2 > D1) {T=D1; D1=D2; D2=T;}
                    
                    if (D0 < 0.0001) T = 0.0;
                    else             T = (D1+D2)/D0;

                    if (T < 1.0001) 
                    {
                        i--;
                        QRaw->NQTris--;
                    }
                }
            }

            TKN_ReadToken(&Text);
        }       
    }

    //---------------------------------------------------------------
    // Close the text file
    //---------------------------------------------------------------
    TKN_CloseFile( &Text );

    //---------------------------------------------------------------
    // Clean up the qraw before returning
    //---------------------------------------------------------------

    // Simplify the UVs
    Simplify3DSMax( QRaw );

    // Clean up the Meshes
    QRAW_CleanQRaw( QRaw );

    // Do integrity check
    return QRAW_CheckIntegrity( QRaw );
}

//========================================================================
// QRAW_WriteQRaw
//========================================================================
xbool QRAW_WriteQRaw( qraw* QRaw, char* FileName )
{
    X_FILE* fh;
    s32 i;
    qtri* QT;

    ASSERT(FileName);
    ASSERT(QRaw);

    fh = x_fopen(FileName,"wt");
    if ( fh == NULL ) 
        return FALSE;

    x_fprintf(fh,"\n");
    x_fprintf(fh,"; Text geometry output from QRAW structures\n");
    x_fprintf(fh,"; =========================================\n");
    x_fprintf(fh,";  %16d : Meshes.\n",QRaw->NMeshes);
    x_fprintf(fh,";  %16d : Triangles.\n",QRaw->NQTris);
    x_fprintf(fh,";  %16d : Materials.\n",QRaw->NMaterials);
    x_fprintf(fh,"\n\n");

    for (i=0; i<QRaw->NQTris; i++)
    {
        QT = &(QRaw->QTri[i]);

        if (!( i & ((1<<3)-1) ))
        {
            x_fprintf(fh,";");
            x_fprintf(fh,":====X=====:====Y=====:====Z=====:====U====:====V====");
            x_fprintf(fh,":====Nx====:====Ny====:====Nz====");
            x_fprintf(fh,":===R===:===G===:===B===:===A===");
            x_fprintf(fh,":==M==");
            x_fprintf(fh,":=====NAME======:\n");
        }

        // Save Vertex 0
        x_fprintf(fh,"[ %10.3f,%10.3f,%10.3f,", QT->XYZ[0].X, QT->XYZ[0].Y, QT->XYZ[0].Z );
        x_fprintf(fh,"%9.4f,%9.4f,", QT->UV[0].X, QT->UV[0].Y );
        x_fprintf(fh,"%10f,%10f,%10f,", QT->Normal[0].X, QT->Normal[0].Y, QT->Normal[0].Z );
        x_fprintf(fh,"%7.3f,%7.3f,%7.3f,", QT->Color[0].X, QT->Color[0].Y, QT->Color[0].Z);
        x_fprintf(fh,"%7.3f,",QT->Alpha[0]);
        x_fprintf(fh,"%5d,",QT->MatrixID[0]);

        x_fprintf(fh," %1c%s%1c,\n",'"',QRaw->MeshName[QT->MeshID],'"');

        // Save Vertex 1
        x_fprintf(fh,"  %10.3f,%10.3f,%10.3f,", QT->XYZ[1].X, QT->XYZ[1].Y, QT->XYZ[1].Z );
        x_fprintf(fh,"%9.4f,%9.4f,", QT->UV[1].X, QT->UV[1].Y );
        x_fprintf(fh,"%10f,%10f,%10f,", QT->Normal[1].X, QT->Normal[1].Y, QT->Normal[1].Z );
        x_fprintf(fh,"%7.3f,%7.3f,%7.3f,", QT->Color[1].X, QT->Color[1].Y, QT->Color[1].Z);
        x_fprintf(fh,"%7.3f,",QT->Alpha[1]);
        x_fprintf(fh,"%5d,",QT->MatrixID[1]);

        x_fprintf(fh," %1c%s%1c,\n",'"',QRaw->Material[QT->MaterialID].MaterialName,'"');

        // Save Vertex 2
        x_fprintf(fh,"  %10.3f,%10.3f,%10.3f,", QT->XYZ[2].X, QT->XYZ[2].Y, QT->XYZ[2].Z );
        x_fprintf(fh,"%9.4f,%9.4f,", QT->UV[2].X, QT->UV[2].Y );
        x_fprintf(fh,"%10f,%10f,%10f,", QT->Normal[2].X, QT->Normal[2].Y, QT->Normal[2].Z );
        x_fprintf(fh,"%7.3f,%7.3f,%7.3f,", QT->Color[2].X, QT->Color[2].Y, QT->Color[2].Z);
        x_fprintf(fh,"%7.3f,",QT->Alpha[2]);
        x_fprintf(fh,"%5d ",QT->MatrixID[2]);

        x_fprintf(fh," ] \n\n");
    }

    for (i=0; i<QRaw->NMaterials; i++)
    {
        if (!( i & ((1<<6)-1) ))
        {
            x_fprintf(fh,";");
            x_fprintf(fh,":==W==:==H==:=UMIRROR=:=VMIRROR=:=UTILE=:=VTILE=:=UOFFSET=:=VOFFSET=:=ROTATION=:============MATERIAL============:====TEX MAP====:\n");
        }

        x_fprintf(fh,"( %5d,%5d,",QRaw->Texture[QRaw->Material[i].TextureIndex].TextureWidth,
                                QRaw->Texture[QRaw->Material[i].TextureIndex].TextureHeight);

        {
            f32 UTile,UMirror;
            f32 VTile,VMirror;

            if (QRaw->Material[i].UMirror>0) {UTile = 0; UMirror = QRaw->Material[i].UTile;}
            else                             {UTile = QRaw->Material[i].UTile; UMirror = 0;}   

            if (QRaw->Material[i].VMirror>0) {VTile = 0; VMirror = QRaw->Material[i].VTile;}
            else                             {VTile = QRaw->Material[i].VTile; VMirror = 0;}   

            x_fprintf(fh,"%9.2f,%9.2f,",UMirror,VMirror);
            x_fprintf(fh,"%7.2f,%7.2f,",UTile,VTile);
        }

        x_fprintf(fh,"%9.2f,%9.2f,",QRaw->Material[i].UOffset,QRaw->Material[i].VOffset);
        x_fprintf(fh,"%10.0f,",RAD_TO_DEG(QRaw->Material[i].Angle));
        x_fprintf(fh," %1c%s%1c, %1c%s%1c )\n",'"', QRaw->Material[i].MaterialName,'"','"',
                                                  QRaw->Texture[QRaw->Material[i].TextureIndex].TextureName,'"');
    }              

    x_fclose(fh);

    return TRUE;
}

//=========================================================================
// QRAW_InitInstance
//=========================================================================
void QRAW_InitInstance( qraw* QRaw )
{
    // Clear the memory to zero
    x_memset( QRaw, 0, sizeof(qraw) );
}

//=========================================================================
// QRAW_KillInstance
//-------------------------------------------------------------------------
// Kill Instance Will not free the QRaw structure. Allocation and deallocation
// of the structure are done aleays outside modules. 
//=========================================================================
void  QRAW_KillInstance( qraw* QRaw )
{
    ASSERT(QRaw);
    if ( QRaw->QTri ) x_free(QRaw->QTri);

    QRAW_InitInstance( QRaw );
}

//=========================================================================
// QRAW_CreateCullMaps
//=========================================================================
void QRAW_CreateCullMaps( qraw* QRaw, s16** Offset, byte** Data, s32* DataSize )
{
/*
    s32     i, j;
    s32     Size;
    byte*   Map;
    byte*   Next;

    printf( "Creating Culling Maps" );

    //---------------------------------------------------------------------
    // Allocate space for the cull map data and offset list.
    // Use a worst case amount for data.  (Maps are 64x64.)
    //---------------------------------------------------------------------

    *Data   = (byte*)x_malloc( QRaw->NMeshes * 768 );
    *Offset = (s16*) x_malloc( QRaw->NMeshes *   2 );

    //---------------------------------------------------------------------
    // Assume the size is 0, then build up.
    //---------------------------------------------------------------------

    *DataSize = 0;
    Next      = *Data;

    //---------------------------------------------------------------------
    // Allocate a local uncompressed cull map.
    //---------------------------------------------------------------------

    Map = (byte*)x_malloc( 512 );            

    //---------------------------------------------------------------------
    // Loop thru each mesh and build the cull map data.
    //---------------------------------------------------------------------
    for( i = 0; i < QRaw->NMeshes; ++i )
    {
        TMASK_Init( Map );

        (*Offset)[i] = (s16)(Next - (*Data));

        for( j = 0; j < QRaw->NQTris; ++j )
        {
            if( QRaw->QTri[j].MeshID == i )
            {
                TMASK_AddTri( QRaw->QTri[j].XYZ[0].Z, QRaw->QTri[j].XYZ[0].X, 
                              QRaw->QTri[j].XYZ[1].Z, QRaw->QTri[j].XYZ[1].X, 
                              QRaw->QTri[j].XYZ[2].Z, QRaw->QTri[j].XYZ[2].X, 
                              TRUE );
            }
        }

        Size         = TMASK_Compress( Next, Map );
        Next        += Size;
        (*DataSize) += (s16)Size;

        printf( "." );

        TMASK_Kill();
    }

    printf( "\n" );
    printf( "Total storage for Culling Maps:  %4d\n", (*DataSize) );
    printf( "Average storage per Culling Map: %4d\n", (*DataSize) / QRaw->NMeshes );

    //---------------------------------------------------------------------
    // Resize the allocation.
    //---------------------------------------------------------------------

    x_realloc( *Data, *DataSize );

    //---------------------------------------------------------------------
    // Lose the temporary storage.
    //---------------------------------------------------------------------
    
    x_free( Map );
*/
}

//=========================================================================
// QRAW_CheckIntegrity
//=========================================================================
xbool QRAW_CheckIntegrity( qraw* QRaw )
{
    s32 i;
    xbool MeshID[MAX_RAW_MESHES];

    //--------------------------------------------------------------------
    // Make sure that we have the memory Okay
    //--------------------------------------------------------------------
    ASSERT( !x_MemSanityCheck() );

    //--------------------------------------------------------------------
    // Check for basic parameters
    //--------------------------------------------------------------------

    if ( ( QRaw->NMeshes < 0 ) || ( QRaw->NMeshes > MAX_RAW_MESHES ) )
    {
        ASSERTS( 0, "QRAW_CheckIntegrity: Uninitialized structure or bad Num of Meshes" );
        return FALSE;
    }

    if ( ( QRaw->NQTris < 0 ) || ( QRaw->NQTris > MAX_RAW_MESHES * 1000 ) )
    {
        ASSERTS( 0, "QRAW_CheckIntegrity: Possible corruption in NQTris" );
        return FALSE;
    }

    //--------------------------------------------------------------------
    // Check Using the MeshName
    //--------------------------------------------------------------------
    for ( i = 0; i < QRaw->NMeshes; i++ )
    {
        s32 Len;
        s32 j;

        //--------------------------------------------------------------------
        // Check For the Length of the string
        //--------------------------------------------------------------------
        Len = x_strlen( QRaw->MeshName[i] );

        if ( ( Len <= 0 ) || ( Len > 32 ) )
        {
            ASSERTS( 0, "QRAW_CheckIntegrity: We don't have valid names for the meshes." );
            return FALSE;
        }

        //--------------------------------------------------------------------
        // Check For the Length of the string
        //--------------------------------------------------------------------
        for ( j = i + 1; j < QRaw->NMeshes; j++ )
        {
            if ( x_stricmp( QRaw->MeshName[i], QRaw->MeshName[j] ) == 0 )
            {
                ASSERTS( 0, "QRAW_CheckIntegrity: We have two meshes name the same." );
                return FALSE;
            }
        }
    }

    //--------------------------------------------------------------------
    // Make sure that all the triangles point to a corect Mesh
    //--------------------------------------------------------------------

    // Clear the MEshID table
    x_memset ( MeshID, 0, sizeof( xbool ) * MAX_RAW_MESHES );

    // Make sure that all the ids fall in the right places
    for ( i = 0 ; i < QRaw->NQTris; i++ )
    {
        if ( ( QRaw->QTri[i].MeshID > QRaw->NMeshes ) || ( QRaw->QTri[i].MeshID < 0 ) )
        {
            ASSERTS( 0, "QRAW_CheckIntegrity: A face had a wrog ID for a Mesh" );
            return FALSE;
        }

        // Mark as used
        MeshID[ QRaw->QTri[i].MeshID ] = TRUE;
    }

    //--------------------------------------------------------------------
    // Check For gaps
    //--------------------------------------------------------------------
    for ( i = 0 ; i < QRaw->NMeshes; i++ )
    {
        if ( MeshID[i] != TRUE )
        {
            ASSERTS( 0, "QRAW_CheckIntegrity: It seems that there is a mesh with not faces" );
            return FALSE;
        }
    }

    //--------------------------------------------------------------------
    // Double check for bad ids
    //--------------------------------------------------------------------
    for ( ; i < MAX_RAW_MESHES; i++ )
    {
        if ( MeshID[i] )
        {
            ASSERTS( 0, "QRAW_CheckIntegrity: Facets are pointing to ids which are out of the range " );
            return FALSE;
        }
    }

    //--------------------------------------------------------------------
    // Check that all names have been upper cased
    //--------------------------------------------------------------------
    for ( i = 0 ; i < QRaw->NMeshes; i++ )
    {
        char UpperMeshName[256];
        x_strcpy(UpperMeshName,QRaw->MeshName[i]);
        x_strtoupper(UpperMeshName);
        if (x_strcmp(QRaw->MeshName[i],UpperMeshName)!=0) 
        {
            ASSERTS( 0, "QRAW_CheckIntegrity: A mesh name is not case insensitive." );
            return FALSE;
        }
    }
    for ( i = 0 ; i < QRaw->NTextures; i++ )
    {
        char UpperTextureName[256];
        x_strcpy(UpperTextureName,QRaw->Texture[i].TextureName);
        x_strtoupper(UpperTextureName);
        if (x_strcmp(QRaw->Texture[i].TextureName,UpperTextureName)!=0) 
        {
            ASSERTS( 0, "QRAW_CheckIntegrity: A texture name is not case insensitive." );
            return FALSE;
        }
    }
    for ( i = 0 ; i < QRaw->NMaterials; i++ )
    {
        char UpperMaterialName[256];
        x_strcpy(UpperMaterialName,QRaw->Material[i].MaterialName);
        x_strtoupper(UpperMaterialName);
        if (x_strcmp(QRaw->Material[i].MaterialName,UpperMaterialName)!=0) 
        {
            ASSERTS( 0, "QRAW_CheckIntegrity: A material name is not case insensitive." );
            return FALSE;
        }
    }

    //--------------------------------------------------------------------
    // Check that textures have decent values
    //--------------------------------------------------------------------
    for ( i = 0 ; i < QRaw->NTextures; i++ )
    {
        if ((QRaw->Texture[i].TextureWidth<=0) ||
            (QRaw->Texture[i].TextureWidth>4096) ||
            (QRaw->Texture[i].TextureHeight<=0) ||
            (QRaw->Texture[i].TextureHeight>4096))
        {
            ASSERTS( 0, fs("QRAW_CheckIntegrity: Texture '%s' has bad Width, Height: (%1d,%1d)",
                        QRaw->Texture[i].TextureName,
                        QRaw->Texture[i].TextureWidth,
                        QRaw->Texture[i].TextureHeight) );
            return FALSE;
        }
    }



    return TRUE;
}

//=========================================================================
// QRAW_GetMeshIndex
//-------------------------------------------------------------------------
// DestIndex: Can be NULL. If that is the case, it will return success or
//            failure depending whether the Mesh is found.
//=========================================================================
xbool QRAW_GetMeshIndex( qraw* QRaw, s32* DestIndex, char* MeshName )
{
    s32 i;

    ASSERT( QRaw     );
    ASSERT( MeshName );

    //---------------------------------------------------------------------
    // Find which ID is he using
    //---------------------------------------------------------------------
    for ( i = 0; i < QRaw->NMeshes; i++ )
    {
        if ( x_stricmp( MeshName, QRaw->MeshName[i]) == 0 )
            break;
    }

    // We didn't find anything
    if ( i == QRaw->NMeshes ) 
        return FALSE;

    // Set the answer and return
    if ( DestIndex )
        *DestIndex = i;

    return TRUE;
}

//=========================================================================
// QRAW_DeleteMesh
//=========================================================================
xbool QRAW_DeleteMesh( qraw* QRaw, char* MeshName )
{
    s32 i, j;
    s32 ID;

    ASSERT( QRaw     );
    ASSERT( MeshName );

    //---------------------------------------------------------------------
    // Find which ID of the mesh
    //---------------------------------------------------------------------
    if ( QRAW_GetMeshIndex( QRaw, &ID, MeshName ) ) 
        return FALSE;

    //---------------------------------------------------------------------
    // Delete all the qtris which use that id
    //---------------------------------------------------------------------
    for ( j = i = 0; i < QRaw->NQTris; i++ )
    {
        // Copy the triangle
        QRaw->QTri[j] = QRaw->QTri[i];

        // Keep facet if it doesn't have the same ID
        if ( QRaw->QTri[j].MeshID != ID ) 
        {
            // Reindex triangle
            if ( QRaw->QTri[j].MeshID > ID )
                QRaw->QTri[j].MeshID--;

            j++;
        }
    }

    //---------------------------------------------------------------------
    // Reindex All the Meshes Names
    //---------------------------------------------------------------------
    for ( i = ID; i < (QRaw->NMeshes-1); i++ )
    {
        x_strcpy( QRaw->MeshName[i], QRaw->MeshName[i+1] );
    }
    x_memset( QRaw->MeshName[QRaw->NMeshes-1], 0, 32 );

    //---------------------------------------------------------------------
    // FixUp the QRaw Struct
    //---------------------------------------------------------------------
    {
        qtri* T;

        // Make sure that we have the right amount of triangles
        QRaw->NQTris = j;

        // Make sure that we have the right amount of meshes
        QRaw->NMeshes--;

        // Make sure that we don't waste memory
        T = (qtri*)x_realloc( QRaw->QTri, sizeof(qtri) * QRaw->NQTris );
        if ( T == NULL ) 
            return FALSE;

        QRaw->QTri = T;
    }

    return QRAW_CheckIntegrity( QRaw );
}

//=========================================================================
// QRAW_CopyMesh
//=========================================================================
xbool QRAW_CopyMesh( qraw* DestQRaw, char* DestMeshName, qraw* SrcQRaw, char* SrcMeshName )
{
    s32       i, j, k;
    s32       SID, DID;
    s32       NSTri, NMat, NTex;
    qmaterial Material[MAX_RAW_MATERIALS];
    qtexture  Texture[MAX_RAW_MATERIALS];
    s32       NewMaterialIndex[MAX_RAW_MATERIALS];
    s32       NewTextureIndex[MAX_RAW_MATERIALS];
    char      DestMeshNameUpper[256];

    ASSERT( DestQRaw     );
    ASSERT( DestMeshName );
    ASSERT( SrcQRaw     );
    ASSERT( SrcMeshName );
    ASSERT((DestQRaw->NMeshes >= 0) && (DestQRaw->NMeshes<1000));

    //---------------------------------------------------------------------
    // Clear the Material and Textures Tables
    //---------------------------------------------------------------------
    x_memset( Material, 0, sizeof(qmaterial) * MAX_RAW_MATERIALS );
    x_memset( Texture,  0, sizeof(qtexture)  * MAX_RAW_MATERIALS );

    x_memset( NewMaterialIndex, -1, sizeof(s32) * MAX_RAW_MATERIALS );
    x_memset( NewTextureIndex,  -1, sizeof(s32) * MAX_RAW_MATERIALS );

    x_strcpy( DestMeshNameUpper, DestMeshName );
    x_strtoupper( DestMeshNameUpper );

    //---------------------------------------------------------------------
    // Check Name colision with destination
    //---------------------------------------------------------------------
    if ( QRAW_GetMeshIndex( DestQRaw, NULL, DestMeshNameUpper ) == TRUE )
        return FALSE;

    //---------------------------------------------------------------------
    // Find meshes IDs
    //---------------------------------------------------------------------
    if ( QRAW_GetMeshIndex( SrcQRaw, &SID, SrcMeshName ) ) 
        return FALSE;

    DID = DestQRaw->NMeshes;

    //---------------------------------------------------------------------
    // Find how many triangles and materials and textures we are going to copy
    //---------------------------------------------------------------------
    for ( NTex = NMat = NSTri = i = 0 ; i < SrcQRaw->NQTris; i++ )
    {
        if ( SrcQRaw->QTri[i].MeshID == SID ) 
        {
            s32 MaterialIndex;

            // get the material index
            MaterialIndex = SrcQRaw->QTri[i].MaterialID;

            // Do we copy this material already?
            if ( NewMaterialIndex[ MaterialIndex ] == -1 )
            {
                s32 TextureIndex;

                //------------------------------------------------------------
                // Fill the material tables
                //------------------------------------------------------------

                // Copy the material into our table
                Material[ MaterialIndex ] = SrcQRaw->Material[ MaterialIndex ];

                // Initialize the Number of references
                Material[ MaterialIndex ].References = 1;

                // Set which ID this material will have
                ASSERT( (DestQRaw->NMaterials + NMat) < MAX_RAW_MATERIALS );
                NewMaterialIndex[ MaterialIndex ] = DestQRaw->NMaterials + NMat;

                // Increment amount of new ametrials
                NMat++;
                
                //------------------------------------------------------------
                // Fill the texture tables
                //------------------------------------------------------------

                // Get the texture index
                TextureIndex = Material[ MaterialIndex ].TextureIndex;

                // Do we got a new texture
                if (  NewTextureIndex[ TextureIndex ] == -1 )
                {                    
                    // Copy the texture into the Table
                    Texture[ TextureIndex ] = SrcQRaw->Texture[ TextureIndex ];

                    // Set the New Texture Index
                    ASSERT( (SrcQRaw->NTextures + NTex) < MAX_RAW_MATERIALS );
                    NewTextureIndex[ TextureIndex ] = DestQRaw->NTextures + NTex;

                    // Increment the count of new textures
                    NTex++;
                }
            }
            else
            {
                Material[ SrcQRaw->QTri[i].MaterialID ].References++;
            }

            NSTri++;
        }
    }

    //---------------------------------------------------------------------
    // Try to make the buffer grow or if it is a new qraw malloc it
    //---------------------------------------------------------------------
    if ( DestQRaw->QTri == 0 )
    {
        qtri* T;

        // Make sure that we don't waste memory
        T = (qtri*)x_malloc( sizeof(qtri) * NSTri );
        if ( T == NULL ) return FALSE;

        DestQRaw->QTri = T;
    }
    else
    {
        qtri* T;

        // Make sure that we don't waste memory
        T = (qtri*)x_realloc( DestQRaw->QTri, sizeof(qtri) * ( DestQRaw->NQTris + NSTri ) );
        if ( T == NULL ) return FALSE;

        DestQRaw->QTri = T;
    }

    //---------------------------------------------------------------------
    // Copy all the new materials and textures into the qraw
    //---------------------------------------------------------------------
    for ( i = 0; i < MAX_RAW_MATERIALS; i++ )
    {
        // Copy the new Material
        if ( NewMaterialIndex[i] != -1 )
        {
            // Reindex to the texture
            Material[ i ].TextureIndex = NewTextureIndex[ Material[ i ].TextureIndex ];

            // Copy the New Material
            DestQRaw->Material[ NewMaterialIndex[i] ] = Material[ i ];
        }

        // Copy the new texture
        if ( NewTextureIndex[ i ] != -1 )
        {
            DestQRaw->Texture[ NewTextureIndex[ i ] ] = Texture[ i ];
        }
    }

    //---------------------------------------------------------------------
    // Copy all the triangles to the new place
    //---------------------------------------------------------------------
    for ( k = j = i = 0; i < SrcQRaw->NQTris; i++ )
    {
        if ( SrcQRaw->QTri[i].MeshID == SID )
        {
            ASSERT( j <= NSTri );

            // Copy Triangle
            DestQRaw->QTri[ DestQRaw->NQTris + j ] = SrcQRaw->QTri[i];

            // Update its meshID
            DestQRaw->QTri[ DestQRaw->NQTris + j ].MeshID = DID;

            // Update the Material Index
            DestQRaw->QTri[ DestQRaw->NQTris + j ].MaterialID = NewMaterialIndex[DestQRaw->QTri[ DestQRaw->NQTris + j ].MaterialID];

            // Get ready for the next triangle
            j++;
        }
    }

    //---------------------------------------------------------------------
    // Fill the Rest of the parameters in the destination struct
    //---------------------------------------------------------------------

    // Update how many textures we got
    DestQRaw->NTextures += NTex;


    // Update how many materials we got
    DestQRaw->NMaterials += NMat;

    // Update how many triangles we got
    DestQRaw->NQTris += NSTri;

    // Update how many mehses we got
    DestQRaw->NMeshes++;

    // Set the new mesh Name
    x_strcpy( DestQRaw->MeshName[DID], DestMeshNameUpper );


    return QRAW_CheckIntegrity( DestQRaw );
}

//=========================================================================
// QRAW_RenameMesh
//=========================================================================
xbool QRAW_RenameMesh( qraw* QRaw, char* NameTo, char* NameFrom )
{
    s32 ID;
    char NameToUpper[256];

    ASSERT( QRaw     );
    ASSERT( NameFrom );
    ASSERT( NameTo   );

    //---------------------------------------------------------------------
    // Find ID
    //---------------------------------------------------------------------
    if ( QRAW_GetMeshIndex( QRaw, &ID, NameFrom ) ) 
        return FALSE;

    //---------------------------------------------------------------------
    // Rename Mesh
    //---------------------------------------------------------------------
    x_strcpy(NameToUpper,NameTo);
    x_strtoupper(NameToUpper);
    x_strcpy( QRaw->MeshName[ID], NameToUpper );

    return QRAW_CheckIntegrity( QRaw );
}

//=========================================================================
// QRAW_MergeMesh
//=========================================================================
xbool QRAW_MergeMesh( qraw* QRaw, char* MergeInto, char* MergeFrom )
{
    s32 i;
    s32 DestID;
    s32 SrcID;

    ASSERT( QRaw      );
    ASSERT( MergeInto );
    ASSERT( MergeFrom );

    //---------------------------------------------------------------------
    // Find IDs
    //---------------------------------------------------------------------
    if ( QRAW_GetMeshIndex( QRaw, &DestID, MergeInto ) ) 
        return FALSE;

    if ( QRAW_GetMeshIndex( QRaw, &SrcID,  MergeFrom ) ) 
        return FALSE;


    //---------------------------------------------------------------------
    // Merge all the IDs to the new ID
    //---------------------------------------------------------------------
    for ( i = 0; i < QRaw->NQTris; i++ )
    {
        // Convert to the new index
        if ( QRaw->QTri[i].MeshID == SrcID )
            QRaw->QTri[i].MeshID = DestID;

        // Make sure to reindex all the ids
        if ( QRaw->QTri[i].MeshID > SrcID )
            QRaw->QTri[i].MeshID--;
    }

    //---------------------------------------------------------------------
    // Reindex All the Meshes Names
    //---------------------------------------------------------------------
    for ( i = SrcID; i < (QRaw->NMeshes-1); i++ )
    {
        x_strcpy( QRaw->MeshName[i], QRaw->MeshName[i+1] );
    }

    //---------------------------------------------------------------------
    // Update structure
    //---------------------------------------------------------------------
    QRaw->NMeshes--;

    return QRAW_CheckIntegrity( QRaw );
}

///////////////////////////////////////////////////////////////////////////

void    QRAW_Transform     ( qraw* QRaw, matrix4* M )
{
    s32 i,j;
    vector3 V;
    ASSERT(QRaw);
    ASSERT(M);
    
    for (i=0; i<QRaw->NQTris; i++)
    {
        for (j=0; j<3; j++)
        {
            V = M->Transform((QRaw->QTri[i].XYZ[j]));
            //M4_TransformVerts(M, &V, &(QRaw->QTri[i].XYZ[j]), 1);
            QRaw->QTri[i].XYZ[j] = V;
        }
    }
}

///////////////////////////////////////////////////////////////////////////

void QRAW_SortMeshes( qraw* QRaw )
{
    s16  Outer;
    s16  Inner;
    s16  Target;

    xbool MeshTaken   [ MAX_RAW_MESHES ];
    s16  MeshIndexMap[ MAX_RAW_MESHES ];
    char NewMeshName [ MAX_RAW_MESHES ][32];

    ASSERT( QRaw );
    QRAW_CheckIntegrity( QRaw );

    // Clear the MeshIndexMap to "not taken".
    for( Outer = 0; Outer < QRaw->NMeshes; ++Outer )
    {
        MeshTaken[Outer] = FALSE;
    }

    // Outer loop:  iterate QRaw->NMeshes times.
    for( Outer = 0; Outer < QRaw->NMeshes; ++Outer )
    {
        // Set target index to an impossible value.
        Target = -1;

        // Need to find the first "not taken" mesh.
        for( Inner = 0; Inner < QRaw->NMeshes; ++Inner )
        {
            if( !MeshTaken[Inner] )
            {
                Target = Inner;
                break;
            }
        }

        ASSERT( Target != -1 );

        // Loop thru the the rest of the meshes and find the "lowest" 
        // name for a mesh that is still "not taken".

        for( Inner = Target+1; Inner < QRaw->NMeshes; ++Inner )
        {
            if( !MeshTaken[Inner] )
            {
                if( x_stricmp( QRaw->MeshName[Inner], 
                              QRaw->MeshName[Target] ) < 0 )
                {
                    Target = Inner;
                }
            }        
        }

        // We now have in Target the index of the mesh which should go
        // in the position indicated by Outer.

        MeshTaken[Target]    = TRUE;
        MeshIndexMap[Target] = Outer;

        x_strcpy( NewMeshName[Outer], QRaw->MeshName[Target] );
    }

    //
    // Place the "new" data into the "old" object.
    //

    // Update all of the QRaw triangles.
    for( Outer = 0; Outer < QRaw->NQTris; ++Outer )
    {
        QRaw->QTri[Outer].MeshID = MeshIndexMap[ QRaw->QTri[Outer].MeshID ];
    }

    // Update the mesh names.
    for( Outer = 0; Outer < QRaw->NMeshes; ++Outer )
    {
        x_strcpy( QRaw->MeshName[Outer], NewMeshName[Outer] );
    }                
}

///////////////////////////////////////////////////////////////////////////

void QRAW_Bounds( qraw* QRaw, vector3** Min, vector3** Max, f32* MaxDist )
{
    s32         i, j;
    vector3    V;
    f32         L;
    f32         MaxL = 0.0f;

    ASSERT(QRaw);

    *Min = (vector3*)x_malloc( sizeof( vector3 ) * QRaw->NMeshes );
    *Max = (vector3*)x_malloc( sizeof( vector3 ) * QRaw->NMeshes );

    ASSERT( *Min );
    ASSERT( *Max );

    for( i = 0; i < QRaw->NMeshes; ++i )
    {
        (*Min)[i].X =  1000000.0f;
        (*Min)[i].Y =  1000000.0f;
        (*Min)[i].Z =  1000000.0f;
        (*Max)[i].X = -1000000.0f;
        (*Max)[i].Y = -1000000.0f;
        (*Max)[i].Z = -1000000.0f;
    }         

    for( i = 0; i < QRaw->NQTris; ++i )
    {
        for( j = 0; j < 3; ++j )
        {
            V = QRaw->QTri[i].XYZ[j];

            (*Min)[QRaw->QTri[i].MeshID].X = MIN( (*Min)[QRaw->QTri[i].MeshID].X, V.X );
            (*Min)[QRaw->QTri[i].MeshID].Y = MIN( (*Min)[QRaw->QTri[i].MeshID].Y, V.Y );
            (*Min)[QRaw->QTri[i].MeshID].Z = MIN( (*Min)[QRaw->QTri[i].MeshID].Z, V.Z );

            (*Max)[QRaw->QTri[i].MeshID].X = MAX( (*Max)[QRaw->QTri[i].MeshID].X, V.X );
            (*Max)[QRaw->QTri[i].MeshID].Y = MAX( (*Max)[QRaw->QTri[i].MeshID].Y, V.Y );
            (*Max)[QRaw->QTri[i].MeshID].Z = MAX( (*Max)[QRaw->QTri[i].MeshID].Z, V.Z );

            L = V.Length();
            MaxL = MAX( MaxL, L );
        }
    }

    if( MaxDist )
        *MaxDist = MaxL;
}

///////////////////////////////////////////////////////////////////////////

void QRAW_OrientedBounds( qraw* QRaw, vector3** Bound )
{
    s32         i, j;
    vector3*   BestMin;
    vector3*   BestMax;
    vector3*   TempMin;
    vector3*   TempMax;
    f32*        BestVolume;
    radian*     BestYAngle;
    vector3    TempV;
    f32         TempVolume;
    radian      Angle;

    ASSERT(QRaw);
    ASSERT(Bound);

    *Bound      = (vector3*)x_malloc( sizeof( vector3 ) * QRaw->NMeshes * 8);
    BestMin     = (vector3*)x_malloc( sizeof( vector3 ) * QRaw->NMeshes);
    BestMax     = (vector3*)x_malloc( sizeof( vector3 ) * QRaw->NMeshes);
    TempMin     = (vector3*)x_malloc( sizeof( vector3 ) * QRaw->NMeshes);
    TempMax     = (vector3*)x_malloc( sizeof( vector3 ) * QRaw->NMeshes);
    BestYAngle  = (radian*)  x_malloc( sizeof( radian )   * QRaw->NMeshes);
    BestVolume  = (f32*)     x_malloc( sizeof( f32 )      * QRaw->NMeshes);
    ASSERT(*Bound);
    ASSERT(BestMin);
    ASSERT(BestMax);
    ASSERT(TempMin);
    ASSERT(TempMax);
    ASSERT(BestYAngle);
    ASSERT(BestVolume);

    // Set impossibly large best volume
    for (i=0; i<QRaw->NMeshes; i++)
    {
        BestMin[i].Zero();
        BestMax[i].Zero();
        BestYAngle[i] = 0;
        BestVolume[i] = 100000.0f*100000.0f*100000.0f;
    }

    // Loop through possible angles to try 0-90
    for (Angle=0.0f; Angle<R_90; Angle+=R_1)
    {
        // Clear temporary bounds
        for (i=0; i<QRaw->NMeshes; i++)
        {
            TempMin[i].X =  1000000.0f;
            TempMin[i].Y =  1000000.0f;
            TempMin[i].Z =  1000000.0f;
            TempMax[i].X = -1000000.0f;
            TempMax[i].Y = -1000000.0f;
            TempMax[i].Z = -1000000.0f;
        }

        // Loop through all the QTris and verts and find bbox at this angle
        for (i=0; i<QRaw->NQTris; i++)
        {
            qtri* QT = &(QRaw->QTri[i]);

            for (j=0; j<3; j++)
            {
                TempV = QT->XYZ[j];
                TempV.RotateY( Angle );
                //V3_RotateY( &TempV, &(QT->XYZ[j]), Angle );
                TempMin[QT->MeshID].X = MIN( TempMin[QT->MeshID].X , TempV.X );
                TempMin[QT->MeshID].Y = MIN( TempMin[QT->MeshID].Y , TempV.Y );
                TempMin[QT->MeshID].Z = MIN( TempMin[QT->MeshID].Z , TempV.Z );
                TempMax[QT->MeshID].X = MAX( TempMax[QT->MeshID].X , TempV.X );
                TempMax[QT->MeshID].Y = MAX( TempMax[QT->MeshID].Y , TempV.Y );
                TempMax[QT->MeshID].Z = MAX( TempMax[QT->MeshID].Z , TempV.Z );
            }
        }

        // Check if new bboxes are better than best bboxes
        for (i=0; i<QRaw->NMeshes; i++)
        {
            TempVolume = ABS(TempMin[i].X-TempMax[i].X) * ABS(TempMin[i].Z-TempMax[i].Z);

            if (TempVolume < BestVolume[i])
            {
                BestMin[i]    = TempMin[i];
                BestMax[i]    = TempMax[i];
                BestVolume[i] = TempVolume;
                BestYAngle[i] = Angle;
            }
        }
    }

    // Build bbox verts
    for (i=0; i<QRaw->NMeshes; i++)
    {
        vector3* B  = &((*Bound)[i*8]);
        vector3* MN = &BestMin[i];
        vector3* MX = &BestMax[i];

        // Build 8 individual vertices
        B[0].X = MN->X;   B[0].Y = MN->Y;   B[0].Z = MN->Z;
        B[1].X = MN->X;   B[1].Y = MN->Y;   B[1].Z = MX->Z;
        B[2].X = MN->X;   B[2].Y = MX->Y;   B[2].Z = MN->Z;
        B[3].X = MN->X;   B[3].Y = MX->Y;   B[3].Z = MX->Z;
        B[4].X = MX->X;   B[4].Y = MN->Y;   B[4].Z = MN->Z;
        B[5].X = MX->X;   B[5].Y = MN->Y;   B[5].Z = MX->Z;
        B[6].X = MX->X;   B[6].Y = MX->Y;   B[6].Z = MN->Z;
        B[7].X = MX->X;   B[7].Y = MX->Y;   B[7].Z = MX->Z;

        // Transform verts back into original space
        for (j=0; j<8; j++)
            B[j].RotateY( -BestYAngle[i] );
    }

    // Cleanup
    x_free(BestMin);
    x_free(BestMax);
    x_free(TempMin);
    x_free(TempMax);
    x_free(BestYAngle);
    x_free(BestVolume);
}

///////////////////////////////////////////////////////////////////////////

void QRAW_SortTextures( qraw* QRaw )
{
    s32         MinTexture;
    s32         TextureIndex[MAX_RAW_MATERIALS];
    s32         i, j;
    qtexture    TTexture;

    ASSERT(QRaw);
    QRAW_CheckIntegrity( QRaw );
    
    // SETUP INITIAL LIST
    for (i=0; i<MAX_RAW_MATERIALS; i++)
        TextureIndex[i] = -1;

    for (i=0; i<QRaw->NTextures; i++)
        TextureIndex[i] = i;

    // SORT
    for (i=0; i<QRaw->NTextures; i++)
    {
        MinTexture = i;

        // Find texture with smallest name
        for (j=i+1; j<QRaw->NTextures; j++)
        {
            if (x_stricmp( QRaw->Texture[j         ].TextureName,
                           QRaw->Texture[MinTexture].TextureName ) < 0)
            {
                MinTexture = j;
            }
        }

        // swap actual textures
        TTexture = QRaw->Texture[ i ];
        QRaw->Texture[ i ] = QRaw->Texture[ MinTexture ];
        QRaw->Texture[ MinTexture ] = TTexture;

        // swap index table
        j                        = TextureIndex[i];
        TextureIndex[i]          = TextureIndex[MinTexture];
        TextureIndex[MinTexture] = j;
    }

    // REMAP MATERIALS
    for (i=0; i<QRaw->NMaterials; i++)
    {
        for (j=0; j<QRaw->NTextures; j++)
            if (TextureIndex[j] == QRaw->Material[i].TextureIndex) break;
        ASSERT(j != QRaw->NTextures);
        QRaw->Material[i].TextureIndex = j;
    }
    
}

///////////////////////////////////////////////////////////////////////////

f32 QRAW_GetShortestEdge( qraw* QRaw )
{
    vector3    E1, E2, E3;
    f32         L1, L2, L3;
    f32         Shortest = -1.0f;
    s32         i;

    // Prime the shortest as the length of tri[0]:edge[0,1].
    if( QRaw->NQTris > 0 )
    {
        vector3 V;
        V = QRaw->QTri[0].XYZ[0] - QRaw->QTri[0].XYZ[1];
        Shortest = V.Length();
    }
    
    // Loop through the facets.
    for( i = 0; i < QRaw->NQTris; ++i )
    {
        E1 = QRaw->QTri[i].XYZ[0] - QRaw->QTri[i].XYZ[1];
        E2 = QRaw->QTri[i].XYZ[1] - QRaw->QTri[i].XYZ[2];
        E3 = QRaw->QTri[i].XYZ[2] - QRaw->QTri[i].XYZ[0];

        L1 = E1.Length();
        L2 = E2.Length();
        L3 = E3.Length();

        if( L1 < Shortest )   Shortest = L1;
        if( L2 < Shortest )   Shortest = L2;
        if( L3 < Shortest )   Shortest = L3;
    }

    return( Shortest );
}

///////////////////////////////////////////////////////////////////////////

typedef struct
{
    vector3  XYZ;
    f32       U, V;
}
qraw_merge_node;


void QRAW_MergeVertices( qraw* QRaw, f32 Epsilon,
                         s32*  NXYZsMerged, 
                         s32*  NUVsMerged, 
                         s32*  NTsRemoved )
{
    qraw_merge_node*    MergedNode   = NULL;
    s32*                MergedNodeID = NULL;
    s32                 MergedNodes  = 0;

    s32         i, j, k;
    vector3    V;
    f32         D;

    Epsilon *= Epsilon;
    ///////////////////////////////////////////////////////////////////////
    // Initialize values.

    if( NXYZsMerged )   *NXYZsMerged = 0;
    if( NUVsMerged  )   *NUVsMerged  = 0;
    if( NTsRemoved  )   *NTsRemoved  = 0;

    ///////////////////////////////////////////////////////////////////////
    // Allocate worst case temporary storage needs.

    MergedNode   = (qraw_merge_node*)x_malloc( QRaw->NQTris * 3 * sizeof(qraw_merge_node) );
    MergedNodeID = (s32*)x_malloc( QRaw->NQTris * 3 * sizeof(s32) );
    ASSERT( MergedNode   );
    ASSERT( MergedNodeID );
                                     
    ///////////////////////////////////////////////////////////////////////
    // FIRST PASS - Merge close verts.  Ignore UV for now.
    ///////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////
    // Add and index all verts into the merged set.

    for( i = 0; i < QRaw->NQTris; ++i )
    for( j = 0; j < 3;            ++j )
    {
        // Search for the vert in the merged list.
        for( k = 0; k < MergedNodes; ++k )
        {
            //V3_Sub( &V, &QRaw->QTri[i].XYZ[j], &MergedNode[k].XYZ );
            //D = V3_Length( &V );
            V.X = QRaw->QTri[i].XYZ[j].X - MergedNode[k].XYZ.X;
            V.Y = QRaw->QTri[i].XYZ[j].Y - MergedNode[k].XYZ.Y;
            V.Z = QRaw->QTri[i].XYZ[j].Z - MergedNode[k].XYZ.Z;
              D = (V.X*V.X) + (V.Y*V.Y) + (V.Z*V.Z);
            if( D < Epsilon )
                break;
        }

        // Was a suitable vert already in the merged list?
        if( k < MergedNodes )
        {
            // Average the vert into the merged vert.
/*
            MergedNode[k].XYZ.X += QRaw->QTri[i].XYZ[j].X;
            MergedNode[k].XYZ.Y += QRaw->QTri[i].XYZ[j].Y;
            MergedNode[k].XYZ.Z += QRaw->QTri[i].XYZ[j].Z;
            MergedNode[k].XYZ.X /= 2.0f;
            MergedNode[k].XYZ.Y /= 2.0f;
            MergedNode[k].XYZ.Z /= 2.0f;
*/
            // Statistics if the caller is interested.
            if( NXYZsMerged )     
                (*NXYZsMerged)++;
        }
        else
        {
            // Set up a new node.
            MergedNode[k].XYZ = QRaw->QTri[i].XYZ[j];
            MergedNodes++;
        }

        // Make the appropriate index entry.
        MergedNodeID[ (i*3) + j ] = k;
    }

    ///////////////////////////////////////////////////////////////////////
    // Copy the values back into the real verts.

    for( i = 0; i < QRaw->NQTris; ++i )
    for( j = 0; j < 3;            ++j )
    {
        //ASSERT( MergedNodeID[(i*3) + j ] < MergedNodes );
        //ASSERT( IN_RANGE( -(Epsilon*2), QRaw->QTri[i].XYZ[j].X - MergedNode[ MergedNodeID[(i*3) + j] ].XYZ.X, (Epsilon*2) ) );
        //ASSERT( IN_RANGE( -(Epsilon*2), QRaw->QTri[i].XYZ[j].Y - MergedNode[ MergedNodeID[(i*3) + j] ].XYZ.Y, (Epsilon*2) ) );
        //ASSERT( IN_RANGE( -(Epsilon*2), QRaw->QTri[i].XYZ[j].Z - MergedNode[ MergedNodeID[(i*3) + j] ].XYZ.Z, (Epsilon*2) ) );

        QRaw->QTri[i].XYZ[j] = MergedNode[ MergedNodeID[(i*3) + j] ].XYZ;
    }

    ///////////////////////////////////////////////////////////////////////
    // Blow away collapsed triangles.

    for( i = 0; i < QRaw->NQTris; ++i )
    {
        if( (MergedNodeID[(i*3)+0] == MergedNodeID[(i*3)+1]) || 
            (MergedNodeID[(i*3)+1] == MergedNodeID[(i*3)+2]) || 
            (MergedNodeID[(i*3)+2] == MergedNodeID[(i*3)+0]) )
        {
            // We have to remove tri[i].  Overwrite it with the 
            // last triangle, then blow away the newly defunct last
            // triangle.

            QRaw->NQTris--;
            QRaw->QTri[i] = QRaw->QTri[QRaw->NQTris];
            x_memset( &QRaw->QTri[QRaw->NQTris], 0, sizeof( qtri ) );

            // Update the index information, too.
            MergedNodeID[(i*3)+0] = MergedNodeID[(QRaw->NQTris*3)+0];
            MergedNodeID[(i*3)+1] = MergedNodeID[(QRaw->NQTris*3)+1];
            MergedNodeID[(i*3)+2] = MergedNodeID[(QRaw->NQTris*3)+2];

            // And we need to counteract the ++i so we don't skip
            // the "next" triangle.
            --i;

            // Statistics if the caller is interested.
            if( NTsRemoved )
                (*NTsRemoved)++;
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // SECOND PASS - Now merge close UVs.
    ///////////////////////////////////////////////////////////////////////

    MergedNodes = 0;

    ///////////////////////////////////////////////////////////////////////
    // Add and index all verts into the merged set.

    for( i = 0; i < QRaw->NQTris; ++i )
    for( j = 0; j < 3;            ++j )
    {           
        f32 U, V;
        
        // Convert UV from parametric to pixel values.
        {
            s32  MaterialID = QRaw->QTri[i].MaterialID;
            s32  TextureID  = QRaw->Material[MaterialID].TextureIndex;
            s32  W          = QRaw->Texture[TextureID].TextureWidth;
            s32  H          = QRaw->Texture[TextureID].TextureHeight;

            U = (QRaw->QTri[i].UV[j].X *= W);   // Value is assigned into
            V = (QRaw->QTri[i].UV[j].Y *= H);   // local storage AND QRaw.
        }

        // Search for the vert in the merged list.
        for( k = 0; k < MergedNodes; ++k )
        {
            if( (QRaw->QTri[i].XYZ[j].X == MergedNode[k].XYZ.X) &&
                (QRaw->QTri[i].XYZ[j].Y == MergedNode[k].XYZ.Y) &&
                (QRaw->QTri[i].XYZ[j].Z == MergedNode[k].XYZ.Z) &&
                (IN_RANGE( -0.1, U-MergedNode[k].U, 0.1 ))      && 
                (IN_RANGE( -0.1, V-MergedNode[k].V, 0.1 )) )
            {
                // Got a suitable match!
                break;
            }
        }

        // Was a suitable vert already in the merged list?
        if( k < MergedNodes )
        {
            // Average the vert into the merged vert.
            MergedNode[k].U += U;
            MergedNode[k].V += V;
            MergedNode[k].U /= 2.0f;
            MergedNode[k].V /= 2.0f;

            // Statistics if the caller is interested.
            if( NUVsMerged )     
                (*NUVsMerged)++;
        }
        else
        {
            // Set up a new node.
            MergedNode[k].XYZ = QRaw->QTri[i].XYZ[j];
            MergedNode[k].U   = U;
            MergedNode[k].V   = V;
            MergedNodes++;
        }

        // Make the appropriate index entry.
        MergedNodeID[ (i*3) + j ] = k;        
    }

    ///////////////////////////////////////////////////////////////////////
    // Copy the values back into the real verts.

    for( i = 0; i < QRaw->NQTris; ++i )
    for( j = 0; j < 3;            ++j )
    {
        f32 U, V;

        ASSERT( MergedNodeID[(i*3) + j ] < MergedNodes );
        ASSERT( IN_RANGE( -0.25, QRaw->QTri[i].UV[j].X - MergedNode[ MergedNodeID[(i*3) + j] ].U, 0.25 ) );
        ASSERT( IN_RANGE( -0.25, QRaw->QTri[i].UV[j].Y - MergedNode[ MergedNodeID[(i*3) + j] ].V, 0.25 ) );

        // Convert UV from pixel values to parametric.
        {
            s32  MaterialID = QRaw->QTri[i].MaterialID;
            s32  TextureID  = QRaw->Material[MaterialID].TextureIndex;
            s32  W          = QRaw->Texture[TextureID].TextureWidth;
            s32  H          = QRaw->Texture[TextureID].TextureHeight;

            U = (MergedNode[ MergedNodeID[(i*3) + j] ].U / W);
            V = (MergedNode[ MergedNodeID[(i*3) + j] ].V / H);
        }

        QRaw->QTri[i].UV[j].X = U;
        QRaw->QTri[i].UV[j].Y = V;
    }                           

    ///////////////////////////////////////////////////////////////////////
    // CLEAN UP
    ///////////////////////////////////////////////////////////////////////

    x_free( MergedNode   );
    x_free( MergedNodeID );
}

///////////////////////////////////////////////////////////////////////////

void QRAW_ComputeNormals( qraw* QRaw )
{
    s32         i, j, a, b;
    vector3*   FacetNormal = NULL;

    ASSERT( QRaw );
    QRAW_CheckIntegrity( QRaw );

    ///////////////////////////////////////////////////////////////////////
    // Step 1:  Allocate storage for a normal per each facet.
    //
    FacetNormal = (vector3*)x_malloc( QRaw->NQTris * sizeof(vector3) );
    ASSERT( FacetNormal );

    ///////////////////////////////////////////////////////////////////////
    // Step 2:  For every face, compute the normal.  
    //
    // Loop through the facets.
    for( i = 0; i < QRaw->NQTris; ++i )
    {
        vector3  V1, V2;
        vector3  Normal;

        // Compute the facet normal.
        V1 = QRaw->QTri[i].XYZ[1] - QRaw->QTri[i].XYZ[0];
        V2 = QRaw->QTri[i].XYZ[2] - QRaw->QTri[i].XYZ[0];

        Normal = Cross( V1, V2 );
        Normal.Normalize();

        // Assign the normal value to each vertex in the facet.
        FacetNormal[i] = Normal;
    }
    
    ///////////////////////////////////////////////////////////////////////
    // Step 3:  Initialize the vertex normals by facet.
    // 

    // All vertex normals get a contribution from their own facet.
    for( i = 0; i < QRaw->NQTris; ++i )
    for( j = 0; j < 3;            ++j )
    {
        QRaw->QTri[i].Normal[j] = FacetNormal[i];
    }                            

    ///////////////////////////////////////////////////////////////////////
    // Step 4:  Compare EVERY vertex to EVERY OTHER vertex.  
    //          (Well almost.  We'll be a bit smart about it.)
    //          If appropriate, let the normals of verts affect one 
    //          another.
    
    for( i = 0;   i < QRaw->NQTris; ++i )
    for( j = 0;   j < 3;            ++j )
    for( a = i+1; a < QRaw->NQTris; ++a )   // NOTE: a = i+1, thus a != i
    for( b = 0;   b < 3;            ++b )
    {
        // Compare tri[i]:vert[j] to tri[a]:vert[b].

        vector3  V;
        f32       D;    
        vector3* pV0;
        vector3* pV1;

        // Same mesh?
        if( QRaw->QTri[i].MeshID != QRaw->QTri[a].MeshID )
            continue;

        // Get distance between verts.
        pV0 = &QRaw->QTri[i].XYZ[j];
        pV1 = &QRaw->QTri[a].XYZ[b];

        // Quick check
        if( ABS(pV0->X-pV1->X) > 0.0001f )
            continue;

        //V3_Sub( &V, &QRaw->QTri[i].XYZ[j], &QRaw->QTri[a].XYZ[b] );
        V.X = pV0->X - pV1->X;
        V.Y = pV0->Y - pV1->Y;
        V.Z = pV0->Z - pV1->Z;
        //D = V3_Length( &V );
        D = (V.X*V.X) + (V.Y*V.Y) + (V.Z*V.Z);

        // Distance within floating point error epsilon?
        //if( D < 0.0001f )
        if( D < (0.0001f*0.0001f) )
        {
            // OK.  
            // (1) The verts are in the same spacial position.
            // (2) The verts are from the same mesh.
            // (3) The verts are from different triangles.
            // Go ahead and make some additional contributions.

            // Vertex normal tri[i]:vert[j] gets contribution from tri[a].
            QRaw->QTri[i].Normal[j].X += FacetNormal[a].X;
            QRaw->QTri[i].Normal[j].Y += FacetNormal[a].Y;
            QRaw->QTri[i].Normal[j].Z += FacetNormal[a].Z;

            // Vertex normal tri[a]:vert[b] gets contribution from tri[i].
            QRaw->QTri[a].Normal[b].X += FacetNormal[i].X;
            QRaw->QTri[a].Normal[b].Y += FacetNormal[i].Y;
            QRaw->QTri[a].Normal[b].Z += FacetNormal[i].Z;
        }        
    }

    ///////////////////////////////////////////////////////////////////////
    // Step 5:  Each vertex normal has received contributions from all
    //          relevant sources.  Now go ahead and normalize vectors.

    for( i = 0;   i < QRaw->NQTris; ++i )
    for( j = 0;   j < 3;            ++j )
    {
        // Beware near zero length normals!
        if( IN_RANGE( -0.0001f, QRaw->QTri[i].Normal[j].X, 0.0001f ) &&
            IN_RANGE( -0.0001f, QRaw->QTri[i].Normal[j].Y, 0.0001f ) &&
            IN_RANGE( -0.0001f, QRaw->QTri[i].Normal[j].Z, 0.0001f ) )
        {
            QRaw->QTri[i].Normal[j].X = 0.0f;
            QRaw->QTri[i].Normal[j].Y = 0.0f;
            QRaw->QTri[i].Normal[j].Z = 1.0f;
        }
        else
        {
            QRaw->QTri[i].Normal[j].Normalize();

            // Truncate normal to 4 digits decimal accuracy
            QRaw->QTri[i].Normal[j].X = ((s32)(QRaw->QTri[i].Normal[j].X*10000.0f))/10000.0f;
            QRaw->QTri[i].Normal[j].Y = ((s32)(QRaw->QTri[i].Normal[j].Y*10000.0f))/10000.0f;
            QRaw->QTri[i].Normal[j].Z = ((s32)(QRaw->QTri[i].Normal[j].Z*10000.0f))/10000.0f;
        }
    }

    ///////////////////////////////////////////////////////////////////////
    // Step 6:  Release allocated storage.

    x_free( FacetNormal );
}

///////////////////////////////////////////////////////////////////////////

void QRAW_N64_AdjustBilerp( qraw* QRaw )
{
    s32         i, j;
    vector3    UV[3];
    vector3    UVMin;
    vector3    UVMax;
    qtri*       QTri;
    qmaterial*  QMat;
    qtexture*   QTex;

    for (i=0; i<QRaw->NQTris; i++)
    {
        QTri = &(QRaw->QTri[i]);
        QMat = &(QRaw->Material[QTri->MaterialID]);
        QTex = &(QRaw->Texture[QMat->TextureIndex]);

        // scale uv's to pixel size
        for (j=0; j<3; j++)
        {
            UV[j].X = (QTri->UV[j].X * (QTex->TextureWidth));
            UV[j].Y = (QTri->UV[j].Y * (QTex->TextureHeight));
            UV[j].Z = 0;
        }

        // Find Min and max
        UVMin.X = UV[0].X;
        UVMin.Y = UV[0].Y;
        UVMax.X = UV[0].X;
        UVMax.Y = UV[0].Y;
        for( j=1; j<3; j++ )
        {
            if (UV[j].X > UVMax.X)  UVMax.X = UV[j].X;                
            if (UV[j].Y > UVMax.Y)  UVMax.Y = UV[j].Y;                
            if (UV[j].X < UVMin.X)  UVMin.X = UV[j].X;                
            if (UV[j].Y < UVMin.Y)  UVMin.Y = UV[j].Y;                
        }

        // adjust for bilerp
        for( j=0; j<3; j++ )
        {
            UV[j].X = UVMin.X + ((UV[j].X - UVMin.X) * (1.0f - (0.975f/(UVMax.X - UVMin.X))));
            UV[j].Y = UVMin.Y + ((UV[j].Y - UVMin.Y) * (1.0f - (0.975f/(UVMax.Y - UVMin.Y))));
        }

        // Scale UV's back down to parametric
        for( j=0; j<3; j++ )
        {
            QTri->UV[j].X = UV[j].X / (f32)QTex->TextureWidth;
            QTri->UV[j].Y = UV[j].Y / (f32)QTex->TextureHeight;
        }
    }
}


//========================================================================
// QRAW_Copy
//========================================================================
void QRAW_Copy( qraw* Dest, qraw* Src )
{
    ASSERT(Dest);
    ASSERT(Src);

    *Dest = *Src;
    Dest->QTri = NULL;

    //-- Grab a "chunk 'o memory"(tm)
    Dest->QTri = (qtri*)x_malloc(Src->NQTris * sizeof(qtri));
    ASSERT(Dest->QTri);

    //-- copy tris
    x_memcpy(Dest->QTri, Src->QTri, Src->NQTris * sizeof(qtri));
}

///////////////////////////////////////////////////////////////////////////
