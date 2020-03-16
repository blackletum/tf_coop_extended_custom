//=============================================================================//
//
// Purpose: 
//
//=============================================================================//
#ifndef TFMAINMENUPAUSEPANEL_H
#define TFMAINMENUPAUSEPANEL_H

#include "tf_menupanelbase.h"
#include "tf_mainmenupanel.h"
#include "steam/steam_api.h"
#include <vgui_controls/HTML.h>

class CAvatarImagePanel;
class CTFButton;

enum PauseMusicStatus
{
	PAUSE_MUSIC_STOP,
	PAUSE_MUSIC_FIND,
	PAUSE_MUSIC_PLAY,
	PAUSE_MUSIC_STOP_FIND,
	PAUSE_MUSIC_STOP_PLAY,
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CTFPauseMenuPanel : public CTFMenuPanelBase
{
	DECLARE_CLASS_SIMPLE(CTFPauseMenuPanel, CTFMenuPanelBase);

public:
	CTFPauseMenuPanel(vgui::Panel* parent, const char *panelName);
	virtual ~CTFPauseMenuPanel();
	bool Init();

	void PerformLayout();
	void ApplySchemeSettings(vgui::IScheme *pScheme);
	void OnThink();
	void OnTick();
	void Show();
	void Hide();
	void OnCommand(const char* command);
	void DefaultLayout();
	void GameLayout();
	void PlayMusic();
	void StopMusic();

private:
	void GetRandomMusic(char *pszBuf, int iBufLength);

	CExLabel			*m_pVersionLabel;
	CAvatarImagePanel	*m_pProfileAvatar; 

	char				m_pzMusicLink[64];	
	int					m_nSongGuid;
	int					m_iLastSongPlayed;
	PauseMusicStatus	m_psMusicStatus;

	CSteamID			m_SteamID;

	static void ConfirmQuit();
	static void ConfirmDisconnect();
};
#endif // TFMAINMENUPAUSEPANEL_H