/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* objDesc.c - L1 type operation. Will call low level l1desc drivers
 */

#include "rodsDef.h"
#include "objDesc.h"
#include "dataObjOpr.h"
#include "rodsDef.h"
#include "rsGlobalExtern.h"
#include "fileChksum.h"
#include "modDataObjMeta.h"
#include "objMetaOpr.h"
#include "collection.h"
#include "resource.h"
#include "dataObjClose.h"
#include "rcGlobalExtern.h"
#include "reGlobalsExtern.h"
#include "reDefines.h"
#include "reSysDataObjOpr.h"
#include "genQuery.h"
#include "rodsClient.h"
#ifdef LOG_TRANSFERS
#include <sys/time.h>
#endif

int
initL1desc ()
{
    memset (L1desc, 0, sizeof (l1desc_t) * NUM_L1_DESC);
    return (0);
}

int
allocL1desc ()
{
    int i;

    for (i = 3; i < NUM_L1_DESC; i++) {
        if (L1desc[i].inuseFlag <= FD_FREE) {
            L1desc[i].inuseFlag = FD_INUSE;
            return (i);
        };
    }

    rodsLog (LOG_NOTICE,
     "allocL1desc: out of L1desc");

    return (SYS_OUT_OF_FILE_DESC);
}

int
isL1descInuse ()
{
    int i;

    for (i = 3; i < NUM_L1_DESC; i++) {
        if (L1desc[i].inuseFlag == FD_INUSE) {
	    return 1;
        };
    }
    return 0;
}

int
initSpecCollDesc ()
{
    memset (SpecCollDesc, 0, sizeof (specCollDesc_t) * NUM_SPEC_COLL_DESC);
    return (0);
}

int
allocSpecCollDesc ()
{
    int i;

    for (i = 1; i < NUM_SPEC_COLL_DESC; i++) {
        if (SpecCollDesc[i].inuseFlag <= FD_FREE) {
            SpecCollDesc[i].inuseFlag = FD_INUSE;
            return (i);
        };
    }

    rodsLog (LOG_NOTICE,
     "allocSpecCollDesc: out of SpecCollDesc");

    return (SYS_OUT_OF_FILE_DESC);
}

int
freeSpecCollDesc (int specCollInx)
{
    if (specCollInx < 1 || specCollInx >= NUM_SPEC_COLL_DESC) {
        rodsLog (LOG_NOTICE,
         "freeSpecCollDesc: specCollInx %d out of range", specCollInx);
        return (SYS_FILE_DESC_OUT_OF_RANGE);
    }

    if (SpecCollDesc[specCollInx].dataObjInfo != NULL) {
        freeDataObjInfo (SpecCollDesc[specCollInx].dataObjInfo);
    }

    memset (&SpecCollDesc[specCollInx], 0, sizeof (specCollDesc_t));

    return (0);
}

int
closeAllL1desc (rsComm_t *rsComm)
{
    int i;

    if (rsComm == NULL) {
	return 0;
    }
    for (i = 3; i < NUM_L1_DESC; i++) {
        if (L1desc[i].inuseFlag == FD_INUSE && 
	  L1desc[i].l3descInx > 2) {
	    l3Close (rsComm, i);
	}
    }
    return (0);
}

int
freeL1desc (int l1descInx)
{
    if (l1descInx < 3 || l1descInx >= NUM_L1_DESC) {
        rodsLog (LOG_NOTICE,
         "freeL1desc: l1descInx %d out of range", l1descInx);
        return (SYS_FILE_DESC_OUT_OF_RANGE);
    }

    if (L1desc[l1descInx].dataObjInfo != NULL) {
	/* for remote zone type L1desc, rescInfo is not from local cache
	 * but malloc'ed */ 
	if (L1desc[l1descInx].remoteZoneHost != NULL &&
	  L1desc[l1descInx].dataObjInfo->rescInfo != NULL)
	    free (L1desc[l1descInx].dataObjInfo->rescInfo);
#if 0	/* no longer need this with irsDataObjClose */
	/* will be freed in _rsDataObjReplS since it needs the new 
	 * replNum and dataID */ 
	if (L1desc[l1descInx].oprType != REPLICATE_DEST)
            freeDataObjInfo (L1desc[l1descInx].dataObjInfo);
#endif
	if (L1desc[l1descInx].dataObjInfo != NULL)
	    freeDataObjInfo (L1desc[l1descInx].dataObjInfo);
    }

    if (L1desc[l1descInx].otherDataObjInfo != NULL) {
        freeAllDataObjInfo (L1desc[l1descInx].otherDataObjInfo);
    }

    if (L1desc[l1descInx].replDataObjInfo != NULL) {
        freeDataObjInfo (L1desc[l1descInx].replDataObjInfo);
    }

    if (L1desc[l1descInx].dataObjInpReplFlag == 1 &&
      L1desc[l1descInx].dataObjInp != NULL) {
	clearDataObjInp (L1desc[l1descInx].dataObjInp);
	free (L1desc[l1descInx].dataObjInp);
    }
    memset (&L1desc[l1descInx], 0, sizeof (l1desc_t));

    return (0);
}

int
fillL1desc (int l1descInx, dataObjInp_t *dataObjInp,
dataObjInfo_t *dataObjInfo, int replStatus, rodsLong_t dataSize)
{
    keyValPair_t *condInput;
    char *tmpPtr;

    condInput = &dataObjInp->condInput;

    if (dataObjInp != NULL) { 
#if 0
        if (getValByKey (&dataObjInp->condInput, REPL_DATA_OBJ_INP_KW) != 
          NULL) {
	    L1desc[l1descInx].dataObjInp = malloc (sizeof (dataObjInp_t));
	    replDataObjInp (dataObjInp, L1desc[l1descInx].dataObjInp);
	    L1desc[l1descInx].dataObjInpReplFlag = 1;
	} else {
	    L1desc[l1descInx].dataObjInp = dataObjInp;
	}
#else
        /* always repl the .dataObjInp */
        L1desc[l1descInx].dataObjInp = malloc (sizeof (dataObjInp_t));
        replDataObjInp (dataObjInp, L1desc[l1descInx].dataObjInp);
        L1desc[l1descInx].dataObjInpReplFlag = 1;
#endif
    } else {
	/* XXXX this can be a problem in rsDataObjClose */
	L1desc[l1descInx].dataObjInp = NULL; 
    }
 
    L1desc[l1descInx].dataObjInfo = dataObjInfo;
    if (dataObjInp != NULL) {
	L1desc[l1descInx].oprType = dataObjInp->oprType;
    }
    L1desc[l1descInx].replStatus = replStatus;
    L1desc[l1descInx].dataSize = dataSize;
    if (condInput != NULL && condInput->len > 0) {
	if ((tmpPtr = getValByKey (condInput, REG_CHKSUM_KW)) != NULL) {
	    L1desc[l1descInx].chksumFlag = REG_CHKSUM;
	    rstrcpy (L1desc[l1descInx].chksum, tmpPtr, NAME_LEN);
	} else if ((tmpPtr = getValByKey (condInput, VERIFY_CHKSUM_KW)) != 
	  NULL) {
	    L1desc[l1descInx].chksumFlag = VERIFY_CHKSUM;
	    rstrcpy (L1desc[l1descInx].chksum, tmpPtr, NAME_LEN);
	}
    }
#ifdef LOG_TRANSFERS
    (void)gettimeofday(&L1desc[l1descInx].openStartTime,
		       (struct timezone *)0);
#endif
    return (0);
}

int
initDataObjInfoWithInp (dataObjInfo_t *dataObjInfo, dataObjInp_t *dataObjInp)
{
    char *rescName, *dataType, *filePath;
    keyValPair_t *condInput;

    condInput = &dataObjInp->condInput;

    memset (dataObjInfo, 0, sizeof (dataObjInfo_t));
    rstrcpy (dataObjInfo->objPath, dataObjInp->objPath, MAX_NAME_LEN);
    rescName = getValByKey (condInput, RESC_NAME_KW);
    if (rescName != NULL) {
        rstrcpy (dataObjInfo->rescName, rescName, LONG_NAME_LEN);
    }
    snprintf (dataObjInfo->dataMode, SHORT_STR_LEN, "%d", dataObjInp->createMode);

    dataType = getValByKey (condInput, DATA_TYPE_KW);
    if (dataType != NULL) {
        rstrcpy (dataObjInfo->dataType, dataType, NAME_LEN);
    } else {
	rstrcpy (dataObjInfo->dataType, "generic", NAME_LEN);
    }

    filePath = getValByKey (condInput, FILE_PATH_KW);
    if (filePath != NULL) {
        rstrcpy (dataObjInfo->filePath, filePath, MAX_NAME_LEN);
    }

    return (0);
}

int
getFileMode (dataObjInp_t *dataObjInp)
{
    int createMode;
    int defFileMode;

    defFileMode = getDefFileMode ();
    if (dataObjInp != NULL && 
      (dataObjInp->createMode & 0110) != 0) {
	if ((defFileMode & 0070) != 0) {
	    createMode = defFileMode | 0110;
	} else {
	    createMode = defFileMode | 0100;
	}
    } else {
	createMode = defFileMode;
    }

    return (createMode);
}

int
getFileFlags (int l1descInx)
{
    int flags;

    dataObjInp_t *dataObjInp = L1desc[l1descInx].dataObjInp;

    if (dataObjInp != NULL) { 
	flags = dataObjInp->openFlags;
    } else {
        flags = O_RDONLY;
    }

    return (flags);
}

int
getFilePathName (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo,
dataObjInp_t *dataObjInp)
{
    char *filePath;
    vaultPathPolicy_t vaultPathPolicy;
    int status;

    if (dataObjInp != NULL && 
      (filePath = getValByKey (&dataObjInp->condInput, FILE_PATH_KW)) != NULL 
      && strlen (filePath) > 0) {
        rstrcpy (dataObjInfo->filePath, filePath, MAX_NAME_LEN);
	return (0);
    }

    /* Make up a physical path */ 
    if (dataObjInfo->rescInfo == NULL) {
        rodsLog (LOG_ERROR,
          "getFilePathName: rescInfo for %s not resolved", 
	  dataObjInp->objPath);
        return (SYS_INVALID_RESC_INPUT);
    }

    status = getVaultPathPolicy (rsComm, dataObjInfo, &vaultPathPolicy);
    if (status < 0) {
	return (status);
    }

    if (vaultPathPolicy.scheme == GRAFT_PATH_S) {
	status = setPathForGraftPathScheme (dataObjInp->objPath, 
	 dataObjInfo->rescInfo->rescVaultPath, vaultPathPolicy.addUserName,
	 rsComm->clientUser.userName, vaultPathPolicy.trimDirCnt, 
	  dataObjInfo->filePath);
    } else {
        status = setPathForRandomScheme (dataObjInp->objPath,
          dataObjInfo->rescInfo->rescVaultPath, rsComm->clientUser.userName,
	  dataObjInfo->filePath);
    }
    if (status < 0) {
	return (status);
    }

    return (status);
}

int
getVaultPathPolicy (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo,
vaultPathPolicy_t *outVaultPathPolicy)
{
    ruleExecInfo_t rei;
    msParam_t *msParam;
    int status;

    if (outVaultPathPolicy == NULL || dataObjInfo == NULL || rsComm == NULL) {
	rodsLog (LOG_ERROR,
	  "getVaultPathPolicy: NULL input");
	return (SYS_INTERNAL_NULL_INPUT_ERR);
    } 
    initReiWithDataObjInp (&rei, rsComm, NULL);
   
    rei.doi = dataObjInfo;
    status = applyRule ("acSetVaultPathPolicy", NULL, &rei, NO_SAVE_REI);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "getVaultPathPolicy: rule acSetVaultPathPolicy error, status = %d",
          status);
        return (status);
    }

    if ((msParam = getMsParamByLabel (&rei.inOutMsParamArray,
      VAULT_PATH_POLICY)) == NULL) {
        /* use the default */
        outVaultPathPolicy->scheme = DEF_VAULT_PATH_SCHEME;
        outVaultPathPolicy->addUserName = DEF_ADD_USER_FLAG;
        outVaultPathPolicy->trimDirCnt = DEF_TRIM_DIR_CNT;
    } else {
        *outVaultPathPolicy = *((vaultPathPolicy_t *) msParam->inOutStruct);
        clearMsParamArray (&rei.inOutMsParamArray, 1);
    }
    /* make sure trimDirCnt is <= 1 */
    if (outVaultPathPolicy->trimDirCnt > DEF_TRIM_DIR_CNT)
	outVaultPathPolicy->trimDirCnt = DEF_TRIM_DIR_CNT;

    return (0);
}

int 
setPathForRandomScheme (char *objPath, char *vaultPath, char *userName,
char *outPath)
{
    int myRandom;
    int dir1, dir2;
    char logicalCollName[MAX_NAME_LEN];
    char logicalFileName[MAX_NAME_LEN];
    int status;

    myRandom = random (); 
    dir1 = myRandom & 0xf;
    dir2 = (myRandom >> 4) & 0xf;

    status = splitPathByKey(objPath,
                           logicalCollName, logicalFileName, '/');

    if (status < 0) {
        rodsLog (LOG_ERROR,
	  "setPathForRandomScheme: splitPathByKey error for %s, status = %d",
	  outPath, status);
        return (status);
    }

    snprintf (outPath, MAX_NAME_LEN,
      "%s/%s/%d/%d/%s.%d", vaultPath, userName, dir1, dir2, 
      logicalFileName, (uint) time (NULL));
    return (0);
}

int 
setPathForGraftPathScheme (char *objPath, char *vaultPath, int addUserName,
char *userName, int trimDirCnt, char *outPath)
{
    int i;
    char *objPathPtr, *tmpPtr;
    int len;

    objPathPtr = objPath + 1;

    for (i = 0; i < trimDirCnt; i++) {
	tmpPtr = strchr (objPathPtr, '/');
	if (tmpPtr == NULL) {
            rodsLog (LOG_ERROR,
              "setPathForGraftPathScheme: objPath %s too short", objPath);
	    break;	/* just use the shorten one */
	} else {
	    /* skip over '/' */
	    objPathPtr = tmpPtr + 1;
	    /* don't skip over the trash path */
	    if (i == 0 && strncmp (objPathPtr, "trash/", 6) == 0) break; 
	}
    }

    if (addUserName > 0 && userName != NULL) {
        len = snprintf (outPath, MAX_NAME_LEN,
          "%s/%s/%s", vaultPath, userName, objPathPtr);
    } else {
        len = snprintf (outPath, MAX_NAME_LEN,
          "%s/%s", vaultPath, objPathPtr);
    }

    if (len >= MAX_NAME_LEN) {
	rodsLog (LOG_ERROR,
	  "setPathForGraftPathScheme: filePath %s too long", objPath);
	return (USER_STRLEN_TOOLONG);
    } else {
        return (0);
    }
}

/* resolveDupFilePath - try to resolve deplicate file path in the same
 * resource.
 */

int
resolveDupFilePath (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo,
dataObjInp_t *dataObjInp)
{
    char tmpStr[NAME_LEN];
    char *filePath;

    if (getSizeInVault (rsComm, dataObjInfo) == SYS_PATH_IS_NOT_A_FILE) {
	/* a dir */
	return (SYS_PATH_IS_NOT_A_FILE);
    }
    if (chkAndHandleOrphanFile (rsComm, dataObjInfo->filePath, 
     dataObjInfo->rescInfo, dataObjInfo->replStatus) >= 0) {
        /* this is an orphan file or has been renamed */
        return 0;
    }

    if (dataObjInp != NULL) {
        filePath = getValByKey (&dataObjInp->condInput, FILE_PATH_KW);
        if (filePath != NULL && strlen (filePath) > 0) {
            return -1;
	}
    }

    if (strlen (dataObjInfo->filePath) >= MAX_NAME_LEN - 3) {
        return -1;
    }

    snprintf (tmpStr, NAME_LEN, ".%d", dataObjInfo->replNum);
    strcat (dataObjInfo->filePath, tmpStr);

    return (0);
}

int
getchkPathPerm (rsComm_t *rsComm, dataObjInp_t *dataObjInp, 
dataObjInfo_t *dataObjInfo)
{
    int chkPathPerm;
    char *filePath;
    rescInfo_t *rescInfo;
    ruleExecInfo_t rei;

    if (rsComm->clientUser.authInfo.authFlag == LOCAL_PRIV_USER_AUTH) {
        return (NO_CHK_PATH_PERM);
    }

    if (dataObjInp == NULL || dataObjInfo == NULL) {
        return (NO_CHK_PATH_PERM);
    }

    rescInfo = dataObjInfo->rescInfo;

    if ((filePath = getValByKey (&dataObjInp->condInput, FILE_PATH_KW)) != NULL 
      && strlen (filePath) > 0) {
        /* the user input a path */
        if (rescInfo == NULL) {
            chkPathPerm = NO_CHK_PATH_PERM;
        } else {
    	    initReiWithDataObjInp (&rei, rsComm, dataObjInp);
	    rei.doi = dataObjInfo;
	    rei.status = CHK_PERM_FLAG;		/* default */
	    applyRule ("acNoChkFilePathPerm", NULL, &rei, NO_SAVE_REI);
	    if (rei.status == CHK_PERM_FLAG) {
                chkPathPerm = RescTypeDef[rescInfo->rescTypeInx].chkPathPerm;
	    } else {
		chkPathPerm = NO_CHK_PATH_PERM;
	    }
        }
    } else {
            chkPathPerm = NO_CHK_PATH_PERM;
    }
    return (chkPathPerm);
}

int
getErrno (int errCode)
{
    int myErrno;

    myErrno = errCode % 1000;

    return (myErrno * (-1));
}

int 
getCopiesFromCond (keyValPair_t *condInput)
{
    char *myValue;

    myValue = getValByKey (condInput, COPIES_KW);

    if (myValue == NULL) {
	return (1);
    } else if (strcmp (myValue, "all") == 0) {
	return (ALL_COPIES);
    } else {
	return (atoi (myValue));
    }
}

int
getWriteFlag (int openFlag)
{
    if (openFlag & O_WRONLY || openFlag & O_RDWR) {
	return (1);
    } else {
	return (0);
    }
}

/* getCondQuery - check whether the datObj query will be based on condition 
 * XXXXX - this routine is deplicated because the replStatus is adjusted in
 * ModDataObjMeta 
 */

int
getCondQuery (keyValPair_t *condInput)
{
    int i;

    if (condInput == NULL) {
	return (0);
    }

    for (i = 0; i < condInput->len; i++) {
        if (strcmp (condInput->keyWord[i], "regChksum") != 0 &&
	  strcmp (condInput->keyWord[i], "verifyChksum") != 0 &&
	  strcmp (condInput->keyWord[i], "copies") != 0) {
	    return (1);
	}
    }

    return (0);
}

rodsLong_t 
getSizeInVault (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo)
{
    rodsStat_t *myStat = NULL;
    int status;
    rodsLong_t mysize;

    status = l3Stat (rsComm, dataObjInfo, &myStat);

    if (status < 0) {
	rodsLog (LOG_DEBUG,
	  "getSizeInVault: l3Stat error for %s. status = %d",
	  dataObjInfo->filePath, status);
	return (status);
    } else {
        if (myStat->st_mode & S_IFDIR) {
            return ((rodsLong_t) SYS_PATH_IS_NOT_A_FILE);
        }
	mysize = myStat->st_size;
	free (myStat);
	return (mysize);
    }
}

/* dataObjChksum - this function has been replaced by procChksumForClose
 */
#if 0
int
dataObjChksum (rsComm_t *rsComm, int l1descInx, keyValPair_t *regParam)
{
    int status;

    char *chksumStr = NULL;	/* computed chksum string */
    dataObjInfo_t *dataObjInfo = L1desc[l1descInx].dataObjInfo;
    int oprType = L1desc[l1descInx].oprType;
    int srcL1descInx;
    dataObjInfo_t *srcDataObjInfo;

    if (L1desc[l1descInx].chksumFlag == VERIFY_CHKSUM) {
	status = _dataObjChksum (rsComm, dataObjInfo, &chksumStr);

	if (status < 0) {
	    return (status);
	}

	if (strlen (L1desc[l1descInx].chksum) > 0) {
	    /* from a put type operation */
	    /* verify against the input value. */
	    if (strcmp (L1desc[l1descInx].chksum, chksumStr) != 0) {
		rodsLog (LOG_NOTICE,
		 "dataObjChksum: mismach chksum for %s. input = %s, compute %s",
		  dataObjInfo->objPath,
		  L1desc[l1descInx].chksum, chksumStr);
		free (chksumStr);
		return (USER_CHKSUM_MISMATCH);
	    }
            if (strcmp (dataObjInfo->chksum, chksumStr) != 0) {
                /* not the same as in rcat */
                addKeyVal (regParam, CHKSUM_KW, chksumStr);
            }
            free (chksumStr);
            return (0);
	} else if (oprType == REPLICATE_DEST) { 
	    if (strlen (dataObjInfo->chksum) > 0) {
	        /* for replication, the chksum in dataObjInfo was duplicated */
                if (strcmp (dataObjInfo->chksum, chksumStr) != 0) {
                    rodsLog (LOG_NOTICE,
                     "dataObjChksum: mismach chksum for %s.Rcat=%s,computed %s",
                     dataObjInfo->objPath, dataObjInfo->chksum, chksumStr);
		    status = USER_CHKSUM_MISMATCH;
		} else {
		    /* not need to register because reg repl will do it */
		    status = 0;
		}
	    } else {
		/* just register it */
		addKeyVal (regParam, CHKSUM_KW, chksumStr);
		status = 0;
	    }
	    free (chksumStr);
	    return (status);
        } else if (oprType == COPY_DEST) { 
	    /* created through copy */
	    srcL1descInx = L1desc[l1descInx].srcL1descInx;
	    if (srcL1descInx <= 2) {
		/* not a valid srcL1descInx */
	        rodsLog (LOG_NOTICE,
	          "dataObjChksum: invalid srcL1descInx %d fopy copy",
		  srcL1descInx);
		/* just register it for now */
                addKeyVal (regParam, CHKSUM_KW, chksumStr);
	        free (chksumStr);
		return (0);
	    } 
	    srcDataObjInfo = L1desc[srcL1descInx].dataObjInfo;
	    
            if (strlen (srcDataObjInfo->chksum) > 0) {
                if (strcmp (srcDataObjInfo->chksum, chksumStr) != 0) {
                    rodsLog (LOG_NOTICE,
                     "dataObjChksum: mismach chksum for %s.Rcat=%s,computed %s",
                     dataObjInfo->objPath, srcDataObjInfo->chksum, chksumStr);
                     status = USER_CHKSUM_MISMATCH;
                } else {
		    addKeyVal (regParam, CHKSUM_KW, chksumStr);
                    status = 0;
                }
            } else {
                /* just register it */
                addKeyVal (regParam, CHKSUM_KW, chksumStr);
                status = 0;
            }
            free (chksumStr);
            return (status);
	} else {
	    addKeyVal (regParam, CHKSUM_KW, chksumStr);
	    free (chksumStr);
	    return (0); 
	}
    }

    /* assume REG_CHKSUM */

    if (strlen (L1desc[l1descInx].chksum) > 0) { 
        /* from a put type operation */

        if (strcmp (dataObjInfo->chksum, L1desc[l1descInx].chksum) != 0) {
            /* not the same as in rcat */
            addKeyVal (regParam, CHKSUM_KW, L1desc[l1descInx].chksum);
	}
	return (0);
    } else if (oprType == COPY_DEST) {
        /* created through copy */
        srcL1descInx = L1desc[l1descInx].srcL1descInx;
        if (srcL1descInx <= 2) {
            /* not a valid srcL1descInx */
            rodsLog (LOG_NOTICE,
              "dataObjChksum: invalid srcL1descInx %d fopy copy",
              srcL1descInx);
	    /* do nothing */
            return (0);
        }
        srcDataObjInfo = L1desc[srcL1descInx].dataObjInfo;
        if (strlen (srcDataObjInfo->chksum) > 0) {
            addKeyVal (regParam, CHKSUM_KW, srcDataObjInfo->chksum);
        }
	return (0);
    }
    return (0);
}
#endif

int 
_dataObjChksum (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo, char **chksumStr)
{
    fileChksumInp_t fileChksumInp;
    int rescTypeInx;
    int rescClass;
    int status;
    rescInfo_t *rescInfo = dataObjInfo->rescInfo;

    rescClass = getRescClass (rescInfo);
    if (rescClass == COMPOUND_CL) return SYS_CANT_CHKSUM_COMP_RESC_DATA;
    else if (rescClass == BUNDLE_CL) return SYS_CANT_CHKSUM_BUNDLED_DATA;

    rescTypeInx = rescInfo->rescTypeInx;

    switch (RescTypeDef[rescTypeInx].rescCat) {
      case FILE_CAT:
        memset (&fileChksumInp, 0, sizeof (fileChksumInp));
        fileChksumInp.fileType = RescTypeDef[rescTypeInx].driverType;
        rstrcpy (fileChksumInp.addr.hostAddr, rescInfo->rescLoc,
          NAME_LEN);
        rstrcpy (fileChksumInp.fileName, dataObjInfo->filePath, MAX_NAME_LEN);
	status = rsFileChksum (rsComm, &fileChksumInp, chksumStr);
        break;
      default:
        rodsLog (LOG_NOTICE,
          "_dataObjChksum: rescCat type %d is not recognized",
          RescTypeDef[rescTypeInx].rescCat);
        status = SYS_INVALID_RESC_TYPE;
        break;
    }
    return (status);
}

/* getNumThreads - get the number of threads.
 * inpNumThr - 0 - server decide
 *	       < 0 - NO_THREADING 	
 *	       > 0 - num of threads wanted
 */

int
getNumThreads (rsComm_t *rsComm, rodsLong_t dataSize, int inpNumThr, 
keyValPair_t *condInput, char *destRescName, char *srcRescName)
{
    ruleExecInfo_t rei;
    dataObjInp_t doinp;
    int status;
    int numDestThr = -1;
    int numSrcThr = -1;
    rescGrpInfo_t *rescGrpInfo;

    if (inpNumThr == NO_THREADING)
        return 0;

    if (dataSize < 0)
        return 1;

    if (dataSize <= MIN_SZ_FOR_PARA_TRAN) {
        if (inpNumThr > 0) {
            inpNumThr = 1;
        } else {
            return 0;
        }
    }

    if (getValByKey (condInput, NO_PARA_OP_KW) != NULL) {
        /* client specify no para opr */
        return (1);
    }

#ifndef PARA_OPR
    return (1);
#endif

    memset (&doinp, 0, sizeof (doinp));
    doinp.numThreads = inpNumThr;

    doinp.dataSize = dataSize;

    initReiWithDataObjInp (&rei, rsComm, &doinp);

    if (destRescName != NULL) {
	rescGrpInfo = NULL;
        status = resolveAndQueResc (destRescName, NULL, &rescGrpInfo);
        if (status >= 0) {
	    rei.rgi = rescGrpInfo;
            status = applyRule ("acSetNumThreads", NULL, &rei, NO_SAVE_REI);
	    freeRescGrpInfo (rescGrpInfo);
            if (status < 0) {
                rodsLog (LOG_ERROR,
	          "getNumThreads: acGetNumThreads error, status = %d",
	          status);
            } else {
	        numDestThr = rei.status;
	        if (numDestThr == 0) return 0;
	    }
        }
    }

    if (srcRescName != NULL) {
	if (numDestThr > 0 && strcmp (destRescName, srcRescName) == 0) 
	    return numDestThr;
	rescGrpInfo = NULL;
        status = resolveAndQueResc (srcRescName, NULL, &rescGrpInfo);
        if (status >= 0) {
            rei.rgi = rescGrpInfo;
            status = applyRule ("acSetNumThreads", NULL, &rei, NO_SAVE_REI);
	    freeRescGrpInfo (rescGrpInfo);
            if (status < 0) {
                rodsLog (LOG_ERROR,
                  "getNumThreads: acGetNumThreads error, status = %d",
                  status);
            } else {
                numSrcThr = rei.status;
	        if (numSrcThr == 0) return 0;
            }
	}
    }

    if (numDestThr > 0) {
	if (getValByKey (condInput, RBUDP_TRANSFER_KW) != NULL) {
	    return 1;
	} else {
            return numDestThr;
	}
    }
    if (numSrcThr > 0) {
        if (getValByKey (condInput, RBUDP_TRANSFER_KW) != NULL) {
            return 1;
        } else {
            return numSrcThr;
	}
    }
    /* should not be here. do one with no resource */
    rei.rgi = NULL;
    status = applyRule ("acSetNumThreads", NULL, &rei, NO_SAVE_REI);
    if (status < 0) {
        rodsLog (LOG_ERROR,
          "getNumThreads: acGetNumThreads error, status = %d",
          status);
	return 0;
    } else {
        if (rei.status > 0)
	    return rei.status;
	else
            return 0;
    }
}

int
initDataOprInp (dataOprInp_t *dataOprInp, int l1descInx, int oprType)
{
    dataObjInfo_t *dataObjInfo;
    dataObjInp_t  *dataObjInp;
#ifdef RBUDP_TRANSFER
    char *tmpStr;
#endif


    dataObjInfo = L1desc[l1descInx].dataObjInfo;
    dataObjInp = L1desc[l1descInx].dataObjInp;

    memset (dataOprInp, 0, sizeof (dataOprInp_t));

    dataOprInp->oprType = oprType;
    dataOprInp->numThreads = dataObjInp->numThreads;
    dataOprInp->offset = dataObjInp->offset;
    if (oprType == PUT_OPR) {
	if (dataObjInp->dataSize > 0) 
	    dataOprInp->dataSize = dataObjInp->dataSize;
        dataOprInp->destL3descInx = L1desc[l1descInx].l3descInx;
        if (L1desc[l1descInx].remoteZoneHost == NULL)
            dataOprInp->destRescTypeInx = dataObjInfo->rescInfo->rescTypeInx;
    } else if (oprType == GET_OPR) {
	if (dataObjInfo->dataSize > 0) {
            dataOprInp->dataSize = dataObjInfo->dataSize;
        } else {
            dataOprInp->dataSize = dataObjInp->dataSize;
        }
        dataOprInp->srcL3descInx = L1desc[l1descInx].l3descInx;
        if (L1desc[l1descInx].remoteZoneHost == NULL)
            dataOprInp->srcRescTypeInx = dataObjInfo->rescInfo->rescTypeInx;
    } else if (oprType == SAME_HOST_COPY_OPR) {
	int srcL1descInx = L1desc[l1descInx].srcL1descInx;
	int srcL3descInx = L1desc[srcL1descInx].l3descInx;
        dataOprInp->dataSize = L1desc[srcL1descInx].dataObjInfo->dataSize;
        dataOprInp->destL3descInx = L1desc[l1descInx].l3descInx;
        if (L1desc[l1descInx].remoteZoneHost == NULL)
            dataOprInp->destRescTypeInx = dataObjInfo->rescInfo->rescTypeInx;
        dataOprInp->srcL3descInx = srcL3descInx;
        dataOprInp->srcRescTypeInx = dataObjInfo->rescInfo->rescTypeInx;
    } else if (oprType == COPY_TO_REM_OPR) {
        int srcL1descInx = L1desc[l1descInx].srcL1descInx;
        int srcL3descInx = L1desc[srcL1descInx].l3descInx;
        dataOprInp->dataSize = L1desc[srcL1descInx].dataObjInfo->dataSize;
        dataOprInp->srcL3descInx = srcL3descInx;
        if (L1desc[srcL1descInx].remoteZoneHost == NULL) {
            dataOprInp->srcRescTypeInx = 
	      L1desc[srcL1descInx].dataObjInfo->rescInfo->rescTypeInx;
	}
#if 0
        if (dataObjInfo->dataSize > 0) {
            dataOprInp->dataSize = dataObjInfo->dataSize;
        } else {
            dataOprInp->dataSize = dataObjInp->dataSize;
        }
        dataOprInp->srcL3descInx = L1desc[l1descInx].l3descInx;
        if (L1desc[l1descInx].remoteZoneHost == NULL)
            dataOprInp->srcRescTypeInx = dataObjInfo->rescInfo->rescTypeInx;
#endif
    }  else if (oprType == COPY_TO_LOCAL_OPR) {
        int srcL1descInx = L1desc[l1descInx].srcL1descInx;
        dataOprInp->dataSize = L1desc[srcL1descInx].dataObjInfo->dataSize;
        dataOprInp->destL3descInx = L1desc[l1descInx].l3descInx;
        if (L1desc[l1descInx].remoteZoneHost == NULL)
            dataOprInp->destRescTypeInx = dataObjInfo->rescInfo->rescTypeInx;
    }
#if 0
    if (oprType == PUT_OPR && dataObjInp->dataSize > 0) {
	dataOprInp->dataSize = dataObjInp->dataSize;
    } else if (dataObjInfo->dataSize > 0) { 
	dataOprInp->dataSize = dataObjInfo->dataSize;
    } else {
        dataOprInp->dataSize = dataObjInp->dataSize;
    }
    if (oprType == PUT_OPR || oprType == COPY_TO_LOCAL_OPR ||
      oprType == SAME_HOST_COPY_OPR) {
        dataOprInp->destL3descInx = L1desc[l1descInx].l3descInx;
	if (L1desc[l1descInx].remoteZoneHost == NULL)
            dataOprInp->destRescTypeInx = dataObjInfo->rescInfo->rescTypeInx;
    } else if (oprType == GET_OPR || COPY_TO_REM_OPR) {
        dataOprInp->srcL3descInx = L1desc[l1descInx].l3descInx;
	if (L1desc[l1descInx].remoteZoneHost == NULL)
            dataOprInp->srcRescTypeInx = dataObjInfo->rescInfo->rescTypeInx;
    }
#endif
    if (getValByKey (&dataObjInp->condInput, STREAMING_KW) != NULL) {
        addKeyVal (&dataOprInp->condInput, STREAMING_KW, "");
    }

    if (getValByKey (&dataObjInp->condInput, NO_PARA_OP_KW) != NULL) {
        addKeyVal (&dataOprInp->condInput, NO_PARA_OP_KW, "");
    }

#ifdef RBUDP_TRANSFER
    if (getValByKey (&dataObjInp->condInput, RBUDP_TRANSFER_KW) != NULL) {
	if (dataObjInfo->rescInfo != NULL) {
	    /* only do unix fs */
	    int rescTypeInx = dataObjInfo->rescInfo->rescTypeInx;
	    if (RescTypeDef[rescTypeInx].driverType == UNIX_FILE_TYPE)
                addKeyVal (&dataOprInp->condInput, RBUDP_TRANSFER_KW, "");
	}
    }

    if (getValByKey (&dataObjInp->condInput, VERY_VERBOSE_KW) != NULL) {
        addKeyVal (&dataOprInp->condInput, VERY_VERBOSE_KW, "");
    }

    if ((tmpStr = getValByKey (&dataObjInp->condInput, RBUDP_SEND_RATE_KW)) !=
      NULL) {
        addKeyVal (&dataOprInp->condInput, RBUDP_SEND_RATE_KW, tmpStr);
    }

    if ((tmpStr = getValByKey (&dataObjInp->condInput, RBUDP_PACK_SIZE_KW)) !=
      NULL) {
        addKeyVal (&dataOprInp->condInput, RBUDP_PACK_SIZE_KW, tmpStr);
    }
#endif


    return (0);
}

int
initDataObjInfoForRepl (rsComm_t *rsComm, dataObjInfo_t *destDataObjInfo, 
dataObjInfo_t *srcDataObjInfo, rescInfo_t *destRescInfo, 
char *destRescGroupName)
{
    memset (destDataObjInfo, 0, sizeof (dataObjInfo_t));
    *destDataObjInfo = *srcDataObjInfo;
    destDataObjInfo->filePath[0] = '\0';
    rstrcpy (destDataObjInfo->rescName, destRescInfo->rescName, NAME_LEN);
    destDataObjInfo->replNum = destDataObjInfo->dataId = 0;
    destDataObjInfo->rescInfo = destRescInfo;

    if (destRescGroupName != NULL && strlen (destRescGroupName) > 0) {
        rstrcpy (destDataObjInfo->rescGroupName, destRescGroupName,
        NAME_LEN);
    } else if (strlen (destDataObjInfo->rescGroupName) > 0) {
	/* need to verify whether destRescInfo belongs to 
	 * destDataObjInfo->rescGroupName */
	if (getRescInGrp (rsComm, destRescInfo->rescName, 
	  destDataObjInfo->rescGroupName, NULL) < 0) {
	    /* destResc is not in destRescGrp */
	    destDataObjInfo->rescGroupName[0] = '\0';
	}
    }

    return (0);
}

int 
convL3descInx (int l3descInx)
{
    if (l3descInx <= 2 || FileDesc[l3descInx].inuseFlag == 0 ||
     FileDesc[l3descInx].rodsServerHost == NULL) {
        return l3descInx;
    }

    if (FileDesc[l3descInx].rodsServerHost->localFlag == LOCAL_HOST) {
        return (l3descInx);
    } else {
        return (FileDesc[l3descInx].fd);
    }
}   

int
dataObjChksumAndReg (rsComm_t *rsComm, dataObjInfo_t *dataObjInfo, 
char **chksumStr) 
{
    keyValPair_t regParam;
    modDataObjMeta_t modDataObjMetaInp;
    int status;

    status = _dataObjChksum (rsComm, dataObjInfo, chksumStr);
    if (status < 0) {
        rodsLog (LOG_NOTICE,
         "dataObjChksumAndReg: _dataObjChksum error for %s, status = %d",
          dataObjInfo->objPath, status);
        return (status);
    }

    /* register it */
    memset (&regParam, 0, sizeof (regParam));
    addKeyVal (&regParam, CHKSUM_KW, *chksumStr);

    modDataObjMetaInp.dataObjInfo = dataObjInfo;
    modDataObjMetaInp.regParam = &regParam;

    status = rsModDataObjMeta (rsComm, &modDataObjMetaInp);

    clearKeyVal (&regParam);

    if (status < 0) {
        rodsLog (LOG_NOTICE,
         "dataObjChksumAndReg: rsModDataObjMeta error for %s, status = %d",
         dataObjInfo->objPath, status);
	/* don't return error because it is not fatal */
    }

    return (0);
}

/* chkAndHandleOrphanFile - Check whether the file is an orphan file.
 * If it is, rename it.  
 * If it belongs to an old copy, move the old path and register it.
 *
 * return 0 - the filePath is NOT an orphan file.
 *        1 - the filePath is an orphan file and has been renamed.
 *        < 0 - error
 */

int
chkAndHandleOrphanFile (rsComm_t *rsComm, char *filePath, rescInfo_t *rescInfo,
int replStatus)
{
    fileRenameInp_t fileRenameInp;
    int status;
    dataObjInfo_t myDataObjInfo;
    int rescTypeInx = rescInfo->rescTypeInx;

    if (RescTypeDef[rescTypeInx].rescCat != FILE_CAT) {
	/* can't do anything with non file type */
	return (-1);
    }

    if (strlen (filePath) + 17 >= MAX_NAME_LEN) {
	/* the new path name will be too long to add "/orphan + random" */
	return (-1);
    }
 
    /* check if the input filePath is assocated with a dataObj */

    memset (&myDataObjInfo, 0, sizeof (myDataObjInfo));
    memset (&fileRenameInp, 0, sizeof (fileRenameInp));
    if ((status = chkOrphanFile (
      rsComm, filePath, rescInfo->rescName, &myDataObjInfo)) == 0) {
        rstrcpy (fileRenameInp.oldFileName, filePath, MAX_NAME_LEN);
	/* not an orphan file */
	if (replStatus > OLD_COPY || isTrashPath (myDataObjInfo.objPath)) {
            modDataObjMeta_t modDataObjMetaInp;
            keyValPair_t regParam;

	    /* a new copy or the current path is in trash. 
	     * rename and reg the path of the old one */
            status = renameFilePathToNewDir (rsComm, REPL_DIR, &fileRenameInp, 
	      rescInfo, 1);
            if (status < 0) {
                return (status);
	    }
	    /* register the change */
	    memset (&regParam, 0, sizeof (regParam));
            addKeyVal (&regParam, FILE_PATH_KW, fileRenameInp.newFileName);
	    modDataObjMetaInp.dataObjInfo = &myDataObjInfo;
	    modDataObjMetaInp.regParam = &regParam;
	    status = rsModDataObjMeta (rsComm, &modDataObjMetaInp);
            clearKeyVal (&regParam);
	    if (status < 0) {
                rodsLog (LOG_ERROR,
                 "chkAndHandleOrphan: rsModDataObjMeta of %s error. stat = %d",
                 fileRenameInp.newFileName, status);
		/* need to rollback the change in path */
                rstrcpy (fileRenameInp.oldFileName, fileRenameInp.newFileName, 
		  MAX_NAME_LEN);
                rstrcpy (fileRenameInp.newFileName, filePath, MAX_NAME_LEN);
    	        status = rsFileRename (rsComm, &fileRenameInp);

    	        if (status < 0) {
        	    rodsLog (LOG_ERROR,
                     "chkAndHandleOrphan: rsFileRename %s failed, status = %d",
          	     fileRenameInp.oldFileName, status);
		    return (status);
		}
		/* this thing still failed */
		return (-1);
	    } else {
		return (0);
	    }
	} else {
            /* this is an old copy. change the path but don't
	     * actually rename it */
            rstrcpy (fileRenameInp.oldFileName, filePath, MAX_NAME_LEN);
            status = renameFilePathToNewDir (rsComm, REPL_DIR, &fileRenameInp,
              rescInfo, 0);
            if (status >= 0) {
                rstrcpy (filePath, fileRenameInp.newFileName, MAX_NAME_LEN);
                return (0);
            } else {
                return (status);
            }
	}

    } else if (status > 0) {
        /* this is an orphan file. need to rename it */
        rstrcpy (fileRenameInp.oldFileName, filePath, MAX_NAME_LEN);
	status = renameFilePathToNewDir (rsComm, ORPHAN_DIR, &fileRenameInp, 
	  rescInfo, 1);
	if (status >= 0) {
	    return (1);
	} else {
            return (status);
	}
    } else {
	/* error */
	return (status);
    }
}

int
renameFilePathToNewDir (rsComm_t *rsComm, char *newDir,
fileRenameInp_t *fileRenameInp, rescInfo_t *rescInfo, int renameFlag)
{
    int len, status;
    char *oldPtr, *newPtr;
    int rescTypeInx = rescInfo->rescTypeInx;
    char *filePath = fileRenameInp->oldFileName;

    fileRenameInp->fileType = RescTypeDef[rescTypeInx].driverType;

    rstrcpy (fileRenameInp->addr.hostAddr, rescInfo->rescLoc, NAME_LEN);

    len = strlen (rescInfo->rescVaultPath);

    if (len <= 0) {
	return (-1);
    }
    
    if (strncmp (filePath, rescInfo->rescVaultPath, len) != 0) {
	/* not in rescVaultPath */
	return -1;
    }

    rstrcpy (fileRenameInp->newFileName, rescInfo->rescVaultPath, MAX_NAME_LEN);
    oldPtr = filePath + len;
    newPtr = fileRenameInp->newFileName + len;

    snprintf (newPtr, MAX_NAME_LEN - len, "/%s%s.%-d", newDir, oldPtr, 
     (uint) random());
    
    if (renameFlag > 0) {
        status = rsFileRename (rsComm, fileRenameInp);
        if (status < 0) {
            rodsLog (LOG_NOTICE,
             "renameFilePathToNewDir:rsFileRename from %s to %s failed,stat=%d",
              filePath, fileRenameInp->newFileName, status);
	    return -1;
        } else {
            return (0); 
        }
    } else {
	return (0);
    }
}

/* syncDataObjPhyPath - sync the path of the phy path with the path of
 * the data ovject. This is unsed by rename to sync the path of the
 * phy path with the new path.
 */

int
syncDataObjPhyPath (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
dataObjInfo_t *dataObjInfoHead)
{
    dataObjInfo_t *tmpDataObjInfo;
    int status;
    int savedStatus = 0;

    tmpDataObjInfo = dataObjInfoHead;
    while (tmpDataObjInfo != NULL) {
	status = syncDataObjPhyPathS (rsComm, dataObjInp, tmpDataObjInfo);
	if (status < 0) {
	    savedStatus = status;
	}
	tmpDataObjInfo = tmpDataObjInfo->next;
    }

    return (savedStatus);
} 

int
syncDataObjPhyPathS (rsComm_t *rsComm, dataObjInp_t *dataObjInp,
dataObjInfo_t *dataObjInfo)
{
    int status, status1;
    fileRenameInp_t fileRenameInp;
    rescInfo_t *rescInfo;
    int rescTypeInx;
    modDataObjMeta_t modDataObjMetaInp;
    keyValPair_t regParam;
    vaultPathPolicy_t vaultPathPolicy;

    if (strcmp (dataObjInfo->rescInfo->rescName, BUNDLE_RESC) == 0)
	return 0;

    status = getVaultPathPolicy (rsComm, dataObjInfo, &vaultPathPolicy);
    if (status < 0) {
	rodsLog (LOG_NOTICE,
          "syncDataObjPhyPathS: getVaultPathPolicy error for %s, status = %d",
	  dataObjInfo->objPath, status);
    } else {
	if (vaultPathPolicy.scheme != GRAFT_PATH_S) {
	    /* no need to sync */
	    return (0);
	}
    }

    if (isInVault (dataObjInfo) == 0) {
	/* not in vault. */
	return (0);
    }

     if (dataObjInfo->rescInfo->rescStatus == INT_RESC_STATUS_DOWN)
        return SYS_RESC_IS_DOWN;

    /* Save the current objPath */
    memset (&fileRenameInp, 0, sizeof (fileRenameInp));
    rstrcpy (fileRenameInp.oldFileName, dataObjInfo->filePath, 
      MAX_NAME_LEN);
    if (dataObjInp == NULL) {
	dataObjInp_t myDdataObjInp;
	memset (&myDdataObjInp, 0, sizeof (myDdataObjInp));
	rstrcpy (myDdataObjInp.objPath, dataObjInfo->objPath, MAX_NAME_LEN);
        status = getFilePathName (rsComm, dataObjInfo, &myDdataObjInp);
    } else {
        status = getFilePathName (rsComm, dataObjInfo, dataObjInp);
    }

    if (status < 0) {
	return status;
    }
    if (strcmp (fileRenameInp.oldFileName, dataObjInfo->filePath) == 0) {
	return (0);
    }

    rescInfo = dataObjInfo->rescInfo;
    /* see if the new file exist */
    if (getSizeInVault (rsComm, dataObjInfo) >= 0) {
        if (chkAndHandleOrphanFile (rsComm, dataObjInfo->filePath,
          rescInfo, OLD_COPY) <= 0) {
            rodsLog (LOG_ERROR,
             "syncDataObjPhyPath:newFileName %s to %s already exists",
              dataObjInfo->filePath);
	    return (SYS_INVALID_FILE_PATH);
	}
    }

    /* rename it */
    rescTypeInx = rescInfo->rescTypeInx;
    fileRenameInp.fileType = RescTypeDef[rescTypeInx].driverType;
    rstrcpy (fileRenameInp.addr.hostAddr, rescInfo->rescLoc, NAME_LEN);
    rstrcpy (fileRenameInp.newFileName, dataObjInfo->filePath,
      MAX_NAME_LEN);
 
    status = rsFileRename (rsComm, &fileRenameInp);
    if (status < 0) {
        rodsLog (LOG_ERROR,
         "syncDataObjPhyPath:rsFileRename from %s to %s failed,stat=%d",
          fileRenameInp.oldFileName, fileRenameInp.newFileName);
	return (status);
    }

    /* register the change */
    memset (&regParam, 0, sizeof (regParam));
    addKeyVal (&regParam, FILE_PATH_KW, fileRenameInp.newFileName);
    modDataObjMetaInp.dataObjInfo = dataObjInfo;
    modDataObjMetaInp.regParam = &regParam;
    status = rsModDataObjMeta (rsComm, &modDataObjMetaInp);
    clearKeyVal (&regParam);
    if (status < 0) {
        char tmpPath[MAX_NAME_LEN]; 
        rodsLog (LOG_ERROR,
         "syncDataObjPhyPath: rsModDataObjMeta of %s error. stat = %d",
         fileRenameInp.newFileName, status);
        /* need to rollback the change in path */
	rstrcpy (tmpPath, fileRenameInp.oldFileName, MAX_NAME_LEN);
        rstrcpy (fileRenameInp.oldFileName, fileRenameInp.newFileName,
          MAX_NAME_LEN);
        rstrcpy (fileRenameInp.newFileName, tmpPath, MAX_NAME_LEN);
        status1 = rsFileRename (rsComm, &fileRenameInp);

        if (status1 < 0) {
            rodsLog (LOG_ERROR,
             "syncDataObjPhyPath: rollback rename %s failed, status = %d",
             fileRenameInp.oldFileName, status1);
        }
	return (status);
    }
    return (0);
}

/* syncCollPhyPath - sync the path of the phy path with the path of
 * the data ovject in the new collection. This is unsed by rename to sync 
 * the path of the phy path with the new path.
 */

int
syncCollPhyPath (rsComm_t *rsComm, char *collection)
{
    int status, i;
    int savedStatus = 0;
    genQueryOut_t *genQueryOut = NULL;
    genQueryInp_t genQueryInp;
    int continueInx;

    status = rsQueryDataObjInCollReCur (rsComm, collection, 
      &genQueryInp, &genQueryOut, NULL, 0);

    if (status<0 && status != CAT_NO_ROWS_FOUND) {
	savedStatus=status; /* return the error code */
    }

    while (status >= 0) {
        sqlResult_t *dataIdRes, *subCollRes, *dataNameRes, *replNumRes, 
	  *rescNameRes, *filePathRes;
	char *tmpDataId, *tmpDataName, *tmpSubColl, *tmpReplNum, 
	  *tmpRescName, *tmpFilePath;
        dataObjInfo_t dataObjInfo;

	memset (&dataObjInfo, 0, sizeof (dataObjInfo));

        if ((dataIdRes = getSqlResultByInx (genQueryOut, COL_D_DATA_ID))
          == NULL) {
            rodsLog (LOG_ERROR,
              "syncCollPhyPath: getSqlResultByInx for COL_COLL_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }
        if ((subCollRes = getSqlResultByInx (genQueryOut, COL_COLL_NAME))
          == NULL) {
            rodsLog (LOG_ERROR,
              "syncCollPhyPath: getSqlResultByInx for COL_COLL_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }
        if ((dataNameRes = getSqlResultByInx (genQueryOut, COL_DATA_NAME))
          == NULL) {
            rodsLog (LOG_ERROR,
              "syncCollPhyPath: getSqlResultByInx for COL_DATA_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }
        if ((replNumRes = getSqlResultByInx (genQueryOut, COL_DATA_REPL_NUM))
          == NULL) {
            rodsLog (LOG_ERROR,
             "syncCollPhyPath:getSqlResultByIn for COL_DATA_REPL_NUM failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }
        if ((rescNameRes = getSqlResultByInx (genQueryOut, COL_D_RESC_NAME))
          == NULL) {
            rodsLog (LOG_ERROR,
             "syncCollPhyPath: getSqlResultByInx for COL_D_RESC_NAME failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }
        if ((filePathRes = getSqlResultByInx (genQueryOut, COL_D_DATA_PATH))
          == NULL) {
            rodsLog (LOG_ERROR,
             "syncCollPhyPath: getSqlResultByInx for COL_D_DATA_PATH failed");
            return (UNMATCHED_KEY_OR_INDEX);
        }
        for (i = 0;i < genQueryOut->rowCnt; i++) {
            tmpDataId = &dataIdRes->value[dataIdRes->len * i];
            tmpDataName = &dataNameRes->value[dataNameRes->len * i];
            tmpSubColl = &subCollRes->value[subCollRes->len * i];
            tmpReplNum = &replNumRes->value[replNumRes->len * i];
            tmpRescName = &rescNameRes->value[rescNameRes->len * i];
            tmpFilePath = &filePathRes->value[filePathRes->len * i];

	    dataObjInfo.dataId = strtoll (tmpDataId, 0, 0);
	    snprintf (dataObjInfo.objPath, MAX_NAME_LEN, "%s/%s",
	      tmpSubColl, tmpDataName);
	    dataObjInfo.replNum = atoi (tmpReplNum);
            rstrcpy (dataObjInfo.rescName, tmpRescName, NAME_LEN);
            status = resolveResc (tmpRescName, &dataObjInfo.rescInfo);
            if (status < 0) {
                rodsLog (LOG_ERROR,
                  "syncCollPhyPath: resolveResc error for %s, status = %d",
                  tmpRescName, status);
                return (status);
            }
            rstrcpy (dataObjInfo.filePath, tmpFilePath, MAX_NAME_LEN);

            status = syncDataObjPhyPathS (rsComm, NULL, &dataObjInfo);
	    if (status < 0) {
		rodsLog (LOG_ERROR,
                  "syncCollPhyPath: syncDataObjPhyPathS error for %s,stat=%d",
                  dataObjInfo.filePath, status);
		savedStatus = status;
            }

	}

        continueInx = genQueryOut->continueInx;

        freeGenQueryOut (&genQueryOut);

        if (continueInx > 0) {
            /* More to come */
            genQueryInp.continueInx = continueInx;
            status =  rsGenQuery (rsComm, &genQueryInp, &genQueryOut);
        } else {
            break;
        }
    }
    clearGenQueryInp (&genQueryInp);

    return (savedStatus);
}

int
isInVault (dataObjInfo_t *dataObjInfo)
{
    int len;

    if (dataObjInfo == NULL || dataObjInfo->rescInfo == NULL) {
	return (SYS_INTERNAL_NULL_INPUT_ERR);
    }

    len = strlen (dataObjInfo->rescInfo->rescVaultPath);

    if (strncmp (dataObjInfo->rescInfo->rescVaultPath, 
      dataObjInfo->filePath, len) == 0) {
        return (1);
    } else {
	return (0);
    }
}

int
initCollHandle ()
{
    memset (CollHandle, 0, sizeof (collHandle_t) * NUM_COLL_HANDLE);
    return (0);
}

int
allocCollHandle ()
{
    int i;

    for (i = 0; i < NUM_COLL_HANDLE; i++) {
        if (CollHandle[i].inuseFlag <= FD_FREE) {
            CollHandle[i].inuseFlag = FD_INUSE;
            return (i);
        };
    }

    rodsLog (LOG_NOTICE,
     "allocCollHandle: out of CollHandle");

    return (SYS_OUT_OF_FILE_DESC);
}

int
freeCollHandle (int handleInx)
{
    if (handleInx < 0 || handleInx >= NUM_COLL_HANDLE) {
        rodsLog (LOG_NOTICE,
         "freeCollHandle: handleInx %d out of range", handleInx);
        return (SYS_FILE_DESC_OUT_OF_RANGE);
    }

    /* don't free specColl. It is in cache */
#if 0
    clearCollHandle (&CollHandle[handleInx], 0);
#else
    clearCollHandle (&CollHandle[handleInx], 1);
#endif
    memset (&CollHandle[handleInx], 0, sizeof (collHandle_t));

    return (0);
}

int
rsInitQueryHandle (queryHandle_t *queryHandle, rsComm_t *rsComm)
{
    if (queryHandle == NULL || rsComm == NULL) {
        return (USER__NULL_INPUT_ERR);
    }

    queryHandle->conn = rsComm;
    queryHandle->connType = RS_COMM;
    queryHandle->querySpecColl = (funcPtr) rsQuerySpecColl;
    queryHandle->genQuery = (funcPtr) rsGenQuery;

    return (0);
}

/* initStructFileOprInp - initialize the structFileOprInp struct for
 * rsStructFileBundle and rsStructFileExtAndReg
 */

int
initStructFileOprInp (rsComm_t *rsComm, 
structFileOprInp_t *structFileOprInp,
structFileExtAndRegInp_t *structFileExtAndRegInp, 
dataObjInfo_t *dataObjInfo)
{
    int status;
    vaultPathPolicy_t vaultPathPolicy;
    int addUserNameFlag;

    memset (structFileOprInp, 0, sizeof (structFileOprInp_t));
    structFileOprInp->specColl = malloc (sizeof (specColl_t));
    memset (structFileOprInp->specColl, 0, sizeof (specColl_t));
    if (strcmp (dataObjInfo->dataType, TAR_DT_STR) == 0 ||
      strcmp (dataObjInfo->dataType, TAR_BUNDLE_TYPE) == 0) {
        structFileOprInp->specColl->type = TAR_STRUCT_FILE_T;
    } else if (strcmp (dataObjInfo->dataType, HAAW_DT_STR) == 0) {
        structFileOprInp->specColl->type = HAAW_STRUCT_FILE_T;
    } else {
        rodsLog (LOG_ERROR,
          "initStructFileOprInp: objType %s of %s is not a struct file",
          dataObjInfo->dataType, dataObjInfo->objPath);
        return SYS_OBJ_TYPE_NOT_STRUCT_FILE;
    }

    rstrcpy (structFileOprInp->specColl->collection,
      structFileExtAndRegInp->collection, MAX_NAME_LEN);
    rstrcpy (structFileOprInp->specColl->objPath,
      structFileExtAndRegInp->objPath, MAX_NAME_LEN);
    structFileOprInp->specColl->collClass = STRUCT_FILE_COLL;
    rstrcpy (structFileOprInp->specColl->resource, dataObjInfo->rescName,
      NAME_LEN);
    rstrcpy (structFileOprInp->specColl->phyPath,
      dataObjInfo->filePath, MAX_NAME_LEN);
    rstrcpy (structFileOprInp->addr.hostAddr, dataObjInfo->rescInfo->rescLoc,
      NAME_LEN);
    /* set the cacheDir */
    status = getVaultPathPolicy (rsComm, dataObjInfo, &vaultPathPolicy);
    if (status < 0) {
        return (status);
    }
    /* don't do other type of Policy except GRAFT_PATH_S */
    if (vaultPathPolicy.scheme == GRAFT_PATH_S) {
	addUserNameFlag = vaultPathPolicy.addUserName;
    } else {
        rodsLog (LOG_ERROR,
          "initStructFileOprInp: vaultPathPolicy.scheme %d for resource %s is not GRAFT_PATH_S",
          vaultPathPolicy.scheme, structFileOprInp->specColl->resource);
        return SYS_WRONG_RESC_POLICY_FOR_BUN_OPR;
    }
    status = setPathForGraftPathScheme (structFileExtAndRegInp->collection,
      dataObjInfo->rescInfo->rescVaultPath, addUserNameFlag,
      rsComm->clientUser.userName, vaultPathPolicy.trimDirCnt,
      structFileOprInp->specColl->cacheDir);

    return (status);
}

int
allocAndSetL1descForZoneOpr (int remoteL1descInx, dataObjInp_t *dataObjInp,
rodsServerHost_t *remoteZoneHost, openStat_t *openStat)
{
    int l1descInx;            
    dataObjInfo_t *dataObjInfo;

    l1descInx = allocL1desc ();
    if (l1descInx < 0) return l1descInx;
    L1desc[l1descInx].remoteL1descInx = remoteL1descInx;
    L1desc[l1descInx].oprType = REMOTE_ZONE_OPR;
    L1desc[l1descInx].remoteZoneHost = remoteZoneHost;
#if 0
    L1desc[l1descInx].dataObjInp = dataObjInp;
#else
    /* always repl the .dataObjInp */
    L1desc[l1descInx].dataObjInp = malloc (sizeof (dataObjInp_t));
    replDataObjInp (dataObjInp, L1desc[l1descInx].dataObjInp);
    L1desc[l1descInx].dataObjInpReplFlag = 1;
#endif
    dataObjInfo = L1desc[l1descInx].dataObjInfo =
      malloc (sizeof (dataObjInfo_t));
    bzero (dataObjInfo, sizeof (dataObjInfo_t));
    rstrcpy (dataObjInfo->objPath, dataObjInp->objPath, MAX_NAME_LEN);

    if (openStat != NULL) {
	dataObjInfo->dataSize = openStat->dataSize;
	rstrcpy (dataObjInfo->dataMode, openStat->dataMode, SHORT_STR_LEN);
	rstrcpy (dataObjInfo->dataType, openStat->dataType, NAME_LEN);
	L1desc[l1descInx].l3descInx = openStat->l3descInx;
	L1desc[l1descInx].replStatus = openStat->replStatus;
	dataObjInfo->rescInfo = malloc (sizeof (rescInfo_t));
	bzero (dataObjInfo->rescInfo, sizeof (rescInfo_t));
	dataObjInfo->rescInfo->rescTypeInx = openStat->rescTypeInx;
    }

    return l1descInx;
}

int
getDefFileMode ()
{
    int defFileMode;
    if (getenv ("DefFileMode") != NULL) {
        defFileMode = strtol (getenv ("DefFileMode"), 0, 0);
    } else {
        defFileMode = DEFAULT_FILE_MODE;
    }
    return defFileMode;
}

int
getDefDirMode ()
{
    int defDirMode;
    if (getenv ("DefDirMode") != NULL) { 
        defDirMode = strtol (getenv ("DefDirMode"), 0, 0);
    } else {
        defDirMode = DEFAULT_DIR_MODE;
    }
    return defDirMode;
}

int
getLogPathFromPhyPath (char *phyPath, rescInfo_t *rescInfo, char *outLogPath)
{
    int len;
    char *tmpPtr;
    zoneInfo_t *tmpZoneInfo = NULL;
    int status;

    if (phyPath == NULL || rescInfo == NULL || outLogPath == NULL)
	return USER__NULL_INPUT_ERR;

    len = strlen (rescInfo->rescVaultPath);
    if (strncmp (rescInfo->rescVaultPath, phyPath, len) != 0) return -1;
    tmpPtr = phyPath + len;

    if (*tmpPtr != '/') return -1;

    tmpPtr ++;
    status = getLocalZoneInfo (&tmpZoneInfo);
    if (status < 0) return status;

    len = strlen (tmpZoneInfo->zoneName);   
    if (strncmp (tmpZoneInfo->zoneName, tmpPtr, len) == 0 &&
      *(tmpPtr + len) == '/') {
	/* start with zoneName */
	tmpPtr += (len + 1);
    }

    snprintf (outLogPath, MAX_NAME_LEN, "/%s/%s", tmpZoneInfo->zoneName,
      tmpPtr);

    return 0;
}
