/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */ 
/* See dataObjOpen.h for a description of this API call.*/

#include "dataObjOpen.h"
#include "dataObjOpenAndStat.h"
#include "rodsLog.h"
#include "objMetaOpr.h"
#include "dataObjOpr.h"
#include "dataObjCreate.h"
#include "fileOpen.h"
#include "subStructFileOpen.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"
#include "reGlobalsExtern.h"
#include "reDefines.h"
#include "reDefines.h"
#include "getRemoteZoneResc.h"
#include "regReplica.h"
#include "regDataObj.h"
#include "dataObjClose.h"
#include "dataObjRepl.h"

int
rsDataObjOpen (rsComm_t *rsComm, dataObjInp_t *dataObjInp)
{
    int status, l1descInx;
    int remoteFlag;
    rodsServerHost_t *rodsServerHost;

    remoteFlag = getAndConnRemoteZone (rsComm, dataObjInp, &rodsServerHost,
      REMOTE_OPEN);

    if (remoteFlag < 0) {
        return (remoteFlag);
    } else if (remoteFlag == REMOTE_HOST) {
	openStat_t *openStat = NULL;
        status = rcDataObjOpenAndStat (rodsServerHost->conn, dataObjInp,
	  &openStat);
        if (status < 0) return status;
        l1descInx = allocAndSetL1descForZoneOpr (status, dataObjInp,
	  rodsServerHost, openStat);
	if (openStat != NULL) free (openStat);
        return (l1descInx);
    } else {
        l1descInx = _rsDataObjOpen (rsComm, dataObjInp);
    }

    return (l1descInx);
}

/* _rsDataObjOpen - handle internal server dataObj open request.
 * valid phyOpenFlag are DO_PHYOPEN, DO_NOT_PHYOPEN and PHYOPEN_BY_SIZE
 *
 * return value - 0-2 - success but did not phy open
 *                > 2 - success with phy open
 *                < 0 - failure
 */
 
int
_rsDataObjOpen (rsComm_t *rsComm, dataObjInp_t *dataObjInp)
{
    int status;
    dataObjInfo_t *dataObjInfoHead = NULL;
    dataObjInfo_t *otherDataObjInfo = NULL;
    dataObjInfo_t *nextDataObjInfo = NULL;
    dataObjInfo_t *tmpDataObjInfo; 
    int l1descInx;
    int writeFlag;
    int rescClass;
    int phyOpenFlag = DO_PHYOPEN;

    if (getValByKey (&dataObjInp->condInput, NO_OPEN_FLAG_KW) != NULL) {
	phyOpenFlag = DO_NOT_PHYOPEN;
    } else if (getValByKey (&dataObjInp->condInput, PHYOPEN_BY_SIZE_KW) 
      != NULL) {
	phyOpenFlag = PHYOPEN_BY_SIZE;
    }

    /* query rcat for dataObjInfo and sort it */

    status = getDataObjInfoIncSpecColl (rsComm, dataObjInp, &dataObjInfoHead);
   
    writeFlag = getWriteFlag (dataObjInp->openFlags);

    if (status < 0) {
        if (dataObjInp->openFlags & O_CREAT && writeFlag > 0) {
            l1descInx = rsDataObjCreate (rsComm, dataObjInp);
            return (l1descInx);
        } else {
	    return (status);
	}
    } else {
        /* screen out any stale copies */
        sortObjInfoForOpen (&dataObjInfoHead, &dataObjInp->condInput,
         writeFlag);

        status = applyPreprocRuleForOpen (rsComm, dataObjInp, &dataObjInfoHead);
        if (status < 0) return status;
    }

    if (writeFlag > 0) {
	/* put the copy with destResc on top */
	status = requeDataObjInfoByDestResc (&dataObjInfoHead, 
	  &dataObjInp->condInput, writeFlag, 1);
	/* status < 0 means there is no copy in the DEST_RESC */
	if (status < 0 && getStructFileType (dataObjInfoHead->specColl) < 0 &&
	  getValByKey (&dataObjInp->condInput, DEST_RESC_NAME_KW) != NULL) {
	    /* we don't have a copy in the DEST_RESC_NAME */
	    status = createEmptyRepl (rsComm, dataObjInp, &dataObjInfoHead);
	    if (status < 0) {
                rodsLog (LOG_ERROR,
                  "_rsDataObjOpen: createEmptyRepl of %s failed, status = %d",
          	  dataObjInfoHead->objPath, status);
                freeAllDataObjInfo (dataObjInfoHead);
		return status;
	    }
	}
    }

    rescClass = getRescClass (dataObjInfoHead->rescInfo);
    if (rescClass == COMPOUND_CL) {
#if 0	/* not sure we need this */
	if (writeFlag > 0 && 
	  getValByKey (&dataObjInp->condInput, FORCE_FLAG_KW) == NULL) {
            rodsLog (LOG_ERROR,
              "_rsDataObjOpen: %s - cannot directly write iinto COMPOUND resc",
              dataObjInfoHead->objPath);
            freeAllDataObjInfo (dataObjInfoHead);
            return SYS_CANT_DIRECTLY_ACC_COMPOUND_RESC;
	}
#endif
	status = stageAndRequeDataToCache (rsComm, &dataObjInfoHead);
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "_rsDataObjOpen: stageAndRequeDataToCache of %s failed stat=%d",
              dataObjInfoHead->objPath, status);
            freeAllDataObjInfo (dataObjInfoHead);
            return status;
        }
    } else if (rescClass == BUNDLE_CL) {
	status = stageBundledData (rsComm, &dataObjInfoHead);
        if (status < 0) {
            rodsLog (LOG_ERROR,
              "_rsDataObjOpen: stageBundledData of %s failed stat=%d",
              dataObjInfoHead->objPath, status);
            freeAllDataObjInfo (dataObjInfoHead);
            return status;
        }
    }

    tmpDataObjInfo = dataObjInfoHead;
    while (tmpDataObjInfo != NULL) {
        nextDataObjInfo = tmpDataObjInfo->next;
        tmpDataObjInfo->next = NULL;
	status = l1descInx = _rsDataObjOpenWithObjInfo (rsComm, dataObjInp,
	  phyOpenFlag, tmpDataObjInfo);

        if (status >= 0) {
	    /* copiesNeeded condition met */
            queDataObjInfo (&otherDataObjInfo, nextDataObjInfo, 1, 1);
            L1desc[l1descInx].otherDataObjInfo = otherDataObjInfo;
	    if (writeFlag > 0) {
	        L1desc[l1descInx].openType = OPEN_FOR_WRITE_TYPE;
	    } else {
               L1desc[l1descInx].openType = OPEN_FOR_READ_TYPE;
	    };
            return (l1descInx);
	}
        tmpDataObjInfo = nextDataObjInfo;
    }
    freeAllDataObjInfo (otherDataObjInfo);

    return (status);
}

/* _rsDataObjOpenWithObjInfo - given a dataObjInfo, open a single copy
 * of the data object.
 *
 * return l1descInx
 */

int
_rsDataObjOpenWithObjInfo (rsComm_t *rsComm, dataObjInp_t *dataObjInp, 
int phyOpenFlag, dataObjInfo_t *dataObjInfo)
{
    int replStatus;
    int status;
    int l1descInx;

    l1descInx = allocL1desc ();

    if (l1descInx < 0) return l1descInx;

    replStatus = dataObjInfo->replStatus | OPEN_EXISTING_COPY;

    /* the size was set to -1 because we don't know the target size.
     * For copy and replicate, the calling routine should modify this
     * dataSize */
    fillL1desc (l1descInx, dataObjInp, dataObjInfo, replStatus, -1);
    if (phyOpenFlag == DO_NOT_PHYOPEN) {
        /* don't actually physically open the file */
        status = 0;
    } else if (phyOpenFlag == PHYOPEN_BY_SIZE) {
        /* open for put or get. May do "dataInclude" */
        if (getValByKey (&dataObjInp->condInput, DATA_INCLUDED_KW) != NULL
          && dataObjInfo->dataSize <= MAX_SZ_FOR_SINGLE_BUF) {
            status = 0;
        } else if (dataObjInfo->dataSize < MAX_SZ_FOR_SINGLE_BUF) {
            status = 0;
        } else {
            status = dataOpen (rsComm, l1descInx);
        }
    } else {
        status = dataOpen (rsComm, l1descInx);
    }

    if (status < 0) {
        freeL1desc (l1descInx);
	return (status);
    } else {
        return (l1descInx);
    }
}

int
dataOpen (rsComm_t *rsComm, int l1descInx)
{
    dataObjInfo_t *myDataObjInfo = L1desc[l1descInx].dataObjInfo;
    int status;


    status = l3Open (rsComm, l1descInx);

    if (status <= 0) {
        rodsLog (LOG_NOTICE,
          "dataOpen: l3Open of %s failed, status = %d",
          myDataObjInfo->filePath, status);
        return (status);
    } else {
        L1desc[l1descInx].l3descInx = status;
	return (0);
    }
}

int
l3Open (rsComm_t *rsComm, int l1descInx)
{
    dataObjInfo_t *dataObjInfo;
    int l3descInx;
    int mode, flags;

    dataObjInfo = L1desc[l1descInx].dataObjInfo;

    if (getStructFileType (dataObjInfo->specColl) >= 0) {
	subFile_t subFile;
	memset (&subFile, 0, sizeof (subFile));
	rstrcpy (subFile.subFilePath, dataObjInfo->subPath,
	  MAX_NAME_LEN);
        rstrcpy (subFile.addr.hostAddr, dataObjInfo->rescInfo->rescLoc,
          NAME_LEN);
	subFile.specColl = dataObjInfo->specColl;
        subFile.mode = getFileMode (L1desc[l1descInx].dataObjInp);
        subFile.flags = getFileFlags (l1descInx);
	l3descInx = rsSubStructFileOpen (rsComm, &subFile); 
    } else {
        mode = getFileMode (L1desc[l1descInx].dataObjInp);
        flags = getFileFlags (l1descInx);
	l3descInx = _l3Open (rsComm, dataObjInfo, mode, flags);
    }
    return (l3descInx);
}

int
_l3Open (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo, int mode, int flags)
{
    int rescTypeInx;
    int l3descInx;
    fileOpenInp_t fileOpenInp;

    rescTypeInx = dataObjInfo->rescInfo->rescTypeInx;

    switch (RescTypeDef[rescTypeInx].rescCat) {
      case FILE_CAT:
        memset (&fileOpenInp, 0, sizeof (fileOpenInp));
        fileOpenInp.fileType = RescTypeDef[rescTypeInx].driverType;
        rstrcpy (fileOpenInp.addr.hostAddr,  dataObjInfo->rescInfo->rescLoc,
          NAME_LEN);
        rstrcpy (fileOpenInp.fileName, dataObjInfo->filePath, MAX_NAME_LEN);
        fileOpenInp.mode = mode;
        fileOpenInp.flags = flags;
        l3descInx = rsFileOpen (rsComm, &fileOpenInp);
        break;
      default:
        rodsLog (LOG_NOTICE,
          "l3Open: rescCat type %d is not recognized",
          RescTypeDef[rescTypeInx].rescCat);
        l3descInx = SYS_INVALID_RESC_TYPE;
        break;
    }
    return (l3descInx);
}

/* l3OpenByHost - call level3 open - this differs from l3Open in that 
 * rsFileOpenByHost is called instead of rsFileOpen
 */

int 
l3OpenByHost (rsComm_t *rsComm, int rescTypeInx, int l3descInx, int flags)
{
    fileOpenInp_t fileOpenInp;
    int newL3descInx;

    switch (RescTypeDef[rescTypeInx].rescCat) {
      case FILE_CAT:
        memset (&fileOpenInp, 0, sizeof (fileOpenInp));
        fileOpenInp.fileType = RescTypeDef[rescTypeInx].driverType;
        rstrcpy (fileOpenInp.fileName, FileDesc[l3descInx].fileName, 
	  MAX_NAME_LEN);
        fileOpenInp.mode = FileDesc[l3descInx].mode;
        fileOpenInp.flags = flags;
        newL3descInx = rsFileOpenByHost (rsComm, &fileOpenInp, 
	  FileDesc[l3descInx].rodsServerHost);
        break;
      default:
        rodsLog (LOG_NOTICE,
          "l3OpenByHost: rescCat type %d is not recognized",
          RescTypeDef[rescTypeInx].rescCat);
        l3descInx = SYS_INVALID_RESC_TYPE;
        break;
    }
    return (newL3descInx);
}

int
applyPreprocRuleForOpen (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
dataObjInfo_t **dataObjInfoHead)
{
    int status;
    ruleExecInfo_t rei;

    initReiWithDataObjInp (&rei, rsComm, dataObjInp);
    rei.doi = *dataObjInfoHead;

    status = applyRule ("acPreprocForDataObjOpen", NULL, &rei, NO_SAVE_REI);

    if (status < 0) {
        if (rei.status < 0) {
            status = rei.status;
        }
        rodsLog (LOG_ERROR,
         "applyPreprocRuleForOpen:acPreprocForDataObjOpen error for %s,stat=%d",
          dataObjInp->objPath, status);
    } else {
        *dataObjInfoHead = rei.doi;
    }
    return (status);
}

int
createEmptyRepl (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
dataObjInfo_t **dataObjInfoHead)
{
    int status;
    char *rescName;
    rescInfo_t *rescInfo;
    rescGrpInfo_t *tmpRescGrpInfo;
    regReplica_t regReplicaInp;
    rescGrpInfo_t *myRescGrpInfo = NULL;
    keyValPair_t *condInput = &dataObjInp->condInput;
    dataObjInfo_t *myDataObjInfo;

    if ((rescName = getValByKey (condInput, DEST_RESC_NAME_KW)) == NULL &&
      (rescName = getValByKey (condInput, BACKUP_RESC_NAME_KW)) == NULL &&
      (rescName = getValByKey (condInput, DEF_RESC_NAME_KW)) == NULL) {
	return USER_NO_RESC_INPUT_ERR;
    }

    status = getRescGrpForCreate (rsComm, dataObjInp, &myRescGrpInfo);
    if (status < 0) return status;

    myDataObjInfo = malloc (sizeof (dataObjInfo_t));
    *myDataObjInfo = *(*dataObjInfoHead);
    tmpRescGrpInfo = myRescGrpInfo;
    while (tmpRescGrpInfo != NULL) {
	rescInfo = tmpRescGrpInfo->rescInfo;
        myDataObjInfo->rescInfo = rescInfo;
        rstrcpy (myDataObjInfo->rescName, rescInfo->rescName, NAME_LEN);
        rstrcpy (myDataObjInfo->rescGroupName, myRescGrpInfo->rescGroupName, 
	  NAME_LEN);
        status = getFilePathName (rsComm, myDataObjInfo, dataObjInp);
	if (status < 0) {
	    myRescGrpInfo = myRescGrpInfo->next;
	    continue;
	}
	status = l3CreateByObjInfo (rsComm, dataObjInp, myDataObjInfo);
	if (status < 0) {
            myRescGrpInfo = myRescGrpInfo->next;
            continue;
        }
	/* close it */
	_l3Close (rsComm, rescInfo->rescTypeInx, status);

	/* register the replica */
        memset (&regReplicaInp, 0, sizeof (regReplicaInp));
        regReplicaInp.srcDataObjInfo = *dataObjInfoHead;
        regReplicaInp.destDataObjInfo = myDataObjInfo;
        if (getValByKey (&dataObjInp->condInput, IRODS_ADMIN_KW) != NULL) {
            addKeyVal (&regReplicaInp.condInput, IRODS_ADMIN_KW, "");
        }
        status = rsRegReplica (rsComm, &regReplicaInp);
        clearKeyVal (&regReplicaInp.condInput);

	break;
    }

    freeAllRescGrpInfo (myRescGrpInfo);

    if (status < 0) {
	free (myDataObjInfo);
    } else {
	/* queue it on top */
	myDataObjInfo->next = *dataObjInfoHead;
	*dataObjInfoHead = myDataObjInfo;
    }
    return status;
}

