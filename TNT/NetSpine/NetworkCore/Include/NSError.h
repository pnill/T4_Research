////////////////////////////////////////////////////////////////////////////
//
//  NetSpine Brand Technology, Property of Acclaim Entertainment.
//	Trademark and Patent Pending 2001.
//
////////////////////////////////////////////////////////////////////////////

/**
* \file     NSError.h
* \brief    Definition of all NetSpine error codes.
* \author   NetSpine Online Developer
* \version  1.0
* \date     5/14/2001
*/


#ifndef __NSERROR_H__
#define __NSERROR_H__
/*-----------------------------------------------------------------*/


#ifdef __cplusplus
extern "C" {
#endif


/**
*  \typedef u32 NS_RETURN ;
*  \brief All public netspine functions return this data type
*/

typedef u32 NS_RETURN ;

#define     UID_ERROR_USERNAME_BADWORD          (1)
#define     UID_ERROR_USERNAME_DUPLICATE        (2)
#define     UID_ERROR_USERNAME_TOO_SHORT        (3)
#define     UID_ERROR_PASSWORD_TOO_SHORT        (4)
#define     UID_ERROR_EMAIL_TOO_SHORT           (5)
#define     UID_ERROR_USERNAME_TOO_LONG         (6)
#define     UID_ERROR_PASSWORD_TOO_LONG         (7)
#define     UID_ERROR_EMAIL_TOO_LONG            (8)
#define     UID_ERROR_USERNAME_NOT_FOUND        (9)
#define     UID_ERROR_PASSWORD_NOT_FOUND        (10)
#define     UID_ERROR_FIELD_NOT_FOUND           (11)
#define     UID_ERROR_FIELD_DATA_INVALID        (12)
#define     UID_ERROR_NO_EXTENDED_REG_INFO      (13)
#define     UID_ERROR_NO_GAME_SPECIFIC_INFO     (14)
#define     UID_ERROR_ACCOUNT_SUSPENDED         (15)
#define     UID_ERROR_ACCOUNT_REVOKED           (16)
#define     UID_ERROR_GAMENAME_NOT_FOUND        (17)
#define     UID_ERROR_INVALID_EMAIL_FORMAT      (18)

//@{
/** Return values shared by all Netspine services */
#define NS_SUCCESS              (0)
#define NS_ERROR                (1)
#define NS_ALREADY_INITIALIZED  (2)
#define NS_NOT_INITIALIZED      (3)
#define NS_INVALID_PARAMS       (4)
#define NS_NOT_CONNECTED        (5)
#define NS_MEMORY_ERROR         (6)
#define NS_NOT_IMPLEMENTED      (7)
#define NS_ALREADY_CONNECTED    (8)
#define NS_WOULDBLOCK           (9)
//@}

//@{
/** Common return values concerning token authentication */
#define NS_TOKEN_EXPIRED        (10)
#define NS_TOKEN_INVALID        (11)
//@}

//@{
/** Common return values concerning file transfers */
#define NS_FILE_NOT_FOUND       (12)
#define NS_FILE_LIST_EMPTY      (13)
#define NS_INVALID_FILE_FORMAT  (14)

#define NS_CONNECT_REFUSED      (15)
#define NS_TIMED_OUT		    (16)
//@}

//@{
/** Common return values concerning devices */
#define NS_DEVICE_INIT_ERROR        (17)
#define NS_DEVICE_INTERNAL_ERROR    (18)
#define NS_DEVICE_CONFIG_ERROR      (19)
#define NS_DEVICE_NOT_DETECTED      (20)
#define NS_DEVICE_LAST              (21)
//@}

//@{
/** TODO: Comments */
#define NS_VERSION_ERROR                    (22)
#define NS_SECURE_CONNECTION_DENIED         (23)
#define NS_SECURE_CONNECTION_ACCEPTED       (24)
#define NS_SECURE_CONNECTION_NOT_AVAILABLE  (25)
//@}

//@{
/** TODO: Comments */
#define NS_TICK_DONE                        (26)
#define NS_THREADS_NOT_SUPPORTED            (27)
//@}

#define NS_CONNECTION_INPROGRESS            (28)
#define NS_CONNECTION_REFUSED               (29)
#define NS_DESTINATION_UNREACHABLE          (30)
#define NS_CONNECTION_ABORTED               (31)
#define NS_CONNECTION_RESET                 (32)

#define NS_INVALID_HEADER                   (33)

/** \brief As new errors are added, offset must be changed */
//#define NS_INTERNAL_LAST_RETURN     (NS_DEVICE_LAST+1)

/** \brief Error base for internal components */
#define NS_INTERNAL_LAST_RETURN     (0x00000100) //

/** \brief Error base for Application */
#define NS_LAST_RETURN              (0x00010000)

#define NS_IS_SUCCESS( error )  ( (NS_SUCCESS == (error)) ? TRUE : FALSE)
#define NS_IS_ERROR( error )    (!(NS_IS_SUCCESS( (error) )))
#define NS_IS_DEVICE_ERROR( error ) ( (NS_DEVICE_INIT_ERROR >= error) && (NS_DEVICE_LAST <= error) )


/*========================================================================
 * Function : XErrnoToNSError()
 * Date     : 9/6/2002
 * Added by : Daniel Melfi
 */
u32 XErrnoToNSError( u32 err );

/*-----------------------------------------------------------------*/
#ifdef __cplusplus
	}
#endif
/*-----------------------------------------------------------------*/



#endif //__NSERROR_H__
