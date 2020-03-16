//=============================================================================//
//
// Purpose: 
//
//=============================================================================//
#ifndef TFMAINMENUPANEL_H
#define TFMAINMENUPANEL_H

#include "tf_menupanelbase.h"
#include "steam/steam_api.h"
#include <vgui_controls/HTML.h>
#include "tf_dialogpanelbase.h"
#include "tf_controls.h"

class CAvatarImagePanel;
class CTFButton;
class CTFBlogPanel;

enum MusicStatus
{
	MUSIC_STOP,
	MUSIC_FIND,
	MUSIC_PLAY,
	MUSIC_STOP_FIND,
	MUSIC_STOP_PLAY,
};

typedef struct
{
	char m_szImage[MAX_PATH];
	char m_szHolidayRestriction[64];
	char m_szStoreText[128];
	char m_szItemURL[128];

} TFCharacterBackgrounds_t;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CTFMainMenuPanel : public CTFMenuPanelBase
{
	DECLARE_CLASS_SIMPLE(CTFMainMenuPanel, CTFMenuPanelBase);

public:
	CTFMainMenuPanel(vgui::Panel* parent, const char *panelName);
	virtual ~CTFMainMenuPanel();
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
	void ShowBlogPanel(bool show);
	void ShowBGVideoPanel(bool show);

private:
	void GetRandomMusic(char *pszBuf, int iBufLength);

	CExLabel			*m_pVersionLabel;
	CAvatarImagePanel	*m_pProfileAvatar; 
	vgui::ImagePanel	*m_pFakeBGImage;
	vgui::ImagePanel	*m_pCharacterImage;

	int					m_iShowFakeIntro;

	char				m_pzMusicLink[64];	
	int					m_nSongGuid;
	int					m_iLastSongPlayed;
	MusicStatus			m_psMusicStatus;

	CSteamID			m_SteamID;
	CTFBlogPanel		*m_pBlogPanel;

	CUtlVector< TFCharacterBackgrounds_t > m_CharacterBackgrounds;

	static void ConfirmQuit();
};


class CTFBlogPanel : public CTFMenuPanelBase
{
	DECLARE_CLASS_SIMPLE(CTFBlogPanel, CTFMenuPanelBase);

public:
	CTFBlogPanel(vgui::Panel* parent, const char *panelName);
	virtual ~CTFBlogPanel();
	void PerformLayout();
	void ApplySchemeSettings(vgui::IScheme *pScheme);
	void LoadBlogPost(const char* URL);
	void Show();
	void Hide();
private:
	vgui::HTML			*m_pHTMLPanel;
};

#endif // TFMAINMENUPANEL_H