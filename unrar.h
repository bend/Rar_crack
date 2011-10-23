#ifndef __UNRAR_H__
#define __UNRAR_H__

#include <stdlib.h>

#define ERAR_END_ARCHIVE        10
#define ERAR_NO_MEMORY          11
#define ERAR_BAD_DATA           12
#define ERAR_BAD_ARCHIVE        13
#define ERAR_UNKNOWN_FORMAT     14
#define ERAR_EOPEN              15
#define ERAR_ECREATE            16
#define ERAR_ECLOSE             17
#define ERAR_EREAD              18
#define ERAR_EWRITE             19
#define ERAR_SMALL_BUF          20
#define ERAR_UNKNOWN            21
#define ERAR_MISSING_PASSWORD   22

#define RAR_OM_LIST              0
#define RAR_OM_EXTRACT           1
#define RAR_OM_LIST_INCSPLIT     2

#define RAR_SKIP              0
#define RAR_TEST              1
#define RAR_EXTRACT           2

#define RAR_VOL_ASK           0
#define RAR_VOL_NOTIFY        1

#define RAR_DLL_VERSION       5

struct RARHeaderData
{
    char           ArcName[260];
    char           FileName[260];
    unsigned int   Flags;
    unsigned int   PackSize;
    unsigned int   UnpSize;
    unsigned int   HostOS;
    unsigned int   FileCRC;
    unsigned int   FileTime;
    unsigned int   UnpVer;
    unsigned int   Method;
    unsigned int   FileAttr;
    char*          CmtBuf;
    unsigned int   CmtBufSize;
    unsigned int   CmtSize;
    unsigned int   CmtState;
};


struct RAROpenArchiveData
{
    char*          ArcName;
    unsigned int   OpenMode;
    unsigned int   OpenResult;
    char*          CmtBuf;
    unsigned int   CmtBufSize;
    unsigned int   CmtSize;
    unsigned int   CmtState;
};

typedef int (*unrar_callback)(unsigned int msg, long UserData, long P1, long P2);


enum UNRARCALLBACK_MESSAGES
{
    UCM_CHANGEVOLUME, UCM_PROCESSDATA, UCM_NEEDPASSWORD
};

void* RAROpenArchive(struct RAROpenArchiveData* ArchiveData);
int    RARCloseArchive(void* hArcData);
int    RARReadHeader(void* hArcData, struct RARHeaderData* HeaderData);
int    RARProcessFile(void* hArcData, int Operation, char* DestPath, char* DestName);
void   RARSetCallback(void* hArcData, unrar_callback Callback, long UserData);
void   RARSetPassword(void* hArcData, char* Password);

#endif
