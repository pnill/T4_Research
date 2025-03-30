////////////////////////////////////////////////////////////////////////////
//
//  NetSpine Brand Technology, Property of Acclaim Entertainment. 
//	Trademark and Patent Pending 2001.
//
////////////////////////////////////////////////////////////////////////////




 /*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
*
*		Filename:	R_BTree.h
*		Date:		05/07/1999
*
*		Desc:		A Red Black Tree API to be used in place of a Binary
*					Search Tree where a worst-case scenario cannot be tolerated.
*					Use this RB-Tree as if a Binary Search Tree , insertions ,
*					deletions, etc... are the same. All balancing is maintained
*					internally.
*
*		Revisions History: 
*
*
\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#ifndef __RBTREE_H__
#define __RBTREE_H__

////////////////////////////////////////////////////////////////////////////
// INCLUDES
#include "c_types.h"

/*-------------------------------------------------------------------------*/
#ifdef __cplusplus
	extern "C" {
#endif
/*-------------------------------------------------------------------------*/

////////////////////////////////////////////////////////////////////////////
// TYPES
typedef struct _tagRBTREE
{
	u32	dwNodeCount;		/* The amount of elements in the list */
	void *pRoot;			/* Pointer to root of Tree.  		  */
}	RBTREE;
typedef u32	RBT_TRAVERSAL_CODE;
typedef u32	RBT_TRAV_RET;
typedef u32	RBT_FOREACH_RET;
typedef RBT_TRAV_RET (*PFN_RBT_FOR_EACH)( s32 sdwKey, void *pNodeData, void* pContextData );


////////////////////////////////////////////////////////////////////////////
// DEFINES
// ForEach function the method of traversal
#define RBT_TRAVERSAL_INORDER		1
#define RBT_TRAVERSAL_PREORDER		2
#define RBT_TRAVERSAL_POSTORDER		3
// Callback Function Return Codes
#define RBT_CONTINUE_TRAVERSAL		0
#define RBT_INTERRUPT_TRAVERSAL		1
// RBT_ForEachNode() Return Codes
#define RBT_COMPLETED_TRAVERSAL		0
#define RBT_INTERRUPTED_TRAVERSAL	1
#define RBT_GENERIC_ERROR			2


////////////////////////////////////////////////////////////////////////////
// PROTOTYPES

/***************************************************************************/
/*			Red Black tree creation and destruction routines			   */
/***************************************************************************/

/*  Initializes a red black tree */
void RBT_InitTree   ( RBTREE *pTree );

/*  Destroys the structure of the tree ,does NOT Destroy the data contained inside nodes. */
void RBT_DestroyTree( RBTREE *pTree );

/***************************************************************************/
/*			Insertions /  Deletions										   */ 
/***************************************************************************/

/*---------------------------------------------------------------------------
 *  Function	: RBT_InsertData()
 *  Description : Inserts new data into a Red Black Tree.requires a valid key,
 *				  and a pointer to application specific data.
 *
 *	Returns		: EXIT_SUCCESS(0) if there is at least one element in tree
 *				, else EXIT_FAILURE(1)
 *
 *  DOES NOT ALLOW DUPLICATE KEYS 
 ---------------------------------------------------------------------------*/
u32 RBT_InsertData( RBTREE *pTree , void *pvDataIn , s32 swKey );


/*---------------------------------------------------------------------------
 *  Function	: RBT_RemoveData()
 *  Description : Removes data from a Red Black Tree.
 *				  requires a valid key to find the data to be removed by.
 *
 *	Returns		: EXIT_SUCCESS(0) if there is at least one element in tree
 *				, else EXIT_FAILURE(1)
 *
 ---------------------------------------------------------------------------*/
u32	RBT_RemoveData( RBTREE *pTree , s32 swKey );


/*-------------------------------------------------------------------
 * Function: RBT_RemoveNode()
 * Descript: Removes the node corresponding to a particular key.
 *           Packs pUserData with the appropriate data pointer.
 *
 * Returns : EXIT_SUCCESS(0) if there is at least one element in tree
 *			, else EXIT_FAILURE(1)
 ---------------------------------------------------------------------------*/
u32 RBT_RemoveNode( RBTREE *pTree, s32 swKey, void **ppUserData ); 


/*---------------------------------------------------------------------------
 *  Function	: RBT_RetrieveData()
 *	Description : extracts the data contained in a particular node, does NOT remove the data.
 *  Returns the address of the data , else NULL if data of key is not found. 
 --------------------------------------------------------------------------*/
void* RBT_RetrieveData( RBTREE *pTree , s32 swKey );


/***************************************************************************/
/*			Helper / Information functions								   */
/***************************************************************************/

/*----------------------------------------------------------------------------
 * Function : RBT_SetNodeData()
 * Desc		: sets the data of a pre-existing node
 * In		: swKey of the node, void pointer to data to set
 * Out		: returns 0 if success, non zero otherwise
 */
u32 RBT_SetNodeData( RBTREE *pTree, s32 swKey, void *pvData );


/*-------------------------------------------------------------------
 * Function : RBT_IsEmpty()
 * Desc     : Determines if tree has at least one or more nodes
 * Returns  : TRUE(1) if tree is empty , else FALSE(0);
 */
X_BOOL RBT_IsEmpty( RBTREE *pTree );


/*-------------------------------------------------------------------
 * Function : RBT_GetMax()
 * Desc     : Returns the maximum element in the tree, doesn't delete
 * Returns  : Returns data pointer if successful, else NULL
 */
void* RBT_GetMax( RBTREE *pTree , s32 *psdwKey );


/*-------------------------------------------------------------------
 * Function : RBT_GetMax()
 * Desc     : Returns the minimum element in the tree, doesn't delete
 * Returns  : Returns data pointer if successful, else NULL
 */
void* RBT_GetMin ( RBTREE *pTree , s32 *psdwKey );


/***************************************************************************/
/* The following functions should NOT be used unless absolutely necessary! */
/***************************************************************************/

/*----------------------------------------------------------------------------
 *  Function	: RBT_GetKey() 
 *	Description : When a key for a particular piece of application data is lost or not known,
 *  Warning method is slow due to entire tree being searched. 
 *
 *	Returns		: EXIT_SUCCESS(0) if there is at least one element in tree 
 *				, else EXIT_FAILURE(1)
 -----------------------------------------------------------------------------*/
u32  RBT_GetKey    ( RBTREE *pTree , void *pvDataToFind , s32 *ppValueOfKey );


/*----------------------------------------------------------------------------
 *  Function	: RBT_ForEachNode() 
 *	Description : Calls an application defined function for every node that is
 *                in the RedBlack tree. Eliminates programmer from having to
 *                traverse tree manually.
 *
 *	Returns		: RBT_COMPLETED_TRAVERSAL ~ Successfully completed
 *				  RBT_INTERRUPTED_TRAVERSAL ~ Application stopped traversal, no error
 *				  RBT_GENERIC_ERROR ~ Application stopped traversal because of error
 *				  
 *				  If callback function returns RBT_INTERRUPT_TRAVERSAL (1),
 *						  the traversal is interrupted, otherwise the callback function
 *				  should return RBT_CONTINUE_TRAVERSAL (0).
 *
 -----------------------------------------------------------------------------*/
RBT_FOREACH_RET  RBT_ForEachNode ( RBTREE *pTree
								 , PFN_RBT_FOR_EACH pfnUserFunction
								 , void* pContext
								 , RBT_TRAVERSAL_CODE dwTraversalCode );

#ifdef X_DEBUG
/*----------------------------------------------------------------------------
 *  Function	: DebugDumpTree()
 *	Send a printout of the tree to the Debug System.
 *  Traversal_Code -> determines the order in which the tree will be printed
 *	RBT_TRAVERSAL_INORDER (1); RBT_TRAVERSAL_PREORDER (2); RBT_TRAVERSAL_PREORDER (3)
 *  default is RBT_TRAVERSAL_INORDER
 *	Function doesnt do anything in release mode.
 ----------------------------------------------------------------------------*/
void RBT_DebugDumpTree ( RBTREE *pTree , u32 dwTraversal_Code );

#endif

/*-------------------------------------------------------------------------*/
#ifdef __cplusplus
	}
#endif
/*-------------------------------------------------------------------------*/
#endif // __RBTREE_H__
