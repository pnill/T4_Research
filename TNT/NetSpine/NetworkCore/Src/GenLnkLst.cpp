
 /*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
*
*		Filename:	GenLnkLst.c
*		Date:		03/23/99
*
*		Desc:		Single linked list source file
*
*
*		Revisions History: 
*
*
\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

#include "x_files.hpp"
#include "GenLnkLst.h"


/*-----------------------------------------------------------------*/
#define	A_DError		x_printf
#define	A_DWarning		x_printf
#define A_DStatus		x_printf
#define A_DDetail		x_printf
#define	A_Dbg           x_printf

#define LIST_INITCODE   (0x01234ABCD)


/*-------------------------------------------------------------------------*/
/*        	 	List Initializations / De-initializations				   */
/*-------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------
 * Function: _Init
 * Desc    : Initializes a list by allocating some memory for a head and setting 
 *           the head to NULL and initializes the count to zero. 
 * Returns : 0 if list has been succesfully initiated 
 *			
 ------------------------------------------------------------------------------------------*/
u32 SLNKLSTPTR_Init(SLNKLSTPTR *pThe_list)
{
ASSERTS_PTR( pThe_list                            , "SLNKLSTPTR_Init()" );


#ifdef X_DEBUG      
         pThe_list->dwDbgIsInitialized = LIST_INITCODE;
#endif

    if( NULL != pThe_list )
    {
        pThe_list->pFirstnode=NULL;
        pThe_list->dwNodeCount = 0;
        return (0); /* proper initialization */
    }
    else
    {
        /* failure */
        return 1;
    }
}




/*------------------------------------------------------------------------------------------
 * Function: _DeInit
 * Desc    : Frees all node and sets the head to 0 node count and first to NULL	
 * Returns : returns 0 if all nodes are removed and count set to zero
 *			
 ------------------------------------------------------------------------------------------*/

u32 SLNKLSTPTR_DeInit(SLNKLSTPTR *pThe_list )
{
ASSERT_PTR( pThe_list );
ASSERTS( pThe_list->dwDbgIsInitialized == LIST_INITCODE , "SLNKLSTPTR_DeInit()" );

    if ( ! SLNKLSTPTR_IsEmpty( pThe_list ) )                                            
    {
          /* if there are any elements in the list remove all of them. */                  
        SLNKLSTPTR_RemoveAllNodes(pThe_list);
    }                                                                                  
    
    pThe_list->dwNodeCount = 0;                                                           
    pThe_list->pFirstnode = NULL;                                                         
    
#ifdef X_DEBUG      
    /*Get rid of our init code*/
    pThe_list->dwDbgIsInitialized = 0;
#endif

return ( 0 );
}



/*------------------------------------------------------------------------------------------
 * Function: _GetNextNode
 * Desc    : returns the address of the next node after the inputted node.	
 * Returns : Returns address of next node or NULL if Failure
 *			
 ------------------------------------------------------------------------------------------*/
#ifdef X_DEBUG
SLSTNODEPTR* SLNKLSTPTR_GetNextNode(SLNKLSTPTR *pThe_list, SLSTNODEPTR *pIndexNode)
{
ASSERTS_PTR( pThe_list  , "SLNKLSTPTR_GetNextNode()" );
ASSERTS_PTR( pIndexNode , "SLNKLSTPTR_GetNextNode()" );
ASSERTS( pThe_list->dwDbgIsInitialized == LIST_INITCODE , "SLNKLSTPTR_GetNextNode()" );

if ( NULL == pIndexNode )
	{
      A_DWarning("Warning, You tried to get the next node of a NULL node in \n");
      A_DWarning("[ SLNKLSTPTR_GetNextNode ] \n");
      A_DWarning("or you tried to get the address of an inaccesable location, returning NULL\n");
	    return ( NULL ); /* exit_failure */
	}
else if ( NULL == pIndexNode->pNext )
	{/* Valid node was passed, with no next.   LAST NODE */
      return ( NULL );
	}
else
	{/* Valid node was passed, with a valid next.  NOT LAST NODE  */
      return ( pIndexNode->pNext );
	}
}

#endif  //X_DEBUG

/*------------------------------------------------------------------------------------------
 * Function: _IsEmpty
 * Desc    : checks to see if list is empty 	
 * Returns : returns TRUE if list is empty, returns FALSE if there is one or more 
 *           elements in list. useful in if statements ie. if(list is empty) -do-
 *																initialize list 		
 ------------------------------------------------------------------------------------------*/
#ifdef X_DEBUG
u32 SLNKLSTPTR_IsEmpty(SLNKLSTPTR *pThe_list )
{
ASSERT_PTR(  pThe_list );
ASSERTS_PTR(  pThe_list , "SLNKLSTPTR_IsEmpty()" );
ASSERTS( pThe_list->dwDbgIsInitialized == LIST_INITCODE , "SLNKLSTPTR_IsEmpty()" );

return(  0 == pThe_list->dwNodeCount );
}
#endif



/*------------------------------------------------------------------------------------------
 * Function:  _GetFirstnode
 * Desc    : extracts the first node of an inputted list	
 * Returns : Address of the first node, or NULL if list is empty
 *			
 ------------------------------------------------------------------------------------------*/
#ifdef X_DEBUG    
SLSTNODEPTR* SLNKLSTPTR_GetHeadNode(SLNKLSTPTR *pThe_list )
{
ASSERTS_PTR(  pThe_list, "SLNKLSTPTR_GetHeadNode()" );
ASSERTS( pThe_list->dwDbgIsInitialized == LIST_INITCODE , "SLNKLSTPTR_GetHeadNode()" );
    
ASSERT_PTR(  pThe_list );
/* Merge the List type with the corresponding function */
    if( ( SLNKLSTPTR_IsEmpty(pThe_list) ) )
    {
        return NULL;
    }    
    else
    {
        return pThe_list->pFirstnode;
    }
}
#endif



/*------------------------------------------------------------------------------------------
 * Function: _GetTailNode
 * Desc    : fetches the address of the last node in the list, (warning) is slow due to
 *          traversing the entire list.	
 * Returns : Address of the last node, or NULL if list is empty
 *			
 ------------------------------------------------------------------------------------------*/
SLSTNODEPTR *  SLNKLSTPTR_GetTailNode_Slow( SLNKLSTPTR  *pThe_list )
{   
    SLSTNODEPTR *pCursor;

ASSERTS_PTR( pThe_list, "SLNKLSTPTR_GetTailNode_Slow()" );
ASSERTS( pThe_list->dwDbgIsInitialized == LIST_INITCODE , "SLNKLSTPTR_GetTailNode_Slow()" );
ASSERT_PTR(  pThe_list );

    if( ( SLNKLSTPTR_IsEmpty(pThe_list) ) )
    {
		A_DWarning("\n You tried to get the next node of an empty list, returning NULL.");
		A_DWarning("\n in SLNKLSTPTR_GetLastNode");
		return NULL;
	}
	else
	{
		pCursor=pThe_list->pFirstnode;
		while( NULL != pCursor->pNext )
			pCursor=pCursor->pNext;
		return pCursor;
    }
}






/*-----------------------------------------------------------------------------------------
 *
 *			List Insertion Functions
 *
 *----------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------
 * Function: _InsertAtHead
 * Desc    : inserts a new element into the head of the list.	
 * Returns : 0 if successful, 1 if x_malloc fails to allocate memory.
 *			
 ------------------------------------------------------------------------------------------*/
u32 SLNKLSTPTR_InsertAtHead(SLNKLSTPTR *pThe_list,void *pInNodeData)
{
        SLSTNODEPTR *pNewNode;

ASSERTS_PTR(  pThe_list   , "SLNKLSTPTR_InsertAtHead()" );
ASSERTS_PTR(  pInNodeData , "SLNKLSTPTR_InsertAtHead()" );
ASSERTS( pThe_list->dwDbgIsInitialized == LIST_INITCODE , "SLNKLSTPTR_InsertAtHead()" );


    /* x_malloc a new node */
    pNewNode = ( SLSTNODEPTR *)x_malloc(sizeof( SLSTNODEPTR ));
    
    if(pNewNode) 
        
    {
        /* x_malloc successful */
        /* pack the data into the node */        
        pNewNode->pNodeData = pInNodeData;
        
        if( SLNKLSTPTR_IsEmpty(pThe_list)  )
            /* if pThe_list is empty, insert at the head */
        {
            pThe_list->pFirstnode = pNewNode;
            pNewNode->pNext = NULL;
            pThe_list->dwNodeCount++;
            /* successful insertion */
            return 0;						
        }
        /* adjust the first node to be the second node */
        else
        {
            pNewNode->pNext = pThe_list->pFirstnode;
            pThe_list->pFirstnode = pNewNode;
            pThe_list->dwNodeCount++;
            /* successful insertion */
            return 0;
        }
    }
    else /* x_malloc unsuccessful */                                                          
    {                                                                                         
        A_DError("\nX_malloc was not able to allocate memory, your in SLNKLSTPTR_InsertAtHead");
        return (1); /* x_memory was not able to allocate new memory, exit_failure */              
    }                                                                                         
}




/*------------------------------------------------------------------------------------------
 * Function: _InsertAfter
 * Desc    : Insert after the node that is inputted,doesnt care what the node is pointing to	
 *           be sure were you are placing stuff.
 * Returns : <todo>
 *			
 ------------------------------------------------------------------------------------------*/
u32 SLNKLSTPTR_InsertAfter(SLNKLSTPTR *pThe_list,SLSTNODEPTR *pIndex_node, void* pInNodeData )
{
    SLSTNODEPTR *pNewNode;

ASSERTS_PTR(  pThe_list   , "SLNKLSTPTR_InsertAfter()" );
ASSERTS_PTR(  pIndex_node , "SLNKLSTPTR_InsertAfter()" );
ASSERTS_PTR(  pInNodeData , "SLNKLSTPTR_InsertAfter()" );
ASSERTS( pThe_list->dwDbgIsInitialized == LIST_INITCODE , "SLNKLSTPTR_InsertAfter()" );                                                               
                                                                                            
    pNewNode = ( SLSTNODEPTR *)x_malloc(sizeof( SLSTNODEPTR ) );                      
                                                                                            
	if(NULL == pNewNode)                                                                    
	{                                                                                       
		A_DError("\nThere was a problem allocating memory,in SLNKLST##TYPE_InsertAfter");   
		return(1); /* returning failure */                                                  
	}                                                                                       
	else                                                                                    
	{                                                                                       
	  pNewNode->pNodeData = pInNodeData; /* pack the data into the node */  
                                                                                            
	  pNewNode->pNext = pIndex_node->pNext;                                                 
	  pIndex_node->pNext = pNewNode;                                                        
	  pThe_list->dwNodeCount++;                                                             
	  return 0;                                                                             
	}                                                                                       
}



/*------------------------------------------------------------------------------------------
 * Function: _InsertBefore
 * Desc    : inserts an element before an inputted node, inserts at the head if the
 *           In Node is NULL, or if the list is empty	
 * Returns : exit_failure if x_malloc could not allocate memeory, otherwise success
 *			
 ------------------------------------------------------------------------------------------*/
u32 SLNKLSTPTR_InsertBefore_Slow(SLNKLSTPTR *pThe_list,SLSTNODEPTR *pIndex_node, void *pInNodeData)
{
    SLSTNODEPTR *pCursor, *pNewNode;

ASSERT_PTR( pThe_list   );
ASSERT_PTR( pIndex_node );
ASSERT_PTR( pInNodeData );
ASSERTS( pThe_list->dwDbgIsInitialized == LIST_INITCODE , "SLNKLSTPTR_InsertBefore_Slow()" );
                                                                                            
	if( SLNKLSTPTR_IsEmpty(pThe_list) )                                                
        {                                                                                   
          A_DError("\nThe list is empty; if your are trying to Insert_Before , where are you pointing too?? ");
          return (1); /* exit_failure */                                                    
        }                                                                                   
    else if(  NULL == pIndex_node )  /* a NULL address sent in */                           
	{	                                                                                    
		A_DWarning("\nWarning, letting you know you sent in a NULL pointer, Inserting at head, (no error)\n");
        SLNKLSTPTR_InsertAtHead(pThe_list, pInNodeData);                               
		return 0;                                                                           
	}	                                                                                    
	else                                                                                    
	{                                                                                       
		pNewNode = (SLSTNODEPTR *)x_malloc(sizeof(SLSTNODEPTR));                      
	                                                                                        
		if( NULL == pNewNode )                                                              
		{                                                                                   
			A_DError("\nThere was a problem allocating memory, in SLNKLSTPTR_InsertBefore_Slow.");
			return(1); /* returning failure */                                              
		}                                                                                   
        else                                                                                
		{/* if Index is not found will insert at the end of the list */                     
				pNewNode->pNodeData = pInNodeData; /* pack the data into the node */
                                                                                            
				pCursor = pThe_list->pFirstnode;                                            
                if( pCursor == pIndex_node )/* your at the head node, inserts before it!*/  
                    {/* adjust the first node to be the second node */                      
        			  pNewNode->pNext = pThe_list->pFirstnode;                              
		        	  pThe_list->pFirstnode = pNewNode;                                     
			          pThe_list->dwNodeCount++;                                             
			          return 0;						/* successful insertion */              
		            }                                                                       
                else                                                                        
                {                                                                           
                    while( ( pCursor->pNext != pIndex_node ) && ( NULL != pCursor->pNext) ) 
					  pCursor=pCursor->pNext;				/* advance the cursor */        
				                                                                            
				pNewNode->pNext = pCursor->pNext;                                           
				pCursor->pNext = pNewNode;                                                  
				pThe_list->dwNodeCount++;                                                   
                /* This means you entered invalid Node data,in release version will insert*/
                /* at end of the list, besides if you want to insert at end, call that one*/
                ASSERT_PTR( pCursor->pNext);                                                
				return 0;        /* successful insertion */						            
                }                                                                           
        }                                                                                   
	}                                                                                       
}



/*------------------------------------------------------------------------------------------
 * Function: _InsertEnd_Slow
 * Desc    : inserts an element at the end of the list, is a slow method due 
 *			 to stepping through entire list
 * Returns : <todo>
 *			
 ------------------------------------------------------------------------------------------*/
u32 SLNKLSTPTR_InsertAtTail_Slow ( SLNKLSTPTR *pThe_list, void *pInNodeData )
{
    SLSTNODEPTR *pCursor, *pNewNode;

ASSERT_PTR( pThe_list );
ASSERT_PTR( pInNodeData );
                                                         
                                                                                            
    /* Merge the list type with the node type */                                        
    pNewNode = ( SLSTNODEPTR *)x_malloc(sizeof( SLSTNODEPTR ) );                          

    if (NULL == pNewNode)                                                                       
    {                                                                                       
        A_DError("\nThere was a problem allocating memory,in LIST_TYPE_InsertAtEnd_Slow");      
        return ( 1 ); /* exit_failure */                                                        
    }                                                                                       
    else                                                                                        
    {     /* pack the data into the node */													
        pNewNode->pNodeData = pInNodeData;									    
        pNewNode->pNext = NULL;                                                                 
    
    
        /* Check to see if the list is empty.*/                                                 
        if ( !(SLNKLSTPTR_IsEmpty(pThe_list) ) )                                           
        {/* The list is NOT empty. */                                                       
            pCursor=pThe_list->pFirstnode;                                                      
            while( pCursor->pNext != NULL )/* traverse the list to you almost reach the end */  
                pCursor=pCursor->pNext;                                                         
            pCursor->pNext=pNewNode; /* insert the new node */                                  
        }                                                                                   
        else                                                                                    
        {/* The list is empty. */                                                           
            pThe_list->pFirstnode = pNewNode;                                                   
        }                                                                                   
    
        pThe_list->dwNodeCount++;                                                               
        return ( 0 );                                                                           
    }                                                                                       

    return ( 1 );                                                                               
}





/*-----------------------------------------------------------------------------------------
 *
 *		List Removal/(Deletion) Functions
 *
 -----------------------------------------------------------------------------------------*/

	
/*------------------------------------------------------------------------------------------
 * Function: _RemoveNode
 * Desc    : removes a node from a list without destroying rest of list	
 * Returns : success and failure
 *			
 ------------------------------------------------------------------------------------------*/
u32 SLNKLSTPTR_RemoveNode(SLNKLSTPTR *pThe_list,SLSTNODEPTR *pIndex_node)
{

    SLSTNODEPTR *pCursor;  

ASSERTS_PTR( pThe_list  , "SLNKLSTPTR_RemoveNode()" );
ASSERTS_PTR( pIndex_node, "SLNKLSTPTR_RemoveNode()" );
ASSERTS( pThe_list->dwDbgIsInitialized == LIST_INITCODE , "SLNKLSTPTR_RemoveNode()" );
                                                                 


    if( (NULL == pThe_list) || (NULL == pIndex_node) )                                      
    {                                                                                       
        A_DDetail("\nYou can't remove a node from an empty list OR a node equal to NULL, sorry");
        return 1; /* can't remove a node from a list thats empty */                           
    }                                                                                       
    else                                                                                    
    {			                                                                            
        SLSTNODEPTR *pLastNode;                                                          
    
        pCursor = pThe_list->pFirstnode;                                                    
        pLastNode=pCursor;      
        
        /* traverse the list until you reach the end */           
        while( NULL != pCursor )
        {
            /* if the Cursor is the same as the node searching,found remove node */        
            if( pCursor == pIndex_node )
            {
                /* if the first Index node is the first node do a special deletion */       
                if(pCursor == pThe_list->pFirstnode)                                         
                {                                                                            
                    pThe_list->pFirstnode=pCursor->pNext;  /* skip over the first node */   
                    pLastNode->pNext = NULL;                                                
                    x_free(pLastNode);                       /* then delete it */           
                    pThe_list->dwNodeCount--;                                               
                    return (0);  /* exit_success */                                         
                }                                                                            
                else                                                                         
                {                                                                            
                    pLastNode->pNext = pCursor->pNext;                                      
                    pCursor->pNext = NULL;                                                  
                    x_free(	pCursor );/*Function uses x_free to return the node to memory*/ 
                    pThe_list->dwNodeCount--;                                               
                    pIndex_node=NULL;                                                       
                    return 0;				                                                
                }
            }
            /* keep on searching */
            else					
            {                                                                               
                pLastNode = pCursor;                                                        
                pCursor = pCursor->pNext;                                                   
            }
        }	                                                                                
    }                                                                                       
    A_DDetail("Warning, could not find the item you were looking for, is the data correct?\n");
    return 1;                                                                               
}





/*-------------------------------------------------------------------------------------------
 * Function: _RemoveAllNodes
 * Desc    : removes each node in the inputted list,sets head node count to 0.	
 * Returns : success if all nodes are removed.
 *			
 ------------------------------------------------------------------------------------------*/
u32 SLNKLSTPTR_RemoveAllNodes(SLNKLSTPTR *pThe_list)
{
    SLSTNODEPTR *pCursor, *pLastNode; 

    ASSERTS_PTR( pThe_list , "SLNKLSTPTR_RemoveAllNodes()" );
    
       
    if( SLNKLSTPTR_IsEmpty(pThe_list) )                                                    
    {                                                                                       
        return 0;                                                                               
    }                                                                                       
    
    pCursor=pThe_list->pFirstnode;                                                              
    
    while ( NULL != pCursor )                                                                   
    {                                                                                       
        pLastNode = pCursor;                                                                    
        
        pCursor=pCursor->pNext;  /* traverse the list until you reach he end, freeing along */  
        /* along the way.										*/  
        x_free(pLastNode);                                                                      
    }                                                                                       
    
    pThe_list->dwNodeCount = 0;                                                                 
    pThe_list->pFirstnode = NULL;   
    
    return 0;                                                                                   
}




/*------------------------------------------------------------------------------------------
 * Function: _RemoveNodeOfData
 * Desc    : searches a list to determine if a piece of data is in the list,if found the
 *           Node is destroyed and all Data removed.	
 * Returns : success if node deleted , failure if Node not found, nothing deleted.
 *			
 ------------------------------------------------------------------------------------------*/
u32 SLNKLSTPTR_RemoveNodeOfData( SLNKLSTPTR *pThe_list, void* pInNodeData )
{
    SLSTNODEPTR *pCursor, *pLastNode; 

ASSERTS_PTR( pThe_list   , "SLNKLSTPTR_RemoveNodeOfData()" );
ASSERTS_PTR( pInNodeData , "SLNKLSTPTR_RemoveNodeOfData()" );
                                                                                           
    /* if list is empty, don't bother searching */
    if ( SLNKLSTPTR_IsEmpty(pThe_list) )    
    {                                                                                       
        A_DWarning("Warning,you tried to remove Data from an empty list\n");                
        return (1); /* exit_failure */                                                      
    }                                                                                       

    /* get the first node */                            
    pCursor = pThe_list->pFirstnode;
    pLastNode = pCursor;
    
    /* if data found , remove it, else quit */                                              
    while(  NULL != pCursor  )                                                              
    {                                                                                       
        if( pCursor->pNodeData == pInNodeData )
        {
            /* if the first Index node is the first node do a special deletion */              
            if(pCursor == pThe_list->pFirstnode)                                            
            {                                                                               
                pThe_list->pFirstnode=pCursor->pNext;  /* skip over the first node */       
                x_free(pLastNode);                       /* then delete it */               
                pThe_list->dwNodeCount--;                                                   
                return (0);  /* exit_success */                                             
            }                                                                               
            else                                                                            
            {	                                                                            
                pLastNode->pNext = pCursor->pNext;                                          
                x_free(	pCursor );                                                          
                pThe_list->dwNodeCount--;                                                   
                return 0;                                                                   
            }
        }
        else                                                                                
        {                                                                                   
            pLastNode = pCursor;                                                            
            pCursor=pCursor->pNext;                                                         
        }
    }

    /* Data not found, returning Failure */
    /* exit_failure */
    return 1; 
}





/*------------------------------------------------------------------------------------------
 * Function: _GetCount
 * Desc    : extracts the count of the list from the head node 
 * Returns : returns the amount of nodes in a list
 *
 ------------------------------------------------------------------------------------------*/
u32 SLNKLSTPTR_GetCount( SLNKLSTPTR *pThe_list, u32 *pdwNodeCount )
{
ASSERTS_PTR( pdwNodeCount, "SLNKLSTPTR_GetCount()" );
ASSERTS_PTR( pThe_list   , "SLNKLSTPTR_GetCount()" );

  if( NULL == pThe_list )																	
		return(1);																			
  else																						
		*pdwNodeCount = pThe_list->dwNodeCount;  /* extract the count from the head */      
return 0;																					

}





/*------------------------------------------------------------------------------------------
 * Function: _DebugDumpLst
 * Desc    : prints out to the debug window the address each element in the Node Data Field
 * Returns : 0 if the head count is the same as the counter, otherwise 1.
 *			
 ------------------------------------------------------------------------------------------*/	
u32 SLNKLSTPTR_DebugDumpLst(SLNKLSTPTR *pThe_list)
{
	SLSTNODEPTR *pCursor;
    u32 dwCounter=1;

    ASSERT_PTR( pThe_list );

	pCursor=pThe_list->pFirstnode;
	A_Dbg("\n The head count data is = %d",pThe_list->dwNodeCount);
	while(pCursor != NULL)
	{
		A_Dbg("\nElement %d has address { 0x00%x }, and points to data { 0x00%x }",dwCounter, pCursor ,pCursor->pNodeData );
		pCursor=pCursor->pNext;

		dwCounter++;
	}
	if( pThe_list->dwNodeCount == (dwCounter-1) )
		return 0;
	else
	{
		A_Dbg("\n There might be a problem, counts were different");
		return 1;
	}
}




/*------------------------------------------------------------------------------------------
 * Function: _FindNodeFromData
 * Desc    : searches a list to determine if a piece of data is in element,	
 * Returns : if so returns address of location else NULL.
 *			
 ------------------------------------------------------------------------------------------*/	
SLSTNODEPTR* SLNKLSTPTR_FindNodeFromData( SLNKLSTPTR *pThe_list, void *pDataToBeFound )
{
    SLSTNODEPTR *pCursor;

ASSERT_PTR(  pThe_list );
ASSERT_PTR(  pDataToBeFound );                                                                    
                                                                                            
                                                                                            
	/* if list is empty, don't bother searching */                                          
	if (SLNKLSTPTR_IsEmpty(pThe_list))                                                 
		return NULL;                                                                        
                                                                                            
	/* get the first node */                                                                
	pCursor = pThe_list->pFirstnode;                                                        
                                                                                            
	/* if null or found the data, we're done */                                             
	while ( (NULL != pCursor) && ( pCursor->pNodeData != pDataToBeFound) )  
	{                                                                                       
		/* get the next node */                                                             
		pCursor = pCursor->pNext;                                                           
	}                                                                                       
                                                                                            
	/* return what we've found or null if not in the list */                                
	return pCursor;                                                                         

}


/*------------------------------------------------------------------------------------------
 * Function: _ForEachNode
 * Desc    : iterates through a list calling the users function for each node in the list,
 *			 WARNING: Any Insertions into a list will cause unpredictable behavior, Deletions
 *           should not cause any negative effects.
 * Returns : <todo>
 *			
 ------------------------------------------------------------------------------------------*/	
void SLNKLSTPTR_ForEachNode( SLNKLSTPTR *pThe_list , PFN_FOREACH pfnUserFunction, void* pContext )
{
    SLSTNODEPTR *pCursor, *pLast;

ASSERT_PTR(  pThe_list );
ASSERT_PTR(  pfnUserFunction );                                                              
                                                                                                
    pCursor = SLNKLSTPTR_GetHeadNode( pThe_list );                                         
																								
    while( NULL != pCursor )                                                                    
    {																							
		pLast = pCursor;																		
		pCursor=pCursor->pNext;																	
        pfnUserFunction((void*)pLast->pNodeData , pContext );                   
    }                                                                                           
                                                                                                
    return;		                                                                                

}



void DLNKLSTPTR_ForEachNode( DLNKLSTPTR  *pThe_list, PFN_FOREACH pfnUserFunction, void* pContext )
{
    DLSTNODEPTR *pCursor, *pLast; 

ASSERT_PTR(  pThe_list );
ASSERT_PTR(  pfnUserFunction );                                                             
                                                                                                
    pCursor = DLNKLSTPTR_GetHeadNode( pThe_list );                                         
																								
    while( NULL != pCursor )                                                                    
    {																							
		pLast = pCursor;																		
		pCursor=pCursor->pNext;																	
        pfnUserFunction((void*)pLast->pNodeData , pContext );                   
    }                                                                                           
                                                                                                
    return;		                                                                                

}


/*****************************************************************************************
******************************************************************************************
**                                                                                      **
** Marks the begining of the DOUBLY LINKED LIST implementations.                        **
**                                                                                      **
** Hopefully the seperation will make it easier to distinguish which set of data types  **
**                                                                                      **
**  are being used.                                                                     **
**                                                                                      **
******************************************************************************************
*****************************************************************************************/

/*--------------------------------------------------------------------------*/
/*        		List Initializations / De-initializations					*/
/*																			*/
/* IF YOU DO NOT INITIALIZE THE LIST THERE WILL BE UNPREDICTABLE BEHAVIOR	*/
/* IF YOU DO NOT DE-INIT MEMORY WILL NOT BE RETURNED TO THE SYSTEM			*/	
/*--------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------
 * Function: _Init  
 *
 * Desc    : Initializes a doubly linked list, sets the count = zero and all node pointers
 *           equal to NULL
 * Returns : <todo>
 *			
 ------------------------------------------------------------------------------------------*/
u32 DLNKLSTPTR_Init( DLNKLSTPTR  *pThe_list )
{
ASSERT_PTR(  pThe_list );
ASSERTS( pThe_list->dwDbgIsInitialized != LIST_INITCODE , "DLNKLSTPTR_Init()" );

#ifdef X_DEBUG
    pThe_list->dwDbgIsInitialized = LIST_INITCODE;
#endif
if( NULL != pThe_list )                                                                     
 {                                                                                           
     pThe_list->dwNodeCount = 0;                                                             
                                                                                             
     pThe_list->pFirstnode = NULL;                                                           
                                                                                             
     pThe_list->pLastnode  = NULL;	                                                         
                                                                                             
     return(0);/* proper initialization */                                                   
 }                                                                                           
 else                                                                                        
                                                                                             
		return 1; /* exit_failure */         
   
}

/*-----------------------------------------------------------------*/


u32 DLNKLSTPTR_DeInit( DLNKLSTPTR  *pThe_list )
{
ASSERT_PTR(  pThe_list );
ASSERTS( pThe_list->dwDbgIsInitialized == LIST_INITCODE , "DLNKLSTPTR_DeInit()" );

/* if the list is empty */           
if ( pThe_list->dwNodeCount )
    {  /* if there are any elements in the list remove all of them. */                      
        /* if RemoveAllNodes fails Dont go on */                                            
    if( DLNKLSTPTR_RemoveAllNodes(pThe_list) )                                     
        return (1);/* exit_failure */                                                    
    }                                                                                       

pThe_list->dwNodeCount = 0;                                                             
pThe_list->pFirstnode = NULL;                                                           
pThe_list->pLastnode  = NULL;                                                           

#ifdef X_DEBUG
     pThe_list->dwDbgIsInitialized = 0;
#endif

return ( 0 );
}



/*-------------------------------------------------------------------------*/
/*																		   */	
/*		List Information and helpers									   */	
/*																		   */	
/*-------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------
 * Function: _IsEmpty 
 *
 * Desc    : determines whether a list has any elements.
 *           
 * Returns : True if a list has any elements, else False.
 *			
 ------------------------------------------------------------------------------------------*/
#ifdef X_DEBUG
u32 DLNKLSTPTR_IsEmpty( DLNKLSTPTR  *pThe_list )
{
 ASSERTS_PTR(  pThe_list , "DLNKLSTPTR_IsEmpty()" );
 ASSERTS( pThe_list->dwDbgIsInitialized == LIST_INITCODE , "DLNKLSTPTR_IsEmpty()" );
     
 if ( 0 == pThe_list->dwNodeCount )
     return ( 1 );
 return ( 0 ); 
}    
#endif //X_DEBUG

/*------------------------------------------------------------------------------------------
 * Function: _GetCount
 *
 * Desc    : extracts the count from the List Head.
 *           
 * Returns : returns the number of elements in a list
 *			
 ------------------------------------------------------------------------------------------*/

u32 DLNKLSTPTR_GetCount( DLNKLSTPTR  *pThe_list , u32 *pdwNodeCount )
{
ASSERTS_PTR( pThe_list   , "DLNKLSTPTR_GetCount()" );
ASSERTS_PTR( pdwNodeCount, "DLNKLSTPTR_GetCount()" );
ASSERTS( pThe_list->dwDbgIsInitialized == LIST_INITCODE , "DLNKLSTPTR_GetCount()" );

	if( NULL == pThe_list )
		return(1);
	else
	    *pdwNodeCount = pThe_list->dwNodeCount;  /* extract the count from the head */

	return 0;
}
 



/*------------------------------------------------------------------------------------------
 * Function: _DebugDumpLst
 *
 * Desc    : Prints a the entire list and the elements in the list, also does a hard count 
 *           of the amount of elements in a list and compares it to the amount in the Head.
 *           
 * Returns : returns 0 if the list count is the same as the head count, else 1.
 *			
 ------------------------------------------------------------------------------------------*/
u32 DLNKLSTPTR_DebugDumpLst( DLNKLSTPTR  *pThe_list )
{

	DLSTNODEPTR *pCursor;
	u32 dwCounter=1;

ASSERT_PTR( pThe_list );
/* set the cursor = tho the forst node */
	pCursor=pThe_list->pFirstnode;

	A_Dbg("\n The head count data is = %d",pThe_list->dwNodeCount);

/* run through the list until cursor reaches the end */
	while(pCursor != NULL)
	{
		A_Dbg("\nNode %2d has address { 0x00%x }, points to data { 0x00%x }  ",dwCounter, pCursor , pCursor->pNodeData);
        A_Dbg(" N[%2d]:pPrev = { 0x00%x }  pNext = { 0x00%x }",dwCounter,pCursor->pPrev, pCursor->pNext);
        pCursor=pCursor->pNext;

		dwCounter++;
	}

    if( (dwCounter-1) == pThe_list->dwNodeCount )
        return(0);
    else
        A_Dbg("The count of the nodes was different from the count contained in the Head");
        return(1);/* exit_failure */
}




/*---------------------------------------------------------------------------
 *
 *			List Insertion / Removal Functions
 *		Insertion functions WILL insert garbage without errors
 *
 *--------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------
 * Function: _InsertAtHead
 *
 * Desc    : Inserts new data at the head of a list.
 *           
 * Returns : if insert is successful , reutrns 0 , else 1.
 *			
 ------------------------------------------------------------------------------------------*/
u32 DLNKLSTPTR_InsertAtHead( DLNKLSTPTR  *pThe_list, void *pInNodeData )
{
    DLSTNODEPTR *pNewNode;

ASSERTS_PTR( pThe_list   , "DLNKLSTPTR_InsertAtHead()" );
ASSERTS_PTR( pInNodeData , "DLNKLSTPTR_InsertAtHead()" );
ASSERTS( pThe_list->dwDbgIsInitialized == LIST_INITCODE , "DLNKLSTPTR_InsertAtHead()" );

                                                                  
                                                                                            
    /* try to allocate some memory for a new node */                                        
    pNewNode = ( DLSTNODEPTR * )x_malloc( sizeof( DLSTNODEPTR ) );                    
                                                                                            
    /* if No memory was allocated, Dont go on */                                            
    if(NULL == pNewNode)                                                                    
    {                                                                                       
       A_DError("\nInsertion failed due to new memory not allocated,in DLNKLSTPTR_InsertAtHead");
       return(1);                                                                           
    }                                                                                       
                                                                                            
    /* if the list is empty , then do a simple insert */                                    
    if( (0 == pThe_list->dwNodeCount) )                                                     
    {                                                                                       
     /* prepare the node for insertion */                                                   
                                                                                            
        /* pack the data into the node */                                                   
        pNewNode->pNodeData = pInNodeData;			                            
                                                                                            
        pNewNode->pNext = NULL;                                                             
        pNewNode->pPrev = NULL;                                                             
                                                                                            
        pThe_list->pFirstnode = pNewNode;                                                   
        /* the last node must also be set in an empty list  */                              
        pThe_list->pLastnode =  pNewNode;                                                   
                                                                                            
        pThe_list->dwNodeCount++;                                                           
                                                                                            
        return(0); /* successful insertion */                                               
    }                                                                                       
    /* else adjust the first node to be the second node */                                  
    /* the last node is always the same the same        */                                  
    else                                                                                    
    {                                                                                       
     /* prepare the node for insertion */                                                   
                                                                                            
        /* pack the data into the node */                                                   
        pNewNode->pNodeData = pInNodeData;										
                                                                                            
        pNewNode->pNext = pThe_list->pFirstnode;                                            
        pNewNode->pPrev = NULL;                                                             
                                                                                            
        pThe_list->pFirstnode->pPrev = pNewNode;                                            
                                                                                            
        pThe_list->pFirstnode =  pNewNode;                                                  
        pThe_list->dwNodeCount++;                                                           
                                                                                            
        return(0);  /* successful insertion */                                              
    }
}




/*------------------------------------------------------------------------------------------
 * Function: _InsertAfter
 *
 * Desc    : Inserts data after a specific Inputted Index Node
 *           
 * Returns : returns 0 if successful, else 1
 *			
 ------------------------------------------------------------------------------------------*/
u32 DLNKLSTPTR_InsertAfter( DLNKLSTPTR *pThe_list, DLSTNODEPTR *pIndex, void* pInNodeData )
{
    DLSTNODEPTR *pNewNode;


ASSERTS_PTR(  pThe_list , "DLNKLSTPTR_InsertAfter()" ); 
ASSERTS_PTR(  pIndex    , "DLNKLSTPTR_InsertAfter()" ); 
ASSERTS_PTR( pInNodeData, "DLNKLSTPTR_InsertAfter()" ); 
ASSERTS( pThe_list->dwDbgIsInitialized == LIST_INITCODE , "DLNKLSTPTR_InsertAfter()" );

                                                                   
                                                                                            
    /* if the list is empty */                                                              
    if ( (0 == pThe_list->dwNodeCount) )                                                    
        /* if the list is empty, just insert at the head  */                                
        return( DLNKLSTPTR_InsertAtHead( pThe_list , pInNodeData ) );                  
                                                                                            
    /* also test to see if your at the tail */                                              
    /* if so you must reset the pLastnode   */                                              
     if(pIndex == pThe_list->pLastnode)                                                     
        return ( DLNKLSTPTR_InsertAtTail( pThe_list , pInNodeData ) );                 
                                                                                            
    /* create some space to insert after    */                                              
    pNewNode = ( DLSTNODEPTR *)x_malloc(sizeof( DLSTNODEPTR ) );                      
                                                                                            
    /* make sure space was created before you go on */                                      
    if(NULL == pNewNode)                                                                    
    {                                                                                       
		A_DError("\nInsertion failed due to new memory not allocated,in DLNKLST##TYPE_InsertAfter");
        /* exit_failure, memory was not allocated   */                                      
        return(1);                                                                          
    }                                                                                       
    else                                                                                    
    {                                                                                       
                                                                                            
        /* prepare NewNode to go after the inputted node                */                  
        /* 1 set the next node's previous field to point to the new node*/                  
        /* 2  NewNode's next point to the index's next                  */                  
        /* 3  the Index Node's next point to the NewNode                */                  
        /* 4  the NewNode's previous field point to the Index node      */                  
                                                                                            
        pIndex->pNext->pPrev = pNewNode;                                                    
        pNewNode->pNext = pIndex->pNext;                                                    
        pIndex->pNext = pNewNode;                                                           
        pNewNode->pPrev = pIndex;                                                           
                                                                                            
        /* pack the data into the new node  */                                              
        pNewNode->pNodeData = pInNodeData;										
                                                                                            
        pThe_list->dwNodeCount++;                                                           
                                                                                            
        /* before your done make sure that the you didnt insert at the end  */              
        /* if so you must reset the pLastnode                               */              
            if(pIndex == pThe_list->pLastnode)                                              
                pThe_list->pLastnode = pNewNode;                                            
                                                                                            
        /*successfull insertion*/                                                           
        return(0);                                                                          
    }                                                                                       
                                                                                            
}

/*------------------------------------------------------------------------------------------
 * Function: _InsertBefore
 *
 * Desc    : Inserts data before a particular inputted node.Does no checking on the 
 *           validity of the node and where its fields point to.
 * Returns : return 0 on success, else 1
 *			
 ------------------------------------------------------------------------------------------*/
u32 DLNKLSTPTR_InsertBefore( DLNKLSTPTR *pThe_list, DLSTNODEPTR *pIndex, void* pInNodeData )
{

    DLSTNODEPTR *pNewNode;


ASSERTS_PTR(  pThe_list , "DLNKLSTPTR_InsertBefore()" );
ASSERTS_PTR(  pIndex    , "DLNKLSTPTR_InsertBefore()" );
ASSERTS_PTR( pInNodeData, "DLNKLSTPTR_InsertBefore()" );

                                                                   
                                                                                            
    /* if the list is empty */                                                              
    if( (0 == pThe_list->dwNodeCount) || (pThe_list->pFirstnode == pIndex) )                
        /* if the list is empty insert at the head of the list */                           
        return( DLNKLSTPTR_InsertAtHead( pThe_list , pInNodeData ) );                  
                                                                                            
    /* get some space for a new node    */                                                  
    pNewNode = (DLSTNODEPTR *)x_malloc( sizeof( DLSTNODEPTR ));                       
                                                                                            
    /* make sure memory was allocated   */                                                  
    if(NULL == pNewNode)                                                                    
    {                                                                                       
		A_DError("\nInsertion failed due to new memory not allocated,in DLNKLST##TYPE_InsertBefore");
        /* exit_failure, memory was not allocated   */                                      
        return(1);                                                                          
    }                                                                                       
                                                                                            
    /* prepare the node for insertion */                                                    
                                                                                            
    /* insert the new node before the index node    */                                      
                                                                                            
        /* pack the data into the new node  */                                              
        pNewNode->pNodeData = pInNodeData;										
                                                                                            
        pNewNode->pPrev = pIndex->pPrev;                                                    
        pIndex->pPrev = pNewNode;                                                           
        pNewNode->pPrev->pNext = pNewNode;                                                  
        pNewNode->pNext = pIndex;                                                           
                                                                                            
        /* Node inserted , update count of nodes    */                                      
        pThe_list->dwNodeCount++;                                                           
                                                                                            
        return(0);  /* successful insertion */                                              

}


/*------------------------------------------------------------------------------------------
 * Function: _InsertAtTail
 *
 * Desc    : Inserts new data at the end of a list, this method is optimal due to the head
 *           containing a pointer to the last node in a list.
 * Returns : returns 0 if succesfull , else 1.
 *			
 ------------------------------------------------------------------------------------------*/
u32 DLNKLSTPTR_InsertAtTail( DLNKLSTPTR *pThe_list, void* pInNodeData )
{
    DLSTNODEPTR *pNewNode;

ASSERTS_PTR(  pThe_list   , "DLNKLSTPTR_InsertAtTail()" );
ASSERTS_PTR(  pInNodeData , "DLNKLSTPTR_InsertAtTail()" );


    /* if the list is empty */
    if( (0 == pThe_list->dwNodeCount) )
        /* if the list is empty, just insert the element at the head     */
        return( DLNKLSTPTR_InsertAtHead( pThe_list, pInNodeData ) );

    else
    {
        pNewNode = (DLSTNODEPTR *)x_malloc( sizeof( DLSTNODEPTR ) );

        /* make sure that there was some memory allocated, if not dont insert */
        if(NULL == pNewNode)
            /* exit_failure */
            return(1);
        else
        {
         /* prep the new node for insertion          */

            /* pack the data into the new node  */
            pNewNode->pNodeData = pInNodeData;

            pNewNode->pNext = NULL;
            pNewNode->pPrev = pThe_list->pLastnode;

         /* insert the node at the end of the list   */
            pThe_list->pLastnode->pNext = pNewNode;
            pThe_list->pLastnode = pNewNode;
            pThe_list->dwNodeCount++;

         /* successful insert  */
            return(0);                                 
        }
	}
}



// Example:: if the index node is the N-th node, then the node to be inserted will be the new 
// N-th node and the node that was previously N will now be of index N + 1;
u32 DLNKLSTPTR_InsertNodeAtIndex(DLNKLSTPTR *pTheList, DLSTNODEPTR *pTheNode, DLSTNODEPTR *pIndex)
{
	DLSTNODEPTR *pIndexCopy;

ASSERTS_PTR(pTheList , "DLNKLSTPTR_InsertNodeAtIndex()");
ASSERTS_PTR(pTheNode , "DLNKLSTPTR_InsertNodeAtIndex()");

	// make sure that an application doesnt enter an invalid node during release
	if(NULL == pTheNode)
		//EXIT_FAILURE
		return(1);

	// see if we are inserting at the head
	if(NULL == pIndex)
	{
		pIndexCopy = pTheList->pFirstnode;
		// Check if the list is empty
		if(pIndexCopy == NULL)
		{
			// this insert is very simple
			pTheNode->pNext = NULL;
		    pTheNode->pPrev = NULL;
	                           
			pTheList->pFirstnode = pTheNode;		

			/* the last node must also be set in an empty list  */
			pTheList->pLastnode =  pTheNode;
                                       
			pTheList->dwNodeCount++;

			//EXIT_SUCCESS
			return(0);
		}
	    else
		{
			// must do more work for insert after the head
			pTheNode->pNext = pTheList->pFirstnode;
			pTheNode->pPrev = NULL;

			pTheList->pFirstnode->pPrev = pTheNode;

			pTheList->pFirstnode =  pTheNode;
			pTheList->dwNodeCount++;

			return(0);  /* successful insertion */
		}
	}
	else
		pIndexCopy = pIndex;


	

	// Prepare pTheNode to go after the inputted node
    // 1 set the next node's previous field to point to the new node
    // 2  pTheNode's next point to the index's next
    // 3  the Index Node's next point to the pTheNode
    // 4  the pTheNode's previous field point to the Index node
                                                                          
    pIndexCopy->pNext->pPrev = pTheNode;
    pTheNode->pNext = pIndexCopy->pNext;
    pIndexCopy->pNext = pTheNode;
    pTheNode->pPrev = pIndexCopy;

	pTheList->dwNodeCount++;
 
	//EXIT_SUCCESS
	return(0);
}


/*--------------------------------------------------------------------------------------------
 *   Deletion / Removal function
 --------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------
 * Function: _RemoveNodeOfData
 *
 * Desc    : Find the node that contains the input data,if found deletes it,else returns error
 *           
 * Returns : return 0 if successful
 *			
 ------------------------------------------------------------------------------------------*/
u32 DLNKLSTPTR_RemoveNodeOfData( DLNKLSTPTR  *pThe_list, void* pInNodeData )
{
    DLSTNODEPTR  *pCursor;


ASSERT_PTR(  pInNodeData );
ASSERT_PTR(  pThe_list   );

                                                                   
                                                                                            
    /* if the list is empty */                                                              
    if( (0 == pThe_list->dwNodeCount) )                                                     
    {                                                                                       
        /* don't bother  searching an empty list */                                         
        A_DDetail("Item not found, the list was empty!");                                   
        return(1);/* exit_failure */                                                        
    }                                                                                       
    else                                                                                    
    {   /* get the first node */                                                            
        pCursor = pThe_list->pFirstnode;                                                    
                                                                                            
        /* scan the list, dont go passed the end */                                         
        while( NULL != pCursor )                                                            
        {                                                                                   
            /* item found? */                                                               
            if( pCursor->pNodeData == pInNodeData)								
            {                                                                               
             /* if the data is found Send the location that the data is in to the function*/
             /* RemoveNode().Return to the caller what the results of the remove node was.*/
                return ( DLNKLSTPTR_RemoveNode( pThe_list , pCursor ) );               
            }                                                                               
            /* keep on searching */                                                         
            else                                                                            
                pCursor = pCursor->pNext;                                                   
                                                                                            
        }                                                                                   
        /* failure in RemoveNodeOfData */                                                       
        return(1);                                                                          
    }                                        
}



/*------------------------------------------------------------------------------------------
 * Function: _RemoveNode
 *
 * Desc    : Removes an inputted node from a list. The data is NOT destroyed, only the NODE.
 *           
 * Returns : 0 if node is successfully removed, else 1.
 *			
 ------------------------------------------------------------------------------------------*/
u32 DLNKLSTPTR_RemoveNode( DLNKLSTPTR  *pThe_list , DLSTNODEPTR  *pIndex )
{

ASSERT_PTR(  pIndex );
ASSERT_PTR(  pThe_list   );

                                                                                            
    /* if the list is empty */                                                              
    if ( (0 == pThe_list->dwNodeCount) )                                                    
    {
        /* dont bother deleted anything from an empty list. */							
        A_DDetail("Why did you want to delete a node from an empty list?? If list is empty where is node pointing to?");
        return(1);/* exit_failure */                                                        
    }                                                                                   

    /* if the index is at the head, the head must be updated.   */                          
    if( NULL == pIndex->pPrev )                                                 
    {                                                                       
        /* adjust the head node to point to the next node */                    
        pThe_list->pFirstnode = pIndex->pNext;                                  
    
        /* adjust the new first nodes previous field to point to NULL   */      
        if ( NULL != pIndex->pNext )                                            
            pIndex->pNext->pPrev = NULL;                                        
    
        /* return the memory to the heap    */                                  
        pIndex->pPrev = NULL;                                                   
        pIndex->pNext = NULL;                                                   
        x_free( pIndex );                                                       
        pThe_list->dwNodeCount--;                                               
    
        /* make sure that if the tree has now become empty to ensure */			
        /* we don't point to garbage */											
        if( pThe_list->dwNodeCount == 0 )										
            pThe_list->pLastnode = NULL;										
    
        /* successful deletion of a node    */                                  
        return(0);                                                              
    }                                                                       


    /* if the index is at the end, the head must be updated.    */              
    if( NULL == pIndex->pNext )                                                 
    {                                                                       
        /* set the node before it to NULL   */                                  
        if ( NULL != pIndex->pPrev )                                            
            pIndex->pPrev->pNext = NULL;                                        
    
        /* set the head node                */                                  
        pThe_list->pLastnode = pIndex->pPrev;                                   
    
        pIndex->pPrev = NULL;                                                   
        pIndex->pNext = NULL;                                                   
        x_free(pIndex);                                                         
        pThe_list->dwNodeCount--;                                               
    
        /* successful deletion of a node    */                                  
        return(0);                                                              
    }                                                                       
    /* else,if the index is somewhere in the middle, adjust all.    */              
    else                                                                        
    {                                                                       
        /* adjust the previous node around the node to be deleted   */
        pIndex->pPrev->pNext = pIndex->pNext;                               
    
        /* adjust the next node around the node to be deleted       */      
        pIndex->pNext->pPrev = pIndex->pPrev;                               
    
        pIndex->pPrev = NULL;                                               
        pIndex->pNext = NULL;                                               
        x_free(pIndex);                                                     
        pThe_list->dwNodeCount--;                                           
    
        /* successful deletion of a node    */                              
        return(0);                                                          
    }                                                                       

}



/*------------------------------------------------------------------------------------------
 * Function: _RemoveAllNodes
 *
 * Desc    : Removes all nodes from a list *Data must be Freed by the User.
 *           
 * Returns : if all nodes are freed returns 0 , else 1;
 *			
 ------------------------------------------------------------------------------------------*/
u32 DLNKLSTPTR_RemoveAllNodes(DLNKLSTPTR *pThe_list )
{
    DLSTNODEPTR*pCursor;

ASSERT_PTR(  pThe_list );
                                                                          
                                                                                                 
    /* if the list is empty , no need to remove */                                               
    if( (0 == pThe_list->dwNodeCount) )                                                          
    {                                                                                            
      	A_DDetail("Warning,tried to delete all nodes from an empty list of type DLNKLST##TYPE\n");   
        A_DDetail("This could be due to DE-INIT after a REMOVE_ALL_NODES, no error returned.\n");   
        return(0);                                                                               
    }                                                                                            
    else                                                                                         
    {                                                                                            
        pCursor=pThe_list->pFirstnode;                                                           
                                                                                                 
        while ( NULL != pCursor->pNext )                                                         
    	{                                                                                        
                 /* traverse the list until you reach the end, freeing along */                  
                 /* along the way.										     */                  
            pCursor=pCursor->pNext;                                                              
		                                                                                         
    	    x_free(pCursor->pPrev);                                                              
    	}                                                                                        
    /* don't forget to free the last element!   */                                               
    x_free(pCursor);                                                                             
                                                                                                 
    /* reset the Head                           */                                               
    pThe_list->dwNodeCount = 0;                                                                  
    pThe_list->pFirstnode = NULL;                                                                
                                                                                                 
    /* done Remove all nodes                    */                                               
    return (0);                                                                                  
    }                          
}





/*----------------------------------------------------------------------------
 *
 *		List walking / manipulation functions
 *	
 *---------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------
 * Function: _FindNodeFromData
 *
 * Desc    : determines if there is a particular piece of data in a list
 *           
 * Returns : if found returns the address of the node that contains the data, else NULL
 *			
 ------------------------------------------------------------------------------------------*/
DLSTNODEPTR*  DLNKLSTPTR_FindNodeFromData( DLNKLSTPTR  *pThe_list , void  *pDataToBeFound )
{      
    DLSTNODEPTR *pCursor;     
    

ASSERT_PTR(  pThe_list );
ASSERT_PTR(  pDataToBeFound  );

                                                                     
                                                                                             
    /* if the list is empty */                                                               
    if( (0 == pThe_list->dwNodeCount) )                                                      
        /* obviously the data's not in the list if it's empty! */                            
        return NULL;																		 
					                                                                         
    pCursor =  pThe_list->pFirstnode;					                                     
    /* keep stepping through the list until you reach the end or you find the data  */       
    while( (NULL != pCursor) && (pDataToBeFound != pCursor->pNodeData))			 
            /* get next node    */                                                           
            pCursor = pCursor->pNext;                                                        
                                                                                             
    /* return the results of the step through    */                                          
    return pCursor;                                                                          
}



/*------------------------------------------------------------------------------------------
 * Function: _GetNextNode
 *
 * Desc    : returns the address of the next node in the list
 *           
 ------------------------------------------------------------------------------------------*/
#ifdef X_DEBUG
DLSTNODEPTR*  DLNKLSTPTR_GetNextNode( DLNKLSTPTR  *pThe_list , DLSTNODEPTR  *pIn_node )
{
ASSERT_PTR(  pThe_list );
ASSERT_PTR(  pIn_node  );
ASSERTS( pThe_list->dwDbgIsInitialized == LIST_INITCODE , "DLNKLSTPTR_GetNextNode()" );

    return pIn_node->pNext;
}
#endif  //X_DEBUG


/*------------------------------------------------------------------------------------------
 * Function: _GetPrevNode
 *
 * Desc    : returns the address of the node that is before the input node
 *           
 ------------------------------------------------------------------------------------------*/
#ifdef X_DEBUG
DLSTNODEPTR*  DLNKLSTPTR_GetPrevNode  ( DLNKLSTPTR  *pThe_list , DLSTNODEPTR  *pIn_node )
{
ASSERT_PTR(  pThe_list );
ASSERT_PTR(  pIn_node  );
ASSERTS( pThe_list->dwDbgIsInitialized == LIST_INITCODE , "DLNKLSTPTR_GetPrevNode()" );

    return pIn_node->pPrev;
}
#endif  //X_DEBUG

/*------------------------------------------------------------------------------------------
 * Function: _GetHeadNode
 *
 * Desc    : extracts the head node of a list
 *           
 * Returns : returns the address of the head node in the list.
 *			
 ------------------------------------------------------------------------------------------*/
#ifdef X_DEBUG
DLSTNODEPTR*  DLNKLSTPTR_GetHeadNode( DLNKLSTPTR  *pThe_list )
{
ASSERT_PTR(  pThe_list );
ASSERTS( pThe_list->dwDbgIsInitialized == LIST_INITCODE , "DLNKLSTPTR_GetHeadNode()" );

    return pThe_list->pFirstnode;
}
#endif// X_DEBUG

/*------------------------------------------------------------------------------------------
 * Function: _GetTailNode
 *
 * Desc    : extracts from the head node in a list the tail
 *           
 * Returns : returns the last node in a list
 *			
 ------------------------------------------------------------------------------------------*/
#ifdef X_DEBUG
DLSTNODEPTR*  DLNKLSTPTR_GetTailNode( DLNKLSTPTR  *pThe_list )
{
ASSERT_PTR(  pThe_list );
return pThe_list->pLastnode;
}
#endif //X_DEBUG
/*-----------------------------------------------------------------*/

#ifdef X_DEBUG
void*   SLNKLSTPTR_GetData( SLNKLSTPTR  *pThe_list, SLSTNODEPTR *pThe_Node )
{
ASSERTS_PTR(  pThe_list , "SLNKLSTPTR_GetData()" );
ASSERTS_PTR(  pThe_Node , "SLNKLSTPTR_GetData()" );
ASSERTS( pThe_list->dwDbgIsInitialized == LIST_INITCODE , "SLNKLSTPTR_GetData()" );

return ((pThe_Node)->pNodeData);
}
/*-----------------------------------------------------------------*/


/*-----------------------------------------------------------------*/

void*   DLNKLSTPTR_GetData( DLNKLSTPTR  *pThe_list, DLSTNODEPTR *pThe_Node )
{
ASSERTS_PTR(  pThe_list , "DLNKLSTPTR_GetData()" );
ASSERTS_PTR(  pThe_Node , "DLNKLSTPTR_GetData()" );
ASSERTS( pThe_list->dwDbgIsInitialized == LIST_INITCODE , "DLNKLSTPTR_GetData()" );

return ((pThe_Node)->pNodeData);
}
/*-----------------------------------------------------------------*/



/*-----------------------------------------------------------------*/


void    SLNKLSTPTR_SetData( SLNKLSTPTR  *pThe_list, SLSTNODEPTR *pThe_Node , void* pInData )
{
ASSERTS_PTR(  pThe_list , "SLNKLSTPTR_SetData()" );
ASSERTS_PTR(  pThe_Node , "SLNKLSTPTR_SetData()" );
ASSERTS_PTR(  pInData   , "SLNKLSTPTR_SetData()" );
ASSERTS( pThe_list->dwDbgIsInitialized == LIST_INITCODE , "SLNKLSTPTR_SetData()" );

((pThe_Node)->pNodeData = pInData);
//Warning optimized Macro version returns nothing.
return;
}
/*-----------------------------------------------------------------*/



void    DLNKLSTPTR_SetData( DLNKLSTPTR  *pThe_list, DLSTNODEPTR *pThe_Node , void* pInData )
{
ASSERTS_PTR(  pThe_list , "DLNKLSTPTR_SetData()" );
ASSERTS_PTR(  pThe_Node , "DLNKLSTPTR_SetData()" );
ASSERTS_PTR(  pInData , "DLNKLSTPTR_SetData()" );
ASSERTS( pThe_list->dwDbgIsInitialized == LIST_INITCODE , "DLNKLSTPTR_SetData()" );

((pThe_Node)->pNodeData = pInData);
//Warning optimized Macro version returns nothing.
return;
}
        

/*-----------------------------------------------------------------*/
#endif //X_DEBUG







/*------------------------------------------------------------------------------------------
 * Function: _MoveNode
 *
 * Desc    : extracts from the head node in a list the tail
 *           
 * Returns : returns the last node in a list
 *			
 ------------------------------------------------------------------------------------------*/

u32 SLNKLSTPTR_MoveNode( SLNKLSTPTR *pSrcList, SLNKLSTPTR *pDestList, u32 dwIndexOfNodeToMove, u32 dwIndexToInsert )
{

SLSTNODEPTR *pSrcCursor,*pDestCursor,*pTrailer;
u32 bFound=FALSE,dwNodeCounter=0;

 ASSERT_PTR(pSrcList);
 ASSERT_PTR(pDestList);

 ASSERT(dwIndexOfNodeToMove > 0 );
 ASSERT(dwIndexToInsert     > 0 );

	pSrcCursor = pSrcList->pFirstnode;
	pTrailer = NULL;

	/* Start searching through the Source list for the node to copy */
	while( (dwIndexOfNodeToMove > dwNodeCounter ) && (NULL != pSrcCursor) )
	{
		dwNodeCounter++;

		if(dwNodeCounter == dwIndexOfNodeToMove)
		{
			/* FOUND!! remove the node from the list */
			bFound = TRUE;

			/* are we at the head node ??? */
			if( NULL == pTrailer )
				/* yes , just go around it*/
				pSrcList->pFirstnode = pSrcCursor->pNext;
			else
				/* no, make trailer go around index */
				pTrailer->pNext = pSrcCursor->pNext;

			/* decrease the list count */
			pSrcList->dwNodeCount--;
			break;
		}
		else
		{
			/* go to the next node */
			pTrailer = pSrcCursor;
			pSrcCursor = pSrcCursor->pNext;
		}
	}

	/* if found, insert the node at the index specified , if end is encountered before index, Insert At Tail */
	if(bFound)
	{
		pDestCursor = pDestList->pFirstnode;
		
		/* reset temporary variables */
		pTrailer = NULL;
		dwNodeCounter = 0;

		/* find the appropriate index to insert the node */
		while( (dwIndexToInsert > dwNodeCounter ) && (NULL != pDestCursor) ) 
		{
			dwNodeCounter++;

			/* found the location ??? */
			if(dwNodeCounter == dwIndexToInsert)
			{
				/* are we inserting at the head? */
				if( NULL == pTrailer )
				{
					pSrcCursor->pNext = pDestCursor;
					pDestList->pFirstnode = pSrcCursor;
				}
				else
				{
					/* not inserting at head */
					pTrailer->pNext = pSrcCursor;
					pSrcCursor->pNext = pDestCursor;
				}

				/* increase the count of the nodes on the destination list */
				pDestList->dwNodeCount++;

				/* EXIT_SUCCESS */
				return (0);
			}
			else
			{
				pTrailer = pDestCursor;
				pDestCursor = pDestCursor->pNext;
			}
		}
		/* we didnt find the node by index.. Hmmmmmm */

		/* is the destination list empty or did we reach the end of the list?? */
		if(pDestList->pFirstnode == NULL)
		{
			pDestList->pFirstnode = pSrcCursor;
			pSrcCursor->pNext = NULL;
		}
		else
		{
			pTrailer->pNext = pSrcCursor;
			pSrcCursor->pNext = NULL;
		}

		/* increase the count of the nodes on the destination list */
		pDestList->dwNodeCount++;
		
		/* EXIT_SUCCESS */
		return (0);
	}
	else
	{
		A_DError("ERROR:: Couldn't move the node!! [SLNKLSTPTR_MoveNode()]\n");
		/* EXIT_FAILURE */
		return(1);
	}
}


u32 SLNKLSTPTR_FindNodeIndex(SLNKLSTPTR *pTheList ,void *pTheData )
{
SLSTNODEPTR *pCursor;
u32 dwCount = 0;

ASSERT_PTR(pTheList);
ASSERT_PTR(pTheData);

	pCursor = pTheList->pFirstnode;

	while(NULL != pCursor)
	{
		dwCount++;
		if( pCursor->pNodeData == pTheData )
			return dwCount;
		else
			pCursor = pCursor->pNext;		
	}
	/* NOT FOUND */
	return(0);
}
