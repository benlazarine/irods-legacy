/**
 * @file  rcDataObjPut.c
 *
 */

/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code.  */ 
/* See dataObjPut.h for a description of this API call.*/

#include "dataObjPut.h"
#include "rcPortalOpr.h"
#include "oprComplete.h"

/**
 * \fn rcDataObjPut (rcComm_t *conn, dataObjInp_t *dataObjInp, 
 *   char *locFilePath)
 *
 * \brief Put (upload) a data object to iRODS. 
 *
 * \user client
 *
 * \category data object operations
 *
 * \since 1.0
 *
 * \author  Mike Wan
 * \date    2007
 *
 * \remark none
 *
 * \note none
 *
 * \usage
 * Put (upload) a data object /myZone/home/john/myfile in myRescource:
 * \n dataObjInp_t dataObjInp;
 * \n char locFilePath[MAX_NAME_LEN];
 * \n bzero (&dataObjInp, sizeof (dataObjInp));
 * \n rstrcpy (dataObjInp.objPath, "/myZone/home/john/myfile", MAX_NAME_LEN);
 * \n rstrcpy (locFilePath, "./mylocalfile", MAX_NAME_LEN);
 * \n dataObjInp.createMode = 0750;
 * \n dataObjInp.dataSize = 12345;
 * \n addKeyVal (&dataObjInp.condInput, DEST_RESC_NAME_KW, "myRescource");
 * \n status = rcDataObjPut (conn, &dataObjInp, locFilePath);
 * \n if (status < 0) {
 * \n .... handle the error
 * \n }
 *
 * \param[in] conn - A rcComm_t connection handle to the server.
 * \param[in] dataObjInp - Elements of dataObjInp_t used :
 *    \li char \b objPath[MAX_NAME_LEN] - full path of the data object.
 *    \li int \b createMode - the file mode of the data object.
 *    \li rodsLong_t \b dataSize - the size of the data object.
 *      Input 0 if not known.
 *    \li int \b numThreads - the number of threads to use. Valid values are:
 *	\n NO_THREADING (-1) - no multi-thread
 *	\n 0 - the server will decide the number of threads. 
 *	  (recommanded setting).
 *	\n A positive integer - specifies the number of threads.
 *    \li keyValPair_t \b condInput - keyword/value pair input. Valid keywords:
 *    \n DATA_TYPE_KW - the data type of the data object.
 *    \n DEST_RESC_NAME_KW - The resource to store this data object
 *    \n FILE_PATH_KW - The physical file path for this data object if the
 *             normal resource vault is not used.
 *    \n FORCE_FLAG_KW - overwrite existing copy. This keyWd has no value
 *    \n ALL_KW - upload to all resources in the resource group if the
 *             input resource (via DEST_RESC_NAME_KW) is a resource group. 
 *            This keyWd has no value.
 *    \n REPL_NUM_KW - If the data object already exist, the replica number
 *            of the copy to overwrite.
 *    \n REG_CHKSUM_KW -  register the target checksum value after the copy.
 *            The value is the md5 checksum value of the local file.
 *    \n VERIFY_CHKSUM_KW - verify and register the target checksum value
 *            after the copy. The value is the md5 checksum value of the 
 *	      local file.
 *    \n RBUDP_TRANSFER_KW - use RBUDP for data transfer. This keyWd has no
 *             value.
 *    \n RBUDP_SEND_RATE_KW - the number of RBUDP packet to send per second
 *          The default is 600000.
 *    \n RBUDP_PACK_SIZE_KW - the size of RBUDP packet. The default is 8192.
 *    \n LOCK_TYPE_KW - set advisory lock type. valid value - WRITE_LOCK_TYPE.
 * \param[in] locFilePath - the path of the local file to upload. This path
 *           can be a relative path.
 *
 * \return integer
 * \retval 0 on success

 * \sideeffect none
 * \pre none
 * \post none
 * \sa none
 * \bug  no known bugs
**/

int
rcDataObjPut (rcComm_t *conn, dataObjInp_t *dataObjInp, char *locFilePath)
{
    int status;
    portalOprOut_t *portalOprOut = NULL;
    bytesBuf_t dataObjInpBBuf;

    if (dataObjInp->dataSize <= 0) {
	dataObjInp->dataSize = getFileSize (locFilePath);
	if (dataObjInp->dataSize < 0) {
	    return (USER_FILE_DOES_NOT_EXIST);
	}
    }

    memset (&conn->transStat, 0, sizeof (transStat_t));
    memset (&dataObjInpBBuf, 0, sizeof (dataObjInpBBuf));

    if (getValByKey (&dataObjInp->condInput, DATA_INCLUDED_KW) != NULL) {
	if (dataObjInp->dataSize > MAX_SZ_FOR_SINGLE_BUF) {
	    rmKeyVal (&dataObjInp->condInput, DATA_INCLUDED_KW);
	} else {
	    status = fillBBufWithFile (conn, &dataObjInpBBuf, locFilePath, 
	      dataObjInp->dataSize);
	    if (status < 0) {
	        rodsLog (LOG_NOTICE,
	          "rcDataObjPut: fileBBufWithFile error for %s", locFilePath);
	        return (status);
	    }
	}
    } else if (dataObjInp->dataSize < MAX_SZ_FOR_SINGLE_BUF) {
	addKeyVal (&dataObjInp->condInput, DATA_INCLUDED_KW, "");
        status = fillBBufWithFile (conn, &dataObjInpBBuf, locFilePath,
	  dataObjInp->dataSize);
        if (status < 0) {
            rodsLog (LOG_NOTICE,
              "rcDataObjPut: fileBBufWithFile error for %s", locFilePath);
            return (status);
	}
    }
    
    dataObjInp->oprType = PUT_OPR;

#ifndef PARA_OPR
    addKeyVal (&dataObjInp->condInput, NO_PARA_OP_KW, "");
#endif

    status = _rcDataObjPut (conn, dataObjInp, &dataObjInpBBuf, &portalOprOut);

    clearBBuf (&dataObjInpBBuf);
 
    if (status < 0 || 
      getValByKey (&dataObjInp->condInput, DATA_INCLUDED_KW) != NULL) {
	if (portalOprOut != NULL)
	    free (portalOprOut);
	return (status);
    }

    if (portalOprOut->numThreads <= 0) { 
	status = putFile (conn, portalOprOut->l1descInx, 
	  locFilePath, dataObjInp->objPath, dataObjInp->dataSize);
#ifdef RBUDP_TRANSFER
    } else if (getUdpPortFromPortList (&portalOprOut->portList) != 0) {
	int veryVerbose;
	/* rbudp transfer */
        /* some sanity check */
        if (portalOprOut->numThreads != 1) {
            rcOprComplete (conn, SYS_INVALID_PORTAL_OPR);
            free (portalOprOut);
            return (SYS_INVALID_PORTAL_OPR);
        }
        conn->transStat.numThreads = portalOprOut->numThreads;
	if (getValByKey (&dataObjInp->condInput, VERY_VERBOSE_KW) != NULL) {
	    printf ("From server: NumThreads=%d, addr:%s, port:%d, cookie=%d\n",
	      portalOprOut->numThreads, portalOprOut->portList.hostAddr,
	      portalOprOut->portList.portNum, portalOprOut->portList.cookie);
	    veryVerbose = 2;
	} else {
	    veryVerbose = 0;
	}
        status = putFileToPortalRbudp (portalOprOut, locFilePath, 
          dataObjInp->objPath, -1, dataObjInp->dataSize, veryVerbose, 0, 0);
#endif  /* RBUDP_TRANSFER */
    } else {
        if (getValByKey (&dataObjInp->condInput, VERY_VERBOSE_KW) != NULL) {
            printf ("From server: NumThreads=%d, addr:%s, port:%d, cookie=%d\n",
              portalOprOut->numThreads, portalOprOut->portList.hostAddr,
              portalOprOut->portList.portNum, portalOprOut->portList.cookie);
	}
	/* some sanity check */
	if (portalOprOut->numThreads >= 20 * DEF_NUM_TRAN_THR) {
    	    rcOprComplete (conn, SYS_INVALID_PORTAL_OPR);
    	    free (portalOprOut);
	    return (SYS_INVALID_PORTAL_OPR);
	}
	conn->transStat.numThreads = portalOprOut->numThreads;
        status = putFileToPortal (conn, portalOprOut, locFilePath, 
	  dataObjInp->objPath, dataObjInp->dataSize);
    }

    /* just send a complete msg */
    if (status < 0) {
	rcOprComplete (conn, status);
    } else {
        status = rcOprComplete (conn, portalOprOut->l1descInx);
    }
    free (portalOprOut);

    if (status >= 0 && conn->fileRestart.info.numSeg > 0) {   /* file restart */
        clearLfRestartFile (&conn->fileRestart);
    }
    return (status);
}

int
_rcDataObjPut (rcComm_t *conn, dataObjInp_t *dataObjInp,
bytesBuf_t *dataObjInpBBuf, portalOprOut_t **portalOprOut)
{
    int status;

    status = procApiRequest (conn, DATA_OBJ_PUT_AN,  dataObjInp,
        dataObjInpBBuf, (void **) portalOprOut, NULL);

    if (*portalOprOut != NULL && (*portalOprOut)->l1descInx < 0) {
        status = (*portalOprOut)->l1descInx;
    }

    return status;
}

