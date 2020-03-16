//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: func_antirush, like func_brush but only solid to players, with hud notification and sound included
//
//=============================================================================//

//TO DO!! Make sound only play for specified team
//TO DO!! Forcefully make this never colide with anything but players. ...Actually not sure about that idea. Eh.

#include "cbase.h"
#include "entityoutput.h"
#include "ndebugoverlay.h"
#include "tf_gamerules.h"
#include "tf_team.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// ----

//-----------------------------------------------------------------------------
// Purpose: an all-in-one brush entity for antirushes
//-----------------------------------------------------------------------------
class CFuncAntirush : public CBaseEntity
{
public:
	DECLARE_CLASS(CFuncAntirush, CBaseEntity);

	virtual void Spawn(void);
	bool CreateVPhysics(void);

	virtual void Precache(void);

	virtual void TurnOff(void);
	virtual void TurnOn(void);
	virtual void TurnOffFeatureless(void);

	string_t antySoundEffect;

	int antyCanMsg;
	int antyCanSound;

	int antyCanEnable;

	void DisplayMessage();
	void PlaySound();

	// Input handlers
	void InputTurnOff(inputdata_t &inputdata);
	void InputTurnOn(inputdata_t &inputdata);
	void InputToggle(inputdata_t &inputdata);
	void InputToggleFeatureless(inputdata_t &inputdata);
	void InputTurnOffFeatureless(inputdata_t &inputdata);

	int m_iDisabled;

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	virtual bool IsOn(void) const;

private:
	string_t antyMessage;
	string_t antyMsgIcon;
	int antyRecipientTeam; // shall handle both text and sound message audience
	int antyBackgroundTeam;
};

//-----

LINK_ENTITY_TO_CLASS(func_antirush, CFuncAntirush);

BEGIN_DATADESC(CFuncAntirush)

DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputTurnOn),
DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputTurnOff),
DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle),
DEFINE_INPUTFUNC(FIELD_VOID, "SimpleDisable", InputTurnOffFeatureless),
DEFINE_INPUTFUNC(FIELD_VOID, "SimpleToggle", InputToggleFeatureless),
DEFINE_KEYFIELD(m_iDisabled, FIELD_INTEGER, "StartDisabled"),
DEFINE_KEYFIELD(antySoundEffect, FIELD_SOUNDNAME, "SoundEffect"),
DEFINE_KEYFIELD(antyMessage, FIELD_STRING, "MessageText"),
DEFINE_KEYFIELD(antyMsgIcon, FIELD_STRING, "MessageIcon"),
DEFINE_KEYFIELD(antyCanMsg, FIELD_INTEGER, "CanMessage"),
DEFINE_KEYFIELD(antyCanSound, FIELD_INTEGER, "CanSound"),
DEFINE_KEYFIELD(antyCanEnable, FIELD_INTEGER, "CanTurnOn"),
DEFINE_KEYFIELD(antyRecipientTeam, FIELD_INTEGER, "RecipientTeam"),
DEFINE_KEYFIELD(antyBackgroundTeam, FIELD_INTEGER, "BackgroundTeam"),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CFuncAntirush, DT_FuncAntirush)
END_SEND_TABLE()

void CFuncAntirush::Spawn(void)
{
	SetMoveType(MOVETYPE_PUSH);  // so it doesn't get pushed by anything

	SetSolid(SOLID_VPHYSICS);
	AddEFlags(EFL_USE_PARTITION_WHEN_NOT_SOLID);

	SetModel(STRING(GetModelName()));

	if (m_iDisabled)
		TurnOffFeatureless();

	// If it can't move/go away, it's really part of the world
	if (!GetEntityName() || !m_iParent)
		AddFlag(FL_WORLDBRUSH);

	CreateVPhysics();

	if (TFGameRules()->IsFreeMode() || !lfe_allow_antirush.GetBool())
		UTIL_Remove(this);
}

void CFuncAntirush::Precache()
{
	BaseClass::Precache();
	PrecacheScriptSound(STRING(antySoundEffect));
}

//-----------------------------------------------------------------------------

//-----
// Purpose: display message (on turnoff)
//-----

void CFuncAntirush::DisplayMessage()
{
	CBroadcastRecipientFilter filter;

	switch (antyRecipientTeam)
	{
	case TF_TEAM_RED:
		filter.RemoveRecipientsByTeam(GetGlobalTeam(TF_TEAM_BLUE));
		break;

	case TF_TEAM_BLUE:
		filter.RemoveRecipientsByTeam(GetGlobalTeam(TF_TEAM_RED));
		break;
	}

	TFGameRules()->SendHudNotification(filter, STRING(antyMessage), STRING(antyMsgIcon), antyBackgroundTeam);
}

//-------------------------------------------------------------

//-----
// Purpose: play sound (on turnoff)
//-----

void CFuncAntirush::PlaySound()
{
	EmitSound(STRING(antySoundEffect));
	//TO DO!! make sound only play for specified team
}

//-------------------------------------------------------------

bool CFuncAntirush::CreateVPhysics(void)
{
	// NOTE: Don't init this static.  It's pretty common for these to be constrained
	// and dynamically parented.  Initing shadow avoids having to destroy the physics
	// object later and lose the constraints.
	IPhysicsObject *pPhys = VPhysicsInitShadow(false, false);
	if (pPhys)
	{
		int contents = modelinfo->GetModelContents(GetModelIndex());
		if (!(contents & (MASK_SOLID | MASK_PLAYERSOLID | MASK_NPCSOLID)))
		{
			// leave the physics shadow there in case it has crap constrained to it
			// but disable collisions with it
			pPhys->EnableCollisions(false);
		}
	}
	return true;
}

/*
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CFuncAntirush::DrawDebugTextOverlays(void)
{
	int nOffset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT)
	{
		char tempstr[512];
		Q_snprintf(tempstr, sizeof(tempstr), "angles: %g %g %g", (double)GetLocalAngles()[PITCH], (double)GetLocalAngles()[YAW], (double)GetLocalAngles()[ROLL]);
		EntityText(nOffset, tempstr, 0);
		nOffset++;
	}

	return nOffset;
}
*/

//-----------------------------------------------------------------------------
// Purpose: Input handler for toggling the hidden/shown state of the brush.
//-----------------------------------------------------------------------------
void CFuncAntirush::InputToggle(inputdata_t &inputdata)
{
	if (IsOn())
	{
		TurnOff();
		return;
	}

	TurnOn();
}

//-----------------------------------------------------------------------------
// Purpose: Input handler for toggling, featureless
//-----------------------------------------------------------------------------
void CFuncAntirush::InputToggleFeatureless(inputdata_t &inputdata)
{
	if (IsOn())
	{
		TurnOffFeatureless();
		return;
	}

	TurnOn();
}

//-----------------------------------------------------------------------------
// Purpose: Input handler for hiding the brush.
//-----------------------------------------------------------------------------
void CFuncAntirush::InputTurnOffFeatureless(inputdata_t &inputdata)
{
	TurnOffFeatureless();
}

//-----------------------------------------------------------------------------
// Purpose: Input handler for hiding the brush, featureless
//-----------------------------------------------------------------------------
void CFuncAntirush::InputTurnOff(inputdata_t &inputdata)
{
	TurnOff();
}


//-----------------------------------------------------------------------------
// Purpose: Input handler for showing the brush.
//-----------------------------------------------------------------------------
void CFuncAntirush::InputTurnOn(inputdata_t &inputdata)
{
	TurnOn();
}

//-----------------------------------------------------------------------------
// Purpose: Hides the brush. --and does the thingys
//-----------------------------------------------------------------------------
void CFuncAntirush::TurnOff(void)
{
	if (!IsOn())
		return;
	
	AddSolidFlags(FSOLID_NOT_SOLID);
	AddEffects(EF_NODRAW);

	m_iDisabled = TRUE;

	//--- extra features 

	if (antyCanMsg)
		DisplayMessage();

	if (antyCanSound && antySoundEffect != NULL_STRING)
		PlaySound();

	if (!antyCanEnable)
		UTIL_Remove(this);
}

//-----------------------------------------------------------------------------
// Purpose: TurnOff without the func_antirush features stuff
//-----------------------------------------------------------------------------
void CFuncAntirush::TurnOffFeatureless(void)
{
	if (!IsOn())
		return;

	AddSolidFlags(FSOLID_NOT_SOLID);
	AddEffects(EF_NODRAW);

	m_iDisabled = TRUE;

}

//-----------------------------------------------------------------------------
// Purpose: Shows the brush.
//-----------------------------------------------------------------------------
void CFuncAntirush::TurnOn(void)
{
	if (IsOn())
		return;

	RemoveSolidFlags(FSOLID_NOT_SOLID);
	RemoveEffects(EF_NODRAW);
}


bool CFuncAntirush::IsOn(void) const
{
	return !IsEffectActive(EF_NODRAW);
}

