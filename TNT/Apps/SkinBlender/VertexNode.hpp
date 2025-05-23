#ifndef VERTEX_NODE_H
#define VERTEX_NODE_H

//=====================================================================================================================================
#include "x_files.hpp"
//=====================================================================================================================================


//=====================================================================================================================================
// VertNode
//=====================================================================================================================================
struct VertNode
{
    void *pSkin;
    s32   MeshID;
    s32   VertID;

    VertNode* pNext;
};


//=====================================================================================================================================
// VertList
//=====================================================================================================================================
struct VertList
{
    VertList( void );
    ~VertList( void );
    void AttachList( void* pSkin, s32 MeshID, s32 VertID );
    void KillList( void );

    void AddNode( void* pSkin, s32 MeshID, s32 VertID );

    //-------------------------------------------------------------------------------------------------------------------------------------
    xbool     bDynamic;
    VertNode* mpHead;
    VertList* mpNextList;
};

#endif // VERTEX_NODE_H
