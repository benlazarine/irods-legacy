/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/

/* 
  User interface to display quota information.
*/

#include "rods.h"
#include "rodsClient.h"

#define BIG_STR 200
#define QUOTA_APPROACH_WARNING_SIZE -10000000000LL

int debug=0;
char quotaTime[20]="";
int printedFlag[3]={0,0,0};
rcComm_t *Conn;
rodsEnv myEnv;

void usage();


/*
 print a number nicely, that is with commas and summary magnitude
 */
int
printNice(char *inArg, int minLen, char *Units) {
   int i, n, k, len;
   int nextComma, firstComma, commaCount;
   char niceString[40];
   char firstPart[10];
   char *numberName;
   char blanks[]="                                        ";
   len=strlen(inArg);
   nextComma = len % 3;
   if (inArg[0]=='-') {
      nextComma = (len-1) % 3;
   }
   if (nextComma == 0) nextComma=3;
   n = 0;
   k = 0;
   firstComma=0;
   commaCount=0;
   for (i=0;i<len;i++) {
      niceString[n++]=inArg[i];
      if (firstComma==0) {
	 firstPart[k++]=inArg[i];
      }
      if (inArg[i]!='-') {
	 nextComma--;
      }
      if (nextComma==0 && i<len-1) {
	 niceString[n++]=',';
	 nextComma=3;
	 firstComma=1;
	 commaCount++;
      }
   }
   niceString[n]='\0';
   firstPart[k]='\0';
   numberName="";
   if (commaCount == 1) numberName="thousand";
   if (commaCount == 2) numberName="million";
   if (commaCount == 3) numberName="billion";
   if (commaCount == 4) numberName="trillion";
   if (commaCount == 5) numberName="quadrillion";
   if (commaCount == 6) numberName="quintillion";
   if (commaCount == 7) numberName="sextillion";
   if (commaCount == 8) numberName="septillion";
   if (commaCount > 8) numberName="very many";
   len = strlen(niceString);
   if (len < minLen) {
      i = minLen-len;
      blanks[i]='\0';
      printf("%s", blanks);
      blanks[i]=' ';
   }
   if (commaCount==0) {
      printf("%s %s", niceString, Units);
   }
   else {
      printf("%s (%s %s) %s", niceString, 
	       firstPart, numberName, Units);
   }
   return(0);
}



/*
  Show user quota information
*/
int
showQuotas(char *userName, int userOrGroup, int rescOrGlobal) 
{
   genQueryInp_t genQueryInp;
   genQueryOut_t *genQueryOut;
   int inputInx[20];
   int inputVal[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   int inputCond[20];
   char *condVal[10];
   char v1[BIG_STR];
   char v2[BIG_STR];
   char v3[BIG_STR];
   int i, j, k, status;
   int  localiTime=0;
   int printCount;
   static int printedTime=0;
   char *colName[10];

   memset (&genQueryInp, 0, sizeof (genQueryInp_t));
   printCount=0;
   i=0;
   if (rescOrGlobal==0) {
      colName[i]="Resource: ";
      inputInx[i++]=COL_QUOTA_RESC_NAME;
   }
   else {
      colName[i]="Resource: ";
      inputInx[i++]=COL_QUOTA_RESC_ID;
   }
   if (userOrGroup==0) {
      colName[i]="User:  ";
   }
   else {
      colName[i]="Group:  ";
   }
   inputInx[i++]=COL_QUOTA_USER_NAME;
   colName[i]="Quota: ";
   inputInx[i++]=COL_QUOTA_LIMIT;
   colName[i]="Over:  ";
   inputInx[i++]=COL_QUOTA_OVER;
   colName[i]="Time";
   inputInx[i++]=COL_QUOTA_MODIFY_TIME;

   genQueryInp.selectInp.inx = inputInx;
   genQueryInp.selectInp.value = inputVal;
   genQueryInp.selectInp.len = i;

   genQueryInp.sqlCondInp.len=0;
   if (userName[0]!='\0') {
      inputCond[0]=COL_QUOTA_USER_NAME;
      sprintf(v1,"='%s'",userName);
      condVal[0]=v1;
      genQueryInp.sqlCondInp.len++;
   }
   inputCond[genQueryInp.sqlCondInp.len] = COL_QUOTA_USER_TYPE;
   if (userOrGroup==0) {
      sprintf(v2,"!='%s'","rodsgroup");
   }
   else {
      sprintf(v2,"='%s'","rodsgroup");
   }
   condVal[genQueryInp.sqlCondInp.len] = v2;
   genQueryInp.sqlCondInp.len++;

   if (rescOrGlobal==1) {
      inputCond[genQueryInp.sqlCondInp.len] = COL_QUOTA_RESC_ID;
      sprintf(v3, "='%s'", "0");
      condVal[genQueryInp.sqlCondInp.len] = v3;
      genQueryInp.sqlCondInp.len++;
   }


   genQueryInp.sqlCondInp.inx = inputCond;
   genQueryInp.sqlCondInp.value = condVal;

   genQueryInp.condInput.len=0;

   genQueryInp.maxRows=MAX_SQL_ROWS;

   genQueryInp.continueInx=0;
   status = rcGenQuery(Conn, &genQueryInp, &genQueryOut);
   if (status == CAT_NO_ROWS_FOUND) {
      printf("None\n\n");
      return(0);
   }
   if (status!=0) {
      printError(Conn, status, "rcGenQuery");
      return(status);
   }

   if (genQueryOut->rowCnt > 0 && printedTime==0) {
      for (i=0;i<1;i++) {
	 for (j=0;j<genQueryOut->attriCnt;j++) {
	    char *tResult;
	    long itime;

	    tResult = genQueryOut->sqlResult[j].value;
	    tResult += i*genQueryOut->sqlResult[j].len;
	    if (j==4) {
	       itime = atoll(tResult);
	       if (itime > localiTime) {
		  localiTime = itime;
		  getLocalTimeFromRodsTime(tResult, quotaTime);
	       }
	    }
	 }
      }
   }
   printCount=0;
   for (i=0;i<genQueryOut->rowCnt;i++) {
      for (j=0;j<genQueryOut->attriCnt;j++) {
	 char *tResult;
	 long itime;
	 tResult = genQueryOut->sqlResult[j].value;
	 tResult += i*genQueryOut->sqlResult[j].len;
	 if (j==4) {
	    itime = atoll(tResult);
	    if (itime > localiTime) {
	       localiTime = itime;
	       getLocalTimeFromRodsTime(tResult, quotaTime);
	    }
	 }
	 else {
	    printf("  %s",colName[j]);
	    if (rescOrGlobal==1 && j==0) {
	       tResult="All";
	    }
	    if (j==3 || j==2) {
	       printNice(tResult, 0, "bytes");
	       if (strncmp(colName[j],"Over:",5)==0) {
		  rodsLong_t ival;
		  ival = atoll(tResult);
		  if (ival > 0) {
		     printf(" OVER QUOTA");
		  }
		  else {
		     if (ival > QUOTA_APPROACH_WARNING_SIZE) {
			printf(" (Nearing quota)");
		     }
		     else {
			printf(" (under quota)");
		     }
		  }

	       }
	       printf("\n");
	    }
	    else {
	       k = strlen(tResult);
	       printf("%s\n",tResult);
	    }
	    printCount++;
	 }
      }
      printf("\n");
   }
   return (0);
}

/*
  Show user quota information
*/
int
showUserUsage(char *userName) 
{
   genQueryInp_t genQueryInp;
   genQueryOut_t *genQueryOut;
   int inputInx[20];
   int inputVal[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   int inputCond[20];
   char *condVal[10];
   char v1[BIG_STR];
   int i, j, k, status;
   int printCount;
   char *tResult;
   char header[]=
      "Resource     User         User-type       Data-stored (bytes)";
   char *pad[13]={"            ",
		  "           ",
		  "          ",
		  "         ",
		  "        ",
		  "       ",
		  "      ",
		  "     ",
		  "    ",
		  "   ",
		  "  ",
		  " ",
		  ""};
   int  localiTime=0;

   memset (&genQueryInp, 0, sizeof (genQueryInp_t));
   printCount=0;
   i=0;
   inputInx[i++]=COL_QUOTA_USAGE_MODIFY_TIME;
   inputInx[i++]=COL_QUOTA_RESC_NAME;
   inputInx[i++]=COL_QUOTA_USER_NAME;
   inputInx[i++]=COL_QUOTA_USER_TYPE;
   inputInx[i++]=COL_QUOTA_USAGE;

   genQueryInp.selectInp.inx = inputInx;
   genQueryInp.selectInp.value = inputVal;
   genQueryInp.selectInp.len = i;

   genQueryInp.sqlCondInp.len=0;
   if (userName[0]!='\0') {
      inputCond[0]=COL_QUOTA_USER_NAME;
      sprintf(v1,"='%s'",userName);
      condVal[0]=v1;

      genQueryInp.sqlCondInp.inx = inputCond;
      genQueryInp.sqlCondInp.value = condVal;
      genQueryInp.sqlCondInp.len++;
   }

   genQueryInp.condInput.len=0;

   genQueryInp.maxRows=MAX_SQL_ROWS;
   genQueryInp.continueInx=0;
   status = rcGenQuery(Conn, &genQueryInp, &genQueryOut);
   if (status == CAT_NO_ROWS_FOUND) {
      printf("No records found, run 'iadmin cu' to set calcuate usage\n");
      return(0);
   }
   if (status!=0) {
      printError(Conn, status, "rcGenQuery");
      return(status);
   }

   printf("%s\n", header);
   printCount=0;

   for (i=0;i<genQueryOut->rowCnt;i++) {
      for (j=1;j<genQueryOut->attriCnt;j++) {
	 char *tResult;
	 tResult = genQueryOut->sqlResult[j].value;
	 tResult += i*genQueryOut->sqlResult[j].len;
	 if (j==4) {
	    printNice(tResult, 14, "");
	 }
	 else {
	    printf("%s ",tResult);
	 }
	 k = strlen(tResult);
	 if (k < 20) printf("%s",pad[k]);
	 printCount++;
      }
      printf("\n");
   }
   for (i=0;i<genQueryOut->rowCnt;i++) {
      long itime;
      tResult = genQueryOut->sqlResult[i].value;
      itime = atoll(tResult);
      if (itime > localiTime) {
	 localiTime = itime;
	 getLocalTimeFromRodsTime(tResult, quotaTime);
      }
   }
   return (0);
}

int
main(int argc, char **argv) {
   int status, nArgs;
   rErrMsg_t errMsg;
   char userName[NAME_LEN];

   rodsArguments_t myRodsArgs;

   rodsLogLevel(LOG_ERROR);

   status = parseCmdLineOpt (argc, argv, "au:vVh", 0, &myRodsArgs);
   if (status) {
      printf("Use -h for help.\n");
      exit(1);
   }
   if (myRodsArgs.help==True) {
      usage();
      exit(0);
   }

   status = getRodsEnv (&myEnv);
   if (status < 0) {
      rodsLog (LOG_ERROR, "main: getRodsEnv error. status = %d",
	       status);
      exit (1);
   }

   Conn = rcConnect (myEnv.rodsHost, myEnv.rodsPort, myEnv.rodsUserName,
                     myEnv.rodsZone, 0, &errMsg);

   if (Conn == NULL) {
      exit (2);
   }

   status = clientLogin(Conn);
   if (status != 0) {
      if (!debug) exit (3);
   }

   strncpy(userName, myEnv.rodsUserName, NAME_LEN);
   if (myRodsArgs.user)  strncpy(userName, myRodsArgs.userString, NAME_LEN);
   if (myRodsArgs.all) userName[0]='\0';

   nArgs = argc - myRodsArgs.optind;

   if (nArgs > 0) {
      if (strncmp(argv[myRodsArgs.optind],"usage",5)==0) {
	 status = showUserUsage(userName);
      }
      else {
	 usage();
      }
   }
   else {
      if (userName[0]=='\0') {
	 printf("Resource quotas for users:\n");
      }
      else {
	 printf("Resource quotas for user %s:\n", userName);
      }
      status = showQuotas(userName, 0, 0); /* users, resc */

      if (userName[0]=='\0') {
	 printf("Global (total) quotas for users:\n");
      }
      else {
	 printf("Global (total) quotas for user %s:\n", userName);
      }
      status = showQuotas(userName, 0, 1); /* users, global */

      printf("Group quotas on resources:\n");
      status = showQuotas("", 1, 0);       /* all groups, resc */

      printf("Group global (total) quotas:\n");
      status = showQuotas("", 1, 1);       /* all groups, global */
   }
   if (quotaTime[0]!='\0') {
      printf("Information was set at %s\n", quotaTime);
   }
   
   rcDisconnect(Conn);

   exit(0);
}

/*
Print the main usage/help information.
 */
void usage()
{
   char *msgs[]={
"Usage: iquota [-uavVh] [UserName] [usage]", 
" ", 
"Show information on iRODS quotas (if any).", 
"By default, information is displayed for the current iRODS user.",
" ",
"The 'over' values indicate whether the user or group is over quota",
"or not and by how much; positive values are over-quota.",
"The 'usage' information shows how much data is stored on each resource,",
"when last determined.",
" ",
"Options:",
"-a All users",
"-u UserName  - for the specified user",
"usage     - show usage information",
" ",
"Examples:",
"iquota",
"iquota -a",
"iquota usage",
"iqouta -a usage",
""};
   int i;
   for (i=0;;i++) {
      if (strlen(msgs[i])==0) break;
      printf("%s\n",msgs[i]);
   }
   printReleaseInfo("iquota");
}