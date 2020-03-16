//=============================================================================//
//
// Purpose: 
//
//=============================================================================//
#ifndef TF_ACHIEVEMENTSDIALOG_H
#define TF_ACHIEVEMENTSDIALOG_H

#include "tf_menupanelbase.h"
#include "steam/steam_api.h"
#include "tf_dialogpanelbase.h"
#include "tf_controls.h"
#include "vgui_controls/PanelListPanel.h"
#include "vgui_controls/Label.h"
#include <KeyValues.h>

class IAchievement;
class CExLabel;

#define ACHIEVED_ICON_PATH "hud/icon_check.vtf"
#define LOCK_ICON_PATH "hud/icon_locked.vtf"

// Loads an achievement's icon into a specified image panel, or turns the panel off if no achievement icon was found.
bool LoadAchievementIcon( vgui::ImagePanel* pIconPanel, IAchievement *pAchievement, const char *pszExt = NULL );

// Updates a listed achievement item's progress bar. 
void UpdateProgressBar( vgui::EditablePanel* pPanel, IAchievement *pAchievement, Color clrProgressBar );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CTFAchievementsDialog : public CTFDialogPanelBase
{
	DECLARE_CLASS_SIMPLE( CTFAchievementsDialog, CTFDialogPanelBase );

public:
	CTFAchievementsDialog( vgui::Panel* parent, const char *panelName );
	virtual ~CTFAchievementsDialog();

	void Show();
	void Hide();
	virtual void OnCommand( const char* command );
	virtual void UpdateAchievementDialogInfo( void );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	MESSAGE_FUNC_PARAMS( OnTextChanged, "TextChanged", data );
	MESSAGE_FUNC_PTR( OnCheckButtonChecked, "CheckButtonChecked", panel );

	void CreateNewAchievementGroup( int iMinRange, int iMaxRange );

	vgui::PanelListPanel	*m_pAchievementsList;
	vgui::ImagePanel		*m_pListBG;

	vgui::EditablePanel		*m_pPercentageBarBackground;
	vgui::EditablePanel		*m_pPercentageBar;

	vgui::ComboBox			*m_pAchievementPackCombo;
	vgui::CheckButton		*m_pHideAchievedButton;

	CPanelAnimationVar( Color, m_clrProgressHighLight, "ProgresHighLightColor", "NewGame.SelectionColor" );

	int m_nUnlocked;

	typedef struct 
	{
		int m_iMinRange;
		int m_iMaxRange;
		int m_iNumAchievements;
		int m_iNumUnlocked;
	} achievement_group_t;

	int m_iNumAchievementGroups;

	achievement_group_t m_AchievementGroups[15];
};

//////////////////////////////////////////////////////////////////////////
// Individual item panel, displaying stats for one achievement
class CTFAchievementDialogItemPanel : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CTFAchievementDialogItemPanel, vgui::EditablePanel );

public:
	CTFAchievementDialogItemPanel( vgui::PanelListPanel *parent, const char* name, int iListItemID );
	~CTFAchievementDialogItemPanel();

	void SetAchievementInfo( IAchievement* pAchievement );
	void UpdateAchievementInfo();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	virtual void OnCursorEntered();
	virtual void OnCursorExited();

private:
	int				m_iOrigX;
	int				m_iOrigY;

	IAchievement* m_pSourceAchievement;
	int	m_iSourceAchievementIndex;

	vgui::PanelListPanel *m_pParent;

	CExLabel *m_pAchievementNameLabel;
	CExLabel *m_pAchievementDescLabel;
	CExLabel *m_pPercentageText;

	vgui::ImagePanel *m_pLockedIcon;
	vgui::ImagePanel *m_pAchievementIcon;

	vgui::EditablePanel		*m_pPercentageBarBackground;
	vgui::EditablePanel		*m_pPercentageBar;

	vgui::IScheme			*m_pSchemeSettings;

	CPanelAnimationVar( Color, m_clrProgressBar, "ProgressBarColor", "140 140 140 255" );
	CPanelAnimationVar( Color, m_clrBGAchieved, "BGAchievedColor", "AchievementsLightGrey" );
	CPanelAnimationVar( Color, m_clrBGUnachieved, "BGUnachievedColor", "AchievementsDarkGrey" );

	int m_iListItemID;
};

#endif // TF_ACHIEVEMENTSDIALOG_H