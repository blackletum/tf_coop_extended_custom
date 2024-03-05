//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Physics cannon
//
//=============================================================================//

#include "cbase.h"

#ifdef CLIENT_DLL
	#include "c_tf_player.h"
	#include "vcollide_parse.h"
	#include "engine/ivdebugoverlay.h"
	#include "iviewrender_beams.h"
	#include "c_te_effect_dispatch.h"
	#include "clienteffectprecachesystem.h"
	#include "iefx.h"
	#include "dlight.h"
	#include "c_physicsprop.h"
	#include "c_physbox.h"

#define CPhysicsProp C_PhysicsProp
#define CPhysBox C_PhysBox

#else
	#include "tf_player.h"
	#include "soundent.h"
	#include "ndebugoverlay.h"
	#include "ai_basenpc.h"
	#include "player_pickup.h"
	#include "physics_prop_ragdoll.h"
	#include "globalstate.h"
	#include "props.h"
	#include "te_effect_dispatch.h"
	#include "vphysics/friction.h"
	#include "util.h"
    #include "physobj.h"
	#include "tf_gamestats.h"
	#include "soundent.h"
	#include "effect_dispatch_data.h"
	#include "tf_weaponbase_rocket.h"
	#include "tf_team.h"
	#include "prop_combine_ball.h"
	#include "physobj.h"
	#include "saverestore_utlvector.h"
	#include "ai_interactions.h"
	#include "ilagcompensationmanager.h"
#endif

#include "gamerules.h"
#include "engine/IEngineSound.h"
#include "in_buttons.h"
#include "IEffects.h"
#include "shake.h"
#include "Sprite.h"
#include "beam_shared.h"
#include "tf_weapon_physcannon.h"
#include "tf_weapon_portalgun.h"
#include "movevars_shared.h"
#include "vphysics/friction.h"
#include "debugoverlay_shared.h"

#include "tf_gamerules.h"
#include "tf_viewmodel.h"
#include "model_types.h"

#include "prop_portal_shared.h"
#include "portal_util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	SPRITE_SCALE	128.0f

static const char *s_pWaitForUpgradeContext = "WaitForUpgrade";

ConVar	g_debug_physcannon( "g_debug_physcannon", "0", FCVAR_REPLICATED | FCVAR_CHEAT );


ConVar physcannon_minforce( "physcannon_minforce", "700", FCVAR_REPLICATED );
ConVar physcannon_maxforce( "physcannon_maxforce", "1500", FCVAR_REPLICATED );
ConVar physcannon_maxmass( "physcannon_maxmass", "250", FCVAR_REPLICATED );
ConVar physcannon_tracelength( "physcannon_tracelength", "250", FCVAR_REPLICATED );
ConVar physcannon_chargetime("physcannon_chargetime", "2", FCVAR_REPLICATED );
ConVar physcannon_pullforce( "physcannon_pullforce", "4000", FCVAR_REPLICATED );
ConVar physcannon_cone( "physcannon_cone", "0.97", FCVAR_REPLICATED );
ConVar physcannon_ball_cone( "physcannon_ball_cone", "0.997", FCVAR_REPLICATED );
ConVar player_throwforce( "player_throwforce", "1000", FCVAR_REPLICATED );

ConVar physcannon_mega_tracelength("physcannon_mega_tracelength", "850", FCVAR_REPLICATED );
ConVar physcannon_mega_pullforce("physcannon_mega_pullforce", "8000", FCVAR_REPLICATED );

ConVar lfe_physcannon_crit_pullforce( "lfe_physcannon_crit_pullforce", "6000", FCVAR_REPLICATED );
ConVar lfe_physcannon_mega_crit_tracelength("lfe_physcannon_mega_crit_tracelength", "1000", FCVAR_REPLICATED );
ConVar lfe_physcannon_mega_crit_pullforce("lfe_physcannon_mega_crit_pullforce", "10000", FCVAR_REPLICATED );
ConVar lfe_physcannon_mega_crit_tertiary_damage("lfe_physcannon_mega_crit_tertiary_damage", "500", FCVAR_REPLICATED );
ConVar lfe_physcannon_mega_crit_tertiary_radius("lfe_physcannon_mega_crit_tertiary_radius", "1024", FCVAR_REPLICATED );
ConVar lfe_physcannon_mega_crit_tertiary_cooldown("lfe_physcannon_mega_crit_tertiary_cooldown", "10", FCVAR_REPLICATED );

#ifdef CLIENT_DLL
//Precahce the effects
CLIENTEFFECT_REGISTER_BEGIN( PrecacheEffectPhysCannon )
	CLIENTEFFECT_MATERIAL( PHYSCANNON_BEAM_SPRITE )
	CLIENTEFFECT_MATERIAL( PHYSCANNON_BEAM_SPRITE_NOZ )
	CLIENTEFFECT_MATERIAL( PHYSCANNON_GLOW_SPRITE )
	CLIENTEFFECT_MATERIAL( PHYSCANNON_ENDCAP_SPRITE )
	CLIENTEFFECT_MATERIAL( PHYSCANNON_CENTER_GLOW )
	CLIENTEFFECT_MATERIAL( PHYSCANNON_BLAST_SPRITE )
	CLIENTEFFECT_MATERIAL( MEGACANNON_BEAM_SPRITE )
	CLIENTEFFECT_MATERIAL( MEGACANNON_BEAM_SPRITE_NOZ )
	CLIENTEFFECT_MATERIAL( MEGACANNON_GLOW_SPRITE )
	CLIENTEFFECT_MATERIAL( MEGACANNON_ENDCAP_SPRITE )
	CLIENTEFFECT_MATERIAL( MEGACANNON_CENTER_GLOW )
	CLIENTEFFECT_MATERIAL( MEGACANNON_BLAST_SPRITE )
CLIENTEFFECT_REGISTER_END()

extern ConVar lfe_muzzlelight;
#else

BEGIN_SIMPLE_DATADESC(thrown_objects_t)
DEFINE_FIELD(fTimeThrown, FIELD_TIME),
DEFINE_FIELD(hEntity, FIELD_EHANDLE),
END_DATADESC()

void PhysCannonBeginUpgrade(CBaseAnimating *pAnim)
{
	CWeaponPhysCannon *pWeaponPhyscannon = assert_cast<	CWeaponPhysCannon* >( pAnim );
	pWeaponPhyscannon->BeginUpgrade();
}

bool PhysCannonAccountableForObject(CBaseCombatWeapon* pPhysCannon, CBaseEntity* pObject)
{
	CWeaponPhysCannon* pCannon = dynamic_cast<CWeaponPhysCannon*>(pPhysCannon);
	if (pCannon)
	{
		return pCannon->IsAccountableForObject(pObject);
	}
	return false;
}

#endif

// -------------------------------------------------------------------------
//  Physcannon trace filter to handle special cases
// -------------------------------------------------------------------------
	class CTraceFilterPhyscannon : public CTraceFilterSimple
	{
	public:
		DECLARE_CLASS(CTraceFilterPhyscannon, CTraceFilterSimple);

		CTraceFilterPhyscannon(const IHandleEntity *passentity, int collisionGroup)
			: CTraceFilterSimple(NULL, collisionGroup), m_pTraceOwner(passentity) {	}

		// For this test, we only test against entities (then world brushes afterwards)
		virtual TraceType_t	GetTraceType() const { return TRACE_ENTITIES_ONLY; }

		bool HasContentsGrate(CBaseEntity *pEntity)
		{
			// FIXME: Move this into the GetModelContents() function in base entity

			// Find the contents based on the model type
			int nModelType = modelinfo->GetModelType(pEntity->GetModel());
			if (nModelType == mod_studio)
			{
				CBaseAnimating *pAnim = dynamic_cast<CBaseAnimating *>(pEntity);
				if (pAnim != NULL)
				{
					CStudioHdr *pStudioHdr = pAnim->GetModelPtr();
					if (pStudioHdr != NULL && (pStudioHdr->contents() & CONTENTS_GRATE))
						return true;
				}
			}
			else if (nModelType == mod_brush)
			{
				// Brushes poll their contents differently
				int contents = modelinfo->GetModelContents(pEntity->GetModelIndex());
				if (contents & CONTENTS_GRATE)
					return true;
			}

			return false;
		}

		virtual bool ShouldHitEntity(IHandleEntity *pHandleEntity, int contentsMask)
		{
			// Only skip ourselves (not things we own)
			if (pHandleEntity == m_pTraceOwner)
				return false;

			// Get the entity referenced by this handle
			CBaseEntity *pEntity = EntityFromEntityHandle(pHandleEntity);
			if (pEntity == NULL)
				return false;

			// Handle grate entities differently
			if (HasContentsGrate(pEntity))
			{
				// See if it's a grabbable physics prop
				CPhysicsProp *pPhysProp = dynamic_cast<CPhysicsProp *>(pEntity);
				if (pPhysProp != NULL)
					return pPhysProp->CanBePickedUpByPhyscannon();

				// Must be a moveable physbox
				CPhysBox *pPhysBox = dynamic_cast<CPhysBox *>(pEntity);
				if (pPhysBox)
					return pPhysBox->CanBePickedUpByPhyscannon();

				// Don't bother with any other sort of grated entity
				return false;
			}

			// Use the default rules
			return BaseClass::ShouldHitEntity(pHandleEntity, contentsMask);
		}

	protected:
		const IHandleEntity *m_pTraceOwner;
	};

	// We want to test against brushes alone
	class CTraceFilterOnlyBrushes : public CTraceFilterSimple
	{
	public:
		DECLARE_CLASS(CTraceFilterOnlyBrushes, CTraceFilterSimple);
		CTraceFilterOnlyBrushes(int collisionGroup) : CTraceFilterSimple(NULL, collisionGroup) {}
		virtual TraceType_t	GetTraceType() const { return TRACE_WORLD_ONLY; }
	};

	//-----------------------------------------------------------------------------
	// this will hit skip the pass entity, but not anything it owns
	// (lets player grab own grenades)
	class CTraceFilterNoOwnerTest : public CTraceFilterSimple
	{
	public:
		DECLARE_CLASS(CTraceFilterNoOwnerTest, CTraceFilterSimple);

		CTraceFilterNoOwnerTest(const IHandleEntity *passentity, int collisionGroup)
			: CTraceFilterSimple(NULL, collisionGroup), m_pPassNotOwner(passentity)
		{
		}

		virtual bool ShouldHitEntity(IHandleEntity *pHandleEntity, int contentsMask)
		{
			if (pHandleEntity != m_pPassNotOwner)
				return BaseClass::ShouldHitEntity(pHandleEntity, contentsMask);

			return false;
		}

	protected:
		const IHandleEntity *m_pPassNotOwner;
	};

//-----------------------------------------------------------------------------
	void UTIL_PhyscannonTraceLine( const Vector &vecAbsStart, const Vector &vecAbsEnd, CBaseEntity *pTraceOwner, trace_t *pTrace )
	{
		// Default to HL2 vanilla
		if ( hl2_episodic.GetBool() == false )
		{
			CTraceFilterNoOwnerTest filter(pTraceOwner, COLLISION_GROUP_NONE);
			UTIL_TraceLine(vecAbsStart, vecAbsEnd, (MASK_SHOT | CONTENTS_GRATE), &filter, pTrace);
			//Ray_t ray; ray.Init( vecAbsStart, vecAbsEnd );
			//UTIL_Portal_TraceRay( ray, MASK_SHOT|CONTENTS_GRATE, &filter, &pTrace );
			return;
		}

		// First, trace against entities
		CTraceFilterPhyscannon filter( pTraceOwner, COLLISION_GROUP_NONE );
		UTIL_TraceLine(vecAbsStart, vecAbsEnd, (MASK_SHOT | CONTENTS_GRATE), &filter, pTrace);
		//Ray_t ray; ray.Init( vecAbsStart, vecAbsEnd );
		//UTIL_Portal_TraceRay( ray, MASK_SHOT|CONTENTS_GRATE, &filter, &pTrace );

		// If we've hit something, test again to make sure no brushes block us
		if ( pTrace->m_pEnt != NULL )
		{
			trace_t testTrace;
			CTraceFilterOnlyBrushes brushFilter(COLLISION_GROUP_NONE);
			Ray_t ray; ray.Init( pTrace->startpos, pTrace->endpos );
			UTIL_Portal_TraceRay( ray, MASK_SHOT, &brushFilter, &testTrace );

			// If we hit a brush, replace the trace with that result
			if (testTrace.fraction < 1.0f || testTrace.startsolid || testTrace.allsolid)
			{
				*pTrace = testTrace;
			}
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Trace a hull for the physcannon
	//-----------------------------------------------------------------------------
	void UTIL_PhyscannonTraceHull(const Vector &vecAbsStart, const Vector &vecAbsEnd, const Vector &vecAbsMins, const Vector &vecAbsMaxs, CBaseEntity *pTraceOwner, trace_t *pTrace)
	{
		// Default to HL2 vanilla
		if ( hl2_episodic.GetBool() == false )
		{
			CTraceFilterNoOwnerTest filter( pTraceOwner, COLLISION_GROUP_NONE );
			UTIL_TraceHull( vecAbsStart, vecAbsEnd, vecAbsMins, vecAbsMaxs, (MASK_SHOT | CONTENTS_GRATE), &filter, pTrace );
			return;
		}

		// First, trace against entities
		CTraceFilterPhyscannon filter(pTraceOwner, COLLISION_GROUP_NONE);
		UTIL_TraceHull( vecAbsStart, vecAbsEnd, vecAbsMins, vecAbsMaxs, (MASK_SHOT | CONTENTS_GRATE), &filter, pTrace );

		// If we've hit something, test again to make sure no brushes block us
		if ( pTrace->m_pEnt != NULL )
		{
			trace_t testTrace;
			CTraceFilterOnlyBrushes brushFilter(COLLISION_GROUP_NONE);
			UTIL_TraceHull( pTrace->startpos, pTrace->endpos, vecAbsMins, vecAbsMaxs, MASK_SHOT, &brushFilter, &testTrace );

			// If we hit a brush, replace the trace with that result
			if (testTrace.fraction < 1.0f || testTrace.startsolid || testTrace.allsolid)
			{
				*pTrace = testTrace;
			}
		}
	}

static void MatrixOrthogonalize( matrix3x4_t &matrix, int column )
{
	Vector columns[3];
	int i;

	for ( i = 0; i < 3; i++ )
	{
		MatrixGetColumn( matrix, i, columns[i] );
	}

	int index0 = column;
	int index1 = (column+1)%3;
	int index2 = (column+2)%3;

	columns[index2] = CrossProduct( columns[index0], columns[index1] );
	columns[index1] = CrossProduct( columns[index2], columns[index0] );
	VectorNormalize( columns[index2] );
	VectorNormalize( columns[index1] );
	MatrixSetColumn( columns[index1], index1, matrix );
	MatrixSetColumn( columns[index2], index2, matrix );
}

#define SIGN(x) ( (x) < 0 ? -1 : 1 )

static QAngle AlignAngles( const QAngle &angles, float cosineAlignAngle )
{
	matrix3x4_t alignMatrix;
	AngleMatrix( angles, alignMatrix );

	// NOTE: Must align z first
	for ( int j = 3; --j >= 0; )
	{
		Vector vec;
		MatrixGetColumn( alignMatrix, j, vec );
		for ( int i = 0; i < 3; i++ )
		{
			if ( fabs(vec[i]) > cosineAlignAngle )
			{
				vec[i] = SIGN(vec[i]);
				vec[(i+1)%3] = 0;
				vec[(i+2)%3] = 0;
				MatrixSetColumn( vec, j, alignMatrix );
				MatrixOrthogonalize( alignMatrix, j );
				break;
			}
		}
	}

	QAngle out;
	MatrixAngles( alignMatrix, out );
	return out;
}


static void TraceCollideAgainstBBox( const CPhysCollide *pCollide, const Vector &start, const Vector &end, const QAngle &angles, const Vector &boxOrigin, const Vector &mins, const Vector &maxs, trace_t *ptr )
{
	physcollision->TraceBox( boxOrigin, boxOrigin + (start-end), mins, maxs, pCollide, start, angles, ptr );

	if ( ptr->DidHit() )
	{
		ptr->endpos = start * (1-ptr->fraction) + end * ptr->fraction;
		ptr->startpos = start;
		ptr->plane.dist = -ptr->plane.dist;
		ptr->plane.normal *= -1;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Finds the nearest ragdoll sub-piece to a location and returns it
// Input  : *pTarget - entity that is the potential ragdoll
//			&position - position we're testing against
// Output : IPhysicsObject - sub-object (if any)
//-----------------------------------------------------------------------------
#ifdef GAME_DLL
IPhysicsObject *GetRagdollChildAtPosition( CBaseEntity *pTarget, const Vector &position )
{
	// Check for a ragdoll
	if ( dynamic_cast<CRagdollProp*>( pTarget ) == NULL )
		return NULL;

	// Get the root
	IPhysicsObject *pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
	int count = pTarget->VPhysicsGetObjectList( pList, ARRAYSIZE( pList ) );
	
	IPhysicsObject *pBestChild = NULL;
	float			flBestDist = 99999999.0f;
	float			flDist;
	Vector			vPos;

	// Find the nearest child to where we're looking
	for ( int i = 0; i < count; i++ )
	{
		pList[i]->GetPosition( &vPos, NULL );
		
		flDist = ( position - vPos ).LengthSqr();

		if ( flDist < flBestDist )
		{
			pBestChild = pList[i];
			flBestDist = flDist;
		}
	}

	// Make this our base now
	pTarget->VPhysicsSwapObject( pBestChild );

	return pTarget->VPhysicsGetObject();
}
#endif
//-----------------------------------------------------------------------------
// Purpose: Computes a local matrix for the player clamped to valid carry ranges
//-----------------------------------------------------------------------------
// when looking level, hold bottom of object 8 inches below eye level
#define PLAYER_HOLD_LEVEL_EYES	-8

// when looking down, hold bottom of object 0 inches from feet
#define PLAYER_HOLD_DOWN_FEET	2

// when looking up, hold bottom of object 24 inches above eye level
#define PLAYER_HOLD_UP_EYES		24

// use a +/-30 degree range for the entire range of motion of pitch
#define PLAYER_LOOK_PITCH_RANGE	30

// player can reach down 2ft below his feet (otherwise he'll hold the object above the bottom)
#define PLAYER_REACH_DOWN_DISTANCE	24

static void ComputePlayerMatrix( CBasePlayer *pPlayer, matrix3x4_t &out )
{
	if ( !pPlayer )
		return;

	QAngle angles = pPlayer->EyeAngles();
	Vector origin = pPlayer->EyePosition();
	
	// 0-360 / -180-180
	//angles.x = init ? 0 : AngleDistance( angles.x, 0 );
	//angles.x = clamp( angles.x, -PLAYER_LOOK_PITCH_RANGE, PLAYER_LOOK_PITCH_RANGE );
	angles.x = 0;

	float feet = pPlayer->GetAbsOrigin().z + pPlayer->WorldAlignMins().z;
	float eyes = origin.z;
	float zoffset = 0;
	// moving up (negative pitch is up)
	if ( angles.x < 0 )
	{
		zoffset = RemapVal( angles.x, 0, -PLAYER_LOOK_PITCH_RANGE, PLAYER_HOLD_LEVEL_EYES, PLAYER_HOLD_UP_EYES );
	}
	else
	{
		zoffset = RemapVal( angles.x, 0, PLAYER_LOOK_PITCH_RANGE, PLAYER_HOLD_LEVEL_EYES, PLAYER_HOLD_DOWN_FEET + (feet - eyes) );
	}
	origin.z += zoffset;
	angles.x = 0;
	AngleMatrix( angles, origin, out );
}


//-----------------------------------------------------------------------------
BEGIN_SIMPLE_DATADESC( game_shadowcontrol_params_t )
	
	DEFINE_FIELD( targetPosition,		FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( targetRotation,		FIELD_VECTOR ),
	DEFINE_FIELD( maxAngular, FIELD_FLOAT ),
	DEFINE_FIELD( maxDampAngular, FIELD_FLOAT ),
	DEFINE_FIELD( maxSpeed, FIELD_FLOAT ),
	DEFINE_FIELD( maxDampSpeed, FIELD_FLOAT ),
	DEFINE_FIELD( dampFactor, FIELD_FLOAT ),
	DEFINE_FIELD( teleportDistance,	FIELD_FLOAT ),

END_DATADESC()

//-----------------------------------------------------------------------------
BEGIN_SIMPLE_DATADESC( CGrabController )

	DEFINE_EMBEDDED( m_shadow ),

	DEFINE_FIELD( m_timeToArrive,		FIELD_FLOAT ),
	DEFINE_FIELD( m_errorTime,			FIELD_FLOAT ),
	DEFINE_FIELD( m_error,				FIELD_FLOAT ),
	DEFINE_FIELD( m_contactAmount,		FIELD_FLOAT ),
	DEFINE_AUTO_ARRAY( m_savedRotDamping,	FIELD_FLOAT ),
	DEFINE_AUTO_ARRAY( m_savedMass,	FIELD_FLOAT ),
	DEFINE_FIELD( m_flLoadWeight,		FIELD_FLOAT ),
	DEFINE_FIELD( m_bCarriedEntityBlocksLOS, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bIgnoreRelativePitch, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_attachedEntity,	FIELD_EHANDLE ),
	DEFINE_FIELD( m_angleAlignment, FIELD_FLOAT ),
	DEFINE_FIELD( m_vecPreferredCarryAngles, FIELD_VECTOR ),
	DEFINE_FIELD( m_bHasPreferredCarryAngles, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flDistanceOffset, FIELD_FLOAT ),
	DEFINE_FIELD( m_attachedAnglesPlayerSpace, FIELD_VECTOR ),
	DEFINE_FIELD( m_attachedPositionObjectSpace, FIELD_VECTOR ),
	DEFINE_FIELD( m_bAllowObjectOverhead, FIELD_BOOLEAN ),

	// Physptrs can't be inside embedded classes
	// DEFINE_PHYSPTR( m_controller ),

END_DATADESC()

const float DEFAULT_MAX_ANGULAR = 360.0f * 10.0f;
const float REDUCED_CARRY_MASS = 1.0f;

CGrabController::CGrabController( void )
{
	m_shadow.dampFactor = 1.0;
	m_shadow.teleportDistance = 0;
	m_errorTime = 0;
	m_error = 0;
	// make this controller really stiff!
	m_shadow.maxSpeed = 1000;
	m_shadow.maxAngular = DEFAULT_MAX_ANGULAR;
	m_shadow.maxDampSpeed = m_shadow.maxSpeed*2;
	m_shadow.maxDampAngular = m_shadow.maxAngular;
	m_attachedEntity = NULL;
	m_vecPreferredCarryAngles = vec3_angle;
	m_bHasPreferredCarryAngles = false;
	m_flDistanceOffset = 0;
}

CGrabController::~CGrabController( void )
{
	DetachEntity( false );
}

void CGrabController::OnRestore()
{
	if ( m_controller )
	{
		m_controller->SetEventHandler( this );
	}
}

void CGrabController::SetTargetPosition( const Vector &target, const QAngle &targetOrientation )
{
	m_shadow.targetPosition = target;
	m_shadow.targetRotation = targetOrientation;

	m_timeToArrive = gpGlobals->frametime;

	CBaseEntity *pAttached = GetAttached();
	if ( pAttached )
	{
		IPhysicsObject *pObj = pAttached->VPhysicsGetObject();
		
		if ( pObj != NULL )
		{
			pObj->Wake();
		}
		else
		{
			DetachEntity( false );
		}
	}
}

void CGrabController::GetTargetPosition( Vector *target, QAngle *targetOrientation )
{
	if ( target )
		*target = m_shadow.targetPosition;

	if ( targetOrientation )
		*targetOrientation = m_shadow.targetRotation;
}
//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CGrabController::ComputeError()
{
	if ( m_errorTime <= 0 )
		return 0;

	CBaseEntity *pAttached = GetAttached();
	if ( pAttached )
	{
		Vector pos;
		IPhysicsObject *pObj = pAttached->VPhysicsGetObject();
		
		if ( pObj )
		{	
			pObj->GetShadowPosition( &pos, NULL );

			float error = (m_shadow.targetPosition - pos).Length();
			if ( m_errorTime > 0 )
			{
				if ( m_errorTime > 1 )
				{
					m_errorTime = 1;
				}
				float speed = error / m_errorTime;
				if ( speed > m_shadow.maxSpeed )
				{
					error *= 0.5;
				}
				m_error = (1-m_errorTime) * m_error + error * m_errorTime;
			}
		}
		else
		{
			DevMsg( "Object attached to Physcannon has no physics object\n" );
			DetachEntity( false );
			return 9999; // force detach
		}
	}
	
	if ( pAttached->IsEFlagSet( EFL_IS_BEING_LIFTED_BY_BARNACLE ) )
	{
		m_error *= 3.0f;
	}
#ifdef GAME_DLL
	// If held across a portal but not looking at the portal multiply error
	CTFPlayer *pPortalPlayer = ToTFPlayer( GetPlayerHoldingEntity( pAttached ) );
	Assert( pPortalPlayer );
	if ( pPortalPlayer->IsHeldObjectOnOppositeSideOfPortal() )
	{
		Vector forward, right, up;
		QAngle playerAngles = pPortalPlayer->EyeAngles();

		float pitch = AngleDistance(playerAngles.x,0);
		playerAngles.x = clamp( pitch, -75, 75 );
		AngleVectors( playerAngles, &forward, &right, &up );

		Vector start = pPortalPlayer->Weapon_ShootPosition();

		// If the player is upside down then we need to hold the box closer to their feet.
		if ( up.z < 0.0f )
			start += pPortalPlayer->GetViewOffset() * up.z;
		if ( right.z < 0.0f )
			start += pPortalPlayer->GetViewOffset() * right.z;

		Ray_t rayPortalTest;
		rayPortalTest.Init( start, start + forward * 256.0f );

		if ( UTIL_IntersectRayWithPortal( rayPortalTest, pPortalPlayer->GetHeldObjectPortal() ) < 0.0f )
		{
			m_error *= 2.5f;
		}
	}
#endif
	m_errorTime = 0;

	return m_error;
}


#define MASS_SPEED_SCALE	60
#define MAX_MASS			40

void CGrabController::ComputeMaxSpeed( CBaseEntity *pEntity, IPhysicsObject *pPhysics )
{
#ifdef GAME_DLL
	m_shadow.maxSpeed = 1000;
	m_shadow.maxAngular = DEFAULT_MAX_ANGULAR;

	// Compute total mass...
	float flMass = PhysGetEntityMass( pEntity );
	float flMaxMass = physcannon_maxmass.GetFloat();
	if ( flMass <= flMaxMass )
		return;

	float flLerpFactor = clamp( flMass, flMaxMass, 500.0f );
	flLerpFactor = SimpleSplineRemapVal( flLerpFactor, flMaxMass, 500.0f, 0.0f, 1.0f );

	float invMass = pPhysics->GetInvMass();
	float invInertia = pPhysics->GetInvInertia().Length();

	float invMaxMass = 1.0f / MAX_MASS;
	float ratio = invMaxMass / invMass;
	invMass = invMaxMass;
	invInertia *= ratio;

	float maxSpeed = invMass * MASS_SPEED_SCALE * 200;
	float maxAngular = invInertia * MASS_SPEED_SCALE * 360;

	m_shadow.maxSpeed = Lerp( flLerpFactor, m_shadow.maxSpeed, maxSpeed );
	m_shadow.maxAngular = Lerp( flLerpFactor, m_shadow.maxAngular, maxAngular );
#endif
}


QAngle CGrabController::TransformAnglesToPlayerSpace( const QAngle &anglesIn, CBasePlayer *pPlayer )
{
	if ( m_bIgnoreRelativePitch )
	{
		matrix3x4_t test;
		QAngle angleTest = pPlayer->EyeAngles();
		angleTest.x = 0;
		AngleMatrix( angleTest, test );
		return TransformAnglesToLocalSpace( anglesIn, test );
	}
	return TransformAnglesToLocalSpace( anglesIn, pPlayer->EyeToWorldTransform());
}

QAngle CGrabController::TransformAnglesFromPlayerSpace( const QAngle &anglesIn, CBasePlayer *pPlayer )
{
	if ( m_bIgnoreRelativePitch )
	{
		matrix3x4_t test;
		QAngle angleTest = pPlayer->EyeAngles();
		angleTest.x = 0;
		AngleMatrix( angleTest, test );
		return TransformAnglesToWorldSpace( anglesIn, test );
	}
	return TransformAnglesToWorldSpace( anglesIn, pPlayer->EyeToWorldTransform() );
}


void CGrabController::AttachEntity( CBasePlayer *pPlayer, CBaseEntity *pEntity, IPhysicsObject *pPhys, bool bIsMegaPhysCannon, const Vector &vGrabPosition, bool bUseGrabPosition )
{
	// play the impact sound of the object hitting the player
	// used as feedback to let the player know he picked up the object
#ifdef GAME_DLL
	PhysicsImpactSound(pPlayer, pPhys, CHAN_STATIC, pPhys->GetMaterialIndex(), pPlayer->VPhysicsGetObject()->GetMaterialIndex(), 1.0, 64);
#endif
	Vector position;
	QAngle angles;
	pPhys->GetPosition(&position, &angles);
	// If it has a preferred orientation, use that instead.
#ifdef GAME_DLL
	Pickup_GetPreferredCarryAngles(pEntity, pPlayer, pPlayer->EntityToWorldTransform(), angles);
#endif

#ifdef GAME_DLL
	//Fix attachment orientation weirdness
	CTFPlayer* pPortalPlayer = ToTFPlayer(pPlayer);
	if ( pPortalPlayer && pPortalPlayer->IsHeldObjectOnOppositeSideOfPortal() )
	{
		Vector vPlayerForward;
		pPlayer->EyeVectors( &vPlayerForward );

		Vector radial = physcollision->CollideGetExtent( pPhys->GetCollide(), vec3_origin, pEntity->GetAbsAngles(), -vPlayerForward );
		Vector player2d = pPlayer->CollisionProp()->OBBMaxs();
		float playerRadius = player2d.Length2D();
		float flDot = DotProduct( vPlayerForward, radial );

		float radius = playerRadius + fabs( flDot );

		float distance = 24 + ( radius * 2.0f );		

		//find out which portal the object is on the other side of....
		Vector start = pPlayer->Weapon_ShootPosition();		
		Vector end = start + ( vPlayerForward * distance );

		CProp_Portal *pObjectPortal = NULL;
		pObjectPortal = pPortalPlayer->GetHeldObjectPortal();

		// If our end point hasn't gone into the portal yet we at least need to know what portal is in front of us
		if ( !pObjectPortal )
		{
			Ray_t rayPortalTest;
			rayPortalTest.Init( start, start + vPlayerForward * 1024.0f );

			int iPortalCount = CProp_Portal_Shared::AllPortals.Count();
			if( iPortalCount != 0 )
			{
				CProp_Portal **pPortals = CProp_Portal_Shared::AllPortals.Base();
				float fMinDist = 2.0f;
				for( int i = 0; i != iPortalCount; ++i )
				{
					CProp_Portal *pTempPortal = pPortals[i];
					if( pTempPortal->m_bActivated &&
						(pTempPortal->m_hLinkedPortal.Get() != NULL) )
					{
						float fDist = UTIL_IntersectRayWithPortal( rayPortalTest, pTempPortal );
						if( (fDist >= 0.0f) && (fDist < fMinDist) )
						{
							fMinDist = fDist;
							pObjectPortal = pTempPortal;
						}
					}
				}
			}
		}

		if( pObjectPortal )
		{
			UTIL_Portal_AngleTransform( pObjectPortal->m_hLinkedPortal->MatrixThisToLinked(), angles, angles );
		}
	}
#endif
	VectorITransform( pEntity->WorldSpaceCenter(), pEntity->EntityToWorldTransform(), m_attachedPositionObjectSpace );
//	ComputeMaxSpeed( pEntity, pPhys );
#ifdef GAME_DLL
	// If we haven't been killed by a grab, we allow the gun to grab the nearest part of a ragdoll
	if ( bUseGrabPosition )
	{
		IPhysicsObject *pChild = GetRagdollChildAtPosition( pEntity, vGrabPosition );
		
		if ( pChild )
		{
			pPhys = pChild;
		}
	}
#endif
	// Carried entities can never block LOS
	m_bCarriedEntityBlocksLOS = pEntity->BlocksLOS();
	pEntity->SetBlocksLOS( false );
	m_controller = physenv->CreateMotionController( this );
	m_controller->AttachObject( pPhys, true );
	// Don't do this, it's causing trouble with constraint solvers.
	//m_controller->SetPriority( IPhysicsMotionController::HIGH_PRIORITY );

	pPhys->Wake();
	PhysSetGameFlags( pPhys, FVPHYSICS_PLAYER_HELD );
	SetTargetPosition( position, angles );
	m_attachedEntity = pEntity;
	IPhysicsObject *pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
	int count = pEntity->VPhysicsGetObjectList( pList, ARRAYSIZE(pList) );
	m_flLoadWeight = 0;
	float damping = 10;
	float flFactor = count / 7.5f;
	if ( flFactor < 1.0f )
	{
		flFactor = 1.0f;
	}
	for ( int i = 0; i < count; i++ )
	{
		float mass = pList[i]->GetMass();
		pList[i]->GetDamping( NULL, &m_savedRotDamping[i] );
		m_flLoadWeight += mass;
		m_savedMass[i] = mass;

		// reduce the mass to prevent the player from adding crazy amounts of energy to the system
		pList[i]->SetMass( REDUCED_CARRY_MASS / flFactor );
		pList[i]->SetDamping( NULL, &damping );
	}
	
	// Give extra mass to the phys object we're actually picking up
	pPhys->SetMass( REDUCED_CARRY_MASS );
	pPhys->EnableDrag( false );

	m_errorTime = bIsMegaPhysCannon ? -1.5f : -1.0f; // 1 seconds until error starts accumulating
	m_error = 0;
	m_contactAmount = 0;

	m_attachedAnglesPlayerSpace = TransformAnglesToPlayerSpace( angles, pPlayer );
	if ( m_angleAlignment != 0 )
	{
		m_attachedAnglesPlayerSpace = AlignAngles( m_attachedAnglesPlayerSpace, m_angleAlignment );
	}

#ifdef GAME_DLL
	// Ragdolls don't offset this way
	if ( dynamic_cast<CRagdollProp*>(pEntity) )
	{
		m_attachedPositionObjectSpace.Init();
	}
	else
	{
		VectorITransform( pEntity->WorldSpaceCenter(), pEntity->EntityToWorldTransform(), m_attachedPositionObjectSpace );
	}

	// If it's a prop, see if it has desired carry angles
	CPhysicsProp *pProp = dynamic_cast<CPhysicsProp *>(pEntity);
	if ( pProp )
	{
		m_bHasPreferredCarryAngles = pProp->GetPropDataAngles( "preferred_carryangles", m_vecPreferredCarryAngles );
		m_flDistanceOffset = pProp->GetCarryDistanceOffset();
	}
	else
	{
		m_bHasPreferredCarryAngles = false;
		m_flDistanceOffset = 0;
	}
#endif
	m_bAllowObjectOverhead = IsObjectAllowedOverhead( pEntity );
}

static void ClampPhysicsVelocity( IPhysicsObject *pPhys, float linearLimit, float angularLimit )
{
	Vector vel;
	AngularImpulse angVel;
	pPhys->GetVelocity( &vel, &angVel );
	float speed = VectorNormalize(vel) - linearLimit;
	float angSpeed = VectorNormalize(angVel) - angularLimit;
	speed = speed < 0 ? 0 : -speed;
	angSpeed = angSpeed < 0 ? 0 : -angSpeed;
	vel *= speed;
	angVel *= angSpeed;
	pPhys->AddVelocity( &vel, &angVel );
}

void CGrabController::DetachEntity( bool bClearVelocity )
{
//	Assert(!PhysIsInCallback());
	CBaseEntity *pEntity = GetAttached();
	if ( pEntity )
	{
		// Restore the LS blocking state
		pEntity->SetBlocksLOS( m_bCarriedEntityBlocksLOS );
		IPhysicsObject *pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
		int count = pEntity->VPhysicsGetObjectList( pList, ARRAYSIZE(pList) );
		for ( int i = 0; i < count; i++ )
		{
			IPhysicsObject *pPhys = pList[i];
			if ( !pPhys )
				continue;

			// on the odd chance that it's gone to sleep while under anti-gravity
			pPhys->EnableDrag( true );
			pPhys->Wake();
			pPhys->SetMass( m_savedMass[i] );
			pPhys->SetDamping( NULL, &m_savedRotDamping[i] );
			PhysClearGameFlags( pPhys, FVPHYSICS_PLAYER_HELD );
			if ( bClearVelocity )
			{
				PhysForceClearVelocity( pPhys );
			}
			else
			{
#ifdef GAME_DLL
				ClampPhysicsVelocity( pPhys, 320, 2.0f * 360.0f );
#endif
			}

		}
	}

	m_attachedEntity = NULL;
	if ( physenv )
		physenv->DestroyMotionController( m_controller );

	m_controller = NULL;
}

static bool InContactWithHeavyObject( IPhysicsObject *pObject, float heavyMass )
{
	bool contact = false;
	IPhysicsFrictionSnapshot *pSnapshot = pObject->CreateFrictionSnapshot();
	while ( pSnapshot->IsValid() )
	{
		IPhysicsObject *pOther = pSnapshot->GetObject( 1 );
		if ( !pOther->IsMoveable() || pOther->GetMass() > heavyMass )
		{
			contact = true;
			break;
		}
		pSnapshot->NextFrictionData();
	}
	pObject->DestroyFrictionSnapshot( pSnapshot );
	return contact;
}

IMotionEvent::simresult_e CGrabController::Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular )
{
	game_shadowcontrol_params_t shadowParams = m_shadow;
	if ( InContactWithHeavyObject( pObject, GetLoadWeight() ) )
	{
		m_contactAmount = Approach( 0.1f, m_contactAmount, deltaTime*2.0f );
	}
	else
	{
		m_contactAmount = Approach( 1.0f, m_contactAmount, deltaTime*2.0f );
	}
	shadowParams.maxAngular = m_shadow.maxAngular * m_contactAmount * m_contactAmount * m_contactAmount;
	m_timeToArrive = pObject->ComputeShadowControl( shadowParams, m_timeToArrive, deltaTime );
	
	// Slide along the current contact points to fix bouncing problems
	Vector velocity;
	AngularImpulse angVel;
	pObject->GetVelocity( &velocity, &angVel );
	PhysComputeSlideDirection( pObject, velocity, angVel, &velocity, &angVel, GetLoadWeight() );
	pObject->SetVelocityInstantaneous( &velocity, NULL );

	linear.Init();
	angular.Init();
	m_errorTime += deltaTime;

	return SIM_LOCAL_ACCELERATION;
}

float CGrabController::GetSavedMass( IPhysicsObject *pObject )
{
	CBaseEntity *pHeld = m_attachedEntity;
	if ( pHeld )
	{
		if ( pObject->GetGameData() == (void*)pHeld )
		{
			IPhysicsObject *pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
			int count = pHeld->VPhysicsGetObjectList( pList, ARRAYSIZE(pList) );
			for ( int i = 0; i < count; i++ )
			{
				if ( pList[i] == pObject )
					return m_savedMass[i];
			}
		}
	}
	return 0.0f;
}

float CGrabController::GetSavedRotDamping(IPhysicsObject* pObject)
{
	CBaseEntity* pHeld = m_attachedEntity;
	if (pHeld)
	{
		if (pObject->GetGameData() == (void*)pHeld)
		{
			IPhysicsObject* pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
			int count = pHeld->VPhysicsGetObjectList(pList, ARRAYSIZE(pList));
			for (int i = 0; i < count; i++)
			{
				if (pList[i] == pObject)
					return m_savedRotDamping[i];
			}
		}
	}
	return 0.0f;
}

//-----------------------------------------------------------------------------
// Is this an object that the player is allowed to lift to a position 
// directly overhead? The default behavior prevents lifting objects directly
// overhead, but there are exceptions for gameplay purposes.
//-----------------------------------------------------------------------------
bool CGrabController::IsObjectAllowedOverhead( CBaseEntity *pEntity )
{
#ifdef GAME_DLL
	// Allow combine balls overhead 
	if( UTIL_IsCombineBallDefinite( pEntity ) )
		return true;

	// Allow props that are specifically flagged as such
	CPhysicsProp *pPhysProp = dynamic_cast<CPhysicsProp *>(pEntity);
	if ( pPhysProp != NULL && pPhysProp->HasInteraction( PROPINTER_PHYSGUN_ALLOW_OVERHEAD ) )
		return true;

	// String checks are fine here, we only run this code one time- when the object is picked up.
	if( pEntity->ClassMatches("grenade_helicopter") )
		return true;

	if( pEntity->ClassMatches("weapon_striderbuster") )
		return true;
#endif

	return false;
}

void CGrabController::SetPortalPenetratingEntity( CBaseEntity *pPenetrated )
{
	m_PenetratedEntity = pPenetrated;
}

//-----------------------------------------------------------------------------
// Player pickup controller
//-----------------------------------------------------------------------------
class CPlayerPickupController : public CBaseEntity
{
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif
	DECLARE_CLASS( CPlayerPickupController, CBaseEntity );
public:
	void			Init( CBasePlayer *pPlayer, CBaseEntity *pObject );
	void			Shutdown( bool bThrown = false );
	virtual void	UpdateOnRemove( void );

#ifdef GAME_DLL
	virtual int		ShouldTransmit( const CCheckTransmitInfo *pInfo );
	virtual int		UpdateTransmitState( void );
#endif

	bool OnControls( CBaseEntity *pControls ) { return true; }
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void OnRestore()
	{
		m_grabController.OnRestore();
	}
	void VPhysicsUpdate( IPhysicsObject *pPhysics ){}
	void VPhysicsShadowUpdate( IPhysicsObject *pPhysics ) {}

	bool IsHoldingEntity( CBaseEntity *pEnt );
	CGrabController &GetGrabController() { return m_grabController; }

private:
	CGrabController		m_grabController;
	CBasePlayer			*m_pPlayer;
};

LINK_ENTITY_TO_CLASS( player_pickup, CPlayerPickupController );

#ifdef GAME_DLL
//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CPlayerPickupController )

	DEFINE_EMBEDDED( m_grabController ),

	// Physptrs can't be inside embedded classes
	DEFINE_PHYSPTR( m_grabController.m_controller ),

	DEFINE_FIELD( m_pPlayer,		FIELD_CLASSPTR ),
	
END_DATADESC()
#endif

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//			*pObject - 
//-----------------------------------------------------------------------------
void CPlayerPickupController::Init( CBasePlayer *pPlayer, CBaseEntity *pObject )
{
#ifdef GAME_DLL
	CTFPlayer *pTFPlayer = ToTFPlayer( pPlayer );
	if ( !pTFPlayer )
	{
		UTIL_Remove( this );
		return;
	}

	SetOwnerEntity( pTFPlayer );

	// Holster player's weapon
	if ( pTFPlayer->GetActiveTFWeapon() )
	{
		// Don't holster the portalgun
		if ( pTFPlayer->GetActiveTFWeapon()->GetWeaponID() == TF_WEAPON_PORTALGUN )
		{
			CWeaponPortalgun *pPortalGun = (CWeaponPortalgun*)(pTFPlayer->GetActiveTFWeapon());
			pPortalGun->OpenProngs( true );
		}
		else
		{
			if ( !pTFPlayer->GetActiveWeapon()->Lower() )
			{
				Shutdown();
				return;
			}
		}
	}

	// If the target is debris, convert it to non-debris
	if ( pObject->GetCollisionGroup() == COLLISION_GROUP_DEBRIS )
	{
		// Interactive debris converts back to debris when it comes to rest
		pObject->SetCollisionGroup( COLLISION_GROUP_INTERACTIVE_DEBRIS );
	}

	// done so I'll go across level transitions with the player
	SetParent( pTFPlayer );
	m_grabController.SetIgnorePitch( true );
	m_grabController.SetAngleAlignment( DOT_30DEGREE );
	m_pPlayer = pTFPlayer;
	IPhysicsObject *pPhysics = pObject->VPhysicsGetObject();

	CTFPlayer* pPortalPlayer = ToTFPlayer( pTFPlayer );
	if ( pPortalPlayer && !pPortalPlayer->m_bSilentDropAndPickup )
		Pickup_OnPhysGunPickup( pObject, m_pPlayer, PICKED_UP_BY_PLAYER );

	m_grabController.AttachEntity( pTFPlayer, pObject, pPhysics, false, vec3_origin, false );

	//PhysDisableEntityCollisions( pTFPlayer, pObject );

	//m_pPlayer->m_Local.m_iHideHUD |= HIDEHUD_WEAPONSELECTION;
	m_pPlayer->SetUseEntity( this );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Y.e.e.t 
//-----------------------------------------------------------------------------
void CPlayerPickupController::Shutdown( bool bThrown )
{
#ifdef GAME_DLL
	CBaseEntity *pObject = m_grabController.GetAttached();

	bool bClearVelocity = false;
	if ( !bThrown && pObject && pObject->VPhysicsGetObject() && pObject->VPhysicsGetObject()->GetContactPoint(NULL,NULL) )
	{
		bClearVelocity = true;
	}

	m_grabController.DetachEntity( bClearVelocity );

	CTFPlayer *pTFPlayer = ToTFPlayer( m_pPlayer );
	if ( !pTFPlayer )
	{
		UTIL_Remove( this );
		return;
	}

	if ( pObject != NULL )
	{
		CTFPlayer* pPortalPlayer = ToTFPlayer( m_pPlayer );
		if ( pPortalPlayer && !pPortalPlayer->m_bSilentDropAndPickup )
			Pickup_OnPhysGunDrop( pObject, m_pPlayer, bThrown ? THROWN_BY_PLAYER : DROPPED_BY_PLAYER );
	}

	//PhysEnableEntityCollisions( pTFPlayer, pObject );
	pTFPlayer->SetUseEntity( NULL );
	if ( !pTFPlayer->m_bSilentDropAndPickup )
	{
		if ( pTFPlayer->GetActiveTFWeapon() )
		{
			if ( pTFPlayer->GetActiveTFWeapon()->GetWeaponID() == TF_WEAPON_PORTALGUN )
			{
				pTFPlayer->SetNextAttack( gpGlobals->curtime + 0.5f );
				CWeaponPortalgun *pPortalGun = (CWeaponPortalgun*)(pTFPlayer->GetActiveWeapon());
				pPortalGun->DelayAttack( 0.5f );
				pPortalGun->OpenProngs( false );
			}
			else
			{
				if ( !pTFPlayer->GetActiveTFWeapon()->ReadyIgnoreSequence() )
				{
					// We tried to restore the player's weapon, but we couldn't.
					// This usually happens when they're holding an empty weapon that doesn't
					// autoswitch away when out of ammo. Switch to next best weapon.
					pTFPlayer->SwitchToNextBestWeapon( NULL );
				}
			}
		}
	}

	UTIL_Remove( this );
#endif
}

//-----------------------------------------------------------------------------
// On Remove
//-----------------------------------------------------------------------------
void CPlayerPickupController::UpdateOnRemove( void )
{
	Shutdown();

	BaseClass::UpdateOnRemove();
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CPlayerPickupController::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_FULLCHECK );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CPlayerPickupController::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	// Always transmit to the owning player.
	if ( m_pPlayer && pInfo->m_pClientEnt == m_pPlayer->edict() )
	{
		return FL_EDICT_ALWAYS;
	}

	return BaseClass::ShouldTransmit( pInfo );
}
#endif

void CPlayerPickupController::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( ToTFPlayer( pActivator ) == m_pPlayer )
	{
		CBaseEntity *pAttached = m_grabController.GetAttached();

		// UNDONE: Use vphysics stress to decide to drop objects
		// UNDONE: Must fix case of forcing objects into the ground you're standing on (causes stress) before that will work
		if ( !pAttached || useType == USE_OFF || (m_pPlayer->m_nButtons & IN_ATTACK2) || m_grabController.ComputeError() > 12 )
		{
			Shutdown();
			return;
		}

		//Adrian: Oops, our object became motion disabled, let go!
		IPhysicsObject *pPhys = pAttached->VPhysicsGetObject();
		if ( !pPhys || pPhys->IsMoveable() == false )
		{
			Shutdown();
			return;
		}

		// +ATTACK will throw phys objects
		if ( m_pPlayer->m_nButtons & IN_ATTACK )
		{
			Shutdown( true );
			Vector vecLaunch;
			m_pPlayer->EyeVectors( &vecLaunch );

			// If throwing from the opposite side of a portal, reorient the direction
			if (((CTFPlayer*)m_pPlayer)->IsHeldObjectOnOppositeSideOfPortal())
			{
				CProp_Portal* pHeldPortal = ((CTFPlayer*)m_pPlayer)->GetHeldObjectPortal();
				UTIL_Portal_VectorTransform(pHeldPortal->MatrixThisToLinked(), vecLaunch, vecLaunch);
			}
			
			( ToTFPlayer( m_pPlayer ) )->SetHeldObjectOnOppositeSideOfPortal( false );
			CTFPlayer *mTFPlayer = ToTFPlayer(m_pPlayer);
			float playerClassFactor = 1.0f;
			switch (mTFPlayer->GetPlayerClass()->GetClassIndex())
			{

			case TF_CLASS_HEAVYWEAPONS:
				playerClassFactor = 7.5f;
				break;
			case TF_CLASS_SOLDIER:
				playerClassFactor = 5.0f;
				break;
			case TF_CLASS_PYRO:
				playerClassFactor = 3.8f;
				break;
			case TF_CLASS_DEMOMAN:
				playerClassFactor = 3.8f;
				break;
			case TF_CLASS_SNIPER:
				playerClassFactor = 2.5f;
				break;
			case TF_CLASS_ENGINEER:
				playerClassFactor = 2.5f;
				break;
			case TF_CLASS_SPY:
				playerClassFactor = 1.5f;
				break;
			case TF_CLASS_MEDIC:
				playerClassFactor = 1.8f;
				break;
			case TF_CLASS_SCOUT:
				playerClassFactor = 0.9f;
				break;
			default:
				playerClassFactor = 1.0f;
				break;
			}
			// JAY: Scale this with mass because some small objects really go flying
			

			float massFactor = pPhys ? clamp( pPhys->GetMass(), 0.5, 15 ) : 7.5;
			massFactor = RemapVal( massFactor, 0.5, 15, 0.5, 4 );
			vecLaunch *= player_throwforce.GetFloat() * playerClassFactor * massFactor;
			pPhys->ApplyForceCenter( vecLaunch );
			AngularImpulse aVel = RandomAngularImpulse(-10, 10) * massFactor;
			pPhys->ApplyTorqueCenter( aVel );
			return;
		}

		if ( useType == USE_SET )
		{
			// update position
			m_grabController.UpdateObject( m_pPlayer, 12 );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEnt - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CPlayerPickupController::IsHoldingEntity( CBaseEntity *pEnt )
{
	if ( pEnt )
	{
		return ( m_grabController.GetAttached() == pEnt );
	}

	return ( m_grabController.GetAttached() != 0 );
}

void PlayerPickupObject( CBasePlayer *pPlayer, CBaseEntity *pObject )
{
#ifdef GAME_DLL
	//Don't pick up if we don't have a phys object.
	if ( pObject->VPhysicsGetObject() == NULL )
		 return;

	if ( pObject->GetBaseAnimating() && pObject->GetBaseAnimating()->IsDissolving() )
		return;

	CPlayerPickupController *pController = (CPlayerPickupController *)CBaseEntity::Create( "player_pickup", pObject->GetAbsOrigin(), vec3_angle, pPlayer );
	
	if ( !pController )
		return;

	pController->Init( pPlayer, pObject );
#endif

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//  CInterpolatedValue class
//----------------------------------------------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------------------------------------------
//  CWeaponPhysCannon class
//----------------------------------------------------------------------------------------------------------------------------------------------------------

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponPhysCannon, DT_WeaponPhysCannon )

BEGIN_NETWORK_TABLE( CWeaponPhysCannon, DT_WeaponPhysCannon )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bActive ) ),
	RecvPropEHandle( RECVINFO( m_hAttachedObject ) ),
	RecvPropVector( RECVINFO( m_attachedPositionObjectSpace ) ),
	RecvPropFloat( RECVINFO( m_attachedAnglesPlayerSpace[0] ) ),
	RecvPropFloat( RECVINFO( m_attachedAnglesPlayerSpace[1] ) ),
	RecvPropFloat( RECVINFO( m_attachedAnglesPlayerSpace[2] ) ),
	RecvPropInt( RECVINFO( m_EffectState ) ),
	RecvPropBool( RECVINFO( m_bOpen ) ),
#else
	SendPropBool( SENDINFO( m_bActive ) ),
	SendPropEHandle( SENDINFO( m_hAttachedObject ) ),
	SendPropVector(SENDINFO( m_attachedPositionObjectSpace ), -1, SPROP_COORD),
	SendPropAngle( SENDINFO_VECTORELEM(m_attachedAnglesPlayerSpace, 0 ), 11 ),
	SendPropAngle( SENDINFO_VECTORELEM(m_attachedAnglesPlayerSpace, 1 ), 11 ),
	SendPropAngle( SENDINFO_VECTORELEM(m_attachedAnglesPlayerSpace, 2 ), 11 ),
	SendPropInt( SENDINFO( m_EffectState ) ),
	SendPropBool( SENDINFO( m_bOpen ) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponPhysCannon )
	DEFINE_PRED_FIELD( m_EffectState,	FIELD_INTEGER,	FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bOpen,			FIELD_BOOLEAN,	FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#else
BEGIN_DATADESC(CWeaponPhysCannon)
	DEFINE_FIELD(m_bOpen, FIELD_BOOLEAN),
	DEFINE_FIELD(m_bActive, FIELD_BOOLEAN),

	DEFINE_FIELD(m_nChangeState, FIELD_INTEGER),
	DEFINE_FIELD(m_flCheckSuppressTime, FIELD_TIME),
	DEFINE_FIELD(m_flElementDebounce, FIELD_TIME),
	//DEFINE_FIELD(m_flElementPosition, FIELD_FLOAT),
	//DEFINE_FIELD(m_flElementDestination, FIELD_FLOAT),
	DEFINE_FIELD(m_nAttack2Debounce, FIELD_INTEGER),
	DEFINE_FIELD(m_bIsCurrentlyUpgrading, FIELD_BOOLEAN),
	DEFINE_FIELD(m_EffectState, FIELD_INTEGER),

	//DEFINE_AUTO_ARRAY(m_hBeams, FIELD_EHANDLE),
	//DEFINE_AUTO_ARRAY(m_hGlowSprites, FIELD_EHANDLE),
	//DEFINE_AUTO_ARRAY(m_hEndSprites, FIELD_EHANDLE),
	//DEFINE_AUTO_ARRAY(m_flEndSpritesOverride, FIELD_TIME),
	//DEFINE_FIELD(m_hCenterSprite, FIELD_EHANDLE),
	//DEFINE_FIELD(m_hBlastSprite, FIELD_EHANDLE),
	DEFINE_FIELD(m_flLastDenySoundPlayed, FIELD_BOOLEAN),
	DEFINE_FIELD(m_bPhyscannonState, FIELD_BOOLEAN),
	DEFINE_SOUNDPATCH(m_sndMotor),

	DEFINE_EMBEDDED(m_grabController),

	// Physptrs can't be inside embedded classes
	DEFINE_PHYSPTR(m_grabController.m_controller),

	DEFINE_THINKFUNC(WaitForUpgradeThink),

	DEFINE_UTLVECTOR(m_ThrownEntities, FIELD_EMBEDDED),

	DEFINE_FIELD(m_flTimeNextObjectPurge, FIELD_TIME),
END_DATADESC()
#endif

LINK_ENTITY_TO_CLASS( tf_weapon_physcannon, CWeaponPhysCannon );
#ifndef CLIENT_DLL
// For HL2 map compatibility.
LINK_ENTITY_TO_CLASS( weapon_physcannon, CWeaponPhysCannon );
#endif
PRECACHE_WEAPON_REGISTER( tf_weapon_physcannon );

enum
{
	ELEMENT_STATE_NONE = -1,
	ELEMENT_STATE_OPEN,
	ELEMENT_STATE_CLOSED,
};

enum
{
	EFFECT_NONE,
	EFFECT_CLOSED,
	EFFECT_READY,
	EFFECT_HOLDING,
	EFFECT_LAUNCH,
};

//-----------------------------------------------------------------------------
// Do we have the super-phys gun?
//-----------------------------------------------------------------------------
bool PlayerHasMegaPhysCannon()
{
	return ( TFGameRules()->MegaPhyscannonActive() == true );
}

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponPhysCannon::CWeaponPhysCannon( void )
{
	m_bOpen					= false;
	m_EffectState			= (int)EFFECT_NONE;
	
	m_nChangeState			= ELEMENT_STATE_NONE;
	m_flCheckSuppressTime	= 0.0f;
	m_flLastDenySoundPlayed	= false;

#ifdef CLIENT_DLL
	m_nOldEffectState		= EFFECT_NONE;
	m_bOldOpen				= false;
#endif

	m_bPhyscannonState = false;
}

void CWeaponPhysCannon::WeaponReset( void )
{
	m_nChangeState			= ELEMENT_STATE_NONE;
	m_flCheckSuppressTime	= 0.0f;
	m_flLastDenySoundPlayed	= false;

	ForceDrop();
	DestroyEffects();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CWeaponPhysCannon::GetSlot( void ) const
{
	// Hack to make place gravity gun in different buckets slots for Engineer and Spy.
	CTFPlayer *pOwner = GetTFPlayerOwner();

	if ( pOwner )
	{
		if ( pOwner->IsPlayerClass( TF_CLASS_ENGINEER ) )
			return 5;
		else if ( pOwner->IsPlayerClass( TF_CLASS_SPY ) )
			return 4;
	}
	
	return BaseClass::GetSlot();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::FallInit( void )
{
#ifndef CLIENT_DLL
	// Skip TF weapon base as it prevents FallInit of base weapon from running.
	CBaseCombatWeapon::FallInit();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Precache
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::Precache( void )
{
#ifndef CLIENT_DLL
	// Set the proper classname so it loads the correct script file.
	SetClassname( "tf_weapon_physcannon" );
#endif

	PrecacheModel( PHYSCANNON_MODEL_SCOUT );
	PrecacheModel( PHYSCANNON_MODEL_SOLDIER );
	PrecacheModel( PHYSCANNON_MODEL_PYRO );
	PrecacheModel( PHYSCANNON_MODEL_DEMO );
	PrecacheModel( PHYSCANNON_MODEL_HEAVY );
	PrecacheModel( PHYSCANNON_MODEL_ENGINEER );
	PrecacheModel( PHYSCANNON_MODEL_MEDIC );
	PrecacheModel( PHYSCANNON_MODEL_SNIPER );
	PrecacheModel( PHYSCANNON_MODEL_SPY );
	PrecacheModel( MEGACANNON_MODEL_SCOUT );
	PrecacheModel( MEGACANNON_MODEL_SOLDIER );
	PrecacheModel( MEGACANNON_MODEL_PYRO );
	PrecacheModel( MEGACANNON_MODEL_DEMO );
	PrecacheModel( MEGACANNON_MODEL_HEAVY );
	PrecacheModel( MEGACANNON_MODEL_ENGINEER );
	PrecacheModel( MEGACANNON_MODEL_MEDIC );
	PrecacheModel( MEGACANNON_MODEL_SNIPER );
	PrecacheModel( MEGACANNON_MODEL_SPY );
	PrecacheModel(PHYSCANNON_MODEL_WILDCARD);
	PrecacheModel(MEGACANNON_MODEL_WILDCARD);

	PrecacheModel( PHYSCANNON_BEAM_SPRITE );
	PrecacheModel( PHYSCANNON_BEAM_SPRITE_NOZ );
	PrecacheModel( MEGACANNON_BEAM_SPRITE );
	PrecacheModel( MEGACANNON_BEAM_SPRITE_NOZ );

	PrecacheModel( MEGACANNON_RAGDOLL_BOOGIE_SPRITE );

	// Precache our alternate model
	PrecacheModel( MEGACANNON_MODEL );

	PrecacheScriptSound( "Weapon_PhysCannon.HoldSound" );

	PrecacheScriptSound( "Weapon_MegaPhysCannon.DryFire" );
	PrecacheScriptSound( "Weapon_MegaPhysCannon.Launch" );
	PrecacheScriptSound( "Weapon_MegaPhysCannon.Pickup" );
	PrecacheScriptSound( "Weapon_MegaPhysCannon.Drop" );
	PrecacheScriptSound( "Weapon_MegaPhysCannon.HoldSound" );
	PrecacheScriptSound( "Weapon_MegaPhysCannon.ChargeZap" );

	PrecacheParticleSystem( "physcannon_super_crit_shockwave" );
	PrecacheParticleSystem( "physcannon_super_crit_attack_glow" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::Spawn(void)
{
	BaseClass::Spawn();

	// Need to get close to pick it up
	CollisionProp()->UseTriggerBounds(false);

	m_bPhyscannonState = IsMegaPhysCannon();

	// The megacannon uses a different skin
	if ( IsMegaPhysCannon() )
		m_nSkin = MEGACANNON_SKIN;

	m_flTimeForceView = -1;
}

//-----------------------------------------------------------------------------
// Purpose: Restore
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::OnRestore()
{
	BaseClass::OnRestore();
	m_grabController.OnRestore();

	m_bPhyscannonState = IsMegaPhysCannon();

	// Tracker 8106:  Physcannon effects disappear through level transition, so
	//  just recreate any effects here
	if ( m_EffectState != EFFECT_NONE )
	{
		DoEffect( m_EffectState, NULL );
	}
}


//-----------------------------------------------------------------------------
// On Remove
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::UpdateOnRemove(void)
{
	DestroyEffects( );
	BaseClass::UpdateOnRemove();
}

#ifdef CLIENT_DLL
void CWeaponPhysCannon::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );

		C_BaseAnimating::AutoAllowBoneAccess boneaccess( true, false );
		StartEffects();
	}

	if ( GetOwner() == NULL )
	{
		if ( m_hAttachedObject )
		{
			m_hAttachedObject->VPhysicsDestroyObject();
		}

		if ( m_hOldAttachedObject )
		{
			m_hOldAttachedObject->VPhysicsDestroyObject();
		}
	}

	// Update effect state when out of parity with the server
	if ( m_nOldEffectState != m_EffectState )
	{
		DoEffect( m_EffectState );
		m_nOldEffectState = m_EffectState;
	}

	// Update element state when out of parity
	if ( m_bOldOpen != m_bOpen )
	{
		if ( m_bOpen )
		{
			m_ElementParameter.InitFromCurrent( 1.0f, 0.2f, INTERP_SPLINE );
		}
		else
		{	
			m_ElementParameter.InitFromCurrent( 0.0f, 0.5f, INTERP_SPLINE );
		}

		m_bOldOpen = (bool) m_bOpen;
	}
}
#endif

//-----------------------------------------------------------------------------
// Sprite scale factor 
//-----------------------------------------------------------------------------
inline float CWeaponPhysCannon::SpriteScaleFactor() 
{
	return IsMegaPhysCannon() ? 0.0f /*1.5f*/ : 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponPhysCannon::Deploy( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();

#ifdef GAME_DLL
	if ( pOwner && !HasItemDefinition() )
		pOwner->Regenerate();
#endif

	CloseElements();
	DoEffect( EFFECT_READY );

	bool bReturn = BaseClass::Deploy();

	m_flNextSecondaryAttack = m_flNextPrimaryAttack = m_flNextTertiaryAttack = gpGlobals->curtime;

	// Unbloat our bounds
	if ( IsMegaPhysCannon() )
		CollisionProp()->UseTriggerBounds( false );

	if ( pOwner )
		pOwner->SetNextAttack( gpGlobals->curtime );

	return bReturn;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::SetViewModel( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	CTFViewModel *vm = dynamic_cast<CTFViewModel*>(pOwner->GetViewModel(m_nViewModelIndex, false));
	if ( !vm )
		return;

	Assert( vm->ViewModelIndex() == m_nViewModelIndex );

	if ( !IsMegaPhysCannon() )
	{
		CTFWeaponBase *pLastWeapon = static_cast<CTFWeaponBase*>(pOwner->GetLastWeapon());
		if (pLastWeapon)
		{
			CTFViewModel *vm2 = dynamic_cast<CTFViewModel*>(pOwner->GetViewModel(pLastWeapon->m_nViewModelIndex, false));

			if ( pLastWeapon && vm2 && pOwner->GetActiveTFWeapon() && pOwner->GetActiveTFWeapon()->GetWeaponID() == TF_WEAPON_PHYSCANNON )
			{
				vm2->SetWeaponModel(NULL, pLastWeapon);
			}
		}
		//fix model issues
		if (pOwner->IsPlayerClass(TF_CLASS_SCOUT))
		{
			vm->SetWeaponModel(PHYSCANNON_MODEL_SCOUT, this);
		}
		else if (pOwner->IsPlayerClass(TF_CLASS_SOLDIER))
		{
			vm->SetWeaponModel(PHYSCANNON_MODEL_SOLDIER, this);
		}
		else if (pOwner->IsPlayerClass(TF_CLASS_PYRO))
		{
			vm->SetWeaponModel(PHYSCANNON_MODEL_PYRO, this);
		}
		else if (pOwner->IsPlayerClass(TF_CLASS_DEMOMAN))
		{
			vm->SetWeaponModel(PHYSCANNON_MODEL_DEMO, this);
		}
		else if (pOwner->IsPlayerClass(TF_CLASS_HEAVYWEAPONS))
		{
			vm->SetWeaponModel(PHYSCANNON_MODEL_HEAVY, this);
		}
		else if (pOwner->IsPlayerClass(TF_CLASS_ENGINEER))
		{
			vm->SetWeaponModel(PHYSCANNON_MODEL_ENGINEER, this);
		}
		else if (pOwner->IsPlayerClass(TF_CLASS_MEDIC))
		{
			vm->SetWeaponModel(PHYSCANNON_MODEL_MEDIC, this);
		}
		else if (pOwner->IsPlayerClass(TF_CLASS_SNIPER))
		{
			vm->SetWeaponModel(PHYSCANNON_MODEL_SNIPER, this);
		}
		else if (pOwner->IsPlayerClass(TF_CLASS_SPY))
		{
			vm->SetWeaponModel(PHYSCANNON_MODEL_SPY, this);
		}
		else if (pOwner->IsPlayerClass(TF_CLASS_WILDCARD))
		{
			vm->SetWeaponModel(PHYSCANNON_MODEL_WILDCARD, this);
		}
		else if (pOwner->IsPlayerClass(TF_CLASS_CIVILIAN))
		{
			vm->SetWeaponModel(PHYSCANNON_MODEL_CIVILIAN, this);
		}
		else if (pOwner->IsPlayerClass(TF_CLASS_COMBINE))
		{
			vm->SetWeaponModel(PHYSCANNON_MODEL_COMBINE, this);
		}
		else if (pOwner->IsPlayerClass(TF_CLASS_ZOMBIEFAST))
		{
			vm->SetWeaponModel(PHYSCANNON_MODEL_ZOMBIEFAST, this);
		}		
	}
	else
	{
		CTFWeaponBase *pLastWeapon = static_cast<CTFWeaponBase*>(pOwner->GetLastWeapon());
		if (pLastWeapon)
		{
			CTFViewModel *vm2 = dynamic_cast<CTFViewModel*>(pOwner->GetViewModel(pLastWeapon->m_nViewModelIndex, false));

			if ( pLastWeapon && vm2 && pOwner->GetActiveTFWeapon()->GetWeaponID() == TF_WEAPON_PHYSCANNON )
			{
				vm2->SetWeaponModel(NULL, pLastWeapon);
			}
		}
		//fix model issues
		if (pOwner->IsPlayerClass(TF_CLASS_SCOUT))
		{
			vm->SetWeaponModel(MEGACANNON_MODEL_SCOUT, this);
		}
		else if (pOwner->IsPlayerClass(TF_CLASS_SOLDIER))
		{
			vm->SetWeaponModel(MEGACANNON_MODEL_SOLDIER, this);
		}
		else if (pOwner->IsPlayerClass(TF_CLASS_PYRO))
		{
			vm->SetWeaponModel(MEGACANNON_MODEL_PYRO, this);
		}
		else if (pOwner->IsPlayerClass(TF_CLASS_DEMOMAN))
		{
			vm->SetWeaponModel(MEGACANNON_MODEL_DEMO, this);
		}
		else if (pOwner->IsPlayerClass(TF_CLASS_HEAVYWEAPONS))
		{
			vm->SetWeaponModel(MEGACANNON_MODEL_HEAVY, this);
		}
		else if (pOwner->IsPlayerClass(TF_CLASS_ENGINEER))
		{
			vm->SetWeaponModel(MEGACANNON_MODEL_ENGINEER, this);
		}
		else if (pOwner->IsPlayerClass(TF_CLASS_MEDIC))
		{
			vm->SetWeaponModel(MEGACANNON_MODEL_MEDIC, this);
		}
		else if (pOwner->IsPlayerClass(TF_CLASS_SNIPER))
		{
			vm->SetWeaponModel(MEGACANNON_MODEL_SNIPER, this);
		}
		else if (pOwner->IsPlayerClass(TF_CLASS_SPY))
		{
			vm->SetWeaponModel(MEGACANNON_MODEL_SPY, this);
		}
		else if (pOwner->IsPlayerClass(TF_CLASS_WILDCARD))
		{
			vm->SetWeaponModel(MEGACANNON_MODEL_WILDCARD, this);
		}
		else if (pOwner->IsPlayerClass(TF_CLASS_CIVILIAN))
		{
			vm->SetWeaponModel(MEGACANNON_MODEL_CIVILIAN, this);
		}
		else if (pOwner->IsPlayerClass(TF_CLASS_COMBINE))
		{
			vm->SetWeaponModel(MEGACANNON_MODEL_COMBINE, this);
		}
		else if (pOwner->IsPlayerClass(TF_CLASS_ZOMBIEFAST))
		{
			vm->SetWeaponModel(MEGACANNON_MODEL_ZOMBIEFAST, this);
		}				
	}
	//BaseClass::SetViewModel();
}

//-----------------------------------------------------------------------------
// Purpose: Force the cannon to drop anything it's carrying
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::ForceDrop( void )
{
	CloseElements();
	DetachObject();
	StopEffects();
}


//-----------------------------------------------------------------------------
// Purpose: Drops its held entity if it matches the entity passed in
// Input  : *pTarget - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponPhysCannon::DropIfEntityHeld( CBaseEntity *pTarget )
{
	if ( pTarget == NULL )
		return false;

	CBaseEntity *pHeld = m_grabController.GetAttached();
	
	if ( pHeld == NULL )
		return false;

	if ( pHeld == pTarget )
	{
		ForceDrop();
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::Drop( const Vector &vecVelocity )
{
	ForceDrop();

#ifndef CLIENT_DLL
	UTIL_Remove( this );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponPhysCannon::CanHolster( void ) const
{ 
	//Don't holster this weapon if we're holding onto something
	if ( m_bActive )
		return false;

	return BaseClass::CanHolster();
};

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponPhysCannon::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	ForceDrop();
	DestroyEffects();

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::DryFire( void )
{
	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	WeaponSound( EMPTY );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::PrimaryFireEffect( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	pOwner->ViewPunch( QAngle( -6, SharedRandomInt( "physcannonfire", -2,2 ) ,0 ) );
	pOwner->EmitSound("Weapon_PhysCannon.Launch");

#ifdef GAME_DLL
	color32 white = { 245, 245, 255, 32 };
	color32 redcrit = { 237, 140, 55, 32 };
	color32 blucrit = { 28, 168, 112, 32 };

	if ( (pOwner->m_Shared.IsCritBoosted() || pOwner->m_Shared.IsMiniCritBoosted()) && pOwner->GetTeamNumber() == TF_STORY_TEAM )
	{
		UTIL_ScreenFade( pOwner, redcrit, 0.1f, 0.0f, FFADE_IN );
	}
	else if ( (pOwner->m_Shared.IsCritBoosted() || pOwner->m_Shared.IsMiniCritBoosted()) && pOwner->GetTeamNumber() == TF_COMBINE_TEAM )
	{
		UTIL_ScreenFade( pOwner, blucrit, 0.1f, 0.0f, FFADE_IN );
	}
	else
	{
		UTIL_ScreenFade( pOwner, white, 0.1f, 0.0f, FFADE_IN );
	}

	if ( IsMegaPhysCannon() ) // super charged gravity gun + full critical = ???
	{
		if ( pOwner->m_Shared.IsCritBoosted() )
		{
			UTIL_ScreenShake( GetAbsOrigin(), 20.0f, 200.0, 1.0, 512.0f, SHAKE_START );
			CSoundEnt::InsertSound( SOUND_DANGER, GetAbsOrigin(), 512, 3.0 );

			WeaponSound( BURST );
		}
	}
	else
	{
		if ( pOwner->m_Shared.IsCritBoosted() )
		{
			WeaponSound( BURST );
		}
	}

	pOwner->SetAnimation( PLAYER_ATTACK1 );
	pOwner->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );
#else
	if ( lfe_muzzlelight.GetBool() )
	{
		dlight_t *dl = effects->CL_AllocDlight( LIGHT_INDEX_TE_DYNAMIC + index );
		dl->origin = pOwner->Weapon_ShootPosition();
		if ( IsMegaPhysCannon() )
		{
			dl->color.r = 255;
			dl->color.g = 128;
			dl->color.b = 0;
		}
		else
		{
			dl->color.r = 0;
			dl->color.g = 128;
			dl->color.b = 255;
		}
		dl->die = gpGlobals->curtime + 0.01f;
		dl->radius = 128.f;
		dl->decay = 512.0f;
	}
#endif

}

#define	MAX_KNOCKBACK_FORCE	128

//-----------------------------------------------------------------------------
// Punt non-physics
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::PuntNonVPhysics( CBaseEntity *pEntity, const Vector &forward, trace_t &tr )
{
	if ( m_hLastPuntedObject == pEntity && gpGlobals->curtime < m_flRepuntObjectTime )
		return;

#ifdef GAME_DLL
	CTakeDamageInfo	info;

	CTFPlayer *pOwner = GetTFPlayerOwner();

	info.SetAttacker( pOwner );
	info.SetInflictor( this );
	info.SetDamage( 50.0f );

	if ( pOwner->m_Shared.IsCritBoosted() )
	{
		info.SetDamageType( DMG_CRUSH | DMG_PHYSGUN | DMG_CRITICAL );
	}
	else if ( pOwner->m_Shared.IsMiniCritBoosted() )
	{
		info.SetDamageType( DMG_CRUSH | DMG_PHYSGUN | DMG_MINICRITICAL );
	}
	else
	{
		info.SetDamageType( DMG_CRUSH | DMG_PHYSGUN );
	}

	info.SetDamageForce( forward*100 );	// Scale?
	info.SetDamagePosition( tr.endpos );

	m_hLastPuntedObject = pEntity;
	m_flRepuntObjectTime = gpGlobals->curtime + 0.5f;

	pEntity->DispatchTraceAttack( info, forward, &tr );

	ApplyMultiDamage();

	//Explosion effect
	DoEffect( EFFECT_LAUNCH, &tr.endpos );
#endif
	
	PrimaryFireEffect();
	SendWeaponAnim( ACT_VM_SECONDARYATTACK );

	m_nChangeState = ELEMENT_STATE_CLOSED;
	m_flElementDebounce = gpGlobals->curtime + 0.5f;
	m_flCheckSuppressTime = gpGlobals->curtime + 0.25f;
}


#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// What happens when the physgun picks up something 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::Physgun_OnPhysGunPickup( CBaseEntity *pEntity, CBasePlayer *pOwner, PhysGunPickup_t reason )
{
	// If the target is debris, convert it to non-debris
	if ( pEntity->GetCollisionGroup() == COLLISION_GROUP_DEBRIS )
	{
		// Interactive debris converts back to debris when it comes to rest
		pEntity->SetCollisionGroup( COLLISION_GROUP_INTERACTIVE_DEBRIS );
	}

#ifdef GAME_DLL
	float mass = 0.0f;
	if (pEntity->VPhysicsGetObject())
	{
		mass = pEntity->VPhysicsGetObject()->GetMass();
	}

	if (reason == PUNTED_BY_CANNON)
	{
		//pOwner->RumbleEffect(RUMBLE_357, 0, RUMBLE_FLAGS_NONE);
		RecordThrownObject(pEntity);
	}

	// Warn Alyx if the player is punting a car around.
	if (hl2_episodic.GetBool() && mass > 250.0f)
	{
		CAI_BaseNPC** ppAIs = g_AI_Manager.AccessAIs();
		int nAIs = g_AI_Manager.NumAIs();

		for (int i = 0; i < nAIs; i++)
		{
			if (ppAIs[i]->Classify() == CLASS_PLAYER_ALLY_VITAL)
			{
				ppAIs[i]->DispatchInteraction(g_interactionPlayerPuntedHeavyObject, pEntity, pOwner);
			}
		}
	}
#endif

	Pickup_OnPhysGunPickup( pEntity, pOwner, reason );
}

//-----------------------------------------------------------------------------
// Purpose: Adds the specified object to the list of objects that have been
//			propelled by this physgun, along with a timestamp of when the
//			object was added to the list. This list is checked when a physics
//			object strikes another entity, to resolve whether the player is
//			accountable for the impact.
//
// Input  : pObject - pointer to the object being thrown by the physcannon.
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::RecordThrownObject(CBaseEntity* pObject)
{
	thrown_objects_t thrown;
	thrown.hEntity = pObject;
	thrown.fTimeThrown = gpGlobals->curtime;

	// Get rid of stale and dead objects in the list.
	PurgeThrownObjects();

	// See if this object is already in the list.
	int count = m_ThrownEntities.Count();

	for (int i = 0; i < count; i++)
	{
		if (m_ThrownEntities[i].hEntity == pObject)
		{
			// Just update the time.
			//Msg("++UPDATING: %s (%d)\n", m_ThrownEntities[i].hEntity->GetClassname(), m_ThrownEntities[i].hEntity->entindex() );
			m_ThrownEntities[i] = thrown;
			return;
		}
	}

	//Msg("++ADDING: %s (%d)\n", pObject->GetClassname(), pObject->entindex() );

	m_ThrownEntities.AddToTail(thrown);
}

//-----------------------------------------------------------------------------
// Purpose: Go through the objects in the thrown objects list and discard any
//			objects that have gone 'stale'. (Were thrown several seconds ago), or
//			have been destroyed or removed.
//
//-----------------------------------------------------------------------------
#define PHYSCANNON_THROWN_LIST_TIMEOUT	10.0f
void CWeaponPhysCannon::PurgeThrownObjects()
{
	bool bListChanged;

	// This is bubble-sorty, but the list is also very short.
	do
	{
		bListChanged = false;

		int count = m_ThrownEntities.Count();
		for (int i = 0; i < count; i++)
		{
			bool bRemove = false;

			if (!m_ThrownEntities[i].hEntity.Get())
			{
				bRemove = true;
			}
			else if (gpGlobals->curtime > (m_ThrownEntities[i].fTimeThrown + PHYSCANNON_THROWN_LIST_TIMEOUT))
			{
				bRemove = true;
			}
			else
			{
				IPhysicsObject* pObject = m_ThrownEntities[i].hEntity->VPhysicsGetObject();

				if (pObject && pObject->IsAsleep())
				{
					bRemove = true;
				}
			}

			if (bRemove)
			{
				//Msg("--REMOVING: %s (%d)\n", m_ThrownEntities[i].hEntity->GetClassname(), m_ThrownEntities[i].hEntity->entindex() );
				m_ThrownEntities.Remove(i);
				bListChanged = true;
				break;
			}
		}
	} while (bListChanged);
}

bool CWeaponPhysCannon::IsAccountableForObject(CBaseEntity* pObject)
{
	// Clean out the stale and dead items.
	PurgeThrownObjects();

	// Now if this object is in the list, the player is accountable for it striking something.
	int count = m_ThrownEntities.Count();

	for (int i = 0; i < count; i++)
	{
		if (m_ThrownEntities[i].hEntity == pObject)
		{
			return true;
		}
	}

	return false;
}
#endif

//-----------------------------------------------------------------------------
// Punt vphysics
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::PuntVPhysics( CBaseEntity *pEntity, const Vector &vecForward, trace_t &tr )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();

	if ( m_hLastPuntedObject == pEntity && gpGlobals->curtime < m_flRepuntObjectTime )
		return;

	m_hLastPuntedObject = pEntity;
	m_flRepuntObjectTime = gpGlobals->curtime + 0.5f;

#ifndef CLIENT_DLL
	CTakeDamageInfo	info;

	Vector forward = vecForward;

	info.SetAttacker( pOwner );
	info.SetInflictor( this );
	info.SetDamage( 0.0f );

	if ( pOwner->m_Shared.IsCritBoosted() )
	{
		info.SetDamageType( DMG_PHYSGUN | DMG_CRITICAL );
	}
	else if ( pOwner->m_Shared.IsMiniCritBoosted() )
	{
		info.SetDamageType( DMG_PHYSGUN | DMG_MINICRITICAL );
	}
	else
	{
		info.SetDamageType( DMG_PHYSGUN );
	}

	pEntity->DispatchTraceAttack( info, forward, &tr );
	ApplyMultiDamage();

	if ( Pickup_OnAttemptPhysGunPickup( pEntity, pOwner, PUNTED_BY_CANNON ) )
	{
		IPhysicsObject *pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
		int listCount = pEntity->VPhysicsGetObjectList( pList, ARRAYSIZE(pList) );
		if ( !listCount )
		{
			//FIXME: Do we want to do this if there's no physics object?
			Physgun_OnPhysGunPickup( pEntity, pOwner, PUNTED_BY_CANNON );
			DryFire();
			return;
		}
				
		if( forward.z < 0 )
		{
			//reflect, but flatten the trajectory out a bit so it's easier to hit standing targets
			forward.z *= -0.65f;
		}
				
		// NOTE: Do this first to enable motion (if disabled) - so forces will work
		// Tell the object it's been punted
		Physgun_OnPhysGunPickup( pEntity, pOwner, PUNTED_BY_CANNON );

		// don't push vehicles that are attached to the world via fixed constraints
		// they will just wiggle...
		if ( (pList[0]->GetGameFlags() & FVPHYSICS_CONSTRAINT_STATIC) && pEntity->GetServerVehicle() )
		{
			forward.Init();
		}

		if ( ( !IsMegaPhysCannon() ) && !Pickup_ShouldPuntUseLaunchForces( pEntity, PHYSGUN_FORCE_PUNTED ) )
		{
			int i;

			// limit mass to avoid punting REALLY huge things
			float totalMass = 0;
			for ( i = 0; i < listCount; i++ )
			{
				totalMass += pList[i]->GetMass();
			}
			float maxMass = 250;
			IServerVehicle *pVehicle = pEntity->GetServerVehicle();
			if ( pVehicle )
			{
				maxMass *= 2.5;	// 625 for vehicles
			}
			float mass = MIN(totalMass, maxMass); // max 250kg of additional force

			// Put some spin on the object
			for ( i = 0; i < listCount; i++ )
			{
				const float hitObjectFactor = 0.5f;
				const float otherObjectFactor = 1.0f - hitObjectFactor;
  				// Must be light enough
				float ratio = pList[i]->GetMass() / totalMass;
				if ( pList[i] == pEntity->VPhysicsGetObject() )
				{
					ratio += hitObjectFactor;
					ratio = MIN(ratio,1.0f);
				}
				else
				{
					ratio *= otherObjectFactor;
				}
  				pList[i]->ApplyForceCenter( forward * 15000.0f * ratio );
  				pList[i]->ApplyForceOffset( forward * mass * 600.0f * ratio, tr.endpos );
			}
		}
		else
		{
			ApplyVelocityBasedForce( pEntity, vecForward, tr.endpos, PHYSGUN_FORCE_PUNTED);
		}
	}

#endif
	// Add recoil
	QAngle	recoil = QAngle( random->RandomFloat( 1.0f, 2.0f ), random->RandomFloat( -1.0f, 1.0f ), 0 );
	pOwner->ViewPunch( recoil );

	//Explosion effect
	DoEffect( EFFECT_LAUNCH, &tr.endpos );

	PrimaryFireEffect();
	SendWeaponAnim( ACT_VM_SECONDARYATTACK );

	m_nChangeState = ELEMENT_STATE_CLOSED;
	m_flElementDebounce = gpGlobals->curtime + 0.5f;
	m_flCheckSuppressTime = gpGlobals->curtime + 0.25f;

	// Don't allow the gun to regrab a thrown object!!
	m_flNextSecondaryAttack = m_flNextPrimaryAttack = m_flNextTertiaryAttack = gpGlobals->curtime + 0.5f;
}

//-----------------------------------------------------------------------------
// Purpose:		
//			called from both punt and launch carried code.
//			ASSUMES: that pEntity is a vphysics entity.
// Input  : - 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::ApplyVelocityBasedForce( CBaseEntity* pEntity, const Vector& forward, const Vector& vecHitPos, PhysGunForce_t reason )
{
#ifdef GAME_DLL
	IPhysicsObject *pPhysicsObject = pEntity->VPhysicsGetObject();
	Assert(pPhysicsObject); // Shouldn't ever get here with a non-vphysics object.
	if (!pPhysicsObject)
		return;

	float flForceMax = physcannon_maxforce.GetFloat();
	float flForce = flForceMax;

	float mass = pPhysicsObject->GetMass();
	if (mass > 100)
	{
		mass = MIN(mass, 1000);
		float flForceMin = physcannon_minforce.GetFloat();
		flForce = SimpleSplineRemapVal(mass, 100, 600, flForceMax, flForceMin);
	}

	Vector vVel = forward * flForce;
	// FIXME: Josh needs to put a real value in for PHYSGUN_FORCE_PUNTED
	AngularImpulse aVel = Pickup_PhysGunLaunchAngularImpulse( pEntity, reason  );

	// Affect the object
	CRagdollProp *pRagdoll = dynamic_cast<CRagdollProp*>(pEntity);
	if (pRagdoll == NULL)
	{
		// The jeep being punted needs special force overrides and has to be episodic maps
		if ( reason == PHYSGUN_FORCE_PUNTED && pEntity->GetServerVehicle() && ( TFGameRules()->IsInHL2EP1Map() || TFGameRules()->IsInHL2EP2Map() ) )
		{
			// We want the point to emanate low on the vehicle to move it along the ground, not to twist it
			Vector vecFinalPos = vecHitPos;
			vecFinalPos.z = pEntity->GetAbsOrigin().z;
			pPhysicsObject->ApplyForceOffset(vVel, vecFinalPos);
		}
		else
		{
			pPhysicsObject->AddVelocity(&vVel, &aVel);
		}
	}
	else
	{
		Vector	vTempVel;
		AngularImpulse vTempAVel;

		ragdoll_t *pRagdollPhys = pRagdoll->GetRagdoll();
		for (int j = 0; j < pRagdollPhys->listCount; ++j)
		{
			pRagdollPhys->list[j].pObject->AddVelocity(&vVel, &aVel);
		}
	}

	//pPhysicsObject->AddVelocity( &vVel, &aVel );

#endif

}

#ifdef GAME_DLL 
//-----------------------------------------------------------------------------
// Punt non-physics
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::PuntRagdoll( CBaseEntity *pEntity, const Vector &vecForward, trace_t &tr )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();

	Pickup_OnPhysGunDrop( pEntity, pOwner, LAUNCHED_BY_CANNON );

	CTakeDamageInfo	info;

	Vector forward = vecForward;
	info.SetAttacker( pOwner );
	info.SetInflictor(this);
	info.SetDamage(0.0f);

	if ( pOwner->m_Shared.IsCritBoosted() )
	{
		info.SetDamageType( DMG_PHYSGUN | DMG_CRITICAL );
	}
	else if ( pOwner->m_Shared.IsMiniCritBoosted() )
	{
		info.SetDamageType( DMG_PHYSGUN | DMG_MINICRITICAL );
	}
	else
	{
		info.SetDamageType( DMG_PHYSGUN );
	}

	pEntity->DispatchTraceAttack(info, forward, &tr);
	ApplyMultiDamage();

	if (Pickup_OnAttemptPhysGunPickup(pEntity, pOwner, PUNTED_BY_CANNON))
	{
		Physgun_OnPhysGunPickup(pEntity, pOwner, PUNTED_BY_CANNON);

		if (forward.z < 0)
		{
			//reflect, but flatten the trajectory out a bit so it's easier to hit standing targets
			forward.z *= -0.65f;
		}

		Vector			vVel = forward * 1500;
		AngularImpulse	aVel = Pickup_PhysGunLaunchAngularImpulse(pEntity, PHYSGUN_FORCE_PUNTED);

		CRagdollProp *pRagdoll = dynamic_cast<CRagdollProp*>(pEntity);
		ragdoll_t *pRagdollPhys = pRagdoll->GetRagdoll();

		int j;
		for (j = 0; j < pRagdollPhys->listCount; ++j)
		{
			pRagdollPhys->list[j].pObject->AddVelocity(&vVel, NULL);
		}
	}

	// Add recoil
	QAngle	recoil = QAngle(random->RandomFloat(1.0f, 2.0f), random->RandomFloat(-1.0f, 1.0f), 0);
	pOwner->ViewPunch(recoil);

	//Explosion effect
	DoEffect(EFFECT_LAUNCH, &tr.endpos);

	PrimaryFireEffect();
	SendWeaponAnim(ACT_VM_SECONDARYATTACK);

	m_nChangeState = ELEMENT_STATE_CLOSED;
	m_flElementDebounce = gpGlobals->curtime + 0.5f;
	m_flCheckSuppressTime = gpGlobals->curtime + 0.25f;

	// Don't allow the gun to regrab a thrown object!!
	m_flNextSecondaryAttack = m_flNextPrimaryAttack = m_flNextTertiaryAttack = gpGlobals->curtime + 0.5f;
}
#endif

//-----------------------------------------------------------------------------
// Trace length
//-----------------------------------------------------------------------------
float CWeaponPhysCannon::TraceLength()
{
	CTFPlayer *pOwner = GetTFPlayerOwner();

	if ( IsMegaPhysCannon() )
	{
		return physcannon_mega_tracelength.GetFloat();
	}
	else if ( IsMegaPhysCannon() && pOwner && pOwner->m_Shared.IsCritBoosted() )
	{
		return lfe_physcannon_mega_crit_tracelength.GetFloat();
	}
	else
	{
		return physcannon_tracelength.GetFloat();
	}
}

#ifndef CLIENT_DLL 
//-----------------------------------------------------------------------------
// If there's any special rejection code you need to do per entity then do it here
// This is kinda nasty but I'd hate to move more physcannon related stuff into CBaseEntity
//-----------------------------------------------------------------------------
bool CWeaponPhysCannon::EntityAllowsPunts(CBaseEntity *pEntity)
{
	if (pEntity->HasSpawnFlags(SF_PHYSBOX_NEVER_PUNT))
	{
		CPhysBox *pPhysBox = dynamic_cast<CPhysBox*>(pEntity);

		if (pPhysBox != NULL)
		{
			if (pPhysBox->HasSpawnFlags(SF_PHYSBOX_NEVER_PUNT))
			{
				return false;
			}
		}
	}

	if (pEntity->HasSpawnFlags(SF_WEAPON_NO_PHYSCANNON_PUNT))
	{
		CBaseCombatWeapon *pWeapon = dynamic_cast<CBaseCombatWeapon*>(pEntity);

		if (pWeapon != NULL)
		{
			if (pWeapon->HasSpawnFlags(SF_WEAPON_NO_PHYSCANNON_PUNT))
			{
				return false;
			}
		}
	}

	return true;
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//
// This mode is a toggle. Primary fire one time to pick up a physics object.
// With an object held, click primary fire again to drop object.
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::PrimaryAttack( void )
{
	if ( m_flNextPrimaryAttack > gpGlobals->curtime )
		return;

	if ( !CanAttack() )
		return;

	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	if ( m_bActive )
	{
		// Punch the object being held!!
		Vector forward;
		pOwner->EyeVectors(&forward);

		// If throwing from the opposite side of a portal, reorient the direction
		if (((CTFPlayer*)pOwner)->IsHeldObjectOnOppositeSideOfPortal())
		{
			CProp_Portal* pHeldPortal = ((CTFPlayer*)pOwner)->GetHeldObjectPortal();
			UTIL_Portal_VectorTransform(pHeldPortal->MatrixThisToLinked(), forward, forward);
		}

		// Validate the item is within punt range
		CBaseEntity *pHeld = m_grabController.GetAttached();
		Assert(pHeld != NULL);

		if (pHeld != NULL)
		{
			float heldDist = (pHeld->WorldSpaceCenter() - pOwner->WorldSpaceCenter()).Length();

			if (heldDist > physcannon_tracelength.GetFloat())
			{
				// We can't punt this yet
				DryFire();
				return;
			}
		}

#ifdef GAME_DLL
		pOwner->RemoveInvisibility();
		pOwner->RemoveDisguise();
#endif

		LaunchObject(forward, physcannon_maxforce.GetFloat());

		PrimaryFireEffect();
		SendWeaponAnim(ACT_VM_SECONDARYATTACK);
		return;
	}

	// If not active, just issue a physics punch in the world.
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

	Vector forward;
	pOwner->EyeVectors(&forward);

	// NOTE: Notice we're *not* using the mega tracelength here
	// when you have the mega cannon. Punting has shorter range.
	Vector start, end;
	start = pOwner->Weapon_ShootPosition();
	float flPuntDistance = physcannon_tracelength.GetFloat();
	VectorMA( start, flPuntDistance, forward, end );

#ifdef GAME_DLL
	class CAutoScopeLagComp
	{
	public:
		CAutoScopeLagComp(CBasePlayer* pOwner) : m_pPlayer(pOwner)
		{
			lagcompensation->StartLagCompensation(pOwner, LAG_COMPENSATE_BOUNDS);
		}

		~CAutoScopeLagComp()
		{
			lagcompensation->FinishLagCompensation(m_pPlayer);
		}
	private:
		CBasePlayer* m_pPlayer;
	};

	CAutoScopeLagComp lagScope(pOwner);
#endif

	trace_t tr;
	UTIL_PhyscannonTraceHull(start, end, -Vector(8, 8, 8), Vector(8, 8, 8), pOwner, &tr);
	bool bValid = true;
	CBaseEntity *pEntity = tr.m_pEnt;
	if (tr.fraction == 1 || !tr.m_pEnt || tr.m_pEnt->IsEFlagSet(EFL_NO_PHYSCANNON_INTERACTION))
	{
		bValid = false;
	}
	else if ((pEntity->GetMoveType() != MOVETYPE_VPHYSICS) && (pEntity->m_takedamage == DAMAGE_NO))
	{
		bValid = false;
	}

	// If the entity we've hit is invalid, try a traceline instead
	if (!bValid)
	{
		UTIL_PhyscannonTraceLine( start, end, pOwner, &tr );
		if (tr.fraction == 1 || !tr.m_pEnt || tr.m_pEnt->IsEFlagSet(EFL_NO_PHYSCANNON_INTERACTION))
		{
			// Play dry-fire sequence
			DryFire();
			return;
		}

		pEntity = tr.m_pEnt;
	}

	if ( ToTFPlayer( pOwner )->IsHeldObjectOnOppositeSideOfPortal() )
	{
		CProp_Portal *pPortal = ToTFPlayer( pOwner )->GetHeldObjectPortal();
		UTIL_Portal_VectorTransform( pPortal->MatrixThisToLinked(), forward, forward );
	}

	// See if we hit something
	if ( pEntity->GetMoveType() != MOVETYPE_VPHYSICS )
	{
		if ( pEntity->m_takedamage == DAMAGE_NO )
		{
			DryFire();
			return;
		}
#ifdef GAME_DLL
		if( pOwner->IsPlayer() && ( !IsMegaPhysCannon() ) )
		{
			// Don't let the player zap any NPC's except regular antlions and headcrabs.
			if( pEntity->IsNPC() && pEntity->Classify() != CLASS_HEADCRAB && !FClassnameIs(pEntity, "npc_antlion") )
			{
				DryFire();
				return;
			}
		}

		pOwner->SpeakWeaponFire();
		CTF_GameStats.Event_PlayerFiredWeapon( pOwner, false );

		if ( IsMegaPhysCannon() && !pEntity->InSameTeam( pOwner ) && !pEntity->IsEFlagSet( EFL_NO_MEGAPHYSCANNON_RAGDOLL ) )
		{
			float flDamage = 10000.0f;
			CALL_ATTRIB_HOOK_FLOAT( flDamage, mult_dmg );
			CTakeDamageInfo info( pOwner, pOwner, flDamage, DMG_GENERIC );

			// Necessary to cause it to do the appropriate death cleanup
			CTakeDamageInfo ragdollInfo( pOwner, pOwner, flDamage, DMG_PHYSGUN | DMG_REMOVENORAGDOLL, LFE_DMG_CUSTOM_PHYSCANNON_MEGA );
			if ( pOwner->m_Shared.IsCritBoosted() )
			{
				ragdollInfo.SetDamageType( DMG_PHYSGUN | DMG_REMOVENORAGDOLL | DMG_CRITICAL );
			}
			else if ( pOwner->m_Shared.IsMiniCritBoosted() )
			{
				ragdollInfo.SetDamageType( DMG_PHYSGUN | DMG_REMOVENORAGDOLL | DMG_MINICRITICAL );
			}

			if ( pEntity->IsNPC() && pEntity->MyNPCPointer()->CanBecomeRagdoll() && !pEntity->MyNPCPointer()->IsInvulnerable() && !pEntity->MyNPCPointer()->InCond( TF_COND_PHASE ) )
			{
				CBaseEntity *pRagdoll = CreateServerRagdoll( pEntity->MyNPCPointer(), 0, info, COLLISION_GROUP_INTERACTIVE_DEBRIS, true );
				if ( pRagdoll )
				{
					PhysSetEntityGameFlags( pRagdoll, FVPHYSICS_NO_SELF_COLLISIONS );
					pRagdoll->SetCollisionBounds( pEntity->CollisionProp()->OBBMins(), pEntity->CollisionProp()->OBBMaxs() );
				}

				pEntity->TakeDamage( ragdollInfo );

				PuntRagdoll( pRagdoll, forward, tr );
				return;
			}
			else if ( pEntity->IsPlayer() && !ToTFPlayer( pEntity )->m_Shared.IsInvulnerable() && !ToTFPlayer( pEntity )->m_Shared.InCond( TF_COND_PHASE ) )
			{
				CBaseEntity *pRagdoll = CreateServerRagdoll( ToTFPlayer( pEntity ), 0, info, COLLISION_GROUP_INTERACTIVE_DEBRIS, true );
				if ( pRagdoll )
				{
					PhysSetEntityGameFlags( pRagdoll, FVPHYSICS_NO_SELF_COLLISIONS );
					pRagdoll->SetCollisionBounds( pEntity->CollisionProp()->OBBMins(), pEntity->CollisionProp()->OBBMaxs() );
				}

				pEntity->TakeDamage( ragdollInfo );

				PuntRagdoll( pRagdoll, forward, tr );
				return;
			}
			else if ( pEntity->IsBaseObject() )
			{
				pEntity->TakeDamage( ragdollInfo );
				DoEffect( EFFECT_LAUNCH, &tr.endpos );
				PrimaryFireEffect();
				SendWeaponAnim( ACT_VM_SECONDARYATTACK );
				return;
			}
		}
#endif
		PuntNonVPhysics( pEntity, forward, tr );
	}
	else
	{
#ifndef CLIENT_DLL 
		if ( EntityAllowsPunts( pEntity ) == false )
		{
			DryFire();
			return;
		}

		if ( !IsMegaPhysCannon() )
		{
			if ( pEntity->VPhysicsIsFlesh( ) )
			{
				DryFire();
				return;
			}
			PuntVPhysics( pEntity, forward, tr );
		}
		else
		{
			if ( dynamic_cast<CRagdollProp*>(pEntity) )
			{
				PuntRagdoll( pEntity, forward, tr );
			}
			else
			{
				PuntVPhysics( pEntity, forward, tr );
			}
		}

		PrimaryFireEffect();
#endif
	}
#ifdef GAME_DLL
	pOwner->RemoveInvisibility();
	pOwner->RemoveDisguise();
#endif
}


//-----------------------------------------------------------------------------
// Purpose: Click secondary attack whilst holding an object to hurl it.
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::SecondaryAttack( void )
{
#ifdef GAME_DLL
	if ( m_flNextSecondaryAttack > gpGlobals->curtime )
		return;

	if ( !CanAttack() )
		return;

	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( !pOwner )
		return;

	pOwner->RemoveInvisibility();
	pOwner->RemoveDisguise();

	// See if we should drop a held item
	if ( ( m_bActive ) && ( pOwner->m_afButtonPressed & IN_ATTACK2 ) )
	{
		// Drop the held object
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5;
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5;
		m_flNextTertiaryAttack = gpGlobals->curtime + 0.5;

		DetachObject();

		DoEffect( EFFECT_READY );

		SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	}
	else
	{
		// Otherwise pick it up
		FindObjectResult_t result = FindObject();
		switch ( result )
		{
		case OBJECT_FOUND:
			EmitSound( "Weapon_PhysCannon.Pickup" );
			SendWeaponAnim( ACT_VM_PRIMARYATTACK );
			m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;

			// We found an object. Debounce the button
			m_nAttack2Debounce |= pOwner->m_nButtons;
			break;

		case OBJECT_NOT_FOUND:
			m_flNextSecondaryAttack = gpGlobals->curtime + 0.1f;

			CloseElements();
			break;

		case OBJECT_BEING_DETACHED:
			m_flNextSecondaryAttack = gpGlobals->curtime + 0.01f;
			break;
		}

		DoEffect( EFFECT_HOLDING );
	}
#endif
}	

//-----------------------------------------------------------------------------
// Purpose: F0rtnite Rift with damage lol.
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::TertiaryAttack( void )
{
	if ( !CanAttack() )
		return;

	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner == NULL )
		return;

	if ( pOwner->m_Shared.IsCritBoosted() && ( m_flNextTertiaryAttack < gpGlobals->curtime ) )
	{
		// Drop the held object
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flNextTertiaryAttack = gpGlobals->curtime + lfe_physcannon_mega_crit_tertiary_cooldown.GetFloat();
		StartEffectBarRegen();
#ifdef GAME_DLL
		PrimaryFireEffect();
		SendWeaponAnim( ACT_VM_SECONDARYATTACK );

		TFGameRules()->BroadcastSound( 255, "Weapon_MegaPhysCannon.SpecialAttackCrit" );

		CTakeDamageInfo info( pOwner, pOwner, lfe_physcannon_mega_crit_tertiary_damage.GetFloat(), DMG_CRITICAL | DMG_PHYSGUN | DMG_REMOVENORAGDOLL, LFE_DMG_CUSTOM_PHYSCANNON_MEGA_TERTIARY );
		RadiusDamage( info, pOwner->GetAbsOrigin(), lfe_physcannon_mega_crit_tertiary_radius.GetFloat(), CLASS_NONE, pOwner );
#else
		//DispatchParticleEffect( "physcannon_super_crit_shockwave", pOwner->GetAbsOrigin(), vec3_angle );
		//DispatchParticleEffect( "physcannon_super_crit_attack_glow", pOwner->GetAbsOrigin(), vec3_angle );
		DispatchParticleEffect( "physcannon_super_crit_shockwave", PATTACH_POINT, this, 1 );
		DispatchParticleEffect( "physcannon_super_crit_attack_glow", PATTACH_POINT, this, 1 );
#endif
	}
	else
	{
		WeaponSound( SPECIAL3 );
		return;
	}
}	

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::WeaponIdle( void )
{
	if ( HasWeaponIdleTimeElapsed() )
	{
		if ( m_bActive )
		{
			//Shake when holding an item
			SendWeaponAnim( ACT_VM_RELOAD );
		}
		else
		{
			//Otherwise idle simply
			SendWeaponAnim( ACT_VM_IDLE );
		}
	}
}

#ifndef CLIENT_DLL

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pObject - 
//-----------------------------------------------------------------------------
bool CWeaponPhysCannon::AttachObject( CBaseEntity *pObject, const Vector &vPosition )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();

	if ( m_bActive )
		return false;

	if ( CanPickupObject( pObject ) == false )
		return false;

	m_grabController.SetIgnorePitch( false );
	m_grabController.SetAngleAlignment( 0 );

	bool bKilledByGrab = false;

	if ( IsMegaPhysCannon() && !pObject->InSameTeam( pOwner ) && !pObject->IsEFlagSet( EFL_NO_MEGAPHYSCANNON_RAGDOLL ) )
	{
		float flDamage = 10000.0f;
		CALL_ATTRIB_HOOK_FLOAT( flDamage, mult_dmg );
		CTakeDamageInfo info( pOwner, pOwner, flDamage, DMG_GENERIC );

		// Necessary to cause it to do the appropriate death cleanup
		CTakeDamageInfo ragdollInfo( pOwner, pOwner, 10000.0, DMG_PHYSGUN | DMG_REMOVENORAGDOLL, LFE_DMG_CUSTOM_PHYSCANNON_MEGA );
		if ( pOwner->m_Shared.IsCritBoosted() )
		{
			ragdollInfo.SetDamageType( DMG_PHYSGUN | DMG_REMOVENORAGDOLL | DMG_CRITICAL );
		}
		else if ( pOwner->m_Shared.IsMiniCritBoosted() )
		{
			ragdollInfo.SetDamageType( DMG_PHYSGUN | DMG_REMOVENORAGDOLL | DMG_MINICRITICAL );
		}

		if ( pObject->IsNPC() && pObject->MyNPCPointer()->CanBecomeRagdoll() && pObject->MyNPCPointer()->Classify() != CLASS_PLAYER_ALLY_VITAL && !pObject->MyNPCPointer()->IsPlayerAlly() && !pObject->MyNPCPointer()->IsInvulnerable() && !pObject->MyNPCPointer()->InCond( TF_COND_PHASE ) )
		{
			Assert( pObject->MyNPCPointer()->CanBecomeRagdoll() );

			CBaseEntity *pRagdoll = CreateServerRagdoll( pObject->MyNPCPointer(), 0, info, COLLISION_GROUP_INTERACTIVE_DEBRIS, true );
			if ( pRagdoll )
			{
				PhysSetEntityGameFlags( pRagdoll, FVPHYSICS_NO_SELF_COLLISIONS );
				pRagdoll->SetCollisionBounds( pObject->CollisionProp()->OBBMins(), pObject->CollisionProp()->OBBMaxs() );
			}

			pObject->TakeDamage( ragdollInfo );

			// Now we act on the ragdoll for the remainder of the time
			pObject = pRagdoll;
			bKilledByGrab = true;
		}
		else if ( pObject->IsPlayer() && !ToTFPlayer( pObject )->m_Shared.IsInvulnerable() && !ToTFPlayer( pObject )->m_Shared.InCond( TF_COND_PHASE ) )
		{
			CBaseEntity *pRagdoll = CreateServerRagdoll( ToTFPlayer( pObject ), 0, info, COLLISION_GROUP_INTERACTIVE_DEBRIS, true );
			if ( pRagdoll )
			{
				PhysSetEntityGameFlags( pRagdoll, FVPHYSICS_NO_SELF_COLLISIONS );
				pRagdoll->SetCollisionBounds( pObject->CollisionProp()->OBBMins(), pObject->CollisionProp()->OBBMaxs() );
			}

			pObject->TakeDamage( ragdollInfo );

			// Now we act on the ragdoll for the remainder of the time
			pObject = pRagdoll;
			bKilledByGrab = true;
		}
	}

	IPhysicsObject *pPhysics = pObject->VPhysicsGetObject();

	// Must be valid
	if ( !pPhysics )
		return false;

	m_bActive = true;
	if( pOwner )
	{
		CBreakableProp* pProp = dynamic_cast<CBreakableProp*>(pObject);

		if (pProp && pProp->HasInteraction( PROPINTER_PHYSGUN_CREATE_FLARE ) )
		{
			pOwner->FlashlightTurnOff();
		}

		// NOTE: This can change the mass; so it must be done before max speed setting
		Physgun_OnPhysGunPickup( pObject, pOwner, PICKED_UP_BY_CANNON );
	}

	// NOTE :This must happen after OnPhysGunPickup because that can change the mass

	m_grabController.AttachEntity(pOwner, pObject, pPhysics, IsMegaPhysCannon(), vPosition, (!bKilledByGrab));

	m_hAttachedObject = pObject;
	m_attachedPositionObjectSpace = m_grabController.m_attachedPositionObjectSpace;
	m_attachedAnglesPlayerSpace = m_grabController.m_attachedAnglesPlayerSpace;

	m_bResetOwnerEntity = false;

	if ( m_hAttachedObject->GetOwnerEntity() == NULL )
	{
		m_hAttachedObject->SetOwnerEntity( pOwner );
		m_bResetOwnerEntity = true;
	}

	// Don't drop again for a slight delay, in case they were pulling objects near them
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.4f;

	OpenElements();
	DoEffect( EFFECT_HOLDING );

	if ( GetMotorSound() )
	{
		(CSoundEnvelopeController::GetController()).Play( GetMotorSound(), 0.0f, 50 );
		(CSoundEnvelopeController::GetController()).SoundChangePitch( GetMotorSound(), Lerp(GetLoadPercentage(), 90.f, 115.f), 0.5f ); //100
		(CSoundEnvelopeController::GetController()).SoundChangeVolume( GetMotorSound(), 0.8f, 0.5f );
	}



	return true;
}

CWeaponPhysCannon::FindObjectResult_t CWeaponPhysCannon::FindObject( void )
{
	CTFPlayer *pPlayer = ToTFPlayer( GetOwner() );

	Assert( pPlayer );
	if ( !pPlayer )
		return OBJECT_NOT_FOUND;

	Vector forward;
	pPlayer->EyeVectors( &forward );

	// Setup our positions
	Vector	start = pPlayer->Weapon_ShootPosition();
	float	testLength = TraceLength() * 4.0f;
	Vector	end = start + forward * testLength;

	// Try to find an object by looking straight ahead
	trace_t tr;
	UTIL_PhyscannonTraceLine( start, end, pPlayer, &tr );

	// Try again with a hull trace
	if ( ( tr.fraction == 1.0 ) || ( tr.m_pEnt == NULL ) || ( tr.m_pEnt->IsWorld() ) )
	{
		UTIL_PhyscannonTraceHull( start, end, -Vector(4,4,4), Vector(4,4,4), pPlayer, &tr );
	}

	CBaseEntity *pEntity = tr.m_pEnt ? tr.m_pEnt->GetRootMoveParent() : NULL;
	bool	bAttach = false;
	bool	bPull = false;

	// If we hit something, pick it up or pull it
	if ( ( tr.fraction != 1.0f ) && ( tr.m_pEnt ) && ( tr.m_pEnt->IsWorld() == false ) )
	{
		// Attempt to attach if within range
		if ( tr.fraction <= 0.25f )
		{
			bAttach = true;
		}
		else if ( tr.fraction > 0.25f )
		{
			bPull = true;
		}
	}

	bool bIsCritboosted = ( pPlayer->m_Shared.IsCritBoosted() );
	// Find anything within a general cone in front
	CBaseEntity *pConeEntity = NULL;
	if ( !IsMegaPhysCannon() )
	{
		if (!bAttach && !bPull)
		{
			pConeEntity = FindObjectInCone(start, forward, physcannon_cone.GetFloat());
		}
	}
	else
	{
		pConeEntity = MegaPhysCannonFindObjectInCone(start, forward,
			physcannon_cone.GetFloat(), physcannon_ball_cone.GetFloat(), bAttach || bPull);
	}
		
	if ( pConeEntity )
	{
		pEntity = pConeEntity;

		// If the object is near, grab it. Else, pull it a bit.
		if ( pEntity->WorldSpaceCenter().DistToSqr( start ) <= (testLength * testLength) )
		{
			bAttach = true;
		}
		else
		{
			bPull = true;
		}
	}

	//SecobMod__Information: Replaced with SP gravity gun code which allows for magnusson devices on the jalopy and zombine grenade pickups.
	if ( CanPickupObject( pEntity ) == false )
	{
		CBaseEntity *pNewObject = Pickup_OnFailedPhysGunPickup(pEntity, start);

		if (pNewObject && CanPickupObject(pNewObject))
		{
			pEntity = pNewObject;
		}
		else
		{
			// Make a noise to signify we can't pick this up
			if (!m_flLastDenySoundPlayed)
			{
				m_flLastDenySoundPlayed = true;
				EmitSound( "Weapon_PhysCannon.TooHeavy" );
				//WeaponSound(SPECIAL3);
			}

			return OBJECT_NOT_FOUND;
		}
	}

	// Check to see if the object is constrained + needs to be ripped off...
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( !Pickup_OnAttemptPhysGunPickup( pEntity, pOwner, PICKED_UP_BY_CANNON ) )
		return OBJECT_BEING_DETACHED;

	if ( bAttach )
	{
		pPlayer->RemoveInvisibility();
		pPlayer->RemoveDisguise();
		
		return AttachObject( pEntity, tr.endpos ) ? OBJECT_FOUND : OBJECT_NOT_FOUND;
	}

	if ( !bPull )
		return OBJECT_NOT_FOUND;

	// FIXME: This needs to be run through the CanPickupObject logic
	IPhysicsObject *pObj = pEntity->VPhysicsGetObject();
	if ( !pObj )
		return OBJECT_NOT_FOUND;

	// If we're too far, simply start to pull the object towards us
	Vector	pullDir = start - pEntity->WorldSpaceCenter();
	VectorNormalize( pullDir );

	if ( !IsMegaPhysCannon() )
	{
		pullDir *= bIsCritboosted ? lfe_physcannon_crit_pullforce.GetFloat() : physcannon_pullforce.GetFloat();
	}
	else
	{
		pullDir *= bIsCritboosted ? lfe_physcannon_mega_crit_pullforce.GetFloat() : physcannon_mega_pullforce.GetFloat();
	}
	
	float mass = PhysGetEntityMass( pEntity );
	if ( mass < 50.0f )
	{
		pullDir *= (mass + 0.5) * (1/50.0f);
	}

	// Nudge it towards us
	pObj->ApplyForceCenter( pullDir );
	return OBJECT_NOT_FOUND;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CBaseEntity *CWeaponPhysCannon::MegaPhysCannonFindObjectInCone(const Vector &vecOrigin,
	const Vector &vecDir, float flCone, float flCombineBallCone, bool bOnlyCombineBalls)
{
	// Find the nearest physics-based item in a cone in front of me.
	CBaseEntity *list[1024];
	float flMaxDist = TraceLength() + 1.0;
	float flNearestDist = flMaxDist;
	bool bNearestIsCombineBall = bOnlyCombineBalls ? true : false;
	Vector mins = vecOrigin - Vector(flNearestDist, flNearestDist, flNearestDist);
	Vector maxs = vecOrigin + Vector(flNearestDist, flNearestDist, flNearestDist);

	CBaseEntity *pNearest = NULL;

	int count = UTIL_EntitiesInBox(list, 1024, mins, maxs, 0);
	for (int i = 0; i < count; i++)
	{
		if (!list[i]->VPhysicsGetObject())
			continue;

		bool bIsCombineBall = FClassnameIs(list[i], "prop_combine_ball");
		if (!bIsCombineBall && bNearestIsCombineBall)
			continue;

		// Closer than other objects
		Vector los;
		VectorSubtract(list[i]->WorldSpaceCenter(), vecOrigin, los);
		float flDist = VectorNormalize(los);

		if (!bIsCombineBall || bNearestIsCombineBall)
		{
			// Closer than other objects
			if (flDist >= flNearestDist)
				continue;

			// Cull to the cone
			if (DotProduct(los, vecDir) <= flCone)
				continue;
		}
		else
		{
			// Close enough?
			if (flDist >= flMaxDist)
				continue;

			// Cull to the cone
			if (DotProduct(los, vecDir) <= flCone)
				continue;

			// NOW: If it's either closer than nearest dist or within the ball cone, use it!
			if ((flDist > flNearestDist) && (DotProduct(los, vecDir) <= flCombineBallCone))
				continue;
		}

		// Make sure it isn't occluded!
		trace_t tr;
		UTIL_PhyscannonTraceLine( vecOrigin, list[i]->WorldSpaceCenter(), GetOwner(), &tr );
		if (tr.m_pEnt == list[i])
		{
			flNearestDist = flDist;
			pNearest = list[i];
			bNearestIsCombineBall = bIsCombineBall;
		}
	}

	return pNearest;
}

//-----------------------------------------------------------------------------
CBaseEntity *CWeaponPhysCannon::FindObjectInCone( const Vector &vecOrigin, const Vector &vecDir, float flCone )
{
	// Find the nearest physics-based item in a cone in front of me.
	CBaseEntity *list[256];
	float flNearestDist = TraceLength() + 1.0;
	Vector mins = vecOrigin - Vector( flNearestDist, flNearestDist, flNearestDist );
	Vector maxs = vecOrigin + Vector( flNearestDist, flNearestDist, flNearestDist );

	CBaseEntity *pNearest = NULL;

	int count = UTIL_EntitiesInBox( list, 256, mins, maxs, 0 );
	for( int i = 0 ; i < count ; i++ )
	{
		if ( !list[ i ]->VPhysicsGetObject() )
			continue;

		// Closer than other objects
		Vector los = ( list[ i ]->WorldSpaceCenter() - vecOrigin );
		float flDist = VectorNormalize( los );
		if( flDist >= flNearestDist )
			continue;

		// Cull to the cone
		if ( DotProduct( los, vecDir ) <= flCone )
			continue;

		// Make sure it isn't occluded!
		trace_t tr;
		UTIL_PhyscannonTraceLine( vecOrigin, list[ i ]->WorldSpaceCenter(), GetOwner(), &tr );
		if( tr.m_pEnt == list[ i ] )
		{
			flNearestDist = flDist;
			pNearest = list[ i ];
		}
	}

	return pNearest;
}

#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CGrabController::UpdateObject( CBasePlayer *pPlayer, float flError )
{
	CBaseEntity* pPenetratedEntity = m_PenetratedEntity.Get();
	if ( pPenetratedEntity )
	{
		//FindClosestPassableSpace( pPenetratedEntity, Vector( 0.0f, 0.0f, 1.0f ) );
		IPhysicsObject* pPhysObject = pPenetratedEntity->VPhysicsGetObject();
		if (pPhysObject)
			pPhysObject->Wake();

		m_PenetratedEntity = NULL; //assume we won
	}

	CBaseEntity *pEntity = GetAttached();
	if ( !pEntity )
		return false;

	if ( ComputeError() > flError )
		return false;

	if ( pPlayer->GetGroundEntity() == pEntity )
		return false;

	if ( !pEntity->VPhysicsGetObject() )
		return false;    

	//Adrian: Oops, our object became motion disabled, let go!
	IPhysicsObject *pPhys = pEntity->VPhysicsGetObject();
	if ( !pPhys || pPhys->IsMoveable() == false )
		return false;

	if ( m_frameCount == gpGlobals->framecount )
		return true;

	m_frameCount = gpGlobals->framecount;
	Vector forward, right, up;
	QAngle playerAngles = pPlayer->EyeAngles();

	float pitch = AngleDistance(playerAngles.x,0);
	playerAngles.x = clamp( pitch, -75, 75 );
	AngleVectors( playerAngles, &forward, &right, &up );

	CTFPlayer* pPortalPlayer = ToTFPlayer( pPlayer );

	int iType = 0;
	CTFWeaponBase *pCannon = dynamic_cast<CTFWeaponBase*>( pPortalPlayer->Weapon_OwnsThisID( TF_WEAPON_PHYSCANNON ) );
	CALL_ATTRIB_HOOK_INT_ON_OTHER( pCannon, iType, set_weapon_mode );
	if ( TFGameRules()->MegaPhyscannonActive() || iType == 1 )
	{
		Vector los = ( pEntity->WorldSpaceCenter() - pPlayer->Weapon_ShootPosition() );
		VectorNormalize(los);

		float flDot = DotProduct(los, forward);

		//Let go of the item if we turn around too fast.
		if (flDot <= 0.35f)
			return false;
	}

	Vector start = pPlayer->Weapon_ShootPosition();

	// If the player is upside down then we need to hold the box closer to their feet.
	if (up.z < 0.0f)
		start += pPlayer->GetViewOffset() * up.z;
	if (right.z < 0.0f)
		start += pPlayer->GetViewOffset() * right.z;

	// Find out if it's being held across a portal
	bool bLookingAtHeldPortal = true;
	CProp_Portal* pPortal = pPortalPlayer->GetHeldObjectPortal();
	if ( !pPortal )
	{
		// If the portal is invalid make sure we don't try to hold it across the portal
		pPortalPlayer->SetHeldObjectOnOppositeSideOfPortal( false );
	}

	if ( pPortalPlayer->IsHeldObjectOnOppositeSideOfPortal() )
	{
		Ray_t rayPortalTest;
		rayPortalTest.Init( start, start + forward * 1024.0f );

		// Check if we're looking at the portal we're holding through
		if ( pPortal )
		{
			if ( UTIL_IntersectRayWithPortal( rayPortalTest, pPortal ) < 0.0f )
			{
				bLookingAtHeldPortal = false;
			}
		}
		else // If our end point hasn't gone into the portal yet we at least need to know what portal is in front of us
		{
			int iPortalCount = CProp_Portal_Shared::AllPortals.Count();
			if ( iPortalCount != 0 )
			{
				CProp_Portal** pPortals = CProp_Portal_Shared::AllPortals.Base();
				float fMinDist = 2.0f;
				for (int i = 0; i != iPortalCount; ++i)
				{
					CProp_Portal* pTempPortal = pPortals[i];
					if ( pTempPortal->m_bActivated && ( pTempPortal->m_hLinkedPortal.Get() != NULL ) )
					{
						float fDist = UTIL_IntersectRayWithPortal( rayPortalTest, pTempPortal );
						if ( (fDist >= 0.0f) && (fDist < fMinDist) )
						{
							fMinDist = fDist;
							pPortal = pTempPortal;
						}
					}
				}
			}
		}
	}
	else
	{
		pPortal = NULL;
	}

	QAngle qEntityAngles = pEntity->GetAbsAngles();

	if (pPortal)
	{
		// If the portal isn't linked we need to drop the object
		if (!pPortal->m_hLinkedPortal.Get())
		{
#ifdef GAME_DLL
			pPlayer->ForceDropOfCarriedPhysObjects();
#endif // !CLIENT_DLL

			return false;
		}

		UTIL_Portal_AngleTransform(pPortal->m_hLinkedPortal->MatrixThisToLinked(), qEntityAngles, qEntityAngles);
	}

	// Now clamp a sphere of object radius at end to the player's bbox
	Vector radial = physcollision->CollideGetExtent( pPhys->GetCollide(), vec3_origin, pEntity->GetAbsAngles(), -forward );
	Vector player2d = pPlayer->CollisionProp()->OBBMaxs();
	float playerRadius = player2d.Length2D();
	float flDot = DotProduct( forward, radial );

	float radius = playerRadius + fabs( flDot );

	float distance = 24 + ( radius * 2.0f );

	Vector end = start + ( forward * distance );

	trace_t	tr;
	CTraceFilterSkipTwoEntities traceFilter( pPlayer, pEntity, COLLISION_GROUP_NONE );
	Ray_t ray;
	ray.Init( start, end );
	//enginetrace->TraceRay( ray, MASK_SOLID_BRUSHONLY, &traceFilter, &tr );
	UTIL_Portal_TraceRay (ray, MASK_SOLID_BRUSHONLY, &traceFilter, &tr );

	if ( tr.fraction < 0.5 )
	{
		end = start + forward * (radius*0.5f);
	}
	else if ( tr.fraction <= 1.0f )
	{
		end = start + forward * ( distance - radius );
	}

	Vector playerMins, playerMaxs, nearest;
	pPlayer->CollisionProp()->WorldSpaceAABB( &playerMins, &playerMaxs );
	Vector playerLine = pPlayer->CollisionProp()->WorldSpaceCenter();
	CalcClosestPointOnLine( end, playerLine+Vector(0,0,playerMins.z), playerLine+Vector(0,0,playerMaxs.z), nearest, NULL );

	Vector delta = end - nearest;
	float len = VectorNormalize(delta);
	if ( len < radius )
	{
		end = nearest + radius * delta;
	}

	QAngle angles = TransformAnglesFromPlayerSpace( m_attachedAnglesPlayerSpace, pPlayer );

	//Show overlays of radius
	if ( g_debug_physcannon.GetBool() )
	{

#ifdef CLIENT_DLL

		debugoverlay->AddBoxOverlay( end, -Vector( 2,2,2 ), Vector(2,2,2), angles, 0, 255, 255, true, 0 );

		debugoverlay->AddBoxOverlay( GetAttached()->WorldSpaceCenter(), 
							-Vector( radius, radius, radius), 
							Vector( radius, radius, radius ),
							angles,
							255, 255, 0,
							true,
							0.0f );

#else

		NDebugOverlay::Box( end, -Vector( 2,2,2 ), Vector(2,2,2), 0, 255, 0, true, 0 );

		NDebugOverlay::Box( GetAttached()->WorldSpaceCenter(), 
							-Vector( radius+5, radius+5, radius+5), 
							Vector( radius+5, radius+5, radius+5 ),
							255, 0, 0,
							true,
							0.0f );
#endif
	}
	
#ifdef GAME_DLL
	// If it has a preferred orientation, update to ensure we're still oriented correctly.
	Pickup_GetPreferredCarryAngles( pEntity, pPlayer, pPlayer->EyeToWorldTransform(), angles );

	// We may be holding a prop that has preferred carry angles
	if ( m_bHasPreferredCarryAngles )
	{
		matrix3x4_t tmp;
		ComputePlayerMatrix( pPlayer, tmp );
		angles = TransformAnglesToWorldSpace( m_vecPreferredCarryAngles, tmp );
	}

#endif

	matrix3x4_t attachedToWorld;
	Vector offset;
	AngleMatrix( angles, attachedToWorld );
	VectorRotate( m_attachedPositionObjectSpace, attachedToWorld, offset );

	// Translate hold position and angles across portal
	if ( pPortalPlayer->IsHeldObjectOnOppositeSideOfPortal() )
	{
		CProp_Portal* pPortalLinked = pPortal->m_hLinkedPortal;
		if (pPortal && pPortal->m_bActivated && pPortalLinked != NULL)
		{
			Vector vTeleportedPosition;
			QAngle qTeleportedAngles;

			if (!bLookingAtHeldPortal && (start - pPortal->GetAbsOrigin()).Length() > distance - radius)
			{
				// Pull the object through the portal
				Vector vPortalLinkedForward;
				pPortalLinked->GetVectors(&vPortalLinkedForward, NULL, NULL);
				vTeleportedPosition = pPortalLinked->GetAbsOrigin() - vPortalLinkedForward * (1.0f + offset.Length());
				qTeleportedAngles = pPortalLinked->GetAbsAngles();
			}
			else
			{
				// Translate hold position and angles across the portal
				VMatrix matThisToLinked = pPortal->MatrixThisToLinked();
				UTIL_Portal_PointTransform(matThisToLinked, end - offset, vTeleportedPosition);
				UTIL_Portal_AngleTransform(matThisToLinked, angles, qTeleportedAngles);
			}

			SetTargetPosition( vTeleportedPosition, qTeleportedAngles );
#ifdef GAME_DLL
			pPortalPlayer->SetHeldObjectPortal(pPortal);
#endif

		}
#ifdef GAME_DLL
		else
		{
			pPlayer->ForceDropOfCarriedPhysObjects();
		}
#endif

	}
	else
	{
		SetTargetPosition( end - offset, angles );
#ifdef GAME_DLL
		pPortalPlayer->SetHeldObjectPortal( NULL );
#endif

	}

	return true;
}

void CWeaponPhysCannon::UpdateObject( void )
{
	CTFPlayer *pPlayer = ToTFPlayer( GetOwner() );
	Assert( pPlayer );

	float flError = IsMegaPhysCannon() ? 18 : 12;

	if ( !m_grabController.UpdateObject( pPlayer, flError ) )
	{
#ifdef GAME_DLL
		pPlayer->SetHeldObjectOnOppositeSideOfPortal( false );
#endif
		DetachObject();
		return;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::DetachObject( bool playSound, bool wasLaunched )
{
#ifdef GAME_DLL
	if ( m_bActive == false )
		return;

	CTFPlayer *pOwner = GetTFPlayerOwner();
	if( pOwner != NULL )
	{
		CTFPlayer* pPortalPlayer = ToTFPlayer(pOwner);
		pPortalPlayer->SetHeldObjectOnOppositeSideOfPortal(false);
	}

	CBaseEntity *pObject = m_grabController.GetAttached();

	m_grabController.DetachEntity( wasLaunched );

	if ( pObject != NULL )
	{
		Pickup_OnPhysGunDrop( pObject, pOwner, wasLaunched ? LAUNCHED_BY_CANNON : DROPPED_BY_CANNON );
	}

	// Stop our looping sound
	if ( GetMotorSound() )
	{
		(CSoundEnvelopeController::GetController()).SoundChangeVolume( GetMotorSound(), 0.0f, 1.0f );
		(CSoundEnvelopeController::GetController()).SoundChangePitch( GetMotorSound(), 40, 1.0f );
	}

	if ( pObject && m_bResetOwnerEntity == true )
	{
		pObject->SetOwnerEntity( NULL );
	}

	m_bActive = false;
	m_hAttachedObject = NULL;

	
	if ( playSound )
	{
		//Play the detach sound
		//WeaponSound( MELEE_MISS );
		EmitSound( "Weapon_PhysCannon.Drop" );
	}

	RecordThrownObject(pObject);
#else

	m_grabController.DetachEntity( wasLaunched );

	if ( m_hAttachedObject )
	{
		m_hAttachedObject->VPhysicsDestroyObject();
	}
#endif
}


#ifdef CLIENT_DLL
void CWeaponPhysCannon::ManagePredictedObject( void )
{
	CBaseEntity *pAttachedObject = m_hAttachedObject.Get();

	if ( m_hAttachedObject )
	{
		// NOTE :This must happen after OnPhysGunPickup because that can change the mass
		if ( pAttachedObject != GetGrabController().GetAttached() )
		{
			IPhysicsObject *pPhysics = pAttachedObject->VPhysicsGetObject();

			if ( pPhysics == NULL )
			{
				solid_t tmpSolid;
				PhysModelParseSolid( tmpSolid, m_hAttachedObject, pAttachedObject->GetModelIndex() );

				pAttachedObject->VPhysicsInitNormal( SOLID_VPHYSICS, 0, false, &tmpSolid );
			}

			pPhysics = pAttachedObject->VPhysicsGetObject();

			if ( pPhysics )
			{
				m_grabController.SetIgnorePitch( false );
				m_grabController.SetAngleAlignment( 0 );

				GetGrabController().AttachEntity( ToBasePlayer( GetOwner() ), pAttachedObject, pPhysics, false, vec3_origin, false );
				GetGrabController().m_attachedPositionObjectSpace = m_attachedPositionObjectSpace;
				GetGrabController().m_attachedAnglesPlayerSpace = m_attachedAnglesPlayerSpace;
			}
		}
	}
	else
	{
		if ( m_hOldAttachedObject && m_hOldAttachedObject->VPhysicsGetObject() )
		{
			GetGrabController().DetachEntity( false );

			m_hOldAttachedObject->VPhysicsDestroyObject();
		}
	}

	m_hOldAttachedObject = m_hAttachedObject;
}

#endif

#ifdef CLIENT_DLL

//-----------------------------------------------------------------------------
// Purpose: Update the pose parameter for the gun
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::UpdateElementPosition( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	float flElementPosition = m_ElementParameter.Interp( gpGlobals->curtime );

	if ( ShouldDrawUsingViewModel() )
	{
		if ( pOwner != NULL )	
		{
			CBaseViewModel *vm = pOwner->GetViewModel();
			
			if ( vm != NULL )
			{
				vm->SetPoseParameter( "active", flElementPosition );
			}
		}
	}
	else
	{
		SetPoseParameter( "active", flElementPosition );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Think function for the client
//-----------------------------------------------------------------------------

void CWeaponPhysCannon::ClientThink( void )
{
	// Update our elements visually
	UpdateElementPosition();

	// Update our effects
	DoEffectIdle();
}

#endif // CLIENT_DLL

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::ItemPreFrame()
{
	BaseClass::ItemPreFrame();

#ifdef CLIENT_DLL
	ManagePredictedObject();
#endif

	// Update the object if the weapon is switched on.
	if( m_bActive )
	{
		UpdateObject();
	}

#ifdef GAME_DLL
	if (gpGlobals->curtime >= m_flTimeNextObjectPurge)
	{
		PurgeThrownObjects();
		m_flTimeNextObjectPurge = gpGlobals->curtime + 0.5f;
	}
#endif
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::CheckForTarget( void )
{
#ifndef CLIENT_DLL
	//See if we're suppressing this
	if ( m_flCheckSuppressTime > gpGlobals->curtime )
		return;

	// holstered
	if ( IsEffectActive( EF_NODRAW ) )
		return;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	if ( m_bActive )
		return;

	Vector	aimDir;
	pOwner->EyeVectors( &aimDir );

	Vector	startPos	= pOwner->Weapon_ShootPosition();
	Vector	endPos;
	VectorMA( startPos, TraceLength(), aimDir, endPos );

	trace_t	tr;
	UTIL_PhyscannonTraceHull( startPos, endPos, -Vector(4,4,4), Vector(4,4,4), pOwner, &tr );

	if ( ( tr.fraction != 1.0f ) && ( tr.m_pEnt != NULL ) )
	{
		// FIXME: Try just having the elements always open when pointed at a physics object
		if ( CanPickupObject( tr.m_pEnt ) || Pickup_ForcePhysGunOpen( tr.m_pEnt, pOwner ) )
		// if ( ( tr.m_pEnt->VPhysicsGetObject() != NULL ) && ( tr.m_pEnt->GetMoveType() == MOVETYPE_VPHYSICS ) )
		{
			m_nChangeState = ELEMENT_STATE_NONE;
			OpenElements();
			return;
		}
	}

	// Close the elements after a delay to prevent overact state switching
	if ( ( m_flElementDebounce < gpGlobals->curtime ) && ( m_nChangeState == ELEMENT_STATE_NONE ) )
	{
		m_nChangeState = ELEMENT_STATE_CLOSED;
		m_flElementDebounce = gpGlobals->curtime + 0.5f;
	}
#endif
}

//-----------------------------------------------------------------------------
// Begin upgrading!
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::BeginUpgrade()
{
	if ( IsMegaPhysCannon() )
		Msg( "Is mega! \n" );
	return;

	if ( m_bIsCurrentlyUpgrading )
		Msg( "Is currently upgrading! \n" );
	return;

	SetSequence(SelectWeightedSequence(ACT_PHYSCANNON_UPGRADE));
	ResetSequenceInfo();

	m_bIsCurrentlyUpgrading = true;

	SetContextThink(&CWeaponPhysCannon::WaitForUpgradeThink, gpGlobals->curtime + 6.0f, s_pWaitForUpgradeContext);

	EmitSound( "WeaponDissolve.Charge" );

	// Bloat our bounds
	CollisionProp()->UseTriggerBounds(true, 32.0f);

	// Turn on the new skin
	m_nSkin = MEGACANNON_SKIN;
}

//-----------------------------------------------------------------------------
// Wait until we're done upgrading
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::WaitForUpgradeThink()
{
	Assert(m_bIsCurrentlyUpgrading);

	StudioFrameAdvance();
	if (!IsActivityFinished())
	{
		SetContextThink(&CWeaponPhysCannon::WaitForUpgradeThink, gpGlobals->curtime + 0.1f, s_pWaitForUpgradeContext);
		return;
	}
#ifdef GAME_DLL
	if (!GlobalEntity_IsInTable("super_phys_gun"))
	{
		GlobalEntity_Add(MAKE_STRING("super_phys_gun"), gpGlobals->mapname, GLOBAL_ON);
	}
	else
	{
		GlobalEntity_SetState(MAKE_STRING("super_phys_gun"), GLOBAL_ON);
	}
#endif
	m_bIsCurrentlyUpgrading = false;

	// This is necessary to get the effects to look different
	DestroyEffects();

	// HACK: Hacky notification back to the level that we've finish upgrading
#ifdef GAME_DLL
	CBaseEntity *pEnt = gEntList.FindEntityByName(NULL, "script_physcannon_upgrade");
	if (pEnt)
	{
		variant_t emptyVariant;
		pEnt->AcceptInput("Trigger", this, this, emptyVariant, 0);
	}
#endif
	StopSound("WeaponDissolve.Charge");

	// Re-enable weapon pickup
	//AddSolidFlags(FSOLID_TRIGGER);
#ifdef GAME_DLL
	for ( int iTeam = FIRST_GAME_TEAM; iTeam < TFTeamMgr()->GetTeamCount(); ++iTeam )
	{
		CTFTeam *pTeam = GetGlobalTFTeam( iTeam );
		if ( pTeam )
		{
			if ( pTeam->HasWeapon( 9000 ) )
				pTeam->RemoveWeapon( 9000 );

			pTeam->AddWeapon( 9001 );
		}
	}
#endif
	SetContextThink(NULL, gpGlobals->curtime, s_pWaitForUpgradeContext);
}

//-----------------------------------------------------------------------------
// Purpose: Idle effect (pulsing)
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::DoEffectIdle(void)
{
#ifdef CLIENT_DLL
	StartEffects();

	const float flScaleFactor = SPRITE_SCALE;

	// Turn on the glow sprites
	for ( int i = PHYSCANNON_GLOW1; i < (PHYSCANNON_GLOW1+NUM_GLOW_SPRITES); i++ )
	{
		if ( IsMegaPhysCannon() )
		{
			m_Parameters[i].GetAlpha().SetAbsolute( random->RandomInt( 32, 48 ) );
			m_Parameters[i].GetScale().SetAbsolute( random->RandomFloat( 0.15, 0.2 ) * flScaleFactor );
		}
		else
		{
			m_Parameters[i].GetScale().SetAbsolute( random->RandomFloat( 0.075f, 0.05f ) * flScaleFactor );
			m_Parameters[i].GetAlpha().SetAbsolute( random->RandomInt( 24, 32 ) );
		}
	}

	// Turn on the glow sprites
	for ( int i = PHYSCANNON_ENDCAP1; i < (PHYSCANNON_ENDCAP1+NUM_ENDCAP_SPRITES); i++ )
	{
		m_Parameters[i].GetScale().SetAbsolute( random->RandomFloat( 3, 5 ) );
		m_Parameters[i].GetAlpha().SetAbsolute( random->RandomInt( 200, 255 ) );
	}
	if ( m_EffectState != EFFECT_HOLDING )
	{
		// Turn beams off
		m_Beams[0].SetVisible( false );
		m_Beams[1].SetVisible( false );
		m_Beams[2].SetVisible( false );
	}
#endif
}	

	// Only do these effects on the mega-cannon
	/*if ( IsMegaPhysCannon() )
	{
		// Randomly arc between the elements and core
		if ( m_EffectState != EFFECT_HOLDING && random->RandomInt(0, 100) == 0 && !engine->IsPaused() )
		{
			// Create our beams
			CBaseEntity* pBeamEnt = nullptr;
			if ( ShouldDrawUsingViewModel() )
			{
				CBasePlayer* pOwner = ToBasePlayer( GetOwner() );
				pBeamEnt = pOwner->GetViewModel();
			}
			else
			{
				pBeamEnt = this;
			}

			if ( pBeamEnt )
			{
					CBeam* pBeam = CBeam::BeamCreate( ShouldDrawUsingViewModel() ? MEGACANNON_BEAM_SPRITE_NOZ : MEGACANNON_BEAM_SPRITE, 1 );
					pBeam->EntsInit( pBeamEnt, pBeamEnt );

					int	startAttachment;
					int	sprite;

					if (random->RandomInt(0, 1))
					{
						startAttachment = LookupAttachment( "fork1t" );
						sprite = PHYSCANNON_ENDCAP1;
					}
					else
					{
						startAttachment = LookupAttachment( "fork2t" );
						sprite = PHYSCANNON_ENDCAP2;
					}

					int endAttachment = 1;

					pBeam->SetStartAttachment(startAttachment);
					pBeam->SetEndAttachment(endAttachment);
					pBeam->SetNoise(random->RandomFloat( 8.0f, 16.0f ));
					pBeam->SetColor( 255, 255, 255 );
					pBeam->SetScrollRate( 25 );
					pBeam->SetBrightness( 128 );
					pBeam->SetWidth( 1 );
					pBeam->SetEndWidth( random->RandomFloat(2, 8) );

					float lifetime = random->RandomFloat( 0.2f, 0.4f );

					pBeam->LiveForTime( lifetime );

					{
						// Turn on the sprite for awhile
						m_Parameters[sprite].ForceVisibleUntil( gpGlobals->curtime + lifetime );
						//m_flEndSpritesOverride[sprite] = gpGlobals->curtime + lifetime;
						EmitSound( "Weapon_MegaPhysCannon.ChargeZap" );
					}
			}
		}

		//if ( m_hCenterSprite != NULL )
		{
			if ( m_EffectState == EFFECT_HOLDING )
			{
				m_Parameters[PHYSCANNON_CORE].GetAlpha().SetAbsolute( random->RandomInt( 32, 64 ));
				m_Parameters[PHYSCANNON_CORE].GetScale().SetAbsolute( random->RandomFloat( 0.2, 0.25 ) * flScaleFactor );
			}
			else
			{
				m_Parameters[PHYSCANNON_CORE].GetAlpha().SetAbsolute( random->RandomInt( 32, 64 ));
				m_Parameters[PHYSCANNON_CORE].GetScale().SetAbsolute( random->RandomFloat( 0.125, 0.15 ) * flScaleFactor );
			}
		}

		//if (m_hBlastSprite != NULL)
		{
			if (m_EffectState == EFFECT_HOLDING)
			{
				m_Parameters[PHYSCANNON_BLAST].GetAlpha().SetAbsolute( random->RandomInt( 125, 150 ) );
				m_Parameters[PHYSCANNON_BLAST].GetScale().SetAbsolute( random->RandomFloat( 0.125, 0.15 ) * flScaleFactor );
			}
			else
			{
				m_Parameters[PHYSCANNON_BLAST].GetAlpha().SetAbsolute( random->RandomInt( 32, 64 ) );
				m_Parameters[PHYSCANNON_BLAST].GetScale().SetAbsolute( random->RandomFloat( 0.075, 0.15 ) * flScaleFactor );
			}
		}
	}
#endif
}*/

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::ItemPostFrame()
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner == NULL )
	{
		// We found an object. Debounce the button
		m_nAttack2Debounce = 0;
		return;
	}

	//Check for object in pickup range
	if ( m_bActive == false )
	{
		CheckForTarget();

		if ( ( m_flElementDebounce < gpGlobals->curtime ) && ( m_nChangeState != ELEMENT_STATE_NONE ) )
		{
			if ( m_nChangeState == ELEMENT_STATE_OPEN )
			{
				OpenElements();
			}
			else if ( m_nChangeState == ELEMENT_STATE_CLOSED )
			{
				CloseElements();
			}

			m_nChangeState = ELEMENT_STATE_NONE;
		}
	}

	// NOTE: Attack2 will be considered to be pressed until the first item is picked up.
	int nAttack2Mask = pOwner->m_nButtons & (~m_nAttack2Debounce);
	if ( nAttack2Mask & IN_ATTACK2 )
	{
		SecondaryAttack();
	}
	else
	{
		// Reset our debouncer
		m_flLastDenySoundPlayed = false;

		if ( m_bActive == false )
		{
			DoEffect( EFFECT_READY );
		}
	}
	
	if (( pOwner->m_nButtons & IN_ATTACK2 ) == 0 )
	{
		m_nAttack2Debounce = 0;
	}

	if ( pOwner->m_nButtons & IN_ATTACK )
	{
		PrimaryAttack();
	}
	else 
	{
		WeaponIdle();
	}

	if ( IsMegaPhysCannon() )
	{
		if ( !( pOwner->m_nButtons & IN_ATTACK ) )
		{
			m_flNextPrimaryAttack = gpGlobals->curtime;
		}

		if ( pOwner->m_afButtonPressed & IN_ATTACK3 )
		{
			TertiaryAttack();
		}
		else 
		{
			WeaponIdle();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::ItemHolsterFrame( void )
{
	BaseClass::ItemHolsterFrame();

#ifdef CLIENT_DLL
	ManagePredictedObject();
#endif
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define PHYSCANNON_DANGER_SOUND_RADIUS 128

void CWeaponPhysCannon::LaunchObject( const Vector &vecDir, float flForce )
{
	CBaseEntity *pObject = m_grabController.GetAttached();

	if ( !(m_hLastPuntedObject == pObject && gpGlobals->curtime < m_flRepuntObjectTime) )
	{
		// FIRE!!!
		if( pObject != NULL )
		{
			DetachObject( false, true );

			m_hLastPuntedObject = pObject;
			m_flRepuntObjectTime = gpGlobals->curtime + 0.5f;

			// Trace ahead a bit and make a chain of danger sounds ahead of the phys object
			// to scare potential targets
			trace_t	tr;
			Vector	vecStart = pObject->GetAbsOrigin();
#ifndef CLIENT_DLL
			Vector	vecSpot;
			int		iLength;
			int		i;
#endif
			Ray_t ray; ray.Init( vecStart, vecStart + vecDir * flForce );
			UTIL_Portal_TraceRay( ray, MASK_SHOT, pObject, COLLISION_GROUP_NONE, &tr );
#ifndef CLIENT_DLL
			if (gpGlobals->maxClients == 1)
			{
				iLength = (tr.startpos - tr.endpos).Length();
				vecSpot = vecStart + vecDir * PHYSCANNON_DANGER_SOUND_RADIUS;

				for (i = PHYSCANNON_DANGER_SOUND_RADIUS; i < iLength; i += PHYSCANNON_DANGER_SOUND_RADIUS)
				{
					CSoundEnt::InsertSound( SOUND_PHYSICS_DANGER, vecSpot, PHYSCANNON_DANGER_SOUND_RADIUS, 0.5, pObject );
					vecSpot = vecSpot + (vecDir * PHYSCANNON_DANGER_SOUND_RADIUS);
				}
			}
#endif

			// Launch
			ApplyVelocityBasedForce( pObject, vecDir, tr.endpos, PHYSGUN_FORCE_LAUNCHED );

			// Don't allow the gun to regrab a thrown object!!
			m_flNextSecondaryAttack = m_flNextPrimaryAttack = m_flNextTertiaryAttack = gpGlobals->curtime + 0.5;
			
			Vector	center = pObject->WorldSpaceCenter();

			//Do repulse effect
			DoEffect( EFFECT_LAUNCH, &center );

			m_hAttachedObject = NULL;
			m_bActive = false;
		}
	}

	// Stop our looping sound
	if ( GetMotorSound() )
	{
		(CSoundEnvelopeController::GetController()).SoundChangeVolume( GetMotorSound(), 0.0f, 1.0f );
		(CSoundEnvelopeController::GetController()).SoundChangePitch( GetMotorSound(), 50, 1.0f );
	}

	//Close the elements and suppress checking for a bit
	m_nChangeState = ELEMENT_STATE_CLOSED;
	m_flElementDebounce = gpGlobals->curtime + 0.1f;
	m_flCheckSuppressTime = gpGlobals->curtime + 0.25f;
}

bool UTIL_IsCombineBall( CBaseEntity *pEntity );

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTarget - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponPhysCannon::CanPickupObject( CBaseEntity *pTarget )
{
#ifndef CLIENT_DLL
	if ( pTarget == NULL )
		return false;

	if ( pTarget->GetBaseAnimating() && pTarget->GetBaseAnimating()->IsDissolving() )
		return false;
	if (pTarget->HasSpawnFlags(SF_PHYSBOX_ALWAYS_PICK_UP) || pTarget->HasSpawnFlags(SF_PHYSBOX_NEVER_PICK_UP))
	{
		// It may seem strange to check this spawnflag before we know the class of this object, since the
		// spawnflag only applies to func_physbox, but it can act as a filter of sorts to reduce the number
		// of irrelevant entities that fall through to this next casting check, which is slower.
		CPhysBox* pPhysBox = dynamic_cast<CPhysBox*>(pTarget);

		if (pPhysBox != NULL)
		{
			if (pTarget->HasSpawnFlags(SF_PHYSBOX_NEVER_PICK_UP))
				return false;
			else
				return true;
		}
	}

	if (pTarget->HasSpawnFlags(SF_PHYSPROP_ALWAYS_PICK_UP))
	{
		// It may seem strange to check this spawnflag before we know the class of this object, since the
		// spawnflag only applies to func_physbox, but it can act as a filter of sorts to reduce the number
		// of irrelevant entities that fall through to this next casting check, which is slower.
		CPhysicsProp* pPhysProp = dynamic_cast<CPhysicsProp*>(pTarget);
		if (pPhysProp != NULL)
			return true;
	}

	if ( pTarget->IsEFlagSet( EFL_NO_PHYSCANNON_INTERACTION ) )
		return false;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner && pOwner->GetGroundEntity() == pTarget )
		return false;

	if ( pTarget->VPhysicsIsFlesh( ) )
		return false;

	IPhysicsObject *pObj = pTarget->VPhysicsGetObject();	

	if ( pObj && pObj->GetGameFlags() & FVPHYSICS_PLAYER_HELD )
		return false;

	if ( UTIL_IsCombineBall( pTarget ) )
	{
		return CBasePlayer::CanPickupObject( pTarget, 0, 0 );
	}

	if ( !IsMegaPhysCannon() )
	{
		if ( pTarget->VPhysicsIsFlesh() )
			return false;

		return CBasePlayer::CanPickupObject(pTarget, physcannon_maxmass.GetFloat(), 0);
	}

	if ( pTarget->IsNPC() && pTarget->MyNPCPointer()->CanBecomeRagdoll() )
		return true;

	if ( pTarget->IsPlayer() && !InSameTeam( pTarget ) )
		return true;

	if ( dynamic_cast<CRagdollProp*>( pTarget ) )
		return true;

	if ( dynamic_cast<CTFBaseRocket*>( pTarget ) )
		return true;

	// massLimit should be 0 since we would've already called it with physcannon_maxmass.GetFloat() above if it weren't a MegaPhysCannon
	return CBasePlayer::CanPickupObject(pTarget, 0, 0);

#else
	return false;
#endif	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::OpenElements( void )
{
	if ( m_bOpen )
		return;

	WeaponSound( SPECIAL2 );

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	SendWeaponAnim( ACT_VM_IDLE );

	m_bOpen = true;

	DoEffect( EFFECT_READY );

#ifdef CLIENT
	// Element prediction 
	m_ElementParameter.InitFromCurrent( 1.0f, 0.2f, INTERP_SPLINE );
	m_bOldOpen = true;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::CloseElements( void )
{
	//TDT USE!!!!!
	CDisablePredictionFiltering disabler;

	// The mega cannon cannot be closed!
	if ( IsMegaPhysCannon() )
	{
		OpenElements();
		return;
	}

	if ( m_bOpen == false )
		return;

	WeaponSound( MELEE_HIT );

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	SendWeaponAnim( ACT_VM_IDLE );

	m_bOpen = false;

	if ( GetMotorSound() )
	{
		(CSoundEnvelopeController::GetController()).SoundChangeVolume( GetMotorSound(), 0.0f, 1.0f );
		(CSoundEnvelopeController::GetController()).SoundChangePitch( GetMotorSound(), 50, 1.0f );
	}
	
	DoEffect( EFFECT_CLOSED );

#ifdef CLIENT
	// Element prediction 
	m_ElementParameter.InitFromCurrent( 0.0f, 0.5f, INTERP_SPLINE );
	m_bOldOpen = false;
#endif
}

#define	PHYSCANNON_MAX_MASS		500


//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CWeaponPhysCannon::GetLoadPercentage( void )
{
	float loadWeight = m_grabController.GetLoadWeight();
	loadWeight /= physcannon_maxmass.GetFloat();	
	loadWeight = clamp( loadWeight, 0.0f, 1.0f );
	return loadWeight;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : CSoundPatch
//-----------------------------------------------------------------------------
CSoundPatch *CWeaponPhysCannon::GetMotorSound(void)
{
	//TDT USE!!!!!
	CDisablePredictionFiltering disabler;
	if (m_sndMotor == NULL)
	{
		CPASAttenuationFilter filter(this);

		if ( IsMegaPhysCannon() )
		{
			m_sndMotor = ( CSoundEnvelopeController::GetController()).SoundCreate( filter, entindex(), CHAN_STATIC, "Weapon_MegaPhysCannon.HoldSound", ATTN_NORM );
		}
		else
		{
			m_sndMotor = ( CSoundEnvelopeController::GetController()).SoundCreate( filter, entindex(), CHAN_STATIC, "Weapon_PhysCannon.HoldSound", ATTN_NORM );
		}

	}

	return m_sndMotor;
}

//-----------------------------------------------------------------------------
// Shuts down sounds
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::StopLoopingSounds()
{
	if ( m_sndMotor != NULL )
	{
		 (CSoundEnvelopeController::GetController()).SoundDestroy( m_sndMotor );
		 m_sndMotor = NULL;
	}

#ifndef CLIENT_DLL
	BaseClass::StopLoopingSounds();
#endif
	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::DestroyEffects( void )
{
#ifdef CLIENT_DLL

	// Free our beams
	m_Beams[0].Release();
	m_Beams[1].Release();
	m_Beams[2].Release();

#endif

	// Stop everything
	StopEffects();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::StopEffects( bool stopSound )
{
	// Turn off our effect state
	DoEffect( EFFECT_NONE );

#ifndef CLIENT_DLL
	//Shut off sounds
	if ( stopSound && GetMotorSound() != NULL )
	{
		(CSoundEnvelopeController::GetController()).SoundFadeOut( GetMotorSound(), 0.1f );
	}
#endif	// !CLIENT_DLL
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::StartEffects( void )
{
#ifdef CLIENT_DLL
	// ------------------------------------------
	// Core
	// ------------------------------------------
	if ( m_Parameters[PHYSCANNON_CORE].GetMaterial(0) == NULL )
	{
		m_Parameters[PHYSCANNON_CORE].GetScale().Init( 0.0f, 1.0f, 0.1f );
		m_Parameters[PHYSCANNON_CORE].GetAlpha().Init( 255.0f, 255.0f, 0.1f );
		m_Parameters[PHYSCANNON_CORE].SetAttachment( 1 );
		
		if ( m_Parameters[PHYSCANNON_CORE].SetMaterial( PHYSCANNON_CENTER_GLOW, 0) == false )
		{
			// This means the texture was not found
			Assert( 0 );
		}

		if (m_Parameters[PHYSCANNON_CORE].SetMaterial(MEGACANNON_CENTER_GLOW, 1) == false)
		{
			// This means the texture was not found
			Assert(0);
		}
	}

	// ------------------------------------------
	// Blast
	// ------------------------------------------
	if ( m_Parameters[PHYSCANNON_BLAST].GetMaterial(0) == NULL )
	{
		m_Parameters[PHYSCANNON_BLAST].GetScale().Init( 0.0f, 1.0f, 0.1f );
		m_Parameters[PHYSCANNON_BLAST].GetAlpha().Init( 255.0f, 255.0f, 0.1f );
		m_Parameters[PHYSCANNON_BLAST].SetAttachment( 1 );
		m_Parameters[PHYSCANNON_BLAST].SetVisible( false );
		
		if ( m_Parameters[PHYSCANNON_BLAST].SetMaterial( PHYSCANNON_BLAST_SPRITE, 0) == false )
		{
			// This means the texture was not found
			Assert( 0 );
		}

		if (m_Parameters[PHYSCANNON_BLAST].SetMaterial(MEGACANNON_BLAST_SPRITE, 1) == false)
		{
			// This means the texture was not found
			Assert(0);
		}
	}

	// ------------------------------------------
	// Glows
	// ------------------------------------------
	const char *attachNamesGlowThirdPerson[NUM_GLOW_SPRITES] = 
	{
		"fork1m",
		"fork1t",
		"fork2m",
		"fork2t",
		"fork3m",
		"fork3t",
	};

	const char *attachNamesGlow[NUM_GLOW_SPRITES] = 
	{
		"fork1b",
		"fork1m",
		"fork1t",
		"fork2b",
		"fork2m",
		"fork2t"
	};

	//Create the glow sprites
	for ( int i = PHYSCANNON_GLOW1; i < (PHYSCANNON_GLOW1+NUM_GLOW_SPRITES); i++ )
	{
		if ( m_Parameters[i].GetMaterial(0) != NULL )
			continue;

		m_Parameters[i].GetScale().SetAbsolute( 0.05f * SPRITE_SCALE );
		m_Parameters[i].GetAlpha().SetAbsolute( 64.0f );
		
		// Different for different views
		if ( ShouldDrawUsingViewModel() )
		{
			m_Parameters[i].SetAttachment( LookupAttachment( attachNamesGlow[i-PHYSCANNON_GLOW1] ) );
		}
		else
		{
			m_Parameters[i].SetAttachment( LookupAttachment( attachNamesGlowThirdPerson[i-PHYSCANNON_GLOW1] ) );
		}
		m_Parameters[i].SetColor( Vector( 255, 128, 0 ) );
		
		if (m_Parameters[i].SetMaterial(PHYSCANNON_GLOW_SPRITE, 0) == false)
		{
			// This means the texture was not found
			Assert(0);
		}

		if (m_Parameters[i].SetMaterial(MEGACANNON_GLOW_SPRITE, 1) == false)
		{
			// This means the texture was not found
			Assert(0);
		}
	}

	// ------------------------------------------
	// End caps
	// ------------------------------------------
	const char *attachNamesEndCap[NUM_ENDCAP_SPRITES] = 
	{
		"fork1t",
		"fork2t",
		"fork3t"
	};
	
	//Create the glow sprites
	for ( int i = PHYSCANNON_ENDCAP1; i < (PHYSCANNON_ENDCAP1+NUM_ENDCAP_SPRITES); i++ )
	{
		if ( m_Parameters[i].GetMaterial(0) != NULL )
			continue;

		m_Parameters[i].GetScale().SetAbsolute( 0.05f * SPRITE_SCALE );
		m_Parameters[i].GetAlpha().SetAbsolute( 255.0f );
		m_Parameters[i].SetAttachment( LookupAttachment( attachNamesEndCap[i-PHYSCANNON_ENDCAP1] ) );
		m_Parameters[i].SetVisible( false );
		
		if (m_Parameters[i].SetMaterial(PHYSCANNON_ENDCAP_SPRITE, 0) == false)
		{
			// This means the texture was not found
			Assert(0);
		}

		if (m_Parameters[i].SetMaterial(MEGACANNON_ENDCAP_SPRITE, 1) == false)
		{
			// This means the texture was not found
			Assert(0);
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Closing effects
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::DoEffectClosed( void )
{
#ifdef CLIENT_DLL

	// Turn off the end-caps
	for ( int i = PHYSCANNON_ENDCAP1; i < (PHYSCANNON_ENDCAP1+NUM_ENDCAP_SPRITES); i++ )
	{
		m_Parameters[i].SetVisible( false );
	}
	
#endif

}

//-----------------------------------------------------------------------------
// Purpose: Ready effects
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::DoEffectReady( void )
{
#ifdef CLIENT_DLL

	// Special POV case
	if ( ShouldDrawUsingViewModel() )
	{
		//Turn on the center sprite
		m_Parameters[PHYSCANNON_CORE].GetScale().InitFromCurrent( 14.0f, 0.2f );
		m_Parameters[PHYSCANNON_CORE].GetAlpha().InitFromCurrent( 128.0f, 0.2f );
		m_Parameters[PHYSCANNON_CORE].SetVisible();
	}
	else
	{
		//Turn off the center sprite
		m_Parameters[PHYSCANNON_CORE].GetScale().InitFromCurrent( 8.0f, 0.2f );
		m_Parameters[PHYSCANNON_CORE].GetAlpha().InitFromCurrent( 0.0f, 0.2f );
		m_Parameters[PHYSCANNON_CORE].SetVisible();
	}

	// Turn on the glow sprites
	for ( int i = PHYSCANNON_GLOW1; i < (PHYSCANNON_GLOW1+NUM_GLOW_SPRITES); i++ )
	{
		m_Parameters[i].GetScale().InitFromCurrent( 0.4f * SPRITE_SCALE, 0.2f );
		m_Parameters[i].GetAlpha().InitFromCurrent( 64.0f, 0.2f );
		m_Parameters[i].SetVisible();
	}

	// Turn on the glow sprites
	for ( int i = PHYSCANNON_ENDCAP1; i < (PHYSCANNON_ENDCAP1+NUM_ENDCAP_SPRITES); i++ )
	{
		m_Parameters[i].SetVisible( false );
	}

#endif
}


//-----------------------------------------------------------------------------
// Holding effects
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::DoEffectHolding( void )
{

#ifdef CLIENT_DLL
	if ( ShouldDrawUsingViewModel() )
	{
		// Scale up the center sprite
		m_Parameters[PHYSCANNON_CORE].GetScale().InitFromCurrent( 16.0f, 0.2f );
		m_Parameters[PHYSCANNON_CORE].GetAlpha().InitFromCurrent( 255.0f, 0.1f );
		m_Parameters[PHYSCANNON_CORE].SetVisible();

		// Prepare for scale up
		m_Parameters[PHYSCANNON_BLAST].SetVisible( false );

		// Turn on the glow sprites
		for ( int i = PHYSCANNON_GLOW1; i < (PHYSCANNON_GLOW1+NUM_GLOW_SPRITES); i++ )
		{
			m_Parameters[i].GetScale().InitFromCurrent( 0.5f * SPRITE_SCALE, 0.2f );
			m_Parameters[i].GetAlpha().InitFromCurrent( 64.0f, 0.2f );
			m_Parameters[i].SetVisible();
		}

		// Turn on the glow sprites
		// NOTE: The last glow is left off for first-person
		for ( int i = PHYSCANNON_ENDCAP1; i < (PHYSCANNON_ENDCAP1+NUM_ENDCAP_SPRITES-1); i++ )
		{
			m_Parameters[i].SetVisible();
		}

		// Create our beams
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
		CBaseEntity *pBeamEnt = pOwner->GetViewModel();

		// Setup the beams
		m_Beams[0].Init( LookupAttachment( "fork1t" ), 1, pBeamEnt, true );
		m_Beams[1].Init( LookupAttachment( "fork2t" ), 1, pBeamEnt, true );

		// Set them visible
		m_Beams[0].SetVisible();
		m_Beams[1].SetVisible();
	}
	else
	{
		// Scale up the center sprite
		m_Parameters[PHYSCANNON_CORE].GetScale().InitFromCurrent( 14.0f, 0.2f );
		m_Parameters[PHYSCANNON_CORE].GetAlpha().InitFromCurrent( 255.0f, 0.1f );
		m_Parameters[PHYSCANNON_CORE].SetVisible();

		// Prepare for scale up
		m_Parameters[PHYSCANNON_BLAST].SetVisible( false );

		// Turn on the glow sprites
		for ( int i = PHYSCANNON_GLOW1; i < (PHYSCANNON_GLOW1+NUM_GLOW_SPRITES); i++ )
		{
			m_Parameters[i].GetScale().InitFromCurrent( 0.5f * SPRITE_SCALE, 0.2f );
			m_Parameters[i].GetAlpha().InitFromCurrent( 64.0f, 0.2f );
			m_Parameters[i].SetVisible();
		}

		// Turn on the glow sprites
		for ( int i = PHYSCANNON_ENDCAP1; i < (PHYSCANNON_ENDCAP1+NUM_ENDCAP_SPRITES); i++ )
		{
			m_Parameters[i].SetVisible();
		}

		// Setup the beams
		m_Beams[0].Init( LookupAttachment( "fork1t" ), 1, this, false );
		m_Beams[1].Init( LookupAttachment( "fork2t" ), 1, this, false );
		m_Beams[2].Init( LookupAttachment( "fork3t" ), 1, this, false );

		// Set them visible
		m_Beams[0].SetVisible();
		m_Beams[1].SetVisible();
		m_Beams[2].SetVisible();
	}

#endif

}


//-----------------------------------------------------------------------------
// Launch effects
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::DoEffectLaunch( Vector *pos )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner == NULL )
		return;

	Vector	endPos;
	Vector	shotDir;

	// See if we need to predict this position
	if ( pos == NULL )
	{
		// Hit an entity if we're holding one
		if ( m_hAttachedObject )
		{
			endPos = m_hAttachedObject->WorldSpaceCenter();
			
			shotDir = endPos - pOwner->Weapon_ShootPosition();
			VectorNormalize( shotDir );
		}
		else
		{
			// Otherwise try and find the right spot
			endPos = pOwner->Weapon_ShootPosition();
			pOwner->EyeVectors( &shotDir );

			trace_t	tr;
			Ray_t ray; ray.Init( endPos, endPos + ( shotDir * MAX_TRACE_LENGTH ) );
			UTIL_Portal_TraceRay( ray, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr );

			endPos = tr.endpos;
			shotDir = endPos - pOwner->Weapon_ShootPosition();
			VectorNormalize( shotDir );
		}
	}
	else
	{
		// Use what is supplied
		endPos = *pos;
		shotDir = ( endPos - pOwner->Weapon_ShootPosition() );
		VectorNormalize( shotDir );
	}

	// End hit
	CPVSFilter filter( endPos );

	// Don't send this to the owning player, they already had it predicted
	if ( IsPredicted() )
		filter.UsePredictionRules();

	// Do an impact hit
	CEffectData	data;
	data.m_vOrigin = endPos;
#ifdef CLIENT_DLL
	data.m_hEntity = GetRefEHandle();
#else
	data.m_nEntIndex = entindex();
#endif

	te->DispatchEffect( filter, 0.0, data.m_vOrigin, "PhyscannonImpact", data );

#ifdef CLIENT_DLL

	//Turn on the blast sprite and scale
	m_Parameters[PHYSCANNON_BLAST].GetScale().Init( 8.0f, 64.0f, 0.1f );
	m_Parameters[PHYSCANNON_BLAST].GetAlpha().Init( 255.0f, 0.0f, 0.2f );
	m_Parameters[PHYSCANNON_BLAST].SetVisible();

#endif

}

//-----------------------------------------------------------------------------
// Purpose: Shutdown for the weapon when it's holstered
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::DoEffectNone( void )
{
#ifdef CLIENT_DLL

	//Turn off main glows
	m_Parameters[PHYSCANNON_CORE].SetVisible( false );
	m_Parameters[PHYSCANNON_BLAST].SetVisible( false );

	for ( int i = PHYSCANNON_GLOW1; i < (PHYSCANNON_GLOW1+NUM_GLOW_SPRITES); i++ )
	{
		m_Parameters[i].SetVisible( false );
	}

	// Turn on the glow sprites
	for ( int i = PHYSCANNON_ENDCAP1; i < (PHYSCANNON_ENDCAP1+NUM_ENDCAP_SPRITES); i++ )
	{
		m_Parameters[i].SetVisible( false );
	}

	m_Beams[0].SetVisible( false );
	m_Beams[1].SetVisible( false );
	m_Beams[2].SetVisible( false );

#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : effectType - 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::DoEffect( int effectType, Vector *pos )
{
	m_EffectState = effectType;

#ifdef CLIENT_DLL
	// Save predicted state
	m_nOldEffectState = m_EffectState;
#endif

	switch( effectType )
	{
	case EFFECT_CLOSED:
		DoEffectClosed( );
		break;

	case EFFECT_READY:
		DoEffectReady( );
		break;

	case EFFECT_HOLDING:
		DoEffectHolding();
		break;

	case EFFECT_LAUNCH:
		DoEffectLaunch( pos );
		break;

	default:
	case EFFECT_NONE:
		DoEffectNone();
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : iIndex - 
// Output : const char
//-----------------------------------------------------------------------------
const char *CWeaponPhysCannon::GetShootSound(int iIndex) const
{
#ifndef CLIENT_DLL
	// Just do this normally if we're a normal physcannon
	if (const_cast<CWeaponPhysCannon *> (this)->IsMegaPhysCannon() == false)
		return BaseClass::GetShootSound(iIndex);

	// We override this if we're the charged up version
	switch( iIndex )
	{
	case EMPTY:
		return "Weapon_MegaPhysCannon.DryFire";
		break;

	case SINGLE:
		return "Weapon_MegaPhysCannon.Launch";
		break;

	case SPECIAL1:
		return "Weapon_MegaPhysCannon.Pickup";
		break;

	case MELEE_MISS:
		return "Weapon_MegaPhysCannon.Drop";
		break;

	default:
		break;
	}
#endif
	return BaseClass::GetShootSound( iIndex );
}

#ifdef CLIENT_DLL

extern void FormatViewModelAttachment( Vector &vOrigin, bool bInverse );

//-----------------------------------------------------------------------------
// Purpose: Gets the complete list of values needed to render an effect from an
//			effect parameter
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::GetEffectParameters( EffectType_t effectID, color32 &color, float &scale, IMaterial **pMaterial, Vector &vecAttachment )
{
	const float dt = gpGlobals->curtime;

	// Get alpha
	float alpha = m_Parameters[effectID].GetAlpha().Interp( dt );
	
	// Get scale
	scale = m_Parameters[effectID].GetScale().Interp( dt ) * SpriteScaleFactor();
	
	// Get material
	*pMaterial = (IMaterial*)m_Parameters[effectID].GetMaterial(IsMegaPhysCannon() ? 1 : 0);

	// Setup the color
	color.r = (int) m_Parameters[effectID].GetColor().x;
	color.g = (int) m_Parameters[effectID].GetColor().y;
	color.b = (int) m_Parameters[effectID].GetColor().z;
	color.a = (int) alpha;

	// Setup the attachment
	int		attachment = m_Parameters[effectID].GetAttachment();
	QAngle	angles;

	// Format for first-person
	if ( ShouldDrawUsingViewModel() )
	{
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
		
		if ( pOwner != NULL )
		{
			pOwner->GetViewModel()->GetAttachment( attachment, vecAttachment, angles );
			::FormatViewModelAttachment( vecAttachment, true );
		}
	}
	else
	{
		GetAttachment( attachment, vecAttachment, angles );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Whether or not an effect is set to display
//-----------------------------------------------------------------------------
bool CWeaponPhysCannon::IsEffectVisible( EffectType_t effectID )
{
	return m_Parameters[effectID].IsVisible();
}

//-----------------------------------------------------------------------------
// Purpose: Draws the effect sprite, given an effect parameter ID
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::DrawEffectSprite( EffectType_t effectID )
{
	color32 color;
	float scale;
	IMaterial *pMaterial;
	Vector	vecAttachment;

	// Don't draw invisible effects
	if ( IsEffectVisible( effectID ) == false )
		return;

	// Get all of our parameters
	GetEffectParameters( effectID, color, scale, &pMaterial, vecAttachment );

	// Msg( "Scale: %.2f\tAlpha: %.2f\n", scale, alpha );

	// Don't render fully translucent objects
	if ( color.a <= 0.0f )
		return;

	// Draw the sprite
	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->Bind( pMaterial, this );
	DrawSprite( vecAttachment, scale, scale, color );
}

//-----------------------------------------------------------------------------
// Purpose: Render our third-person effects
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::DrawEffects( void )
{
	// Draw the core effects
	DrawEffectSprite( PHYSCANNON_CORE );
	DrawEffectSprite( PHYSCANNON_BLAST );
	
	// Draw the glows
	for ( int i = PHYSCANNON_GLOW1; i < (PHYSCANNON_GLOW1+NUM_GLOW_SPRITES); i++ )
	{
		DrawEffectSprite( (EffectType_t) i );
	}

	// Draw the endcaps
	for ( int i = PHYSCANNON_ENDCAP1; i < (PHYSCANNON_ENDCAP1+NUM_ENDCAP_SPRITES); i++ )
	{
		DrawEffectSprite( (EffectType_t) i );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Third-person function call to render world model
//-----------------------------------------------------------------------------
int CWeaponPhysCannon::DrawModel( int flags )
{
	// Only render these on the transparent pass
	if ( flags & STUDIO_TRANSPARENCY )
	{
		if ( ShouldDrawUsingViewModel() )
			return 0;

		DrawEffects();
		return 1;
	}

	// Only do this on the opaque pass
	return BaseClass::DrawModel( flags );
}

//-----------------------------------------------------------------------------
// Purpose: First-person function call after viewmodel has been drawn
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::ViewModelDrawn( C_BaseViewModel *pBaseViewModel )
{
	// Render our effects
	DrawEffects();

	// Pass this back up
	BaseClass::ViewModelDrawn( pBaseViewModel );
}

//-----------------------------------------------------------------------------
// Purpose: We are always considered transparent
//-----------------------------------------------------------------------------
bool CWeaponPhysCannon::IsTransparent( void )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::NotifyShouldTransmit( ShouldTransmitState_t state )
{
	BaseClass::NotifyShouldTransmit(state);

	if ( state == SHOULDTRANSMIT_END )
	{
		DoEffect( EFFECT_NONE );
	}
}

#endif

//-----------------------------------------------------------------------------
// EXTERNAL API
//-----------------------------------------------------------------------------
void PhysCannonForceDrop( CBaseCombatWeapon *pActiveWeapon, CBaseEntity *pOnlyIfHoldingThis )
{
	CWeaponPhysCannon *pCannon = dynamic_cast<CWeaponPhysCannon *>(pActiveWeapon);
	if ( pCannon )
	{
		if ( pOnlyIfHoldingThis )
		{
			pCannon->DropIfEntityHeld( pOnlyIfHoldingThis );
		}
		else
		{
			pCannon->ForceDrop();
		}
	}
}

bool PlayerPickupControllerIsHoldingEntity( CBaseEntity *pPickupControllerEntity, CBaseEntity *pHeldEntity )
{
	CPlayerPickupController *pController = dynamic_cast<CPlayerPickupController *>(pPickupControllerEntity);

	return pController ? pController->IsHoldingEntity( pHeldEntity ) : false;
}


float PhysCannonGetHeldObjectMass( CBaseCombatWeapon *pActiveWeapon, IPhysicsObject *pHeldObject )
{
	float mass = 0.0f;
	CWeaponPhysCannon *pCannon = dynamic_cast<CWeaponPhysCannon *>(pActiveWeapon);
	if ( pCannon )
	{
		CGrabController &grab = pCannon->GetGrabController();
		mass = grab.GetSavedMass( pHeldObject );
	}

	return mass;
}

CBaseEntity *PhysCannonGetHeldEntity( CBaseCombatWeapon *pActiveWeapon )
{
	CWeaponPhysCannon *pCannon = dynamic_cast<CWeaponPhysCannon *>(pActiveWeapon);
	if ( pCannon )
	{
		CGrabController &grab = pCannon->GetGrabController();
		return grab.GetAttached();
	}

	return NULL;
}

CBaseEntity *GetPlayerHeldEntity(CBasePlayer *pPlayer)
{
	CBaseEntity *pObject = NULL;
	CPlayerPickupController *pPlayerPickupController = (CPlayerPickupController *)(pPlayer->GetUseEntity());

	if (pPlayerPickupController)
	{
		pObject = pPlayerPickupController->GetGrabController().GetAttached();
	}

	return pObject;
}

CBasePlayer *GetPlayerHoldingEntity( CBaseEntity *pEntity )
{
	for( int i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );
		if ( pPlayer )
		{
			if ( GetPlayerHeldEntity( pPlayer ) == pEntity || PhysCannonGetHeldEntity( pPlayer->GetActiveWeapon() ) == pEntity )
				return pPlayer;
		}
	}
	return NULL;
}

void ShutdownPickupController( CBaseEntity* pPickupControllerEntity )
{
	CPlayerPickupController* pController = dynamic_cast<CPlayerPickupController*>(pPickupControllerEntity);
	if ( pController )
		pController->Shutdown( false );
}

CGrabController *GetGrabControllerForPlayer( CBasePlayer *pPlayer )
{
	CPlayerPickupController *pPlayerPickupController = (CPlayerPickupController *)(pPlayer->GetUseEntity());
	if( pPlayerPickupController )
		return &(pPlayerPickupController->GetGrabController());

	return NULL;
}

CGrabController *GetGrabControllerForPhysCannon( CBaseCombatWeapon *pActiveWeapon )
{
	CWeaponPhysCannon *pCannon = dynamic_cast<CWeaponPhysCannon *>(pActiveWeapon);
	if ( pCannon )
		return &(pCannon->GetGrabController());

	return NULL;
}

void GetSavedParamsForCarriedPhysObject( CGrabController *pGrabController, IPhysicsObject *pObject, float *pSavedMassOut, float *pSavedRotationalDampingOut )
{
	if ( !pGrabController )
		return;

	CBaseEntity *pHeld = pGrabController->m_attachedEntity;
	if( pHeld )
	{
		if( pObject->GetGameData() == (void*)pHeld )
		{
			IPhysicsObject *pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
			int count = pHeld->VPhysicsGetObjectList( pList, ARRAYSIZE(pList) );
			for ( int i = 0; i < count; i++ )
			{
				if ( pList[i] == pObject )
				{
					if( pSavedMassOut )
						*pSavedMassOut = pGrabController->m_savedMass[i];

					if( pSavedRotationalDampingOut )
						*pSavedRotationalDampingOut = pGrabController->m_savedRotDamping[i];

					return;
				}
			}
		}
	}

	if( pSavedMassOut )
		*pSavedMassOut = 0.0f;

	if( pSavedRotationalDampingOut )
		*pSavedRotationalDampingOut = 0.0f;

	return;
}

void UpdateGrabControllerTargetPosition( CBasePlayer *pPlayer, Vector *vPosition, QAngle *qAngles )
{
	CGrabController *pGrabController = GetGrabControllerForPlayer( pPlayer );

	if ( !pGrabController )
		return;

	pGrabController->UpdateObject( pPlayer, 12 );
	pGrabController->GetTargetPosition( vPosition, qAngles );
}

float PlayerPickupGetHeldObjectMass( CBaseEntity *pPickupControllerEntity, IPhysicsObject *pHeldObject )
{
	float mass = 0.0f;
	CPlayerPickupController *pController = dynamic_cast<CPlayerPickupController *>(pPickupControllerEntity);
	if ( pController )
	{
		CGrabController &grab = pController->GetGrabController();
		mass = grab.GetSavedMass( pHeldObject );
	}
	return mass;
}

void GrabController_SetPortalPenetratingEntity( CGrabController *pController, CBaseEntity *pPenetrated )
{
	pController->SetPortalPenetratingEntity( pPenetrated );
}

#ifdef CLIENT_DLL
extern void FX_GaussExplosion( const Vector &pos, const Vector &dir, int type );

void CallbackPhyscannonImpact( const CEffectData &data )
{
	C_BaseEntity *pEnt = data.GetEntity();
	if ( pEnt == NULL )
		return;

	Vector	vecAttachment;
	QAngle	vecAngles;

	C_WeaponPhysCannon *pWeapon = dynamic_cast< C_WeaponPhysCannon * >( pEnt );
	if ( !pWeapon )
		return;

	pWeapon->GetAttachment( 1, vecAttachment, vecAngles );

	Vector	dir = ( data.m_vOrigin - vecAttachment );
	VectorNormalize( dir );

	// Do special first-person fix-up
	if ( pWeapon->GetOwner() == CBasePlayer::GetLocalPlayer() )
	{
		// Translate the attachment entity to the viewmodel
		C_BasePlayer *pPlayer = dynamic_cast<C_BasePlayer *>( pWeapon->GetOwner() );

		if ( pPlayer && !::input->CAM_IsThirdPerson() )
		{
			pEnt = pPlayer->GetViewModel();

			// Format attachment for first-person view!
			::FormatViewModelAttachment( vecAttachment, true );
		}

		// Explosions at the impact point
		FX_GaussExplosion( data.m_vOrigin, -dir, 0 );

		// Draw a beam
		BeamInfo_t beamInfo;

		beamInfo.m_pStartEnt = pEnt;
		beamInfo.m_nStartAttachment = 1;
		beamInfo.m_pEndEnt = NULL;
		beamInfo.m_nEndAttachment = -1;
		beamInfo.m_vecStart = vec3_origin;
		beamInfo.m_vecEnd = data.m_vOrigin;

		beamInfo.m_pszModelName = pWeapon->IsMegaPhysCannon() ? MEGACANNON_BEAM_SPRITE : PHYSCANNON_BEAM_SPRITE;

		beamInfo.m_flHaloScale = 0.0f;
		beamInfo.m_flLife = 0.1f;
		beamInfo.m_flWidth = 12.0f;
		beamInfo.m_flEndWidth = 4.0f;
		beamInfo.m_flFadeLength = 0.0f;
		beamInfo.m_flAmplitude = 0;
		beamInfo.m_flBrightness = 255.0;
		beamInfo.m_flSpeed = 0.0f;
		beamInfo.m_nStartFrame = 0.0;
		beamInfo.m_flFrameRate = 30.0;
		beamInfo.m_flRed = 255.0;
		beamInfo.m_flGreen = 255.0;
		beamInfo.m_flBlue = 255.0;
		beamInfo.m_nSegments = 16;
		beamInfo.m_bRenderable = true;
		beamInfo.m_nFlags = FBEAM_ONLYNOISEONCE;

		beams->CreateBeamEntPoint( beamInfo );
	}
	else
	{
		// Explosion at the starting point
		FX_GaussExplosion( vecAttachment, dir, 0 );
	}
}

DECLARE_CLIENT_EFFECT( "PhyscannonImpact", CallbackPhyscannonImpact );
#endif

//------------------------------------------------------------------------------------
// Purpose: Default Touch function for player picking up a weapon (not AI) - OVERRIDE.
//------------------------------------------------------------------------------------
void CWeaponPhysCannon::DefaultTouch( CBaseEntity *pOther )
{
#ifdef GAME_DLL
	// Can't pick up dissolving weapons
	if ( IsDissolving() )
		return;

	// if it's not a player, ignore
	CTFPlayer *pPlayer = ToTFPlayer( pOther );
	if ( !pPlayer )
		return;

	if( HasSpawnFlags( SF_WEAPON_NO_PLAYER_PICKUP ) )
		return;

	SetRemoveable( true );

	BaseClass::DefaultTouch( pOther );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPhysCannon::OnPickedUp( CBaseCombatCharacter *pNewOwner )
{
#ifdef GAME_DLL
	if ( !pNewOwner )
		return;

	if ( !HasItemDefinition() )
	{
		m_OnPlayerPickup.FireOutput( pNewOwner, this );

		if ( IsMegaPhysCannon() )
			ToTFPlayer( pNewOwner )->SpeakConceptIfAllowed( MP_CONCEPT_MVM_LOOT_ULTRARARE );
		else
			ToTFPlayer( pNewOwner )->SpeakConceptIfAllowed( MP_CONCEPT_MVM_LOOT_RARE );

		CTFTeam *pTeam = GetGlobalTFTeam( pNewOwner->GetTeamNumber() );
		if ( pTeam )
		{
			int iItemID = 9000;
			if ( IsMegaPhysCannon() )
				iItemID = 9001;

			pTeam->AddWeapon( iItemID );
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponPhysCannon::HasChargeBar( void )
{
	return IsMegaPhysCannon();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
float CWeaponPhysCannon::InternalGetEffectBarRechargeTime( void )
{
	return lfe_physcannon_mega_crit_tertiary_cooldown.GetFloat();
}

bool CWeaponPhysCannon::IsMegaPhysCannon( void )
{
	int iType = 0;
	CALL_ATTRIB_HOOK_INT( iType, set_weapon_mode );

	return ( PlayerHasMegaPhysCannon() || iType == 1 ); //SecobMod__Information: Force this to return true if always enabled.
}

// -----------------------------------------------------------------------------
// Purpose:
// -----------------------------------------------------------------------------
int CWeaponPhysCannon::GetActivityWeaponRole( void )
{
	CTFPlayer *pOwner = GetTFPlayerOwner();
	if ( pOwner )
	{
		if ( pOwner->IsPlayerClass( TF_CLASS_SCOUT ) || pOwner->IsPlayerClass( TF_CLASS_ENGINEER ) )
			return TF_WPN_TYPE_PRIMARY;
	}

	return TF_WPN_TYPE_SECONDARY;
}


IMPLEMENT_NETWORKCLASS_ALIASED( WeaponPhysCannon_Secondary, DT_WeaponPhysCannon_Secondary )

BEGIN_NETWORK_TABLE( CWeaponPhysCannon_Secondary, DT_WeaponPhysCannon_Secondary )
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponPhysCannon_Secondary )
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( tf_weapon_physcannon_secondary, CWeaponPhysCannon_Secondary );
PRECACHE_WEAPON_REGISTER( tf_weapon_physcannon_secondary );

//-----------------------------------------------------------------------------
// Purpose: Precache
//-----------------------------------------------------------------------------
void CWeaponPhysCannon_Secondary::Precache( void )
{
#ifndef CLIENT_DLL
	// Set the proper classname so it loads the correct script file.
	SetClassname( "tf_weapon_physcannon_secondary" );
#endif

	BaseClass::Precache();
}