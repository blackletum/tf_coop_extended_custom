//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_ai_basenpc.h"
#include "engine/ivdebugoverlay.h"

#if defined( HL2_DLL ) || defined( HL2_EPISODIC )
#include "c_basehlplayer.h"
#endif

#include "death_pose.h"

#ifdef TF_CLASSIC_CLIENT
#include "tier0/vprof.h"
#include "c_tf_player.h"
#include "tf_shareddefs.h"
#include "iclientmode.h"
#include "vgui/ILocalize.h"
#include "soundenvelope.h"
#include "IEffects.h"
#include "tf_gamerules.h"
#include "r_efx.h"
#include "dlight.h"
#include "c_entitydissolve.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#if defined( CAI_BaseNPC )
#undef CAI_BaseNPC
#endif

#define PING_MAX_TIME	2.0

extern ConVar lfe_muzzlelight;

extern ConVar tf_playergib_forceup;
extern ConVar tf_playergib_force;
extern ConVar tf_playergib_maxspeed;

static void BuildNPCDecapitatedTransform( C_BaseAnimating *pAnimating )
{
	if ( pAnimating )
	{
		int iBone = pAnimating->LookupBone( "ValveBiped.Bip01_Head1" );
		if ( iBone != -1 )
			MatrixScaleByZero( pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "ValveBiped.Bip01_Neck1" );
		if ( iBone != -1 )
			MatrixScaleByZero( pAnimating->GetBoneForWrite( iBone ) );

		// zombie
		iBone = pAnimating->LookupBone( "ValveBiped.HC_Rear_Bone" );
		if ( iBone != -1 )
			MatrixScaleByZero( pAnimating->GetBoneForWrite( iBone ) );

		// sauce
		iBone = pAnimating->LookupBone( "Bip02 Head" );
		if ( iBone != -1 )
			MatrixScaleByZero( pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "Bip02 Neck" );
		if ( iBone != -1 )
			MatrixScaleByZero( pAnimating->GetBoneForWrite( iBone ) );
	}
}

static void BuildNPCNeckScaleTransformations( C_BaseAnimating *pAnimating, float iScale )
{
	if ( pAnimating )
	{
		int iBone = pAnimating->LookupBone( "ValveBiped.Bip01_Head1" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "ValveBiped.Bip01_Neck1" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		// zombie
		iBone = pAnimating->LookupBone( "ValveBiped.HC_Rear_Bone" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		// sauce
		iBone = pAnimating->LookupBone( "Bip02 Head" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "Bip02 Neck" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );
	}
}

static void BuildNPCTorsoScaleTransformations( C_BaseAnimating *pAnimating, float iScale )
{
	if ( pAnimating )
	{
		int iBone = pAnimating->LookupBone( "ValveBiped.Bip01_Spine" );
		/*if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );*/

		iBone = pAnimating->LookupBone( "ValveBiped.Bip01_Spine1" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		/*iBone = pAnimating->LookupBone( "ValveBiped.Bip01_Spine2" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "ValveBiped.Bip01_Spine3" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );*/

		iBone = pAnimating->LookupBone( "ValveBiped.Bip01_Spine4" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		// zombie
		iBone = pAnimating->LookupBone( "HCfast.body" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		// ant
		iBone = pAnimating->LookupBone( "Antlion.Body_Bone" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		// sauce
		/*iBone = pAnimating->LookupBone( "Bip02 Spine" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );*/

		iBone = pAnimating->LookupBone( "Bip02 Spine1" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		/*iBone = pAnimating->LookupBone( "Bip02 Spine2" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );*/
	}
}

static void BuildNPCHandScaleTransformations( C_BaseAnimating *pAnimating, float iScale )
{
	if ( pAnimating )
	{
		int iBone = pAnimating->LookupBone( "ValveBiped.Bip01_L_Hand" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "ValveBiped.Bip01_L_Finger2" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "ValveBiped.Bip01_L_Finger21" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "ValveBiped.Bip01_L_Finger22" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "ValveBiped.Bip01_L_Finger1" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "ValveBiped.Bip01_L_Finger11" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "ValveBiped.Bip01_L_Finger12" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "ValveBiped.Bip01_L_Finger0" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "ValveBiped.Bip01_L_Finger01" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "ValveBiped.Bip01_L_Finger02" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );



		iBone = pAnimating->LookupBone( "ValveBiped.Bip01_R_Hand" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "ValveBiped.Bip01_R_Finger2" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "ValveBiped.Bip01_R_Finger21" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "ValveBiped.Bip01_R_Finger22" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "ValveBiped.Bip01_R_Finger1" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "ValveBiped.Bip01_R_Finger11" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "ValveBiped.Bip01_R_Finger12" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "ValveBiped.Bip01_R_Finger0" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "ValveBiped.Bip01_R_Finger01" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "ValveBiped.Bip01_R_Finger02" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		// zombie
		iBone = pAnimating->LookupBone( "arm_bone2_L" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "arm_bone2_R" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		// ant
		iBone = pAnimating->LookupBone( "ArmL3_Bone" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "ArmR3_Bone" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		// sauce
		iBone = pAnimating->LookupBone( "Bip02 L Hand" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

		iBone = pAnimating->LookupBone( "Bip02 R Hand" );
		if ( iBone != -1 )
			MatrixScaleBy( iScale, pAnimating->GetBoneForWrite( iBone ) );

	}
}

IMPLEMENT_CLIENTCLASS_DT( C_AI_BaseNPC, DT_AI_BaseNPC, CAI_BaseNPC )
	RecvPropDataTable( RECVINFO_DT( m_AttributeManager ), 0, &REFERENCE_RECV_TABLE( DT_AttributeManager ) ),

	RecvPropInt( RECVINFO( m_lifeState ) ),
	RecvPropBool( RECVINFO( m_bPerformAvoidance ) ),
	RecvPropBool( RECVINFO( m_bIsMoving ) ),
	RecvPropBool( RECVINFO( m_bFadeCorpse ) ),
	RecvPropInt( RECVINFO ( m_iDeathPose) ),
	RecvPropInt( RECVINFO( m_iDeathFrame) ),
	RecvPropInt( RECVINFO( m_iHealth ) ),
	RecvPropInt( RECVINFO( m_iMaxHealth ) ),
	RecvPropInt( RECVINFO( m_iSpeedModRadius ) ),
	RecvPropInt( RECVINFO( m_iSpeedModSpeed ) ),
	RecvPropInt( RECVINFO( m_bSpeedModActive ) ),
	RecvPropBool( RECVINFO( m_bImportanRagdoll ) ),
	RecvPropFloat( RECVINFO( m_flTimePingEffect ) ),
	RecvPropString( RECVINFO( m_szClassname ) ),
#ifdef TF_CLASSIC_CLIENT
	RecvPropInt( RECVINFO( m_nPlayerCond ) ),
	RecvPropInt( RECVINFO( m_nPlayerCondEx ) ),
	RecvPropInt( RECVINFO( m_nPlayerCondEx2 ) ),
	RecvPropInt( RECVINFO( m_nPlayerCondEx3 ) ),
	RecvPropInt( RECVINFO( m_nPlayerCondEx4 ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_flCondExpireTimeLeft ), RecvPropFloat( RECVINFO( m_flCondExpireTimeLeft[0] ) ) ),
	RecvPropInt( RECVINFO( m_nNumHealers ) ),
	RecvPropTime( RECVINFO( m_flStunExpireTime ) ),
	RecvPropInt( RECVINFO( m_nStunFlags ) ),
	RecvPropFloat( RECVINFO( m_flStunMovementSpeed ) ),
	RecvPropFloat( RECVINFO( m_flStunResistance ) ),
	RecvPropBool( RECVINFO( m_bBurningDeath ) ),
	RecvPropBool( RECVINFO( m_bIceDeath ) ),
	RecvPropBool( RECVINFO( m_bAshDeath ) ),
	RecvPropBool( RECVINFO( m_bKingRuneBuffActive ) ),
	RecvPropInt( RECVINFO( m_iDamageCustomDeath ) ),
	RecvPropString( RECVINFO( m_pszLocalizeName ) ),
	RecvPropEHandle( RECVINFO( m_hItem ) ),
	RecvPropFloat( RECVINFO( m_flHeadScale ) ),
	RecvPropFloat( RECVINFO( m_flTorsoScale ) ),
	RecvPropFloat( RECVINFO( m_flHandScale ) ),

	RecvPropInt( RECVINFO( m_nTFFlags ) )
#endif
END_RECV_TABLE()
#ifdef TF_CLASSIC_CLIENT
BEGIN_PREDICTION_DATA_NO_BASE( C_AI_BaseNPC )
	DEFINE_PRED_FIELD( m_nPlayerCond, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nPlayerCondEx, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nPlayerCondEx2, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nPlayerCondEx3, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nPlayerCondEx4, FIELD_INTEGER, FTYPEDESC_INSENDTABLE )
END_PREDICTION_DATA()
#endif
extern ConVar cl_npc_speedmod_intime;

bool NPC_IsImportantNPC( C_BaseAnimating *pAnimating )
{
	C_AI_BaseNPC *pBaseNPC = dynamic_cast < C_AI_BaseNPC* > ( pAnimating );

	if ( pBaseNPC == NULL )
		return false;

	return pBaseNPC->ImportantRagdoll();
}

C_AI_BaseNPC::C_AI_BaseNPC()
{
#ifdef TF_CLASSIC_CLIENT
	m_pAttributes = this;

	m_pBurningSound = NULL;
	m_pBurningEffect = NULL;
	m_flBurnEffectStartTime = 0;
	m_flBurnEffectEndTime = 0;
	m_pOverhealEffect = NULL;

	m_aGibs.Purge();

	m_hRagdoll.Set( NULL );

	m_pCritSound = NULL;
	m_pCritEffect = NULL;

	m_flWaterImpactTime = 0.0f;
#endif
}

//-----------------------------------------------------------------------------
// Makes ragdolls ignore npcclip brushes
//-----------------------------------------------------------------------------
unsigned int C_AI_BaseNPC::PhysicsSolidMaskForEntity( void ) const 
{
	// This allows ragdolls to move through npcclip brushes
	if ( !IsRagdoll() )
	{
		int teamContents = 0;
		switch ( GetTeamNumber() )
		{
		case TF_TEAM_RED:
			teamContents = CONTENTS_BLUETEAM | CONTENTS_GREENTEAM | CONTENTS_YELLOWTEAM;
			break;

		case TF_TEAM_BLUE:
			teamContents = CONTENTS_REDTEAM | CONTENTS_GREENTEAM | CONTENTS_YELLOWTEAM;
			break;

		case TF_TEAM_GREEN:
			teamContents = CONTENTS_REDTEAM | CONTENTS_BLUETEAM | CONTENTS_YELLOWTEAM;
			break;

		case TF_TEAM_YELLOW:
			teamContents = CONTENTS_REDTEAM | CONTENTS_BLUETEAM | CONTENTS_GREENTEAM;
			break;

		default:
			teamContents = CONTENTS_REDTEAM | CONTENTS_BLUETEAM | CONTENTS_GREENTEAM | CONTENTS_YELLOWTEAM;
			break;
		}

		return MASK_NPCSOLID | teamContents;
	}

	return MASK_SOLID;
}

void C_AI_BaseNPC::ClientThink( void )
{
	BaseClass::ClientThink();

	UpdateColdBreath();

#ifdef HL2_DLL
	C_BaseHLPlayer *pPlayer = dynamic_cast<C_BaseHLPlayer*>( C_BasePlayer::GetLocalPlayer() );

	if ( ShouldModifyPlayerSpeed() == true )
	{
		if ( pPlayer )
		{
			float flDist = (GetAbsOrigin() - pPlayer->GetAbsOrigin()).LengthSqr();

			if ( flDist <= GetSpeedModifyRadius() )
			{
				if ( pPlayer->m_hClosestNPC )
				{
					if ( pPlayer->m_hClosestNPC != this )
					{
						float flDistOther = (pPlayer->m_hClosestNPC->GetAbsOrigin() - pPlayer->GetAbsOrigin()).Length();

						//If I'm closer than the other NPC then replace it with myself.
						if ( flDist < flDistOther )
						{
							pPlayer->m_hClosestNPC = this;
							pPlayer->m_flSpeedModTime = gpGlobals->curtime + cl_npc_speedmod_intime.GetFloat();
						}
					}
				}
				else
				{
					pPlayer->m_hClosestNPC = this;
					pPlayer->m_flSpeedModTime = gpGlobals->curtime + cl_npc_speedmod_intime.GetFloat();
				}
			}
		}
	}
#endif // HL2_DLL

#ifdef HL2_EPISODIC
	C_BaseHLPlayer *pPlayer = static_cast<C_BaseHLPlayer*>( C_BasePlayer::GetLocalPlayer() );

	if ( pPlayer && m_flTimePingEffect > gpGlobals->curtime )
	{
		float fPingEffectTime = m_flTimePingEffect - gpGlobals->curtime;
		
		if ( fPingEffectTime > 0.0f )
		{
			Vector vRight, vUp;
			Vector vMins, vMaxs;

			float fFade;

			if( fPingEffectTime <= 1.0f )
			{
				fFade = 1.0f - (1.0f - fPingEffectTime);
			}
			else
			{
				fFade = 1.0f;
			}

			GetRenderBounds( vMins, vMaxs );
			AngleVectors (pPlayer->GetAbsAngles(), NULL, &vRight, &vUp );
			Vector p1 = GetAbsOrigin() + vRight * vMins.x + vUp * vMins.z;
			Vector p2 = GetAbsOrigin() + vRight * vMaxs.x + vUp * vMins.z;
			Vector p3 = GetAbsOrigin() + vUp * vMaxs.z;

			int r = 0 * fFade;
			int g = 255 * fFade;
			int b = 0 * fFade;

			debugoverlay->AddLineOverlay( p1, p2, r, g, b, true, 0.05f );
			debugoverlay->AddLineOverlay( p2, p3, r, g, b, true, 0.05f );
			debugoverlay->AddLineOverlay( p3, p1, r, g, b, true, 0.05f );
		}
	}
#endif

	if ( IsAlive() && lfe_muzzlelight.GetBool() )
	{
		if ( IsInvulnerable() )
		{
			dlight_t *dl = effects->CL_AllocDlight( LIGHT_INDEX_TE_DYNAMIC + index );
			dl->origin = WorldSpaceCenter();

			switch ( GetTeamNumber() )
			{
			case TF_TEAM_RED:
				dl->color.r = 255; dl->color.g = 30; dl->color.b = 10; dl->style = 0;
				break;
			case TF_TEAM_BLUE:
				dl->color.r = 10; dl->color.g = 30; dl->color.b = 255; dl->style = 0;
				break;
			case TF_TEAM_GREEN:
				dl->color.r = 10; dl->color.g = 255; dl->color.b = 30; dl->style = 0;
				break;
			case TF_TEAM_YELLOW:
				dl->color.r = 255; dl->color.g = 255; dl->color.b = 30; dl->style = 0;
				break;
			}

			dl->radius = 128.0f;
			dl->decay = 512.0f;
			dl->die = gpGlobals->curtime + 0.01f;
		}

		if ( IsCritBoosted() )
		{
			dlight_t *dl = effects->CL_AllocDlight( LIGHT_INDEX_MUZZLEFLASH + index );
			dl->origin = GetAbsOrigin();
			if ( m_hCritEffectHost.Get() )
				dl->origin = m_hCritEffectHost.Get()->GetAbsOrigin();

			switch ( GetTeamNumber() )
			{
			case TF_TEAM_RED:
				dl->color.r = 94; dl->color.g = 8; dl->color.b = 5; dl->style = 0;
				break;
			case TF_TEAM_BLUE:
				dl->color.r = 6; dl->color.g = 21; dl->color.b = 80; dl->style = 0;
				break;
			case TF_TEAM_GREEN:
				dl->color.r = 6; dl->color.g = 100; dl->color.b = 21; dl->style = 0;
				break;
			case TF_TEAM_YELLOW:
				dl->color.r = 28; dl->color.g = 28; dl->color.b = 9; dl->style = 0;
			}

			dl->radius = 128.0f;
			dl->decay = 512.0f;
			dl->die = gpGlobals->curtime + 0.01f;
		}
		else if ( IsMiniCritBoosted() )
		{
			dlight_t *dl = effects->CL_AllocDlight( LIGHT_INDEX_MUZZLEFLASH + index );
			dl->origin = GetAbsOrigin();
			if ( m_hCritEffectHost.Get() )
				dl->origin = m_hCritEffectHost.Get()->GetAbsOrigin();

			switch ( GetTeamNumber() )
			{
			case TF_TEAM_RED:
				dl->color.r = 94; dl->color.g = 8; dl->color.b = 5; dl->style = 0;
				break;
			case TF_TEAM_BLUE:
				dl->color.r = 6; dl->color.g = 21; dl->color.b = 80; dl->style = 0;
				break;
			case TF_TEAM_GREEN:
				dl->color.r = 6; dl->color.g = 21; dl->color.b = 100; dl->style = 0;
				break;
			case TF_TEAM_YELLOW:
				dl->color.r = 28; dl->color.g = 28; dl->color.b = 9; dl->style = 0;
			}

			dl->radius = 128.0f;
			dl->decay = 512.0f;
			dl->die = gpGlobals->curtime + 0.01f;
		}
		else if ( InCond( TF_COND_SODAPOPPER_HYPE ) )
		{
			dlight_t *dl = effects->CL_AllocDlight( LIGHT_INDEX_MUZZLEFLASH + index );
			dl->origin = GetAbsOrigin();
			if ( m_hCritEffectHost.Get() )
				dl->origin = m_hCritEffectHost.Get()->GetAbsOrigin();

			dl->color.r = 50; dl->color.g = 2; dl->color.b = 50; dl->style = 0;

			dl->radius = 128.0f;
			dl->decay = 512.0f;
			dl->die = gpGlobals->curtime + 0.01f;
		}

		if ( InCond( TF_COND_BURNING ) && !IsEffectActive( EF_DIMLIGHT ) )
		{
			dlight_t *dl = effects->CL_AllocDlight( LIGHT_INDEX_PLAYER_BRIGHT + index );
			dl->origin = WorldSpaceCenter();

			if ( IsLocalPlayerUsingVisionFilterFlags( TF_VISION_FILTER_PYRO ) )
			{
				dl->color.r = 230; dl->color.g = 65; dl->color.b = 245;
			}
			else
			{
				dl->color.r = 255; dl->color.g = 100; dl->color.b = 10;
			}

			dl->radius = 200.f;
			dl->decay = 512.0f;
			dl->die = gpGlobals->curtime + 0.01f;
		}
	}
}

void C_AI_BaseNPC::OnPreDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnPreDataChanged( updateType );

#ifdef TF_CLASSIC_CLIENT
	m_iOldTeam = GetTeamNumber();
	m_nOldConditions = m_nPlayerCond;
	m_nOldConditionsEx = m_nPlayerCondEx;
	m_nOldConditionsEx2 = m_nPlayerCondEx2;
	m_nOldConditionsEx3 = m_nPlayerCondEx3;
	m_nOldConditionsEx4 = m_nPlayerCondEx4;
	m_bWasCritBoosted = IsCritBoosted();
	m_bWasMiniCritBoosted = IsMiniCritBoosted();
#endif
}

void C_AI_BaseNPC::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	//if ( ( ShouldModifyPlayerSpeed() == true ) || ( m_flTimePingEffect > gpGlobals->curtime ) )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}

#ifdef TF_CLASSIC_CLIENT
	if ( type == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );

		InitInvulnerableMaterial();
	}

	// Update conditions from last network change
	SyncConditions( m_nPlayerCond, m_nOldConditions, 0, 0 );
	SyncConditions( m_nPlayerCondEx, m_nOldConditionsEx, 0, 32 );
	SyncConditions( m_nPlayerCondEx2, m_nOldConditionsEx2, 0, 64 );
	SyncConditions( m_nPlayerCondEx3, m_nOldConditionsEx3, 0, 96 );
	SyncConditions( m_nPlayerCondEx4, m_nOldConditionsEx4, 0, 128 );

	m_nOldConditions = m_nPlayerCond;
	m_nOldConditionsEx = m_nPlayerCondEx;
	m_nOldConditionsEx2 = m_nPlayerCondEx2;
	m_nOldConditionsEx3 = m_nPlayerCondEx3;
	m_nOldConditionsEx4 = m_nPlayerCondEx4;

	C_BaseCombatWeapon *pActiveWpn = this->GetActiveWeapon();
	if ( pActiveWpn )
	{
		UpdateCritBoostEffect();
	}

	if ( InCond( TF_COND_BURNING ) && !m_pBurningSound )
	{
		StartBurningSound();
	}
/*
	int nNewWaterLevel = GetWaterLevel();

	if ( nNewWaterLevel != m_nOldWaterLevel )
	{
		if ( ( m_nOldWaterLevel == WL_NotInWater ) && ( nNewWaterLevel > WL_NotInWater ) )
		{
			// Set when we do a transition to/from partially in water to completely out
			m_flWaterEntryTime = gpGlobals->curtime;
		}

		// If player is now up to his eyes in water and has entered the water very recently (not just bobbing eyes in and out), play a bubble effect.
		if ( ( nNewWaterLevel == WL_Eyes ) && ( gpGlobals->curtime - m_flWaterEntryTime ) < 0.5f ) 
		{
			CNewParticleEffect *pEffect = ParticleProp()->Create( "water_playerdive", PATTACH_ABSORIGIN_FOLLOW );
			ParticleProp()->AddControlPoint( pEffect, 1, NULL, PATTACH_WORLDORIGIN, NULL, WorldSpaceCenter() );
		}
		// If player was up to his eyes in water and is now out to waist level or less, play a water drip effect
		else if ( m_nOldWaterLevel == WL_Eyes && ( nNewWaterLevel < WL_Eyes ) && !bJustSpawned )
		{
			CNewParticleEffect *pWaterExitEffect = ParticleProp()->Create( "water_playeremerge", PATTACH_ABSORIGIN_FOLLOW );
			ParticleProp()->AddControlPoint( pWaterExitEffect, 1, this, PATTACH_ABSORIGIN_FOLLOW );
			m_bWaterExitEffectActive = true;
		}
	}
*/
#endif
}

#ifdef TF_CLASSIC_CLIENT
//-----------------------------------------------------------------------------
// Purpose: check the newly networked conditions for changes
//-----------------------------------------------------------------------------
void C_AI_BaseNPC::SyncConditions( int nCond, int nOldCond, int nUnused, int iOffset )
{
	if ( nCond == nOldCond )
		return;

	int nCondChanged = nCond ^ nOldCond;
	int nCondAdded = nCondChanged & nCond;
	int nCondRemoved = nCondChanged & nOldCond;

	int i;
	for ( i = 0; i < 32; i++ )
	{
		if ( nCondAdded & (1<<i) )
		{
			OnConditionAdded( i + iOffset );
		}
		else if ( nCondRemoved & (1<<i) )
		{
			OnConditionRemoved( i + iOffset );
		}
	}
}
#endif

void C_AI_BaseNPC::UpdateOnRemove( void )
{
#ifdef TF_CLASSIC_CLIENT
	ParticleProp()->OwnerSetDormantTo( true );
	ParticleProp()->StopParticlesInvolving( this );

	RemoveAllCond();
#endif

	BaseClass::UpdateOnRemove();
}

bool C_AI_BaseNPC::GetRagdollInitBoneArrays( matrix3x4_t *pDeltaBones0, matrix3x4_t *pDeltaBones1, matrix3x4_t *pCurrentBones, float boneDt )
{
	bool bRet = true;

	if ( !ForceSetupBonesAtTime( pDeltaBones0, gpGlobals->curtime - boneDt ) )
		bRet = false;

	GetRagdollCurSequenceWithDeathPose( this, pDeltaBones1, gpGlobals->curtime, m_iDeathPose, m_iDeathFrame );
	float ragdollCreateTime = PhysGetSyncCreateTime();
	if ( ragdollCreateTime != gpGlobals->curtime )
	{
		// The next simulation frame begins before the end of this frame
		// so initialize the ragdoll at that time so that it will reach the current
		// position at curtime.  Otherwise the ragdoll will simulate forward from curtime
		// and pop into the future a bit at this point of transition
		if ( !ForceSetupBonesAtTime( pCurrentBones, ragdollCreateTime ) )
			bRet = false;
	}
	else
	{
		if ( !SetupBones( pCurrentBones, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, gpGlobals->curtime ) )
			bRet = false;
	}

	return bRet;
}

#ifdef TF_CLASSIC_CLIENT
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	C_AI_BaseNPC::InternalDrawModel( int flags )
{
	bool bUseInvulnMaterial = IsInvulnerable();
	if ( bUseInvulnMaterial )
	{
		modelrender->ForcedMaterialOverride( *GetInvulnMaterialRef() );
	}

	int ret = BaseClass::InternalDrawModel( flags );

	if ( bUseInvulnMaterial )
	{
		modelrender->ForcedMaterialOverride( NULL );
	}

	return ret;
}

//-----------------------------------------------------------------------------
// Purpose: Don't take damage decals while invulnerable
//-----------------------------------------------------------------------------
void C_AI_BaseNPC::AddDecal( const Vector& rayStart, const Vector& rayEnd,
							const Vector& decalCenter, int hitbox, int decalIndex, bool doTrace, trace_t& tr, int maxLODToDecal )
{
	if ( InCond( TF_COND_STEALTHED ) )
	{
		return;
	}

	if ( IsInvulnerable() )
	{ 
		Vector vecDir = rayEnd - rayStart;
		VectorNormalize(vecDir);
		g_pEffects->Ricochet( rayEnd - (vecDir * 8), -vecDir );
		return;
	}

	// don't decal from inside NPC
	if ( tr.startsolid )
	{
		return;
	}

	BaseClass::AddDecal( rayStart, rayEnd, decalCenter, hitbox, decalIndex, doTrace, tr, maxLODToDecal );
}

extern C_EntityDissolve *DissolveEffect( C_BaseEntity *pTarget, float flTime );

C_BaseAnimating *C_AI_BaseNPC::BecomeRagdollOnClient()
{
	MoveToLastReceivedPosition( true );
	GetAbsOrigin();

	C_BaseAnimating *pRagdoll = CreateRagdollCopy();
	if ( pRagdoll )
	{
		matrix3x4_t boneDelta0[MAXSTUDIOBONES];
		matrix3x4_t boneDelta1[MAXSTUDIOBONES];
		matrix3x4_t currentBones[MAXSTUDIOBONES];
		const float boneDt = 0.1f;

		bool bInitAsClient = false;
		bool bInitBoneArrays = GetRagdollInitBoneArrays( boneDelta0, boneDelta1, currentBones, boneDt );

		bool bFreeze = false;
		if ( m_iDamageCustomDeath == TF_DMG_CUSTOM_GOLD_WRENCH || m_bIceDeath )
			bFreeze = true;

		if ( bInitBoneArrays )
		{
			bInitAsClient = pRagdoll->InitAsClientRagdoll( boneDelta0, boneDelta1, currentBones, boneDt, bFreeze );
		}

		if ( !bInitAsClient || !bInitBoneArrays )
		{
			Warning( "C_BaseAnimating::BecomeRagdollOnClient failed. pRagdoll:%p bInitBoneArrays:%d bInitAsClient:%d\n",
					 pRagdoll, bInitBoneArrays, bInitAsClient );
			pRagdoll->Release();
			return NULL;
		}

		m_hRagdoll.Set( pRagdoll );

		if ( m_bBurningDeath )
			pRagdoll->ParticleProp()->Create( "burningplayer_corpse", PATTACH_ABSORIGIN_FOLLOW );

		if ( m_bAshDeath )
			pRagdoll->ParticleProp()->Create( "drg_fiery_death", PATTACH_ABSORIGIN_FOLLOW );

		if ( ( m_iDamageCustomDeath == TF_DMG_CUSTOM_PLASMA ) || ( m_iDamageCustomDeath == TF_DMG_CUSTOM_PLASMA_CHARGED ) )
		{
			C_EntityDissolve *pDissolver = DissolveEffect( pRagdoll, gpGlobals->curtime );
			if ( pDissolver )
			{
				if ( GetTeamNumber() == TF_TEAM_RED )
					pDissolver->SetEffectColor( Vector( BitsToFloat( 0x42AFF333 ), BitsToFloat( 0x43049999 ), BitsToFloat( 0x4321ECCD ) ) );
				else
					pDissolver->SetEffectColor( Vector( BitsToFloat( 0x4337999A ), BitsToFloat( 0x42606666 ), BitsToFloat( 0x426A999A ) ) );

				pDissolver->SetOwnerEntity( NULL );
				pDissolver->SetRenderMode( kRenderTransColor );
				pDissolver->m_vDissolverOrigin = pRagdoll->GetLocalOrigin();
			}
		}
	
		if ( m_bIceDeath )
		{
			pRagdoll->MaterialOverride( "models/player/shared/ice_player.vmt" );
			pRagdoll->ParticleProp()->Create( "xms_icicle_impact_dryice", PATTACH_ABSORIGIN_FOLLOW );
		}

		if ( m_iDamageCustomDeath == TF_DMG_CUSTOM_GOLD_WRENCH )
		{
			pRagdoll->MaterialOverride( "models/player/shared/gold_player.vmt" );
		}

		if ( m_iDamageCustomDeath == TF_DMG_CUSTOM_DECAPITATION )
		{
			pRagdoll->ParticleProp()->Create( "blood_decap_arterial_spray", PATTACH_POINT_FOLLOW, "eyes" );
			pRagdoll->ParticleProp()->Create( "blood_decap_fountain", PATTACH_POINT_FOLLOW, "eyes" );
			pRagdoll->ParticleProp()->Create( "blood_decap_streaks", PATTACH_POINT_FOLLOW, "eyes" );
		}
	}

	return pRagdoll;

	// Let the C_LFRagdoll take care of this.
	//return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
// Output : IRagdoll*
//-----------------------------------------------------------------------------
IRagdoll* C_AI_BaseNPC::GetRepresentativeRagdoll() const
{
	if ( m_hRagdoll.Get() )
	{
		C_BaseAnimating *pRagdoll = static_cast<C_BaseAnimating *>( m_hRagdoll.Get() );
		if ( !pRagdoll )
			return NULL;

		return pRagdoll->m_pRagdoll;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
Vector C_AI_BaseNPC::GetObserverCamOrigin( void )
{
	if ( !IsAlive() )
	{
		IRagdoll *pRagdoll = GetRepresentativeRagdoll();
		if ( pRagdoll )
			return pRagdoll->GetRagdollOrigin();
	}

	return BaseClass::GetObserverCamOrigin();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_AI_BaseNPC::GetTargetIDString( wchar_t *sIDString, int iMaxLenInBytes )
{
	sIDString[0] = '\0';

	C_TFPlayer *pLocalTFPlayer = C_TFPlayer::GetLocalTFPlayer();
	
	if ( !pLocalTFPlayer )
		return;

	if ( InSameTeam( pLocalTFPlayer ) || pLocalTFPlayer->IsPlayerClass( TF_CLASS_SPY ) || pLocalTFPlayer->GetTeamNumber() == TEAM_SPECTATOR )
	{
		const char *pszClassname = GetLocalizeName();
		wchar_t *wszNPCName = L"";
		wszNPCName = g_pVGuiLocalize->Find( pszClassname );

		if ( !wszNPCName )
		{
			wchar_t wszNPCNameBuf[MAX_PLAYER_NAME_LENGTH];
			g_pVGuiLocalize->ConvertANSIToUnicode( pszClassname, wszNPCNameBuf, sizeof(wszNPCNameBuf) );
			wszNPCName = wszNPCNameBuf;
		}

		const char *printFormatString = NULL;

		int nSeeEnemyHealth = 0;
		CALL_ATTRIB_HOOK_INT_ON_OTHER( pLocalTFPlayer, nSeeEnemyHealth, see_enemy_health );

		if ( pLocalTFPlayer->GetTeamNumber() == TEAM_SPECTATOR || InSameTeam( pLocalTFPlayer ) )
		{
			printFormatString = "#TF_playerid_sameteam";
		}
		else if ( pLocalTFPlayer->IsPlayerClass( TF_CLASS_SPY ) || nSeeEnemyHealth )
		{
			// Spy can see enemy's health.
			printFormatString = "#TF_playerid_diffteam";
		}

		wchar_t *wszPrepend = L"";

		if ( printFormatString )
		{
			g_pVGuiLocalize->ConstructString( sIDString, iMaxLenInBytes, g_pVGuiLocalize->Find(printFormatString), 3, wszPrepend, wszNPCName );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_AI_BaseNPC::GetTargetIDDataString( wchar_t *sDataString, int iMaxLenInBytes )
{
	sDataString[0] = '\0';
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_AI_BaseNPC::StartBurningSound( void )
{
	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

	if ( !m_pBurningSound )
	{
		CLocalPlayerFilter filter;
		m_pBurningSound = controller.SoundCreate( filter, entindex(), "Player.OnFire" );
	}

	controller.Play( m_pBurningSound, 0.0, 100 );
	controller.SoundChangeVolume( m_pBurningSound, 1.0, 0.1 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_AI_BaseNPC::StopBurningSound( void )
{
	if ( m_pBurningSound )
	{
		CSoundEnvelopeController::GetController().SoundDestroy( m_pBurningSound );
		m_pBurningSound = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_AI_BaseNPC::InitInvulnerableMaterial( void )
{
	const char *pszMaterial = NULL;

	int iTeam = GetTeamNumber();

	switch ( iTeam )
	{
		case TF_TEAM_RED:
			pszMaterial = "models/effects/invulnfx_red.vmt";
			break;
		case TF_TEAM_BLUE:
			pszMaterial = "models/effects/invulnfx_blue.vmt";
			break;
		case TF_TEAM_GREEN:
			pszMaterial = "models/effects/invulnfx_blue.vmt";
			break;
		case TF_TEAM_YELLOW:
			pszMaterial = "models/effects/invulnfx_blue.vmt";
			break;
		default:
			break;
	}

	if ( pszMaterial )
	{
		m_InvulnerableMaterial.Init( pszMaterial, TEXTURE_GROUP_CLIENT_EFFECTS );
	}
	else
	{
		m_InvulnerableMaterial.Shutdown();
	}
}

void C_AI_BaseNPC::UpdateCritBoostEffect( bool bForceHide /*= false*/ )
{
	bool bShouldShow = !bForceHide;

	if ( bShouldShow )
	{
		if ( !IsCritBoosted() && !InCond( TF_COND_CRITBOOSTED_DEMO_CHARGE ) )
		{
			bShouldShow = false;
		}
		else if ( IsStealthed() )
		{
			bShouldShow = false;
		}
	}

	if ( bShouldShow )
	{
		// Update crit effect model.
		if ( m_pCritEffect )
		{
			if ( m_hCritEffectHost.Get() )
				m_hCritEffectHost->ParticleProp()->StopEmission( m_pCritEffect );

			m_pCritEffect = NULL;
		}

		C_BaseCombatWeapon *pWeapon = GetActiveWeapon();

		// Don't add crit effect to weapons without a model.
		if ( pWeapon && pWeapon->GetWorldModelIndex() != 0 )
		{
			m_hCritEffectHost = pWeapon;
		}
		else
		{
			m_hCritEffectHost = this;
		}

		if ( m_hCritEffectHost.Get() )
		{
			const char *pszEffect = ConstructTeamParticle( "critgun_weaponmodel_%s", GetTeamNumber(), true, g_aTeamNamesShort );
			m_pCritEffect = m_hCritEffectHost->ParticleProp()->Create( pszEffect, PATTACH_ABSORIGIN_FOLLOW );
		}

		if ( !m_pCritSound )
		{
			CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
			CLocalPlayerFilter filter;
			m_pCritSound = controller.SoundCreate( filter, this->entindex(), "Weapon_General.CritPower" );
			controller.Play( m_pCritSound, 1.0, 100 );
		}
	}
	else
	{
		if ( m_pCritEffect )
		{
			if ( m_hCritEffectHost.Get() )
				m_hCritEffectHost->ParticleProp()->StopEmission( m_pCritEffect );

			m_pCritEffect = NULL;
		}

		m_hCritEffectHost = NULL;

		if ( m_pCritSound )
		{
			CSoundEnvelopeController::GetController().SoundDestroy( m_pCritSound );
			m_pCritSound = NULL;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_AI_BaseNPC::UpdateOverhealEffect( void )
{
	bool bShouldShow = true;

	if ( !InCond( TF_COND_HEALTH_OVERHEALED ) )
		bShouldShow = false;

	if ( bShouldShow )
	{
		if ( !m_pOverhealEffect )
		{
			const char *pszEffect = ConstructTeamParticle( "overhealedplayer_%s_pluses", GetTeamNumber() );
			m_pOverhealEffect = ParticleProp()->Create( pszEffect, PATTACH_ABSORIGIN_FOLLOW );
		}
	}
	else
	{
		if ( m_pOverhealEffect )
		{
			ParticleProp()->StopEmission( m_pOverhealEffect );
			m_pOverhealEffect = NULL;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_AI_BaseNPC::UpdateRuneIcon( bool bHasRune )
{
	if ( bHasRune )
	{
		if ( !m_pRuneEffect )
			m_pRuneEffect = ParticleProp()->Create( GetPowerupIconName( GetCarryingRuneType(), GetTeamNumber() ), PATTACH_POINT_FOLLOW, "eyes" );
	}
	else
	{
		if ( m_pRuneEffect )
		{
			ParticleProp()->StopEmission( m_pRuneEffect );
			m_pRuneEffect = NULL;
		}
	}
}

extern ConVar	tf_avoidteammates;

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : collisionGroup - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_AI_BaseNPC::ShouldCollide( int collisionGroup, int contentsMask ) const
{
	if ( ( ( collisionGroup == COLLISION_GROUP_PLAYER_MOVEMENT ) && tf_avoidteammates.GetBool() ) ||
		collisionGroup == TFCOLLISION_GROUP_ROCKETS ||
		collisionGroup == HL2COLLISION_GROUP_COMBINE_BALL ||
		collisionGroup == HL2COLLISION_GROUP_COMBINE_BALL_NPC )
	{
		switch( GetTeamNumber() )
		{
		case TF_TEAM_RED:
			if ( !( contentsMask & CONTENTS_REDTEAM ) )
				return false;
			break;

		case TF_TEAM_BLUE:
			if ( !( contentsMask & CONTENTS_BLUETEAM ) )
				return false;
			break;

		case TF_TEAM_GREEN:
			if ( !(contentsMask & CONTENTS_GREENTEAM ) )
				return false;
			break;

		case TF_TEAM_YELLOW:
			if ( !(contentsMask & CONTENTS_YELLOWTEAM ) )
				return false;
			break;
		}
	}
	return BaseClass::ShouldCollide( collisionGroup, contentsMask );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_AI_BaseNPC::OnAddMegaHeal( void )
{
	// Start the heal effect
	if ( !m_pMegaHeal )
	{
		const char *pszEffectName = ConstructTeamParticle( "medic_megaheal_%s", GetTeamNumber() );

		m_pMegaHeal = ParticleProp()->Create( pszEffectName, PATTACH_ABSORIGIN_FOLLOW );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_AI_BaseNPC::OnRemoveMegaHeal( void )
{
	if ( m_pMegaHeal )
	{
		ParticleProp()->StopEmission( m_pMegaHeal );
		m_pMegaHeal = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_AI_BaseNPC::InitNPCGibs( void )
{
	// Clear out the gib list and create a new one.
	m_aGibs.Purge();
	BuildGibList( m_aGibs, GetModelIndex(), 1.0f, COLLISION_GROUP_NONE );

	if ( TFGameRules() && ( TFGameRules()->IsBirthdayOrPyroVision() || TFGameRules()->IsLFBirthday() ) )
	{
		for ( int i = 0; i < m_aGibs.Count(); i++ )
		{
			if ( RandomFloat(0,1) < 0.75 )
			{
				Q_strncpy( m_aGibs[i].modelName, g_pszBDayGibs[ RandomInt(0,ARRAYSIZE(g_pszBDayGibs)-1) ] , sizeof(m_aGibs[i].modelName) );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &vecOrigin - 
//			&vecVelocity - 
//			&vecImpactVelocity - 
//-----------------------------------------------------------------------------
void C_AI_BaseNPC::CreateNPCGibs( const Vector &vecOrigin, const Vector &vecVelocity, float flImpactScale, bool bBurning, bool bHeadGib )
{
	// Make sure we have Gibs to create.
	if ( m_aGibs.Count() == 0 )
		return;

	AngularImpulse angularImpulse( RandomFloat( 0.0f, 120.0f ), RandomFloat( 0.0f, 120.0f ), 0.0 );

	Vector vecBreakVelocity = vecVelocity;
	vecBreakVelocity.z += tf_playergib_forceup.GetFloat();
	VectorNormalize( vecBreakVelocity );
	vecBreakVelocity *= tf_playergib_force.GetFloat();

	// Cap the impulse.
	float flSpeed = vecBreakVelocity.Length();
	if ( flSpeed > tf_playergib_maxspeed.GetFloat() )
	{
		VectorScale( vecBreakVelocity, tf_playergib_maxspeed.GetFloat() / flSpeed, vecBreakVelocity );
	}

	breakablepropparams_t breakParams( vecOrigin, GetLocalAngles(), vecBreakVelocity, angularImpulse );
	breakParams.impactEnergyScale = 1.0f;

	// Break up the player.
	m_hSpawnedGibs.Purge();

	if ( bHeadGib )
	{
		if ( !UTIL_IsLowViolence() )
		{
			CUtlVector<breakmodel_t> list;
			for ( int i=0; i<m_aGibs.Count(); ++i )
			{
				breakmodel_t breakModel = m_aGibs[i];
				if ( !V_strcmp( breakModel.modelName, "models/gibs/hgibs.mdl" ) )
					list.AddToHead( breakModel );
			}

			m_hFirstGib = CreateGibsFromList( list, GetModelIndex(), NULL, breakParams, this, -1, false, true, &m_hSpawnedGibs, bBurning );
			if ( m_hFirstGib )
			{
				Vector velocity, impulse;
				IPhysicsObject *pPhys = m_hFirstGib->VPhysicsGetObject();
				if ( pPhys )
				{
					pPhys->GetVelocity( &velocity, &impulse );
					impulse.x *= 6.0f;
					pPhys->AddVelocity( &velocity, &impulse );
				}
			}
		}
	}
	else
	{
		m_hFirstGib = CreateGibsFromList( m_aGibs, GetModelIndex(), NULL, breakParams, this, -1, false, true, &m_hSpawnedGibs, bBurning );
	}

	// Gib skin numbers don't match player skin numbers so we gotta fix it up here.
	for ( int i = 0; i < m_hSpawnedGibs.Count(); i++ )
	{
		C_BaseAnimating *pGib = static_cast<C_BaseAnimating *>( m_hSpawnedGibs[i].Get() );

		pGib->m_nSkin = GetSkin();
	}

	DropPartyHat( breakParams, vecBreakVelocity );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_AI_BaseNPC::DropPartyHat( breakablepropparams_t &breakParams, Vector &vecBreakVelocity )
{
	// do nothing
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_AI_BaseNPC::BuildTransformations( CStudioHdr *pStudioHdr, Vector *pos, Quaternion q[], const matrix3x4_t &cameraTransform, int boneMask, CBoneBitList &boneComputed )
{
	BaseClass::BuildTransformations( pStudioHdr, pos, q, cameraTransform, boneMask, boneComputed );

	bool bBuild = false;

	if ( m_flHeadScale != 1.0f )
	{
		bBuild = true;
		BuildNPCNeckScaleTransformations( this, m_flHeadScale );
		BuildBigHeadTransformation( this, pStudioHdr, pos, q, cameraTransform, boneMask, boneComputed, m_flHeadScale );
	}

	if ( m_flTorsoScale != 1.0f )
	{
		bBuild = true;
		BuildNPCTorsoScaleTransformations( this, m_flTorsoScale );
	}

	if ( m_flHandScale != 1.0f )
	{
		bBuild = true;
		BuildNPCHandScaleTransformations( this, m_flHandScale );
	}
	
	if ( bBuild )
		m_BoneAccessor.SetWritableBones( BONE_USED_BY_ANYTHING );
}

#endif
