#include "VertexNode.hpp"


//=====================================================================================================================================
// VertList
//=====================================================================================================================================
VertList::VertList( void )
{
    mpHead     = NULL;
    mpNextList = NULL;
    bDynamic   = FALSE;
}

//-------------------------------------------------------------------------------------------------------------------------------------
VertList::~VertList( void )
{
    KillList( );
}

//-------------------------------------------------------------------------------------------------------------------------------------
void VertList::AttachList( void* pSkin, s32 MeshID, s32 VertID )
{
    VertList* pListList;

    VertList* pNewList = new VertList;
    ASSERT( pNewList );

    pListList = this;

    // Walk the list of lists until you find the end of the road.
    while( pListList->mpNextList )
        pListList = pListList->mpNextList;

    pListList->mpNextList = pNewList;
    pNewList->mpNextList = NULL;

    pNewList->AddNode( pSkin, MeshID, VertID );
    pNewList->bDynamic = TRUE;
}


//-------------------------------------------------------------------------------------------------------------------------------------
void VertList::KillList( void )
{
    VertList* pCurrentList;
    VertList* pPrevList;

    // Destroy any connected lists.
    do
    {
        pCurrentList = this;
        pPrevList    = this;

        // While there are lists in this list loop until you get to the end.
        while( pCurrentList->mpNextList )
        {
            pPrevList    = pCurrentList;
            pCurrentList = pCurrentList->mpNextList;
        }

        // Well, we are down to the root list, so clean out it's nodes.
        VertNode* pNode;
        VertNode* pNextNode;

        pNode = pCurrentList->mpHead;
        while( pNode )
        {
            pNextNode = pNode->pNext;
            delete pNode;
            pNode = pNextNode;
        }
        pCurrentList->mpHead = NULL;

        if( pCurrentList != this && pCurrentList->bDynamic )
        {
            delete pPrevList->mpNextList;
            pPrevList->mpNextList = NULL;
        }

    }while( mpNextList );
}


//-------------------------------------------------------------------------------------------------------------------------------------
void VertList::AddNode( void* pSkin, s32 MeshID, s32 VertID )
{
    VertNode* pNewNode;
    VertNode* pNextNode;

    pNewNode = new VertNode;
    ASSERT( pNewNode );

    pNewNode->pNext = NULL;
    pNewNode->pSkin  = pSkin;
    pNewNode->MeshID = MeshID;
    pNewNode->VertID = VertID;


    // Test to see if it's the first node.
    if( mpHead == NULL )
    {
        mpHead = pNewNode;
    }
    else
    {
        // Find the end of the list.
        pNextNode = mpHead;
        while( pNextNode->pNext != NULL )
            pNextNode = pNextNode->pNext;

        pNextNode->pNext = pNewNode;
    }
}