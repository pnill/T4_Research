/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
*
*		Filename:	R_BTree.c
*		Date:		05/07/1999
*
*		Desc:		Red Black Tree source file
*
*
*		Revisions History: 
*
*
\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

#include "cpc.h"
#include "R_BTree.h"

/* Private Structure of a node for a Red Black Tree */
typedef struct _tagRBTREENODE
{
	s32			swKey;			/* The value of which all elements will be compared upon.*/
	void		*pvNodeData;	/* Custom data is assigned here   */
	u8          color;			/* a value to hold the color of a particular node.	*/

    struct _tagRBTREENODE	*pLeft;
	struct _tagRBTREENODE	*pRight;
	
}	RBTREENODE;


 // prototype for helper function
u32 DeleteBalanceUp( RBTREE *pTree, RBTREENODE *pCursorRef, s32 ParentKey);
//u32 EnumTreeRecursion( RBTREENODE *, SLNKLSTU32 *, u32 );
XCHAR color( RBTREENODE *pNode );

/*-------------------------------------------------------------------------*/
/*			Red Black tree creation and destruction routines			   */
/*-------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------------------------/
 * Function	: RBT_InitTree()
 * Desc		: Initialize variables of a new tree
 */
void RBT_InitTree( RBTREE *pTree )
{
ASSERT_PTR( pTree );
	
	pTree->dwNodeCount = 0;
	pTree->pRoot = NULL;

	return;
};




/*----------------------------------------------------------------------------------------------/
 * Function	: DestroyTreeInternal()
 * Desc		: Destroys a tree recursively deallocating each node.
 */
void DestroyTreeInternal( RBTREENODE *pNode )
{
	if(pNode != NULL)
	{	
		DestroyTreeInternal(pNode->pLeft);
		DestroyTreeInternal(pNode->pRight);
		x_free( pNode );
	}
	return;
};



/*----------------------------------------------------------------------------------------------/
 *	Function	: RBT_DestroyTree()
 *	Desc		:
 */
void RBT_DestroyTree( RBTREE *pTree )
{
	ASSERT(NULL != pTree);
	
	if( pTree->pRoot != NULL )
		DestroyTreeInternal( (RBTREENODE*)pTree->pRoot );
	pTree->pRoot = NULL;
	pTree->dwNodeCount = 0;
	return;
};



/*-------------------------------------------------------------------------*/
/*			Functions specific to red black trees						   */ 
/*-------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------------/
 * Function	: IsFourNode()
 * Desc		:
 * In		: pointer to a red-black tree node
 * Out		: boolean
 */
X_BOOL IsFourNode(RBTREENODE *pNode)
{
	if(   ( ( NULL != pNode->pLeft ) && ( 'R' == pNode->pLeft->color ) )
	    &&
	      ( ( NULL != pNode->pRight ) && ( 'R' == pNode->pRight->color ) )
	  )
	{
		A_DDetail("Is 4 Node\n");
		return TRUE;
	}
	return FALSE;
}



/*----------------------------------------------------------------------------------------------/
 * Macro	: RotateSingle()
 * Desc		:
 * In		:
 * Out		:
 */
#define RotateSingle(newkey , pNode)										\
{																			\
	RBTREENODE *pTempParentNode;											\
																			\
	A_DDetail("single rotation\n");											\
	if(	newkey < pGParent->swKey )											\
	{																		\
		pTempParentNode = pGParent->pLeft;									\
		pGParent->pLeft = pTempParentNode->pRight;							\
		pTempParentNode->pRight = pGParent;									\
		pGParent->color = 'R';												\
		pGParent = pTempParentNode;											\
	}																		\
	else																	\
	{																		\
		pTempParentNode = pGParent->pRight;									\
		pGParent->pRight = pTempParentNode->pLeft;							\
		pTempParentNode->pLeft = pGParent;									\
		pGParent->color = 'R';												\
		pGParent = pTempParentNode;											\
	}																		\
	pGParent->color = 'B';													\
}



/*----------------------------------------------------------------------------------------------/
 * Macro	: RotateDouble()
 * Desc		:
 * In		:
 * Out		:
 */
#define RotateDouble( newkey , pNode )										\
{																			\
	A_DDetail("double rotation\n");											\
	if( newkey < pGParent->swKey )											\
	{																		\
		pGParent->pLeft= pCursor;											\
		pParent->pRight= pCursor->pLeft;									\
		pCursor->pLeft = pParent;											\
	}																		\
	else																	\
	{																		\
		pGParent->pRight= pCursor;											\
		pParent->pLeft  = pCursor->pRight;									\
		pCursor->pRight = pParent;											\
	}																		\
}																			\



/*----------------------------------------------------------------------------------------------/
 * Macro	: HandleNew()
 * Desc		:
 * In		:
 * Out		:
 */
#define HandleNew(swNewKey)													\
{																			\
	/* make sure we didn't create a new invalid condition */				\
	if( pParent->color == 'R' )												\
	{																		\
		X_BOOL bModifyRoot;													\
																			\
		if(pGParent == pTree->pRoot)										\
			bModifyRoot=TRUE;												\
		else																\
			bModifyRoot=FALSE;												\
																			\
		if( (swNewKey < pGParent->swKey) != (swNewKey < pParent->swKey) )	\
			RotateDouble(swNewKey,pGParent);								\
																			\
		RotateSingle(swNewKey,pGParent);									\
																			\
		if(bModifyRoot==TRUE)												\
			pTree->pRoot=pGParent;											\
		else																\
			if(swNewKey < pGGParent->swKey)									\
				pGGParent->pLeft  = pGParent;								\
			else															\
			pGGParent->pRight = pGParent;									\
	}																		\
	((RBTREENODE *)pTree->pRoot)->color = 'B';								\
}



/*----------------------------------------------------------------------------------------------/
 * Macro	: SplitNode()
 * Desc		:
 * In		:
 * Out		:
 */
#define SplitNode(swNewKey)													\
{																			\
	A_DDetail("splitting a four node\n");									\
	pParent->color = 'R';													\
	pParent->pLeft->color  = 'B';											\
	pParent->pRight->color = 'B';											\
	if(pParent != pTree->pRoot)												\
	{																		\
		/* make sure we didn't create an invalid condition */				\
		if(pGParent->color == 'R')											\
		{																	\
		 /* if we did we need to break from the insertion to restore the	\
		     Red Black property */											\
			pTree = RestoreRBTree(pTree,pParent->swKey);					\
			pGParent = pParent = pCursor = pTree->pRoot;					\
																			\
		}																	\
	}																		\
}




/*----------------------------------------------------------------------------------------------/
 * Function	: RestoreRBTree()
 * Desc		: restore the red black property 
 * In		:
 * Out		:
 */
RBTREE* RestoreRBTree(RBTREE *pTree , s32 StopKey)
{
	 RBTREENODE *pCursor, *pParent, *pGParent, *pGGParent;
	
	 pGGParent = pGParent = pParent = pCursor = pTree->pRoot;
	 while(pCursor->swKey != StopKey)
	 {
		pGGParent = pGParent;
		pGParent = pParent;
		pParent  = pCursor;
		/* advance the cursor */
		pCursor = ( StopKey < pCursor->swKey ) ? pCursor->pLeft : pCursor->pRight;
	 }

	HandleNew(StopKey);

	return pTree;
}



/*-------------------------------------------------------------------------*/
/*						**INSERT**										   */ 
/*-------------------------------------------------------------------------*/


/*------------------------------------------------------------------/
 * Function	: RBT_InsertData()
 * Desc		: without repeats (temp)
 * In		:
 * Out		:
 */
u32 RBT_InsertData( RBTREE *pTree, void *pvDataIn, s32 swKey_In )
{
	 RBTREENODE *pCursor = NULL, *pParent = NULL, *pGParent = NULL, *pGGParent = NULL;

	ASSERT( NULL != pTree );
	ASSERT( NULL != pvDataIn );

	/* Initialize the variables we'll need to hold important places in the tree */
	pGGParent = pGParent = pParent = pCursor = pTree->pRoot;

	/* walk the tree as if a Regular BST */
	while(NULL != pCursor )
	{
		pGGParent = pGParent;
		pGParent = pParent;
		pParent  = pCursor;

		/* check for duplicate keys */
		if(swKey_In == pCursor->swKey)
			/* Not necessarily an error , but this version does not allow duplicate keys */
			return(1);

		/* advance the cursor */
		pCursor = ( swKey_In < pCursor->swKey ) ? pCursor->pLeft : pCursor->pRight;
	
		if(IsFourNode(pParent))
			SplitNode(swNewKey);
	}

	/* we reach the bottom , insert the new node */
	pCursor = (RBTREENODE *)x_malloc(sizeof ( RBTREENODE ) );

	/* make sure there was some memory allocated, otherwise a waste of time! */
	if( NULL == pCursor )
	{
		A_DError("\nERROR: There was an error malloc-ing space for a new node .");
		/* EXIT_FAILURE */
		return(1);
	}
	/* pack the data */
	pCursor->pLeft  = NULL;
	pCursor->pRight = NULL;
	pCursor->color  = 'R';
	pCursor->pvNodeData = pvDataIn;
	pCursor->swKey	= swKey_In;

	/* check to see if tree is empty */
	if(RBT_IsEmpty(pTree))
	{
		/* insert the new item at the top of the tree */
		pCursor->color = 'B';
		pTree->pRoot = pCursor;
		pTree->dwNodeCount++;
		return(0);
	}
	if( swKey_In < pParent->swKey )
	{
		pParent->pLeft	= pCursor;
	}
	else
	{
		pParent->pRight = pCursor;
	}
	HandleNew(swKey_In);
	/* If you got here , insertion was successfull. */
	pTree->dwNodeCount++;
	return(0);
};
/*----------------------------------------------------------------------------------------------*/



/*------------------------------------------------------------------/
 *  Function : RBT_SetNodeData()
 *  Desc	 : 
 */
u32 RBT_SetNodeData( RBTREE *pTree, s32 swKey, void *pvData )
{
	RBTREENODE *pCursor;

	ASSERT( NULL != pTree );

	if(RBT_IsEmpty(pTree))
		return -1;
	
	pCursor = pTree->pRoot;

	while( NULL != pCursor )
	{
		if(swKey == pCursor->swKey)
		{
			pCursor->pvNodeData = pvData;
			return 0;
		}
		pCursor = (swKey < pCursor->swKey ) ? pCursor->pLeft : pCursor->pRight;
	}
	return -1;
}




/*-------------------------------------------------------------------------/
 * Function	: RBT_RetrieveData()
 * Desc		:
 */
void* RBT_RetrieveData( RBTREE *pTree, s32 swKey  )
{
	RBTREENODE *pCursor;

	ASSERT( NULL != pTree );

	if(RBT_IsEmpty(pTree))
		return NULL;

	pCursor = pTree->pRoot;

	while(pCursor != NULL)
	{
		if( swKey == pCursor->swKey )
			return pCursor->pvNodeData;

		pCursor = ( swKey < pCursor->swKey ) ? pCursor->pLeft : pCursor->pRight;
	}
	return NULL;
};



/*-------------------------------------------------------------------------/
 * Function	: RBT_IsEmpty()
 * Desc		: Checks if a tree is empty
 */

X_BOOL RBT_IsEmpty( RBTREE *pTree )
{
	ASSERT(NULL != pTree);
	
	if(pTree->dwNodeCount <= 0)
		return(1);		/* true , it is empty */
	else
		return(0);	/* false , has at least 1 element */
}




/*----------------------------------------------------------------------------------------------/
 * Function	: GetKeyInternal()
 * Desc		: Locates the node in the tree which contains the data pointed to by pvDatatofind
 */
void GetKeyInternal( RBTREENODE *pCursor , void *pvDatatofind , X_BOOL *bFound , s32 *pKey )
{
	if(pCursor != NULL)
	{
		if(pCursor->pvNodeData == pvDatatofind)
		{
			*bFound = TRUE;
			*pKey   = pCursor->swKey;
		}
		else
		{
			GetKeyInternal( pCursor->pLeft , pvDatatofind, bFound , pKey);
			GetKeyInternal( pCursor->pRight, pvDatatofind, bFound , pKey);
		}
	}
}



/*----------------------------------------------------------------------------------------------/
 * Function	: RBT_GetKey()
 * Desc		:
 */
u32 RBT_GetKey( RBTREE *pTree , void *pvDataToFind , s32 *ppValueOfKey )
{
	X_BOOL		bFound;

	ASSERT( NULL != pTree );
	ASSERT( NULL != pvDataToFind );

	/* make sure that the tree is valid */
	if(RBT_IsEmpty(pTree))
		return 0;

	bFound = FALSE;
	GetKeyInternal(pTree->pRoot, pvDataToFind, &bFound , ppValueOfKey );

	if(bFound == FALSE)
		return(1);		/* EXIT_FAILURE */
	else
		return(0);		/* EXIT_SUCCESS */
};


/*----------------------------------------------------------------------------------------------/
 * Function	: DebugDumpTreeInOrder()
 * Desc		:
 */
void DebugDumpTreeInOrder( RBTREENODE *pNode, u32 dwTempCnt)
{
	dwTempCnt++;
	if(pNode != NULL)
	{
		DebugDumpTreeInOrder(pNode->pLeft,dwTempCnt);
		A_DDetail("Node_Value = %d , Node_Color = %c , Node_Height = %d\n", pNode->swKey , pNode->color , dwTempCnt);
		DebugDumpTreeInOrder(pNode->pRight,dwTempCnt);
	}
	return;
}



/*----------------------------------------------------------------------------------------------/
 * Function	: DebugDumpTreePostOrder()
 * Desc		:
 */
void DebugDumpTreePostOrder( RBTREENODE *pNode, u32 dwTempCnt)
{
	dwTempCnt++;
	if(pNode != NULL)
	{
		DebugDumpTreePostOrder(pNode->pLeft,dwTempCnt);
		DebugDumpTreePostOrder(pNode->pRight,dwTempCnt);
		A_DDetail("Node_Value = %d , Node_Color = %c , Node_Height = %d\n", pNode->swKey , pNode->color , dwTempCnt);
	}
	return;

}



/*----------------------------------------------------------------------------------------------/
 * Function	: DebugDumpTreePreOrder()
 * Desc		:
 */
void DebugDumpTreePreOrder( RBTREENODE *pNode, u32 dwTempCnt)
{
	dwTempCnt++;
	if(pNode != NULL)
	{
		A_DDetail("Node_Value = %d , Node_Color = %c , Node_Height = %d\n", pNode->swKey , pNode->color , dwTempCnt);
		DebugDumpTreePreOrder(pNode->pLeft,dwTempCnt);
		DebugDumpTreePreOrder(pNode->pRight,dwTempCnt);
	}
	return;
};




/*----------------------------------------------------------------------------------------------/
 * Function	: RBT_DebugDumpTree()
 * Desc		:
 */
void  RBT_DebugDumpTree( RBTREE *pTree , u32 dwTraversal_Code )
{
//#ifdef X_DEBUG

	u32 dwTempCount;
ASSERT( NULL != pTree );
	
	dwTempCount=0;
	
	switch(dwTraversal_Code)
	{
	case 1:	 A_DDetail("\nTree Root Height=%d , InOrder Traversal\n",dwTempCount);
			 DebugDumpTreeInOrder(pTree->pRoot , dwTempCount);
		 	 break;

	case 2:  A_DDetail("\nTree Root Height=%d , PreOrder Traversal\n",dwTempCount);
			 DebugDumpTreePreOrder(pTree->pRoot , dwTempCount);
			 break;

	case 3:	 A_DDetail("\nTree Root Height=%d , PostOrder Traversal\n",dwTempCount);
			 DebugDumpTreePostOrder(pTree->pRoot , dwTempCount);
			 break;

	default: A_DDetail("\nTree Root Height=%d , InOrder Traversal\n",dwTempCount);
			 DebugDumpTreeInOrder(pTree->pRoot,dwTempCount);
		     break;
	}
	
	A_DDetail("\n");
	return;

//#else

	return;

//#endif
};



/*----------------------------------------------------------------------------------------------/
 * Function	: RBT_GetMax()
 * Desc		: Returns the maximum element in the tree, doesn't delete 
 */
void* RBT_GetMax( RBTREE *pTree , s32 *psdwKey )
{
	RBTREENODE *pCursor;
	ASSERT( NULL != pTree );
	
	pCursor = pTree->pRoot;
	if(pCursor == NULL)
		return NULL;
	else
		/* go left until you reach the bottom of the tree */
		while(pCursor!=NULL)
			pCursor = pCursor->pRight;

	if(psdwKey == NULL)
		return(pCursor->pvNodeData);
	else
		*psdwKey = pCursor->swKey;
		return(pCursor->pvNodeData);
};



/*----------------------------------------------------------------------------------------------/
 * Function	: RBT_GetMin()
 * Desc		: Returns the minimum element in the tree, doesn't delete 
 */
void* RBT_GetMin( RBTREE *pTree , s32 *psdwKey)
{
	RBTREENODE *pCursor;
	ASSERT( NULL != pTree );
	
	pCursor = pTree->pRoot;
	if(pCursor == NULL)
		return NULL;
	else
		/* go left until you reach the bottom of the tree */
		while(pCursor!=NULL)
			pCursor = pCursor->pLeft;

	if(psdwKey == NULL)
		return(pCursor->pvNodeData);
	else
		*psdwKey = pCursor->swKey;
		return(pCursor->pvNodeData);
};



/*------------------------------------------------------------------
 Static Function Used For RBT_ForEachNode():
 *  Traversal_Code -> determines the order in which the tree will be traversed
 *	RBT_TRAVERSAL_INORDER (1); RBT_TRAVERSAL_PREORDER (2); RBT_TRAVERSAL_PREORDER (3)
 *  default is RBT_TRAVERSAL_INORDER
------------------------------------------------------------------*/

static RBT_FOREACH_RET RBT_ForEachRecursive ( RBTREENODE *pNode
											, PFN_RBT_FOR_EACH pfnUserFunction
											, void* pContext
											, u32 dwTraversalCode )
{
	if( NULL == pNode )
	{
		return( RBT_COMPLETED_TRAVERSAL );
	}

	switch( dwTraversalCode )
	{
	case RBT_TRAVERSAL_PREORDER:	// preorder (2) : middle, left, right
		{
			if( 0 != pfnUserFunction( pNode->swKey, pNode->pvNodeData, pContext ) )
				break;

			if( RBT_INTERRUPTED_TRAVERSAL == RBT_ForEachRecursive( pNode->pLeft,  pfnUserFunction, pContext, 2 ) )
				break;
			if( RBT_INTERRUPTED_TRAVERSAL == RBT_ForEachRecursive( pNode->pRight, pfnUserFunction, pContext, 2 ) )
				break;

			return( RBT_COMPLETED_TRAVERSAL );
		}

	case RBT_TRAVERSAL_POSTORDER:	// postorder (3): left, right, middle
		{
			if( RBT_INTERRUPTED_TRAVERSAL == RBT_ForEachRecursive( pNode->pLeft,  pfnUserFunction, pContext, 3 ) )
				break;

			if( RBT_INTERRUPTED_TRAVERSAL == RBT_ForEachRecursive( pNode->pRight, pfnUserFunction, pContext, 3 ) )
				break;

			if( 0 != pfnUserFunction( pNode->swKey, pNode->pvNodeData, pContext ) )
				break;

			return( RBT_COMPLETED_TRAVERSAL );
		}
	default:// inorder (1) : left, middle, right
		{
#ifdef X_DEBUG
			if( 1 != dwTraversalCode )
			{
				A_DDetail( "\nWarning, no or invalid traversal code passed to RBT_ForEach()\n" );
			}
#endif
			if( RBT_INTERRUPTED_TRAVERSAL == RBT_ForEachRecursive( pNode->pLeft,  pfnUserFunction, pContext, 1 ) )
				break;

			if( 0 != pfnUserFunction( pNode->swKey, pNode->pvNodeData, pContext ) )
				break;

			if( RBT_INTERRUPTED_TRAVERSAL == RBT_ForEachRecursive( pNode->pRight, pfnUserFunction, pContext, 1 ) )
				break;

			return ( RBT_COMPLETED_TRAVERSAL );
		}
	}

	return( RBT_INTERRUPTED_TRAVERSAL );
}

/*----------------------------------------------------------------------------
 *  Function	: RBT_ForEachNode() 
 *	Description : Applies the function of type PFN_RBT_FOR_EACH for every node
 *				  in the tree.
 *
 *  Traversal_Code -> determines the order in which the tree will be traversed
 *	RBT_TRAVERSAL_INORDER (1); RBT_TRAVERSAL_PREORDER (2); RBT_TRAVERSAL_PREORDER (3)
 *  default is RBT_TRAVERSAL_INORDER
 *
 *	Returns		: RBT_COMPLETED_TRAVERSAL (0),if the callback function has been successfully 
 *					applied to all nodes (or if tree is empty), otherwise 
 *				  RBT_INTERRUPTED_TRAVERSAL (1) if traversal was interrupted.
 *
 *				  If callback function returns RBT_INTERRUPT_TRAVERSAL (1),
 *				  the traversal is interrupted, otherwise the callback function
 *				  should return RBT_CONTINUE_TRAVERSAL (0).
 *
 *	Note: A return value of RBT_INTERRUPTED_TRAVERSAL (1) does NOT necessarily mean that 
 *  an error occurred
 *
 *	Sample Prototype for function of type PFN_RBT_FOR_EACH:
 *
 *	RBT_TRAV_RET RBT_CALLBACK_FunctionName( s32 sdwRBT_Key, void *pRBT_NodeData, void *pUserParameter );
 -----------------------------------------------------------------------------*/
RBT_FOREACH_RET RBT_ForEachNode	 ( RBTREE *pTree
								 , PFN_RBT_FOR_EACH pfnUserFunction
								 , void* pContext
								 , u32 dwTraversalCode )
{
	ASSERTS( pTree, "RBT_ForEachNode" );
	ASSERTS( pfnUserFunction, "RBT_ForEachNode" );

	return RBT_ForEachRecursive( pTree->pRoot, pfnUserFunction, pContext, dwTraversalCode );
}


///*------------------------------------------------------------------/
// * Function	: EnumerateTreeHelper()
// * Desc		: Callback function for RBT_ForEachNode() used in RBT_EnumerateTree()
// * In		:
// * Out		:
// */
//static u32 RBT_CALLBACK_EnumTreeHelper( s32 sdwKey, void *pUnused, void *pListHead )
//{
//	SLNKLSTU32_InsertAtHead( (SLNKLSTU32 *)pListHead, (u32)sdwKey );
//	return 0;
//}
//
///*----------------------------------------------------------------------------------------------
// * Function	: RBT_EnumerateTree()
// * Desc		: RBT_TRAVERSAL_INORDER (1); RBT_TRAVERSAL_PREORDER (2); RBT_TRAVERSAL_PREORDER (3)
// *			  default is RBT_TRAVERSAL_INORDER
// *			  IMPORTANT--> *cast list items to s32 for proper key values* <--IMPORTANT
// * Returns	: 0 = Success, 1 = Failure;
// */
//u32  RBT_EnumerateTree( RBTREE *pTree , SLNKLSTU32 *pListHead, u32 dwTraversalCode )
//{
//	ASSERTS( NULL != pTree, "RBT_EnumerateTree" );
//	ASSERTS( NULL != pListHead, "RBT_EnumerateTree" );
//
//	return RBT_ForEachNode( pTree, RBT_CALLBACK_EnumTreeHelper, pListHead, dwTraversalCode ); 
//}


/*----------------------------------------------------------------------------------------------/
 * Function	: validRecursion()
 * Desc		: 
 */
u32 validRecursion( RBTREENODE *pNode )
{
	u32 udwTemp = 0, udwTemp2 = 0;

	if( NULL != pNode )
	{
		if( 'R' == color(pNode) )
		{
			if( 'R' == color( pNode->pLeft )
				||
				'R' == color( pNode->pRight )
			  )
			{
				A_DWarning( "\nRed red encountered\n!!" );
				ASSERTS( FALSE, "Red node with Red child encountered!\n" );
			}
		}
		udwTemp = validRecursion( pNode->pLeft );
		udwTemp2 = validRecursion( pNode->pRight );
		if( (udwTemp2 != udwTemp) || ( -1 == udwTemp) )
		{
			return -1;
		}
		if( 'B' == pNode->color )
		{
			udwTemp++;
		}
		return udwTemp;
	}
	return(0);
}


/*----------------------------------------------------------------------------------------------/
 * Function : IsValidTree()
 * Desc		: Checks to see if a tree maintains red-black properties
 */
X_BOOL IsValidRBTree( RBTREE *pTree )
{
	RBTREENODE *pNode = NULL;

	ASSERTS( NULL != pTree, "isValidTree(): NULL tree pointer\n" );
	pNode = pTree->pRoot;

	if( 'B' != pNode->color )
	{
		A_DWarning( "\nRoot node is not black!\n" );
		return FALSE;
	}
	if( -1 == validRecursion( pNode ) )
	{
		RBT_DebugDumpTree( pTree , 1 );
		A_DWarning( "\nTree doesn't maintain valid red-black structure!\n" );
		return FALSE;
	}
return TRUE;
}



/*----------------------------------------------------------------------------------------------/
 * Function	: RBT_RemoveData()
 * Desc		: Removes the node with the given swapKey from the tree and rebalances the tree 
 *				if necessary.
 * In		: Takes a pointer to a tree and a signed 32 bit integer corresponding to the 
 *				swKey of a valid node in the tree.
 * Out		: Unsigned 32 bit integer.
 */
u32 RBT_RemoveData( RBTREE *pTree , s32 swSearchKey )
{
	RBTREENODE	*pCursor = NULL,	*pParent = NULL,		
				*pGParent = NULL,	*pBestNode = NULL,		
				*pTrailer = NULL,	*pBalancePoint = NULL; 
	X_BOOL		bFound = FALSE;

	ASSERT_PTR( pTree );
	
	// make sure that the tree isn't empty
	if( RBT_IsEmpty( pTree ) )
	{
		A_DWarning( "The Tree is empty , cant delete.\n" );
		return(0);
	}

	A_DDetail("begining search for key\n");
	pCursor = pTree->pRoot;
	
	/*------------ deleting last node in tree --------------------*/
	if( (NULL == pCursor->pLeft) && (NULL == pCursor->pRight) )
	{
		x_free( pTree->pRoot );
		pTree->pRoot = NULL;
		pTree->dwNodeCount--;
		return(0);
	}

	/* --- Tree isn't empty , start searching the tree for the key to be deleted. ---*/
	while( pCursor != NULL )
	{
		if( pCursor->swKey == swSearchKey )
		{
			bFound = TRUE;
			break;
		}

		// track the progress of the pointers
		pGParent = pParent;
		pParent = pCursor;
		pCursor = ( swSearchKey < pCursor->swKey ) ? pCursor->pLeft : pCursor->pRight;
	}

	/*----- if key was not found in tree ------*/
	if( !bFound )
	{
		A_DWarning( "Could not find key to be deleted, %d\n", swSearchKey );
		return(1);
	}
	/*------------ key was found -------------*/
	else if( bFound )
	{
		A_DDetail("Found key, removing key and data contained in node.\n");

		/*---------------------- find the successor ------------------------*/
		if( (NULL == pCursor->pRight) || (NULL == pCursor->pLeft) )
		{
			pTrailer = pParent;
			pBestNode = pCursor;
		}
		else if( pCursor->pRight != NULL )
		{
			pTrailer = pCursor;
			pBestNode = pCursor->pRight;
			while( pBestNode->pLeft != NULL)
			{
				pTrailer = pBestNode;
				pBestNode = pBestNode->pLeft;
			}
		}
		else if( pCursor->pLeft != NULL )
		{
			pTrailer = pCursor;
			pBestNode = pCursor->pLeft;
			while( pBestNode->pRight != NULL )
			{
				pTrailer = pBestNode;
				pBestNode = pBestNode->pRight;
			}
		}
		/*---------------------- found successor (pBestNode) -----------*/

		/*---------------------- fix pointers --------------------------*/
		pCursor->swKey = pBestNode->swKey;	/* copy swapKey from successor */
		pCursor->pvNodeData = pBestNode->pvNodeData; /* copy node data from successor */
		if( pBestNode == pTree->pRoot )	/* check if the successor is the root */
		{
			if( NULL != pBestNode->pLeft )
			{
				pTree->pRoot = pBestNode->pLeft;
			}
			else
			{
				pTree->pRoot = pBestNode->pRight;
			}
			( (RBTREENODE *)pTree->pRoot)->color = 'B';
			pTree->dwNodeCount--;
			x_free( pBestNode );
			
#ifdef X_DEBUG
	ASSERTS( IsValidRBTree( pTree ),"RBT_RemoveData()");	/* for debugging */
#endif

			return( 0 );
		}
		else if( (pBestNode == pTrailer->pLeft) && (NULL != pBestNode) )
		{
			pTrailer->pLeft = pBalancePoint = ( NULL != pBestNode->pLeft ) ? pBestNode->pLeft : pBestNode->pRight;
		}
		else if( NULL != pBestNode )
		{
			pTrailer->pRight = pBalancePoint = ( NULL != pBestNode->pLeft ) ? pBestNode->pLeft : pBestNode->pRight;
		}

		/* if we deleted a black node we must rebalance */
		if( 'B' == pBestNode->color )
		{
				DeleteBalanceUp( pTree, pBalancePoint, pTrailer->swKey );
		}

		x_free( pBestNode );	/* deallocate the successor */
	}
	pTree->dwNodeCount--;

#ifdef X_DEBUG
	ASSERTS( IsValidRBTree( pTree ),"RBT_RemoveData()");	/* for debugging */
#endif

	return( 0 );
}




/*------------------------------------------------------------------/
 * Function	: color()
 * Desc		: Returns the color of a node
 * In		: Returns 'B' if the node is NULL, node->color other wise.
 */
XCHAR color( RBTREENODE * pNode )
{
	if( NULL == pNode )
	{
		return 'B';
	}
	else
	{
		return pNode->color;
	}
}
/*-----------------------------------------------------------------*/



/*------------------------------------------------------------------/
 * Function : getParent()
 * Desc		: Gets a reference to a node's parent.
 * In		:
 * Out		: A pointer to the parent of the node corresponding to sKey
 */
RBTREENODE * getParent( RBTREE *pTree, s32 sKey )
{
	RBTREENODE *pCursor = NULL, *pParent = NULL;

	ASSERT_PTR( pTree );

	pCursor = pTree->pRoot;
	while( NULL != pCursor )
	{
		if( pCursor->swKey == sKey )
		{
			return pParent;
		}
		else
		{
			pParent = pCursor;
			pCursor = ( sKey < pCursor->swKey ) ? pCursor->pLeft : pCursor->pRight;
		}
	}
	return NULL;
}



/*-------------------------------------------------------------------------------
 *	Function	:	DeleteBalanceUp()
 *	Desc		:	Called from RBT_RemoveData(), balances the tree from bottom up
 */
u32 DeleteBalanceUp( RBTREE *pTree, RBTREENODE *pCursorRef, s32 udwParentKey )
{
	RBTREENODE *pCursor = NULL, *pParent = NULL;
	RBTREENODE *pGParent = NULL, *pGGParent = NULL, *pSib = NULL;

	/*----------------- find the node ----------------------*/
	pCursor = pCursorRef;
	pParent = pTree->pRoot;
	while( NULL != pParent )
	{
		if( udwParentKey == pParent->swKey )
		{
			break;
		}
		pGGParent = pGParent;
		pGParent = pParent;
		pParent = ( udwParentKey < pParent->swKey ) ? pParent->pLeft : pParent->pRight;
	}
	/*--------------- found the node ------------------------*/

	if( 'R' == color(pCursor) )
	{
		pCursor->color = 'B';
		return( 0 );
	}

	/*-------------------- check the cases --------------------*/
	while( (pCursor != pTree->pRoot) && ('B' == color(pCursor)) )
	{
		if( pCursor == pParent->pLeft )	/*--- from the left ---*/
		{
			pSib = pParent->pRight;
			if( 'R' == color(pSib) )	/*--- case I ---*/
			{
				pSib->color = 'B';
				pParent->color = 'R';
				//ROTATE LEFT
				if(pParent == pTree->pRoot )
				{
					pTree->pRoot = pSib;
					pGParent = pTree->pRoot;
				}
				else
				{
					if( pParent == pGParent->pLeft )
					{
						pGParent->pLeft = pSib;
						pGParent = pGParent->pLeft;
					}		
					else
					{
						pGParent->pRight = pSib;
						pGParent = pGParent->pRight;
					}
					pGGParent = getParent( pTree, pGParent->swKey );
				}
				pParent->pRight = pSib->pLeft;
				pSib->pLeft = pParent;
				pSib = pParent->pRight;
				// END ROTATE LEFT
			}
			if( ('B' == color(pSib->pLeft)) && ('B' == color(pSib->pRight)) )	/*--- case II ---*/
			{
				pSib->color = 'R';
				pCursor = pParent;
				pParent = pGParent;
				if(NULL == pParent)
					pParent = getParent(pTree, pCursor->swKey);
				if( pParent != NULL )
				{
					pSib = ( pCursor == pParent->pLeft ) ? pParent->pRight : pParent->pLeft;
				}
				pGParent = pGGParent;
				if( NULL != pGGParent )
				{
					pGGParent = getParent( pTree, pGGParent->swKey );
				}
			}
			else
			{
				if( 'B' == color(pSib->pRight) )	/*--- case III ---*/
				{	// pSib Left is red!!
					pSib->color = 'R';
					// ROTATE RIGHT
					pParent->pRight = pSib->pLeft;
					pSib->pLeft->color = 'B';
					pSib->pLeft = pSib->pLeft->pRight;
					pParent->pRight->pRight = pSib;
					pSib = pParent->pRight;
					// END ROTATE RIGHT
				}
				pSib->color = pParent->color;	/*--- case IV ---*/
				pParent->color = 'B';
				pSib->pRight->color = 'B';
				// LEFT ROTATE
				if( pParent == pTree->pRoot )
				{
					pTree->pRoot = pSib;
					pGParent = pTree->pRoot;
				}
				else
				{
					if( pParent == pGParent->pLeft )
					{
						pGParent->pLeft = pSib;
						pGParent = pGParent->pLeft;
					}	
					else
					{
						pGParent->pRight = pSib;
						pGParent = pGParent->pRight;
					}
					pGGParent = getParent( pTree, pGParent->swKey );
				}
				pParent->pRight = pSib->pLeft;
				pSib->pLeft = pParent;
				pSib = pParent->pRight;
				// END ROTATE LEFT
				return( 0 );
			}
		}
		else	/*--- and now from the right ---*/
		{
			pSib = pParent->pLeft;
			if( 'R' == color(pSib) )	/*--- case I ---*/
			{
				pSib->color = 'B';
				pParent->color = 'R';
				// ROTATE RIGHT
				if( pParent == pTree->pRoot )
				{
					pTree->pRoot = pSib;
					pGParent = pTree->pRoot;
				}
				else
				{
					if( pParent == pGParent->pLeft )
					{
						pGParent->pLeft = pSib;
						pGParent = pGParent->pLeft;
					}
					else
					{
						pGParent->pRight = pSib;
						pGParent = pGParent->pRight;
					}
					pGGParent = getParent( pTree, pGParent->swKey );
				}
				pParent->pLeft = pSib->pRight;
				pSib->pRight = pParent;
				pSib = pParent->pLeft;
				// END ROTATE RIGHT
			}
			if( ('B' == color(pSib->pRight)) && ('B' == color(pSib->pLeft)) )	/*--- case II ---*/
			{
				pSib->color = 'R';
				pCursor = pParent;
				pParent = pGParent;
				if(NULL == pParent)
					pParent = getParent(pTree, pCursor->swKey);
				if( pCursor != pTree->pRoot )
				{
					pSib = ( pCursor == pParent->pLeft ) ? pParent->pRight : pParent->pLeft;
				}
				pGParent = pGGParent;
				if( NULL != pGGParent )
				{
					pGGParent = getParent( pTree, pGGParent->swKey );
				}
			}
			else
			{
				if( 'B' == color(pSib->pLeft) )	/*--- case III ---*/ 
				{	//pSib->pRight is red!!
					pSib->color = 'R';
					// ROTATE LEFT
					pParent->pLeft = pSib->pRight;
					pSib->pRight->color = 'B';
					pSib->pRight = pSib->pRight->pLeft;
					pParent->pLeft->pLeft = pSib;	
					pSib = pParent->pLeft;
					// END ROTATE LEFT
				}
				pSib->color = pParent->color;	/*--- case IV ---*/
				pParent->color = 'B';
				pSib->pLeft->color = 'B';
				// RIGHT ROTATE
				if( pParent == pTree->pRoot )
				{
					pTree->pRoot = pSib;
					pGParent = pTree->pRoot;
				}
				else
				{
					if( pParent == pGParent->pLeft )
					{
						pGParent->pLeft = pSib;
						pGParent = pGParent->pLeft;
					}
					else
					{
						pGParent->pRight = pSib;
						pGParent = pGParent->pRight;
					}
					pGGParent = getParent( pTree, pGParent->swKey );
				}
				pParent->pLeft = pSib->pRight;
				pSib->pRight = pParent;
				pSib = pParent->pLeft;
				// END ROTATE RIGHT
				return( 0 );
			}
		}
	}
	pCursor->color = 'B';
	return( 0 );
}
/*----------------------------------------------------------------------------------------------/
 * Function	: RBT_RemoveNode()
 * Desc		: Removes the node with the given swapKey from the tree and rebalances the tree 
 *				if necessary.
 * In		: Takes a pointer to a tree and a signed 32 bit integer corresponding to the 
 *				swKey of a valid node in the tree.
 * Out		: Unsigned 32 bit integer.
 */
u32 RBT_RemoveNode( RBTREE *pTree , s32 swSearchKey, void **ppUserDataPtr )
{
	RBTREENODE	*pCursor = NULL,	*pParent = NULL,		
				*pGParent = NULL,	*pBestNode = NULL,		
				*pTrailer = NULL,	*pBalancePoint = NULL; 
	X_BOOL		bFound = FALSE;

	ASSERT_PTR( pTree );
	
	// make sure that the tree isn't empty
	if( RBT_IsEmpty( pTree ) )
	{
		A_DWarning( "The Tree is empty , cant delete.\n" );
		return(0);
	}

	A_DDetail("begining search for key\n");
	pCursor = pTree->pRoot;
	
	/*------------ deleting last node in tree --------------------*/
	if( (NULL == pCursor->pLeft) && (NULL == pCursor->pRight) )
	{
		*ppUserDataPtr = ((RBTREENODE*)pTree->pRoot)->pvNodeData;
		x_free( pTree->pRoot );
		pTree->pRoot = NULL;
		pTree->dwNodeCount--;
		return(0);
	}

	/* --- Tree isn't empty , start searching the tree for the key to be deleted. ---*/
	while( pCursor != NULL )
	{
		if( pCursor->swKey == swSearchKey )
		{
			bFound = TRUE;
			break;
		}

		// track the progress of the pointers
		pGParent = pParent;
		pParent = pCursor;
		pCursor = ( swSearchKey < pCursor->swKey ) ? pCursor->pLeft : pCursor->pRight;
	}

	/*----- if key was not found in tree ------*/
	if( !bFound )
	{
		A_DWarning( "Could not find key to be deleted, %d\n", swSearchKey );
#ifdef X_DEBUG
	ASSERTS( IsValidRBTree( pTree ), "RBT_RemoveNode");
#endif
		return(1);
	}
	/*------------ key was found -------------*/
	else if( bFound )
	{
		A_DDetail("Found key, removing key and data contained in node.\n");

		/*---------------------- find the successor ------------------------*/
		if( (NULL == pCursor->pRight) || (NULL == pCursor->pLeft) )
		{
			pTrailer = pParent;
			pBestNode = pCursor;
		}
		else if( pCursor->pRight != NULL )
		{
			pTrailer = pCursor;
			pBestNode = pCursor->pRight;
			while( pBestNode->pLeft != NULL)
			{
				pTrailer = pBestNode;
				pBestNode = pBestNode->pLeft;
			}
		}
		else if( pCursor->pLeft != NULL )
		{
			pTrailer = pCursor;
			pBestNode = pCursor->pLeft;
			while( pBestNode->pRight != NULL )
			{
				pTrailer = pBestNode;
				pBestNode = pBestNode->pRight;
			}
		}
		/*---------------------- found successor (pBestNode) -----------*/

		/*---------------------- fix pointers --------------------------*/
		pCursor->swKey = pBestNode->swKey;	/* copy swapKey from successor */
		pCursor->pvNodeData = pBestNode->pvNodeData; /* copy node data from successor */
		if( pBestNode == pTree->pRoot )	/* check if the successor is the root */
		{
			if( NULL != pBestNode->pLeft )
			{
				pTree->pRoot = pBestNode->pLeft;
			}
			else
			{
				pTree->pRoot = pBestNode->pRight;
			}
			( (RBTREENODE *)pTree->pRoot)->color = 'B';
			pTree->dwNodeCount--;
			*ppUserDataPtr = pBestNode->pvNodeData;
			x_free( pBestNode );
#ifdef X_DEBUG
	ASSERTS( IsValidRBTree( pTree ), "RBT_RemoveNode");
#endif
			return( 0 );
		}
		else if( (pBestNode == pTrailer->pLeft) && (NULL != pBestNode) )
		{
			pTrailer->pLeft = pBalancePoint = ( NULL != pBestNode->pLeft ) ? pBestNode->pLeft : pBestNode->pRight;
		}
		else if( NULL != pBestNode )
		{
			pTrailer->pRight = pBalancePoint = ( NULL != pBestNode->pLeft ) ? pBestNode->pLeft : pBestNode->pRight;
		}

		/* if we deleted a black node we must rebalance */
		if( 'B' == pBestNode->color )
		{
				DeleteBalanceUp( pTree, pBalancePoint, pTrailer->swKey );
		}

		*ppUserDataPtr = pBestNode->pvNodeData;
		x_free( pBestNode );	/* deallocate the successor */
	}

	pTree->dwNodeCount--;

#ifdef X_DEBUG
	ASSERTS( IsValidRBTree( pTree ), "RBT_RemoveNode");
#endif
	return( 0 );
}