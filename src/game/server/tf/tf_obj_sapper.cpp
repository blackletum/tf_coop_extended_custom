//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Slowly damages the object it's attached to
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "tf_player.h"
#include "tf_team.h"
#include "tf_gamerules.h"
#include "tf_obj.h"
#include "tf_obj_sentrygun.h"
#include "tf_obj_sapper.h"
#include "ndebugoverlay.h"
#include "tf_gamestats.h"
#include "ai_basenpc.h"
#include "soundenvelope.h"
#include "lfe_buildpoint.h"

// ------------------------------------------------------------------------ //

#define SAPPER_MINS				Vector(0, 0, 0)
#define SAPPER_MAXS				Vector(1, 1, 1)

const char *g_sapperModel = "models/buildables/sapper_placed.mdl";
const char *g_sapperModelPlacement = "models/buildables/sapper_placement.mdl";
const char *g_sapperSDModel = "models/buildables/sd_sapper_placed.mdl";
const char *g_sapperSDModelPlacement = "models/buildables/sd_sapper_placement.mdl";
const char *g_sapperP2Model = "models/buildables/p2rec_placed.mdl";
const char *g_sapperP2ModelPlacement = "models/buildables/p2rec_placement.mdl";
const char *g_sapperXmasModel = "models/buildables/sapper_xmas_placed.mdl";
const char *g_sapperXmasModelPlacement = "models/buildables/sapper_xmas_placement.mdl";
const char *g_sapperBreadModel = "models/buildables/breadmonster_sapper_placed.mdl";
const char *g_sapperBreadModelPlacement = "models/buildables/breadmonster_sapper_placement.mdl";

#define SAPPER_MODEL_SENTRY_1	"models/buildables/sapper_sentry1.mdl"
#define SAPPER_MODEL_SENTRY_2	"models/buildables/sapper_sentry2.mdl"
#define SAPPER_MODEL_SENTRY_3	"models/buildables/sapper_sentry3.mdl"
#define SAPPER_MODEL_TELEPORTER	"models/buildables/sapper_teleporter.mdl"
#define SAPPER_MODEL_DISPENSER	"models/buildables/sapper_dispenser.mdl"

#define SAPPER_MODEL_SENTRY_1_PLACEMENT		"models/buildables/sapper_placement_sentry1.mdl"
#define SAPPER_MODEL_SENTRY_2_PLACEMENT		"models/buildables/sapper_placement_sentry2.mdl"
#define SAPPER_MODEL_SENTRY_3_PLACEMENT		"models/buildables/sapper_placement_sentry3.mdl"
#define SAPPER_MODEL_TELEPORTER_PLACEMENT	"models/buildables/sapper_placement_teleporter.mdl"
#define SAPPER_MODEL_DISPENSER_PLACEMENT	"models/buildables/sapper_placement_dispenser.mdl"

BEGIN_DATADESC( CObjectSapper )
	DEFINE_THINKFUNC( SapperThink ),
	DEFINE_SOUNDPATCH( m_pSappingSound ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CObjectSapper, DT_ObjectSapper )
END_SEND_TABLE();

LINK_ENTITY_TO_CLASS(obj_attachment_sapper, CObjectSapper);
PRECACHE_REGISTER(obj_attachment_sapper);

ConVar	obj_sapper_health( "obj_sapper_health", "100", FCVAR_NONE, "Sapper health" );
ConVar	obj_sapper_amount( "obj_sapper_amount", "25", FCVAR_NONE, "Amount of health inflicted by a Sapper object per second" );

#define SAPPER_THINK_CONTEXT		"SapperThink"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CObjectSapper::CObjectSapper()
{
	m_iHealth = GetBaseHealth();
	SetMaxHealth( GetBaseHealth() );

	UseClientSideAnimation();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CObjectSapper::UpdateOnRemove()
{
	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CObjectSapper::Spawn()
{
	SetModel( GetSapperModelName( SAPPER_MODEL_PLACEMENT ) );

	m_takedamage = DAMAGE_YES;
	m_iHealth = GetBaseHealth();

	SetType( OBJ_ATTACHMENT_SAPPER );

	BaseClass::Spawn();

	Vector mins = SAPPER_MINS;
	Vector maxs = SAPPER_MAXS;
	CollisionProp()->SetSurroundingBoundsType( USE_SPECIFIED_BOUNDS, &mins, &maxs );

    m_fObjectFlags.Set( m_fObjectFlags | OF_ALLOW_REPEAT_PLACEMENT );

	SetSolid( SOLID_NONE );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CObjectSapper::Precache()
{
	int iModelIndex = PrecacheModel( GetSapperModelName( SAPPER_MODEL_PLACED ) );
	PrecacheGibsForModel( iModelIndex );

	PrecacheModel( GetSapperModelName( SAPPER_MODEL_PLACEMENT ) );

	PrecacheScriptSound( "Weapon_Sapper.Plant" );
	PrecacheScriptSound( GetSapperSoundName() );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CObjectSapper::StopLoopingSounds( void )
{
	if ( m_pSappingSound )
	{
		CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
		controller.Shutdown( m_pSappingSound );
		controller.SoundDestroy( m_pSappingSound );
		m_pSappingSound = NULL;
	}

	BaseClass::StopLoopingSounds();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CObjectSapper::FinishedBuilding( void )
{
	BaseClass::FinishedBuilding();

	if ( GetParentObject() )
	{
		GetParentObject()->OnAddSapper();
	}

	if ( GetParentEntity() )
	{
		if ( GetParentEntity()->IsPlayer() )
		{
			CTFPlayer *pPlayer = dynamic_cast<CTFPlayer*>( GetParentEntity() );
			if ( pPlayer && !pPlayer->m_Shared.InCond( TF_COND_SAPPED ) )
			{
				pPlayer->m_Shared.AddCond( TF_COND_SAPPED );
				SetParent( pPlayer );
			}
		}

		if ( GetParentEntity()->IsNPC() )
		{
			CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC*>( GetParentEntity() );
			if ( pNPC && !pNPC->InCond( TF_COND_SAPPED ) )
			{
				pNPC->AttachSapper( GetBuilder() );
				SetParent( pNPC );
			}
		}

		if( GetParentEntity()->ClassMatches( "lfe_buildpoint" ) )
		{
			CInfoBuildPoint *pBuildPoint = dynamic_cast<CInfoBuildPoint*>( GetParentEntity() );
			if ( pBuildPoint )
				pBuildPoint->FireOutputs( GetBuilder(), this );
		}
	}

	EmitSound( "Weapon_Sapper.Plant" );

	// start looping timer, killed when we die
	CPASFilter filter( GetAbsOrigin() );
		
	EmitSound_t ep;
	ep.m_nChannel = CHAN_WEAPON;
	ep.m_pSoundName = GetSapperSoundName();
	ep.m_flVolume = 1.0f;
	ep.m_SoundLevel = SNDLVL_NORM;

	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
	m_pSappingSound = controller.SoundCreate( filter, entindex(), ep );
	controller.Play( m_pSappingSound, 1.0f, 100 ); 

	m_flSapperDamageAccumulator = 0;
	m_flLastThinkTime = gpGlobals->curtime;

	SetContextThink( &CObjectSapper::SapperThink, gpGlobals->curtime + 0.1, SAPPER_THINK_CONTEXT );
}

//-----------------------------------------------------------------------------
// Purpose: Change our model based on the object we are attaching to
//-----------------------------------------------------------------------------
void CObjectSapper::SetupAttachedVersion( void )
{
	CBaseEntity *pObject = GetParentEntity();
	if ( !pObject || ( pObject->IsNPC() && !pObject->IsAlive() ) )
	{
		DestroyObject();
		return;
	}

	if ( IsPlacing() )
		SetModel( GetSapperModelName( SAPPER_MODEL_PLACEMENT ) );

	BaseClass::SetupAttachedVersion();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CObjectSapper::OnGoActive( void )
{
	// set new model
	CBaseEntity *pObject = ( GetParentEntity() );
	if ( !pObject || ( pObject->IsNPC() && !pObject->IsAlive() ) )
	{
		DestroyObject();
		return;
	}

	SetModel( GetSapperModelName( SAPPER_MODEL_PLACED ) );

	UTIL_SetSize( this, SAPPER_MINS, SAPPER_MAXS );
	SetSolid( SOLID_NONE );

	BaseClass::OnGoActive();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CObjectSapper::GetSapperModelName( SapperModel_t iModelType )
{
	CTFPlayer *pBuilder = GetBuilder();
	if ( pBuilder )
	{
		CEconItemView *pSapper = pBuilder->GetLoadoutItem( pBuilder->GetPlayerClass()->GetClassIndex(), LOADOUT_POSITION_BUILDING );
		if ( pSapper )
		{
			if ( pSapper->GetItemDefIndex() == 810 )
			{
				if ( iModelType == SAPPER_MODEL_PLACEMENT )
					return g_sapperSDModelPlacement;

				return g_sapperSDModel;
			}
			else if ( pSapper->GetItemDefIndex() == 933 )
			{
				if ( iModelType == SAPPER_MODEL_PLACEMENT )
					return g_sapperP2ModelPlacement;

				return g_sapperP2Model;
			}
			else if ( pSapper->GetItemDefIndex() == 1080 )
			{
				if ( iModelType == SAPPER_MODEL_PLACEMENT )
					return g_sapperXmasModelPlacement;

				return g_sapperXmasModel;
			}
			else if ( pSapper->GetItemDefIndex() == 1102 )
			{
				if ( iModelType == SAPPER_MODEL_PLACEMENT )
					return g_sapperBreadModelPlacement;

				return g_sapperBreadModel;
			}
		}
	}

	if ( iModelType == SAPPER_MODEL_PLACEMENT )
		return g_sapperModelPlacement;

	return g_sapperModel;
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CObjectSapper::GetSapperSoundName( void )
{
	CTFPlayer *pBuilder = GetBuilder();
	if ( pBuilder )
	{
		CEconItemView *pSapper = pBuilder->GetLoadoutItem( pBuilder->GetPlayerClass()->GetClassIndex(), LOADOUT_POSITION_BUILDING );
		if ( pSapper )
		{
			if ( pSapper->GetItemDefIndex() == 810 )
			{
				return "Weapon_sd_sapper.Timer";
			}
			else if ( pSapper->GetItemDefIndex() == 933 )
			{
				return "Weapon_p2rec.Timer";
			}
			else if ( pSapper->GetItemDefIndex() == 1080 )
			{
				return "Weapon_Sapper_xmas.Timer";
			}
			else if ( pSapper->GetItemDefIndex() == 1102 )
			{
				return "Weapon_breadmonster_sapper.Timer";
			}
		}
	}

	return "Weapon_Sapper.Timer";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CObjectSapper::DetachObjectFromObject( void )
{
	if ( GetParentObject() )
	{
		GetParentObject()->OnRemoveSapper();
	}

	if ( GetParentEntity() )
	{
		if ( GetParentEntity()->IsPlayer() )
		{
			CTFPlayer *pPlayer = dynamic_cast<CTFPlayer*>( GetParentEntity() );
			if ( pPlayer )
				pPlayer->m_Shared.RemoveCond( TF_COND_SAPPED );
		}

		if ( GetParentEntity()->IsNPC() )
		{
			CAI_BaseNPC *pNPC = dynamic_cast<CAI_BaseNPC*>( GetParentEntity() );
			if ( pNPC )
				pNPC->RemoveCond( TF_COND_SAPPED );
		}
	}

	BaseClass::DetachObjectFromObject();
}

//-----------------------------------------------------------------------------
// Purpose: Slowly destroy the object I'm attached to
//-----------------------------------------------------------------------------
void CObjectSapper::SapperThink( void )
{
	if ( !GetTeam() )
		return;

	CBaseEntity *pObject = GetParentEntity();
	if ( !pObject || ( pObject->IsNPC() && !pObject->IsAlive() ) )
	{
		DestroyObject();
		return;
	}

	SetNextThink( gpGlobals->curtime + 0.1, SAPPER_THINK_CONTEXT );

	// Don't bring objects back from the dead
	if ( !pObject->IsAlive() )
		return;

	// how much damage to give this think?
	float flTimeSinceLastThink = gpGlobals->curtime - m_flLastThinkTime;
	float flDamageToGive = ( flTimeSinceLastThink ) * obj_sapper_amount.GetFloat();

	// add to accumulator
	m_flSapperDamageAccumulator += flDamageToGive;

	int iDamage = (int)m_flSapperDamageAccumulator;

	m_flSapperDamageAccumulator -= iDamage;

	CALL_ATTRIB_HOOK_INT_ON_OTHER( GetBuilder(), iDamage, mult_sapper_damage );

	CTakeDamageInfo info;
	info.SetDamage( iDamage );
	info.SetAttacker( this );
	info.SetInflictor( this );
	info.SetDamageType( DMG_CRUSH );

	pObject->TakeDamage( info );

	m_flLastThinkTime = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CObjectSapper::OnTakeDamage( const CTakeDamageInfo &info )
{
	if ( info.GetDamageCustom() != TF_DMG_WRENCH_FIX )
	{
		return 0;
	}

	return BaseClass::OnTakeDamage( info );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CObjectSapper::Killed( const CTakeDamageInfo &info )
{
	// If the sapper is removed by someone other than builder, award bonus points.
	CTFPlayer *pScorer = ToTFPlayer( TFGameRules()->GetDeathScorer( info.GetAttacker(), info.GetInflictor(), this ) );
	if ( pScorer )
	{
		CBaseObject *pObject = GetParentObject();
		if ( pScorer->GetTeamNumber() != pObject->GetBuilder()->GetTeamNumber() )
		{
			return;
		}
		if ( pObject && pScorer != pObject->GetBuilder() )
		{
			CTF_GameStats.Event_PlayerAwardBonusPoints( pScorer, this, 1 );
 		}
	}

	BaseClass::Killed( info );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CObjectSapper::GetBaseHealth( void )
{
	int iBaseHealth = obj_sapper_health.GetInt();
	CALL_ATTRIB_HOOK_INT_ON_OTHER( GetBuilder(), iBaseHealth, mult_sapper_health );
	return iBaseHealth;
}

