//=============================================================================//
//
// Purpose: 
//
//=============================================================================//
#ifndef CREATEMULTIPLAYERGAMESERVERPAGE_H
#define CREATEMULTIPLAYERGAMESERVERPAGE_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_dialogpanelbase.h"

class CTFCvarToggleCheckButton;

//-----------------------------------------------------------------------------
// Purpose: server options page of the create game server dialog
//-----------------------------------------------------------------------------
class CTFCreateMultiplayerGameServerPage : public CTFDialogPanelBase
{
	DECLARE_CLASS_SIMPLE( CTFCreateMultiplayerGameServerPage, CTFDialogPanelBase );

public:
	CTFCreateMultiplayerGameServerPage( vgui::Panel *parent, const char *name );
	~CTFCreateMultiplayerGameServerPage();

	virtual bool Init();

	// returns currently entered information about the server
	void SetMap( const char *name );
	bool IsRandomMapSelected();
	const char *GetMapName();

	// Bots
	void EnableBots( KeyValues *data );
	int GetBotQuota( void );
	bool GetBotsEnabled( void );

	virtual void OnTick();
protected:
	virtual void OnApplyChanges();
	virtual void OnCommand( const char *command );
	MESSAGE_FUNC( OnCheckButtonChecked, "CheckButtonChecked" );

private:
	void LoadMapList();
	void LoadMaps( const char *pszPathID );

	vgui::ComboBox *m_pMapList;
	vgui::ComboBox *m_pGamemodeList;
	vgui::CheckButton *m_pEnableBotsCheck;
	CTFCvarToggleCheckButton *m_pEnableTutorCheck;
	KeyValues *m_pSavedData;

	enum { DATA_STR_LENGTH = 64 };
	char m_szMapName[DATA_STR_LENGTH];
};


#endif // CREATEMULTIPLAYERGAMESERVERPAGE_H