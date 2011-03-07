/**
 * @file	icatAdminMS.c
 *
 */

/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
#include "reGlobalsExtern.h"
#include "icatHighLevelRoutines.h"


/**
 * \fn msiCreateUser (ruleExecInfo_t *rei)	
 *
 * \brief   This microservice creates a new user
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author  Wayne Schroeder
 * \date  2008 or before
 *
 * \remark Jewel Ward - msi documentation, 2009-06-18
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-25
 *
 * \note Should not be used outside of the rules defined in core.irb.
 * This is called via an 'iadmin' command.
 *
 * \usage
 * 
 * As seen in server/config/reConfigs/core.irb
 *
 * acCreateUserF1||msiCreateUser##acCreateDefaultCollections##msiAddUserToGroup(public)##msiCommit|msiRollback##msiRollback##msiRollback##nop
 *
 * From the irods wiki: https://www.irods.org/index.php/Rules
 *
 * The "acCreateUser" rule, for example, calls "msiCreateUser". 
 * If that succeeds it invokes the "acCreateDefaultCollections" rule (which calls other rules and msi routines). 
 * Then, if they all succeed, the "msiCommit" function is called to save persistent state information.
 *
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence rei->rsComm->clientUser.authFlag (must be admin)
 * \DolVarModified none
 * \iCatAttrDependence checks various tables
 * \iCatAttrModified updates various tables
 * \sideeffect none
 *
 * \return integer
 * \retval (i)
 * \pre none
 * \post none
 * \sa none
 * \bug  no known bugs
**/
int msiCreateUser(ruleExecInfo_t *rei)
{
  int i;
  /**** This is Just a Test Stub  ****/
  if (reTestFlag > 0 ) {
    if (reTestFlag == COMMAND_TEST_1 || reTestFlag == HTML_TEST_1) {
      print_uoi(rei->uoio);
    }
    else {
      rodsLog (LOG_NOTICE,"   Calling msiCreateUser For \n");
      print_uoi(rei->uoio);
    }
    if (reLoopBackFlag > 0) {
      rodsLog (LOG_NOTICE,
	       "   Test mode, returning without performing normal operations (chlRegUserRE)");
      return(0);
    }
  }
  /**** End of Test Stub  ****/

#ifdef RODS_CAT
  i =  chlRegUserRE(rei->rsComm, rei->uoio);
#else
  i =  SYS_NO_ICAT_SERVER_ERR;
#endif
  return(i);
}

/**
 * \fn msiCreateCollByAdmin (msParam_t *xparColl, msParam_t *xchildName, ruleExecInfo_t *rei)
 *
 * \brief   This microservice creates a collection by an administrator
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author  Wayne Schroeder
 * \date  2008 or before
 *
 * \remark Jewel Ward - msi documentation, 2009-06-18
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-25
 *
 * \note Should not be used outside of the rules defined in core.irb.
 * This is called via an 'iadmin' command.
 *
 * \usage
 * 
 * As seen in server/config/reConfigs/core.irb
 *
 * acCreateCollByAdmin(*parColl,*childColl)||msiCreateCollByAdmin(*parColl,*childColl)|nop
 *
 * \param[in] xparColl - a msParam of type STR_MS_T
 * \param[in] xchildName - a msParam of type STR_MS_T
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence rei->rsComm->clientUser.authFlag (must be admin)
 * \DolVarModified none
 * \iCatAttrDependence checks various tables
 * \iCatAttrModified updates various tables
 * \sideeffect none
 *
 * \return integer
 * \retval (i)
 * \pre none
 * \post none
 * \sa none
 * \bug  no known bugs
**/
int msiCreateCollByAdmin(msParam_t* xparColl, msParam_t* xchildName, ruleExecInfo_t *rei)
{
    int i;
    collInfo_t collInfo;
  char *parColl;
  char *childName;

  parColl = (char *) xparColl->inOutStruct;
  childName = (char *) xchildName->inOutStruct;
  /**** This is Just a Test Stub  ****/
  if (reTestFlag > 0 ) {
    if (reTestFlag == COMMAND_TEST_1 || reTestFlag == HTML_TEST_1) {
      fprintf(stdout,"  NewCollection =%s/%s\n",
	       parColl,childName);
    }
    else {
      rodsLog (LOG_NOTICE,"   Calling msiCreateCollByAdmin Coll: %s/%s\n",
	       parColl,childName);
    }
    if (reLoopBackFlag > 0) {
      rodsLog (LOG_NOTICE,
	       "   Test mode, returning without performing normal operations (chlRegCollByAdmin)");
      return(0);
    }
  }
  /**** End of Test Stub  ****/
  memset(&collInfo, 0, sizeof(collInfo_t));
  snprintf(collInfo.collName, sizeof(collInfo.collName), 
	   "%s/%s",parColl,childName);
  snprintf(collInfo.collOwnerName, sizeof(collInfo.collOwnerName),
	   "%s",rei->uoio->userName);
  snprintf(collInfo.collOwnerZone, sizeof(collInfo.collOwnerZone),
	   "%s",rei->uoio->rodsZone);
	   

#ifdef RODS_CAT
  i =  chlRegCollByAdmin(rei->rsComm, &collInfo );
#else
  i =  SYS_NO_RCAT_SERVER_ERR;
#endif
  return(i);
}

/**
 * \fn msiDeleteCollByAdmin (msParam_t *xparColl, msParam_t *xchildName, ruleExecInfo_t *rei)
 *
 * \brief   This microservice deletes a collection by administrator 
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author  Wayne Schroeder
 * \date  2008 or before
 *
 * \remark Jewel Ward - msi documentation, 2009-06-18
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-25
 *
 * \note Should not be used outside of the rules defined in core.irb.
 * This is called via an 'iadmin' command.
 *
 * \usage
 * 
 * As seen in server/config/reConfigs/core.irb
 *
 * acDeleteCollByAdmin(*parColl,*childColl)||msiDeleteCollByAdmin(*parColl,*childColl)|nop
 *
 *
 * \param[in] xparColl - a msParam of type STR_MS_T
 * \param[in] xchildName - a msParam of type STR_MS_T
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence rei->rsComm->clientUser.authFlag (must be admin)
 * \DolVarModified none
 * \iCatAttrDependence checks various tables
 * \iCatAttrModified updates various tables
 * \sideeffect none
 *
 * \return integer
 * \retval (i)
 * \pre none
 * \post none
 * \sa none
 * \bug  no known bugs
**/
int msiDeleteCollByAdmin(msParam_t* xparColl, msParam_t* xchildName, ruleExecInfo_t *rei)
{
   int i;
   collInfo_t collInfo;
  char *parColl;
  char *childName;

  parColl = (char *) xparColl->inOutStruct;
  childName = (char *) xchildName->inOutStruct;
   /**** This is Just a Test Stub  ****/
   if (reTestFlag > 0 ) {
      if (reTestFlag == COMMAND_TEST_1 || reTestFlag == HTML_TEST_1) {
	 fprintf(stdout,"  NewCollection =%s/%s\n",
		 parColl,childName);
      }
      else {
	 rodsLog (LOG_NOTICE,"   Calling msiDeleteCallByAdmin Coll: %s/%s\n",
		  parColl,childName);
      }
      rodsLog (LOG_NOTICE,
	       "   Test mode, returning without performing normal operations (chlDelCollByAdmin)");
      return(0);
   }
   /**** End of Test Stub  ****/

   snprintf(collInfo.collName, sizeof(collInfo.collName), 
	    "%s/%s",parColl,childName);
   snprintf(collInfo.collOwnerName, sizeof(collInfo.collOwnerName),
	    "%s",rei->uoio->userName);
   snprintf(collInfo.collOwnerZone, sizeof(collInfo.collOwnerZone),
	    "%s",rei->uoio->rodsZone);

#ifdef RODS_CAT
   i = chlDelCollByAdmin(rei->rsComm, &collInfo );
#else
   i = SYS_NO_RCAT_SERVER_ERR;
#endif
   if (i == CAT_UNKNOWN_COLLECTION) {
      /* Not sure where this kind of logic belongs, chl, rules,
         or here; but for now it's here.  */
      /* If the error is that it does not exist, return OK. */
      freeRErrorContent(&rei->rsComm->rError); /* remove suberrors if any */
      return(0); 
   }
   return(i);
}

/**
 * \fn msiDeleteUser (ruleExecInfo_t *rei)
 *
 * \brief   This microservice deletes a user
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author  Wayne Schroeder
 * \date  2008 or before
 *
 * \remark Jewel Ward - msi documentation, 2009-06-18
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-25
 *
 * \note Should not be used outside of the rules defined in core.irb.
 * This is called via an 'iadmin' command.
 *
 * \usage
 * 
 * As seen in server/config/reConfigs/core.irb
 *
 * acDeleteUserF1||acDeleteDefaultCollections##msiDeleteUser##msiCommit|msiRollback##msiRollback##nop
 *
 *
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence rei->rsComm->clientUser.authFlag (must be admin)
 * \DolVarModified none
 * \iCatAttrDependence checks various tables
 * \iCatAttrModified updates various tables
 * \sideeffect none
 *
 * \return integer
 * \retval (i)
 * \pre none
 * \post none
 * \sa none
 * \bug  no known bugs
**/
int 
msiDeleteUser(ruleExecInfo_t *rei)
{
  int i;
  /**** This is Just a Test Stub  ****/
  if (reTestFlag > 0 ) {
    if (reTestFlag == COMMAND_TEST_1 || reTestFlag == HTML_TEST_1) {
      print_uoi(rei->uoio);
    }
    else {
      rodsLog (LOG_NOTICE,"   Calling chlDeleteUser For \n");
      print_uoi(rei->uoio);
    }
    rodsLog (LOG_NOTICE,
	     "   Test mode, returning without performing normal operations (chlDelUserRE)");
    return(0);
  }
  /**** End of Test Stub  ****/

#ifdef RODS_CAT
  i =  chlDelUserRE(rei->rsComm, rei->uoio);
#else
  i = SYS_NO_RCAT_SERVER_ERR;
#endif
  return(i);
}

/**
 * \fn msiAddUserToGroup (msParam_t *msParam, ruleExecInfo_t *rei)
 *
 * \brief   This microservice adds a user to a group
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author  Wayne Schroeder
 * \date  2008 or before
 *
 * \remark Jewel Ward - msi documentation, 2009-06-18
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-25
 *
 * \note Should not be used outside of the rules defined in core.irb.
 * This is called via an 'iadmin' command.
 *
 * \usage
 * 
 * As seen in server/config/reConfigs/core.irb
 *
 * testrule||msiCreateUser##acCreateDefaultCollections##msiAddUserToGroup(public)##msiCommit|msiRollback##msiRollback##msiRollback##nop
 *
 *
 * \param[in] msParam - a msParam of type STR_MS_T, the name of the group
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence rei->rsComm->clientUser (must be group admin)
 * \DolVarModified none
 * \iCatAttrDependence checks various tables
 * \iCatAttrModified updates various tables
 * \sideeffect none
 *
 * \return integer
 * \retval (i)
 * \pre none
 * \post none
 * \sa none
 * \bug  no known bugs
**/
int 
msiAddUserToGroup(msParam_t *msParam, ruleExecInfo_t *rei)
{
  int i;
#ifdef RODS_CAT
  char *groupName;
#endif
  if (reTestFlag > 0 ) {  /* Test stub mode */
    if (reTestFlag == COMMAND_TEST_1 || reTestFlag == HTML_TEST_1) {
      print_uoi(rei->uoio);
    }
    else {
      rodsLog (LOG_NOTICE,"   Calling chlModGroup For \n");
      print_uoi(rei->uoio);
    }
    rodsLog (LOG_NOTICE,
	     "   Test mode, returning without performing normal operations (chlModGroup)");
    return(0);
  }
#ifdef RODS_CAT
  if (strncmp(rei->uoio->userType,"rodsgroup",9)==0) {
     return(0);
  }
  groupName =  (char *) msParam->inOutStruct;
  i =  chlModGroup(rei->rsComm, groupName, "add", rei->uoio->userName,
		   rei->uoio->rodsZone);
#else
  i = SYS_NO_RCAT_SERVER_ERR;
#endif
  return(i);
}

/**
 * \fn msiRenameLocalZone (msParam_t *oldName, msParam_t *newName, ruleExecInfo_t *rei)
 *
 * \brief   This microservice renames the local zone by updating various tables
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author  Wayne Schroeder
 * \date  October 2008
 *
 * \remark Jewel Ward - msi documentation, 2009-06-18
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-25
 *
 * \note Should not be used outside of the rules defined in core.irb.
 * This is called via an 'iadmin' command.
 *
 * \usage
 * 
 * As seen in server/config/reConfigs/core.irb
 *
 * acRenameLocalZone(*oldZone,*newZone)||msiRenameCollection(\*oldZone,*newZone)##msiRenameLocalZone(*oldZone,*newZone)##msiCommit|msiRollback##msiRollback##nop
 * (Note that the \ should be / but was changed to avoid a compiler warning
 *  about a slash * appearing in a comment.)
 *
 * \param[in] oldName - a msParam of type STR_MS_T
 * \param[in] newName - a msParam of type STR_MS_T
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence rei->rsComm->clientUser (must be admin)
 * \DolVarModified none
 * \iCatAttrDependence checks various tables
 * \iCatAttrModified updates various tables
 * \sideeffect none
 *
 * \return integer
 * \retval status
 * \pre none
 * \post none
 * \sa none
 * \bug  no known bugs
**/
int
msiRenameLocalZone(msParam_t* oldName, msParam_t* newName, ruleExecInfo_t *rei)
{
   int status;
   char *oldNameStr;
   char *newNameStr;

   oldNameStr = (char *) oldName->inOutStruct;
   newNameStr = (char *) newName->inOutStruct;
#ifdef RODS_CAT
   status = chlRenameLocalZone(rei->rsComm, oldNameStr, newNameStr);
#else
   status = SYS_NO_RCAT_SERVER_ERR;
#endif
   return(status);
}

/**
 * \fn msiRenameCollection (msParam_t *oldName, msParam_t *newName, ruleExecInfo_t *rei)
 *
 * \brief   This function renames a collection; used via a Rule with #msiRenameLocalZone
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author  Wayne Schroeder
 * \date  October 2008
 *
 * \remark Jewel Ward - msi documentation, 2009-06-18
 * \remark Terrell Russell - reviewed msi documentation, 2009-06-25
 *
 * \note Should not be used outside of the rules defined in core.irb.
 * This is called via an 'iadmin' command.
 *
 * \usage
 * 
 * As seen in server/config/reConfigs/core.irb
 *
 * acRenameLocalZone(*oldZone,*newZone)||msiRenameCollection(\*oldZone,*newZone)##msiRenameLocalZone(*oldZone,*newZone)##msiCommit|msiRollback##msiRollback##nop
 * (Note that the \ should be / but was changed to avoid a compiler warning
 *  about a slash * appearing in a comment.)
 *
 * \param[in] oldName - a msParam of type STR_MS_T
 * \param[in] newName - a msParam of type STR_MS_T
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence rei->rsComm->clientUser (must have access (admin))
 * \DolVarModified none
 * \iCatAttrDependence checks various tables
 * \iCatAttrModified updates various tables
 * \sideeffect none
 *
 * \return integer
 * \retval status
 * \pre none
 * \post none
 * \sa none
 * \bug  no known bugs
**/
int
msiRenameCollection(msParam_t* oldName, msParam_t* newName, ruleExecInfo_t *rei)
{
   int status;
   char *oldNameStr;
   char *newNameStr;

   oldNameStr = (char *) oldName->inOutStruct;
   newNameStr = (char *) newName->inOutStruct;
#ifdef RODS_CAT
   status = chlRenameColl(rei->rsComm, oldNameStr, newNameStr);
#else
   status = SYS_NO_RCAT_SERVER_ERR;
#endif
   return(status);
}

/**
 * \fn msiAclPolicy(msParam_t *msParam, ruleExecInfo_t *rei)
 *
 * \brief   When called (e.g. from acAclPolicy) and with "STRICT" as the
 *    argument, this will set the ACL policy (for GeneralQuery) to be
 *    extended (most strict).  
 *
 * \module core
 *
 * \since pre-2.1
 *
 * \author  Wayne Schroeder
 * \date  March 2009
 *
 * \remark Terrell Russell - msi documentation, 2009-06-30
 *
 * \note Should not be used outside of the rules defined in core.irb.
 * Once set STRICT, strict mode remains in force (users can't call it in
 * another rule to change the mode back to non-strict).
 * See core.irb.
 *
 * \usage None
 *
 * \param[in] msParam - a msParam of type STR_MS_T - can have value 'STRICT'
 * \param[in,out] rei - The RuleExecInfo structure that is automatically
 *    handled by the rule engine. The user does not include rei as a
 *    parameter in the rule invocation.
 *
 * \DolVarDependence 
 * \DolVarModified 
 * \iCatAttrDependence 
 * \iCatAttrModified 
 * \sideeffect none
 *
 * \return integer
 * \retval status
 * \pre none
 * \post none
 * \sa none
 * \bug  no known bugs
**/
int
msiAclPolicy(msParam_t *msParam, ruleExecInfo_t *rei)
{
#if 0
   msParamArray_t *myMsParamArray;
   int flag=1;
#endif
   char *inputArg;

   inputArg =  (char *) msParam->inOutStruct;
   if (inputArg != NULL) {
      if (strncmp(inputArg,"STRICT",6)==0) {
#if 0
	 /* No longer need this as we're calling
	    chlGenQueryAccessControlSetup directly below (in case
	    msiAclPolicy will be called in a different manner than via
	    acAclPolicy sometime).
	    Leaving it in (ifdef'ed out tho) in case needed later.
	 */
	 myMsParamArray = mallocAndZero (sizeof (msParamArray_t));
	 addMsParamToArray (myMsParamArray, "STRICT", INT_MS_T, &flag,
			    NULL, 0);
	 rei->inOutMsParamArray=*myMsParamArray;
#endif
#ifdef RODS_CAT
	 chlGenQueryAccessControlSetup(NULL, NULL, 0, 2);
#endif
      }
   }
   else {
#ifdef RODS_CAT
      chlGenQueryAccessControlSetup(NULL, NULL, 0, 0);
#endif
   }
   return (0);
}
