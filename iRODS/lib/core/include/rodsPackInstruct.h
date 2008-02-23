/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* packInstruct.h - header file for pack instruction definition 
 */



#ifndef PACK_INSTRUCT_H
#define PACK_INSTRUCT_H

#define IRODS_STR_PI "str myStr[MAX_NAME_LEN];"
#define STR_PI "str myStr;" 
#define STR_PTR_PI "str *myStr;" 
#define PI_STR_PI "piStr myStr[MAX_NAME_LEN];" 
#define INT_PI "int myInt;"
#define BUF_LEN_PI "int myInt;"
#define DOUBLE_PI "double myDouble;"

/* packInstruct for msgHeader_t */
#define MsgHeader_PI "str type[HEADER_TYPE_LEN]; int msgLen; int errorLen; int bsLen; int intInfo;"

/* packInstruct for startupPack_t */
#define StartupPack_PI "int irodsProt; int reconnFlag; int connectCnt; str proxyUser[NAME_LEN]; str proxyRcatZone[NAME_LEN]; str clientUser[NAME_LEN]; str clientRcatZone[NAME_LEN]; str relVersion[NAME_LEN]; str apiVersion[NAME_LEN]; str option[NAME_LEN];"

/* packInstruct for version_t */

#define Version_PI "int status; str relVersion[NAME_LEN]; str apiVersion[NAME_LEN]; int reconnPort; str reconnAddr[LONG_NAME_LEN]; int cookie;"

/* packInstruct for rErrMsg_t */

#define RErrMsg_PI "int status; str msg[ERR_MSG_LEN];"

/* packInstruct for rError_t */

#define RError_PI "int count; struct *RErrMsg_PI[count];"

#define RHostAddr_PI "str hostAddr[LONG_NAME_LEN]; str rodsZone[NAME_LEN]; int port; int dummyInt;"

#define RODS_STAT_T_PI "double st_size; int st_dev; int st_ino; int st_mode; int st_nlink; int st_uid; int st_gid; int st_rdev; int st_atim; int st_mtim; int st_ctim; int st_blksize; int st_blocks; int st_vfstype; int st_vfs; int st_type; int st_gen; int st_flag;"

#define RODS_DIRENT_T_PI "int d_offset; int d_ino; int d_reclen; int d_namlen; str d_name[DIR_LEN];" 

#define KeyValPair_PI "int ssLen; str *keyWord[ssLen]; str *svalue[ssLen];" 

#define InxIvalPair_PI "int iiLen; int *inx(iiLen); int *ivalue(iiLen);" 

#define InxValPair_PI "int isLen; int *inx(isLen); str *svalue[isLen];" 

#define DataObjInp_PI "str objPath[MAX_NAME_LEN]; int createMode; int openFlags; double offset; double dataSize; int numThreads; int oprType; struct *SpecColl_PI; struct KeyValPair_PI;"

#define PortList_PI "int portNum; int cookie; int sock; int windowSize; str hostAddr[LONG_NAME_LEN];"

#define PortalOprOut_PI "int status; int l1descInx; int numThreads; str chksum[NAME_LEN]; struct PortList_PI;"

#define DataOprInp_PI "int oprType; int numThreads; int srcL3descInx; int destL3descInx; int srcRescTypeInx; int destRescTypeInx; double offset; double dataSize; struct KeyValPair_PI;"

#define CollInp_PI "str collName[MAX_NAME_LEN]; struct KeyValPair_PI;"

#define GenQueryInp_PI "int maxRows; int continueInx; int partialStartIndex; int options; struct KeyValPair_PI; struct InxIvalPair_PI; struct InxValPair_PI;"
#define SqlResult_PI "int attriInx; int reslen; str *value(rowCnt)(reslen);"  

#define GenQueryOut_PI "int rowCnt; int attriCnt; int continueInx; int totalRowCount; struct SqlResult_PI[MAX_SQL_ATTR];"
#define DataObjInfo_PI "str objPath[MAX_NAME_LEN]; str rescName[NAME_LEN]; str rescGroupName[NAME_LEN]; str dataType[NAME_LEN]; double dataSize; str chksum[NAME_LEN]; str version[NAME_LEN]; str filePath[MAX_NAME_LEN]; str *rescInfo; str dataOwnerName[NAME_LEN]; str dataOwnerZone[NAME_LEN]; int  replNum; int  replStatus; str statusString[NAME_LEN]; double  dataId; double collId; int  dataMapId; str dataComments[LONG_NAME_LEN]; str dataExpiry[TIME_LEN]; str dataCreate[TIME_LEN]; str dataModify[TIME_LEN]; str dataAccess[NAME_LEN]; int  dataAccessInx; str destRescName[NAME_LEN]; str backupRescName[NAME_LEN]; str subPath[MAX_NAME_LEN]; int *specColl; int *next;"

#define TransStat_PI "int numThreads; double bytesWritten;"

#define RescGrpInfo_PI "str rescGroupName[NAME_LEN]; str *rescName; int *cacheNext; struct *RescGrpInfo_PI;"
#define AuthInfo_PI "str authScheme[NAME_LEN]; int authFlag; int flag; int ppid; str host[NAME_LEN]; str authStr[NAME_LEN];"
#define UserOtherInfo_PI "str userInfo[NAME_LEN]; str userComments[NAME_LEN]; str userCreate[TIME_LEN]; str userModify[TIME_LEN];"

#define UserInfo_PI "str userName[NAME_LEN]; str rodsZone[NAME_LEN]; str userType[NAME_LEN]; int sysUid; struct AuthInfo_PI; struct UserOtherInfo_PI;"
#define CollInfo_PI "double collId; str collName[MAX_NAME_LEN]; str collParentName[MAX_NAME_LEN]; str collOwnerName[NAME_LEN]; str collOwnerZone[NAME_LEN]; int collMapId; str collComments[LONG_NAME_LEN]; str collInheritance[LONG_NAME_LEN]; str collExpiry[TIME_LEN]; str collCreate[TIME_LEN]; str collModify[TIME_LEN]; str collAccess[NAME_LEN]; int collAccessInx; str collType[NAME_LEN]; str collInfo1[MAX_NAME_LEN]; str collInfo2[MAX_NAME_LEN]; int *next;"

#define Rei_PI "int status; str statusStr[MAX_NAME_LEN]; int *rsComm;struct *MsParamArray_PI; struct MsParamArray_PI; int l1descInx; struct *DataObjInp_PI; struct *DataOprInp_PI; struct *fileOpenInp_PI; struct *DataObjInfo_PI; struct *RescGrpInfo_PI; struct *UserInfo_PI; struct *UserInfo_PI; struct *CollInfo_PI; struct *DataObjInp_PI; struct *DataOprInp_PI; struct *fileOpenInp_PI; struct *RescGrpInfo_PI; struct *UserInfo_PI; struct *KeyValPair_PI; str ruleSet[RULE_SET_DEF_LENGTH]; int *next;" 

#define ReArg_PI "int myArgc; str *myArgv[myArgc];"
#define ReiAndArg_PI "struct *Rei_PI; struct ReArg_PI;"

#define BytesBuf_PI "int buflen; char *buf(buflen);"
#define BinBytesBuf_PI "int buflen; bin *buf(buflen);"

#if 0
#define MsParam_PI "str *label; piStr *type; ?*type; struct *BytesBuf_PI;"
#else
#define MsParam_PI "str *label; piStr *type; ?type *inOutStruct; struct *BinBytesBuf_PI;"
#endif

#define MsParamArray_PI "int paramLen; int oprType; struct *MsParam_PI[paramLen];"
 
#define TagStruct_PI "int ssLen; str *preTag[ssLen]; str *postTag[ssLen]; str *keyWord[ssLen];" 

#define RodsObjStat_PI "double objSize; int objType; int numCopies; str dataId[NAME_LEN]; str chksum[NAME_LEN]; str ownerName[NAME_LEN]; str ownerZone[NAME_LEN]; str createTime[TIME_LEN]; str modifyTime[TIME_LEN]; struct *SpecColl_PI;" 
#define ReconnMsg_PI "int cookie; int reconnOpr;"
#define VaultPathPolicy_PI "int scheme; int addUserName; int trimDirCnt;"
#define StrArray_PI "int len; int size; str *value(len)(size);" 
#define IntArray_PI "int len; int *value(len);" 
#if 0
#define SpecCollMeta_PI "str objType[NAME_LEN]; str collection[MAX_NAME_LEN]; str collInfo1[MAX_NAME_LEN]; str collInfo2[MAX_NAME_LEN];" 
#endif
#define SpecColl_PI "int class; int type; str collection[MAX_NAME_LEN]; str objPath[MAX_NAME_LEN]; str resource[NAME_LEN]; str phyPath[MAX_NAME_LEN]; str cacheDir[MAX_NAME_LEN]; int cacheDirty; int replNum;"
#define SubFile_PI "struct RHostAddr_PI; str subFilePath[MAX_NAME_LEN]; int mode; int flags; double offset; struct *SpecColl_PI;" 
#define XmsgTicketInfo_PI "int sendTicket; int rcvTicket; int expireTime; int flag;"
#define SendXmsgInfo_PI "int msgNumber; str msgType[HEADER_TYPE_LEN]; int numRcv; int flag; str *msg; int numDel; str *delAddress[numDel]; int *delPort(numDel); str *miscInfo;"
#define GetXmsgTicketInp_PI "int expireTime;"
#define SendXmsgInp_PI "struct XmsgTicketInfo_PI; struct SendXmsgInfo_PI;"
#define RcvXmsgInp_PI "int rcvTicket; int msgNumber;"
#define RcvXmsgOut_PI "str msgType[HEADER_TYPE_LEN]; str sendUserName[NAME_LEN]; str *msg;"
/* XXXXX start of HDF5 PI */
#define h5File_PI "int fopID; str *filename; int ffid; struct *h5Group_PI; struct h5error_PI;int ftime;"
#define h5error_PI "str major[MAX_ERROR_SIZE]; str minor[MAX_ERROR_SIZE];"
#define h5Group_PI "int gopID; int gfid; int gobjID[OBJID_DIM]; str *gfullpath; str *dummyParent; int nGroupMembers; struct *h5Group_PI[nGroupMembers]; int nDatasetMembers; struct *h5Dataset_PI[nDatasetMembers]; int nattributes; struct *h5Attribute_PI[nattributes]; struct h5error_PI;int gtime;"
/* XXXXX need to fix the type dependence */
#define h5Attribute_PI "int aopID; int afid; str *aname; str *aobj_path; int aobj_type; int aclass; struct h5Datatype_PI; struct h5Dataspace_PI; int nvalue; ? aclass:3,6,9 = str *value[nvalue][NULL_TERM]:default= char *value[nvalue]; struct h5error_PI;"
/* XXXXX need to fix the type dependence */
#define h5Dataset_PI "int dopID; int dfid; int dobjID[OBJID_DIM]; int dclass; str *dfullpath; int nattributes; struct *h5Attribute_PI[nattributes]; struct h5Datatype_PI; struct h5Dataspace_PI; int nvalue; ? dclass:3,6,9 = str *value[nvalue][NULL_TERM]:default= char %value[nvalue]; struct h5error_PI;int dtime;"
#define h5Datatype_PI "int tclass; int torder; int tsign; int tsize; int ntmenbers; int *mtypes[ntmenbers]; str *mnames[ntmenbers];"
#define h5Dataspace_PI "int rank; int dims[H5S_MAX_RANK]; int npoints; int start[H5DATASPACE_MAX_RANK]; int stride[H5DATASPACE_MAX_RANK]; int count[H5DATASPACE_MAX_RANK];"
/* XXXXX end of HDF5 PI */

#endif	/* PACK_INSTRUCT_H */
