 /*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
*
*		Filename:	GenLnkLst.h
*		Date:		03/15/99
*
*		Desc:		Single and Double linked list containers.
*
*
*		Revisions History: 03/23/1999
*
*
\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#ifndef __GENLNKLST_H__
#define __GENLNKLST_H__

/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
	extern "C" {
#endif
/*-----------------------------------------------------------------------*/

/*------------------------------------------------------------------------
 * Definition of a LISTs and NODEs.
 *-----------------------------------------------------------------------*/

//LINK LIST STRUCTURES
typedef struct _tagSLNKLSTPTR   SLNKLSTPTR;     //SINGLE link LIST, with nodes of POINTER.

typedef struct _tagDLNKLSTPTR   DLNKLSTPTR;     //DOUBLE link LIST, with nodes of POINTER.

//NODE STRUCTURES
typedef struct _tagSLSTNODEPTR  SLSTNODEPTR;    //SINGLE link NODE, data is a POINTER

typedef struct _tagDLSTNODEPTR  DLSTNODEPTR;    //DOUBLE link NODE, data is a POINTER


/*--------------------------------------------------------------------------*/
/*        		List Initializations / De-initializations					*/
/*																			*/
/* IF YOU DO NOT INITIALIZE THE LIST THERE WILL BE UNPREDICTABLE BEHAVIOR	*/
/* IF YOU DO NOT DE-INIT MEMORY WILL NOT BE RETURNED TO THE SYSTEM			*/	
/*--------------------------------------------------------------------------*/

	/* Initializes a single linked list, returns 0 on success */
u32 SLNKLSTPTR_Init  ( SLNKLSTPTR  *pThe_list );

u32 DLNKLSTPTR_Init  ( DLNKLSTPTR  *pThe_list );

	/* De-Initialize  a list, returns 0 on exit_succes. If you De-Init on an
       uninitialized list the program will GPF , if lucky.                 */ 
u32 SLNKLSTPTR_DeInit  ( SLNKLSTPTR  *pThe_list );

u32 DLNKLSTPTR_DeInit  ( DLNKLSTPTR  *pThe_list );


/*-------------------------------------------------------------------------*/
/*																		   */	
/*		List Information                                                   */	
/*																		   */	
/*-------------------------------------------------------------------------*/

	/* is_empty; returns nonzero if list is empty */
u32 SLNKLSTPTR_IsEmpty ( SLNKLSTPTR  *pThe_list );

u32 DLNKLSTPTR_IsEmpty ( DLNKLSTPTR  *pThe_list );

	/* extracts the count of the list from the head node and stuffs the dwNodeCount
	   with this value. Returns 0 if successful , otherwise 1 on failure.		  */
u32 SLNKLSTPTR_GetCount  ( SLNKLSTPTR  *pThe_list , u32 *dwNodeCount ); 

u32 DLNKLSTPTR_GetCount  ( DLNKLSTPTR  *pThe_list , u32 *dwNodeCount ); 


/*---------------------------------------------------------------------------
 *
 *			List Insertion / Removal Functions
 *		Insertion functions WILL insert garbage without errors
 *
 *--------------------------------------------------------------------------*/

	/* inserts an element at the head of the list, returns 0 on success  */
u32 SLNKLSTPTR_InsertAtHead  ( SLNKLSTPTR  *pThe_list, void *pInNodeData );

u32 DLNKLSTPTR_InsertAtHead  ( DLNKLSTPTR  *pThe_list, void *pInNodeData );


/* insert after the node that is inputted  */
/* CAUTION, must be taken when inserting after an Index Node, if that node points to some
unknown area it will cause a GPF,if you send a Temporary index node that still has data
fields the function will NOT return an error and point to ??, returns 0 on success  */ 
u32 SLNKLSTPTR_InsertAfter ( SLNKLSTPTR *pThe_list, SLSTNODEPTR *pIndex, void* pInNodeData );

u32 DLNKLSTPTR_InsertAfter ( DLNKLSTPTR *pThe_list, DLSTNODEPTR *pIndex, void* pInNodeData );


/* inserts an element at the end of the list, is a slow method due to  */
/* stepping through entire list	, returns 0 on success 				   */
u32 SLNKLSTPTR_InsertAtTail_Slow  ( SLNKLSTPTR  *pThe_list , void *pInNodeData );

u32 DLNKLSTPTR_InsertAtTail ( DLNKLSTPTR *pThe_list, void* pInNodeData );


/* inserts a piece of data in front an inputted node, if Index_node=NULL will insert at head
   and return success, if IndexNode=?? will insert at end, will return 0, else 1 exit_failure */ 
u32 SLNKLSTPTR_InsertBefore_Slow (SLNKLSTPTR *pThe_list, SLSTNODEPTR *pIndex_node, void* pInNodeData );

u32 DLNKLSTPTR_InsertBefore ( DLNKLSTPTR *pThe_list, DLSTNODEPTR *pIndex, void* pInNodeData );

u32 DLNKLSTPTR_InsertNodeAtIndex(DLNKLSTPTR *pTheList, DLSTNODEPTR *pTheNode, DLSTNODEPTR *pIndex);

/*----deletion / removal---- User is responsible for the freeing of his data */ 

/* Will remove a node from a list if it contains the data inputted,
   will not remove anything if not found, , returns 0 on success , 1 if otherwise */ 

u32 SLNKLSTPTR_RemoveNodeOfData  ( SLNKLSTPTR  *pThe_list, void* pInNodeData );

u32 DLNKLSTPTR_RemoveNodeOfData  ( DLNKLSTPTR  *pThe_list, void* pInNodeData );


/* deletes a node from a list,, returns 0 on success  */

u32 SLNKLSTPTR_RemoveNode  ( SLNKLSTPTR  *pThe_list , SLSTNODEPTR  *pIndex_node );

u32 DLNKLSTPTR_RemoveNode  ( DLNKLSTPTR  *pThe_list , DLSTNODEPTR  *pIndex_node );


/* removes each node in the inputted list, sets head-node count to 0, returns 0 on success */

u32 SLNKLSTPTR_RemoveAllNodes  ( SLNKLSTPTR  *pThe_list );

u32 DLNKLSTPTR_RemoveAllNodes  ( DLNKLSTPTR  *pThe_list );


/*----------------------------------------------------------------------------
 *
 *		List walking / manipulation functions
 *	
 *---------------------------------------------------------------------------*/


/* fetches the adress of the first node in the list, returns the address of the first node
	,if the list is empty,  NULL */
SLSTNODEPTR*  SLNKLSTPTR_GetHeadNode  ( SLNKLSTPTR  *pThe_list );

DLSTNODEPTR*  DLNKLSTPTR_GetHeadNode  ( DLNKLSTPTR  *pThe_list );


/* gets the adress of the last node in the list, slow due to walking of the list, returns the
   the address of the last node, otherwise NULL if exit_failure.  */    
SLSTNODEPTR*  SLNKLSTPTR_GetTailNode_Slow  ( SLNKLSTPTR  *pThe_list );

DLSTNODEPTR*  DLNKLSTPTR_GetTailNode  ( DLNKLSTPTR  *pThe_list );


/*  fetches the address of the next node after the inputted node, returns the address
    of the next node in the list, otherwise NULL. CAUTION must be taken to assure that
    you fetch the next node of a valid node.                                            */

SLSTNODEPTR*  SLNKLSTPTR_GetNextNode  ( SLNKLSTPTR  *pThe_list , SLSTNODEPTR  *pIn_node );

DLSTNODEPTR*  DLNKLSTPTR_GetNextNode  ( DLNKLSTPTR  *pThe_list , DLSTNODEPTR  *pIn_node );


/* fetches the address of the previous node in the list. Caution should be taken to assure
   that a valid node is passed to the funtion                                           */
DLSTNODEPTR*  DLNKLSTPTR_GetPrevNode  ( DLNKLSTPTR  *pThe_list , DLSTNODEPTR  *pIn_node );


/*-------------------------------------------------------------------------*/
/*																		   */	
/*		List helpers									                   */	
/*																		   */	
/*-------------------------------------------------------------------------*/



void*   SLNKLSTPTR_GetData( SLNKLSTPTR  *pThe_list, SLSTNODEPTR *pThe_Node );

void*   DLNKLSTPTR_GetData( DLNKLSTPTR  *pThe_list, DLSTNODEPTR *pThe_Node );	            


void    SLNKLSTPTR_SetData( SLNKLSTPTR  *pThe_list, SLSTNODEPTR *pThe_Node , void* pInData );   

void    DLNKLSTPTR_SetData( DLNKLSTPTR  *pThe_list, DLSTNODEPTR *pThe_Node , void* pInData );    


/* searches a list for a piece of data, returns NULL if NOT found otherwise the address
   of the node that contains the data.  */
SLSTNODEPTR*  SLNKLSTPTR_FindNodeFromData ( SLNKLSTPTR  *pThe_list , void  *pDataToBeFound );

DLSTNODEPTR*  DLNKLSTPTR_FindNodeFromData ( DLNKLSTPTR  *pThe_list , void  *pDataToBeFound );

/* Sends to A_Dbg the address of the node and the address of the data.Used for debugging.
   returns 0 on success, returns 1 if the head count and the node count is different.    */
u32 SLNKLSTPTR_DebugDumpLst  ( SLNKLSTPTR  *pThe_list );

u32 DLNKLSTPTR_DebugDumpLst  ( DLNKLSTPTR  *pThe_list );


/*For every element in the list calls a user defined function */
/*The user defined function gets the node data and a context pointer */    
/*WARNING: Any Insertions into a list will cause unpredictable behavior, Deletions	*
 *           should not cause any negative effects.									*/

/*------------------------------------------------------------------------
 *  Defintion of a Datatype that contains a pointer to a user function.
 *-----------------------------------------------------------------------*/
typedef void(PFN_FOREACH) ( void *pNodeData, void* pContextData );

void SLNKLSTPTR_ForEachNode  ( SLNKLSTPTR  *pThe_list, PFN_FOREACH pfnUserFunction, void* pContext );
void DLNKLSTPTR_ForEachNode  ( DLNKLSTPTR  *pThe_list, PFN_FOREACH pfnUserFunction, void* pContext );


/* returns the index that the data is at */
u32 SLNKLSTPTR_FindNodeIndex(SLNKLSTPTR *pTheList ,void *pTheData );
//u32 DLSTNODEPTR_FindIndex( pTheList , pTheData );

/* moves a node from a Source list to a Destination List */
u32 SLNKLSTPTR_MoveNode( SLNKLSTPTR *pSrcList, SLNKLSTPTR *pDestList, u32 dwIndexOfNodeToMove, u32 dwIndexToInsert );
//u32 DLNKLSTPTR_MoveNode( pSrcList , pDestList , dwIndexOfNodeToMove , dwIndexToInsert );

/* copies a node from a Source list to a Destination List */
//u32 SLNKLSTPTR_CopyNode( pSrcList , pDestList , dwIndexOfNodeToCopy , dwIndexToInsert );
//u32 DLNKLSTPTR_CopyNode( pSrcList , pDestList , dwIndexOfNodeToCopy , dwIndexToInsert );

/*-----------------------------------------------------------------*/
// BELOW ARE RUN TIME OPTIMIZATIONS FOR RELEASE PRODUCT
/*-----------------------------------------------------------------*/

#ifndef X_DEBUG
//RELEASE IMPLEMENTATION

#define	SLNKLSTPTR_GetHeadNode(pThe_list)	            ( (pThe_list)->pFirstnode )

#define	DLNKLSTPTR_GetHeadNode(pThe_list)	            ( (pThe_list)->pFirstnode )

#define	SLNKLSTPTR_GetNextNode(pThe_list, pThe_node)    ( (pThe_node)->pNext )
    
#define	DLNKLSTPTR_GetNextNode(pThe_list, pThe_node)	( (pThe_node)->pNext )


#define	SLNKLSTPTR_IsEmpty(pThe_list)                   ( 0 == (pThe_list)->dwNodeCount )

#define DLNKLSTPTR_IsEmpty(pThe_list)                   ( 0 == (pThe_list)->dwNodeCount )

#define	DLNKLSTPTR_GetTailNode(pThe_list )	            ( (pThe_list)->pLastnode)

#define	DLNKLSTPTR_GetPrevNode( pThe_list, pThe_node )	        ( (pThe_node)->pPrev )

#define	SLNKLSTPTR_GetData( pThe_list, pThe_Node )	            ((pThe_Node)->pNodeData)

#define	DLNKLSTPTR_GetData( pThe_list, pThe_Node )	            ((pThe_Node)->pNodeData)

#define	SLNKLSTPTR_SetData( pThe_list, pThe_Node , theData )    ((pThe_Node)->pNodeData = theData)

#define	DLNKLSTPTR_SetData( pThe_list, pThe_Node , theData )    ((pThe_Node)->pNodeData = theData)

#endif



/*------------------------------------------------------------------------
 * Definition of a SINGLELY LINKED LIST that contains void* data. 
 *-----------------------------------------------------------------------*/

struct _tagSLSTNODEPTR
{ 
    void*	pNodeData;             /* <---- PUT YOUR APPLICATION DATA HERE */
	struct	_tagSLSTNODEPTR *pNext;
};

struct _tagSLNKLSTPTR
{
	u32			dwNodeCount;       /* holds the count of all the nodes in the list */
    SLSTNODEPTR *pFirstnode;       /* points to the first node of a single linked list */

    #ifdef X_DEBUG
    u32         dwDbgIsInitialized;
    #endif
};


/*------------------------------------------------------------------------
 * Definition of a DOUBLE LINKED LIST NODE that contains pointers to user specific data. 
 *-----------------------------------------------------------------------*/

struct _tagDLSTNODEPTR
{ 
    void*	pNodeData;                         /* <---- PUT YOUR APPLICATION DATA HERE */
  	struct	_tagDLSTNODEPTR *pPrev;
    struct	_tagDLSTNODEPTR *pNext;
};

struct _tagDLNKLSTPTR
{
	u32			dwNodeCount;                   /* holds the count of all the nodes in the list */
    struct	_tagDLSTNODEPTR *pFirstnode;       /* points to the first node of a single linked list */
    struct	_tagDLSTNODEPTR *pLastnode;	

    #ifdef X_DEBUG
    u32         dwDbgIsInitialized;
    #endif
};



/*-------------------------------------------------------------------------------*/
#ifdef __cplusplus
	}
#endif

/*********************************************************************************/


#endif // __GENLNKLST_H__
