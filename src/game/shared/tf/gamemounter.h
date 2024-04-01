#pragma once

extern bool bMapMountChange;

void AddRequiredSearchPaths(bool bMainMenu = false, const char *pMapName = NULL);
bool AddRequiredMapSearchPaths( const char *pMapName, bool bRemoveOld = false );

void RegisterMounterCallbacks();

void SetDisconnected(bool bStatus);
bool IsDisconnected();

char *GetActiveGameName();

void ParseAvailableMods(void);

void UnRegCMD(char szCMD[24], char szMapName[MAX_PATH], char szTagString[32], char szTransitionPoint[256]);
void ReRegChangeCMD(void);
void RegCBack(void);