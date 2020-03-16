#include "cbase.h"
#include "ai_basenpc.h"
#include "ai_blended_movement.h"
#include "ai_behavior_actbusy.h"
#include "npc_citizen17.h"
#include "npc_combines.h"
#include "npc_zombie.h"
#include "particle_parse.h"
#include "tf_player.h"
#include "team.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
// >> CNPC_Custom
//=========================================================
class CNPC_Custom : public CNPC_Citizen
{
	DECLARE_CLASS( CNPC_Custom, CNPC_Citizen );

public:

	CNPC_Custom( void );
	~CNPC_Custom();

	virtual void Precache();
	virtual void Spawn( void );
	virtual Class_T Classify( void );
	virtual void Event_Killed( const CTakeDamageInfo &info );

	CNPC_Custom * GetEntity() { return this; }

	void InputEnable( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );

private:

	int m_intHealth;
	int m_intMaxHealth;
	int m_intBloodColor;
	int m_intCaps;
	Class_T m_intClassify;
	float m_floatFOV;
	string_t m_strHullName;
	string_t m_strClassName;
	bool m_boolPatrool;
	bool m_startDisabled;
	bool m_boolIsBoss;
	COutputEvent m_outputOnKilled;

	DECLARE_DATADESC();
};

BEGIN_DATADESC( CNPC_Custom )
	DEFINE_KEYFIELD( m_intHealth, 		FIELD_INTEGER, 	"health" ),
	DEFINE_KEYFIELD( m_intMaxHealth, 	FIELD_INTEGER, 	"maxhealth" ),
	DEFINE_KEYFIELD( m_intBloodColor, 	FIELD_INTEGER, 	"bloodcolor" ),
	DEFINE_KEYFIELD( m_intCaps, 		FIELD_INTEGER, 	"capabilities" ),
	DEFINE_KEYFIELD( m_intClassify, 	FIELD_INTEGER, 	"classify" ),
	DEFINE_KEYFIELD( m_floatFOV, 		FIELD_FLOAT, 	"fieldofview" ),
	DEFINE_KEYFIELD( m_strHullName, 	FIELD_STRING, 	"hullname" ),
	DEFINE_KEYFIELD( m_strClassName, 	FIELD_STRING, 	"class_name" ),
	DEFINE_KEYFIELD( m_boolPatrool, 	FIELD_BOOLEAN, 	"shouldpatrol" ),
	DEFINE_KEYFIELD( m_startDisabled, 	FIELD_BOOLEAN, 	"start_disabled" ),
	DEFINE_KEYFIELD( m_boolIsBoss, 		FIELD_BOOLEAN, 	"boss_npc" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", 	InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", 	InputDisable ),

	DEFINE_OUTPUT( m_outputOnKilled , "OnKilled" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( npc_custom, CNPC_Custom );

//=========================================================
// Constructor
//=========================================================
CNPC_Custom::CNPC_Custom( void )
{
	m_startDisabled = false;
	m_boolPatrool = false;
	m_boolIsBoss = false;
}

//=========================================================
// Destructor
//=========================================================
CNPC_Custom::~CNPC_Custom( void )
{
}

//=========================================================
// Precache - precaches all resources this NPC needs
//=========================================================
void CNPC_Custom::Precache( void )
{
	m_Type = CT_UNIQUE;

	PrecacheModel( STRING( GetModelName() ) );

	CNPC_Citizen::Precache();
}

//=========================================================
// Classify - indicates this NPC's place in the 
// relationship table.
//=========================================================
Class_T	CNPC_Custom::Classify( void )
{
	if ( m_intClassify != NULL )
		return m_intClassify;

	return CLASS_CITIZEN_REBEL;
}

//=========================================================
// Spawn
//=========================================================
void CNPC_Custom::Spawn( void )
{
	if ( m_strClassName != NULL_STRING )
		SetClassname( STRING( m_strClassName ) );

	char *szModel = (char *)STRING( GetModelName() );
	if ( !szModel || !*szModel )
	{
		Warning( "npc_custom at %.0f %.0f %0.f missing modelname\n", GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z );
		UTIL_Remove( this );
		return;
	}

	Precache();

	SetModel( STRING( GetModelName() ) );

	CNPC_Citizen::Spawn();

	if ( m_intBloodColor != NULL )
		m_bloodColor = m_intBloodColor;

	if ( m_intHealth != NULL )
		m_iHealth = m_intHealth;
	else
		m_iHealth = 500;

	if ( m_intMaxHealth != NULL )
		m_iMaxHealth = m_intMaxHealth;
	else
		m_iMaxHealth = GetHealth();

	// indicates the width of this NPC's forward view cone ( as a dotproduct result )
	if ( m_floatFOV != NULL )
		m_flFieldOfView = m_floatFOV;
	
	m_NPCState = NPC_STATE_NONE;

	if ( m_boolPatrool )
		m_bShouldPatrol = true;
	else
		m_bShouldPatrol = false;

	if ( m_intCaps != NULL )
		CapabilitiesAdd( m_intCaps );

	if ( m_strHullName != NULL_STRING )
		SetHullType( NAI_Hull::LookupId( STRING( m_strHullName ) ) );
	else
		SetHullType( HULL_HUMAN );

	SetHullSizeNormal( );

	if ( m_startDisabled )
	{
		AddFlag( EF_NODRAW );
		SetSolid( SOLID_NONE );
	}
}

//=========================================================
// InputEnable
//=========================================================
void CNPC_Custom::InputEnable( inputdata_t &inputdata )
{
	RemoveFlag( EF_NODRAW );
	SetSolid( SOLID_VPHYSICS );
	m_startDisabled = false;
}

//=========================================================
// InputDisable
//=========================================================
void CNPC_Custom::InputDisable( inputdata_t &inputdata )
{
	AddFlag( EF_NODRAW );
	SetSolid( SOLID_NONE );
	m_startDisabled = true;
}

//=========================================================
// Event Killed
//=========================================================
void CNPC_Custom::Event_Killed( const CTakeDamageInfo &info )
{
	if ( m_boolIsBoss )
		m_outputOnKilled.FireOutput( info.GetAttacker(), this );

	BaseClass::Event_Killed( info );
}

//=========================================================
// >> CNPC_Custom_Combine
//=========================================================
class CNPC_Custom_Combine : public CNPC_CombineS
{
	DECLARE_CLASS( CNPC_Custom_Combine, CNPC_CombineS );

public:

	CNPC_Custom_Combine( void );
	~CNPC_Custom_Combine();

	virtual void Precache();
	virtual void Spawn( void );
	virtual Class_T Classify( void );
	virtual void Event_Killed( const CTakeDamageInfo &info );

	CNPC_Custom_Combine * GetEntity() { return this; }

	void InputEnable( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );

private:

	int m_intHealth;
	int m_intMaxHealth;
	int m_intBloodColor;
	int m_intCaps;
	Class_T m_intClassify;
	float m_floatFOV;
	string_t m_strHullName;
	string_t m_strClassName;
	bool m_boolPatrool;
	bool m_startDisabled;
	bool m_boolIsBoss;
	COutputEvent m_outputOnKilled;

	DECLARE_DATADESC();
};

BEGIN_DATADESC( CNPC_Custom_Combine )
	DEFINE_KEYFIELD( m_intHealth, 		FIELD_INTEGER, 	"health" ),
	DEFINE_KEYFIELD( m_intMaxHealth, 	FIELD_INTEGER, 	"maxhealth" ),
	DEFINE_KEYFIELD( m_intBloodColor, 	FIELD_INTEGER, 	"bloodcolor" ),
	DEFINE_KEYFIELD( m_intCaps, 		FIELD_INTEGER, 	"capabilities" ),
	DEFINE_KEYFIELD( m_intClassify, 	FIELD_INTEGER, 	"classify" ),
	DEFINE_KEYFIELD( m_floatFOV, 		FIELD_FLOAT, 	"fieldofview" ),
	DEFINE_KEYFIELD( m_strHullName, 	FIELD_STRING, 	"hullname" ),
	DEFINE_KEYFIELD( m_strClassName, 	FIELD_STRING, 	"class_name" ),
	DEFINE_KEYFIELD( m_boolPatrool, 	FIELD_BOOLEAN, 	"shouldpatrol" ),
	DEFINE_KEYFIELD( m_startDisabled, 	FIELD_BOOLEAN, 	"start_disabled" ),
	DEFINE_KEYFIELD( m_boolIsBoss, 		FIELD_BOOLEAN, 	"boss_npc" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", 	InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", 	InputDisable ),

	DEFINE_OUTPUT( m_outputOnKilled , "OnKilled" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( npc_custom_combine, CNPC_Custom_Combine );

//=========================================================
// Constructor
//=========================================================
CNPC_Custom_Combine::CNPC_Custom_Combine( void )
{
	m_startDisabled = false;
	m_boolPatrool = false;
	m_boolIsBoss = false;
}

//=========================================================
// Destructor
//=========================================================
CNPC_Custom_Combine::~CNPC_Custom_Combine( void )
{
}

//=========================================================
// Precache - precaches all resources this NPC needs
//=========================================================
void CNPC_Custom_Combine::Precache( void )
{
	PrecacheModel( STRING( GetModelName() ) );

	CNPC_CombineS::Precache();
}

//=========================================================
// Classify - indicates this NPC's place in the 
// relationship table.
//=========================================================
Class_T	CNPC_Custom_Combine::Classify( void )
{
	if ( m_intClassify != NULL )
		return m_intClassify;

	return CLASS_COMBINE;
}

//=========================================================
// Spawn
//=========================================================
void CNPC_Custom_Combine::Spawn( void )
{
	if ( m_strClassName != NULL_STRING )
		SetClassname( STRING( m_strClassName ) );

	char *szModel = (char *)STRING( GetModelName() );
	if ( !szModel || !*szModel )
	{
		Warning( "npc_custom_combine at %.0f %.0f %0.f missing modelname\n", GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z );
		UTIL_Remove( this );
		return;
	}

	Precache();

	SetModel( STRING( GetModelName() ) );

	CNPC_CombineS::Spawn();

	if ( m_intBloodColor != NULL )
		m_bloodColor = m_intBloodColor;

	if ( m_intHealth != NULL )
		m_iHealth = m_intHealth;
	else
		m_iHealth = 500;

	if ( m_intMaxHealth != NULL )
		m_iMaxHealth = m_intMaxHealth;
	else
		m_iMaxHealth = GetHealth();

	// indicates the width of this NPC's forward view cone ( as a dotproduct result )
	if ( m_floatFOV != NULL )
		m_flFieldOfView = m_floatFOV;
	else
		m_flFieldOfView = -0.2;
	
	m_NPCState = NPC_STATE_NONE;

	if ( m_boolPatrool )
		m_bShouldPatrol = true;
	else
		m_bShouldPatrol = false;

	if ( m_intCaps != NULL )
		CapabilitiesAdd( m_intCaps );

	if ( m_strHullName != NULL_STRING )
		SetHullType( NAI_Hull::LookupId( STRING( m_strHullName ) ) );
	else
		SetHullType( HULL_HUMAN );

	SetHullSizeNormal( );

	if ( m_startDisabled )
	{
		AddFlag( EF_NODRAW );
		SetSolid( SOLID_NONE );
	}
}

//=========================================================
// InputEnable
//=========================================================
void CNPC_Custom_Combine::InputEnable( inputdata_t &inputdata )
{
	RemoveFlag( EF_NODRAW );
	SetSolid( SOLID_VPHYSICS );
	m_startDisabled = false;
}

//=========================================================
// InputDisable
//=========================================================
void CNPC_Custom_Combine::InputDisable( inputdata_t &inputdata )
{
	AddFlag( EF_NODRAW );
	SetSolid( SOLID_NONE );
	m_startDisabled = true;
}

//=========================================================
// Event Killed
//=========================================================
void CNPC_Custom_Combine::Event_Killed( const CTakeDamageInfo &info )
{
	if ( m_boolIsBoss )
		m_outputOnKilled.FireOutput( info.GetAttacker(), this );

	BaseClass::Event_Killed( info );
}

envelopePoint_t envMoanIgnited[] =
{
	{	1.0f, 1.0f,
		0.5f, 1.0f,
	},
	{	1.0f, 1.0f,
		30.0f, 30.0f,
	},
	{	0.0f, 0.0f,
		0.5f, 1.0f,
	},
};

//=========================================================
// >> CNPC_Custom_Zombie
//=========================================================
class CNPC_Custom_Zombie : public CAI_BlendingHost<CZombie>
{
	DECLARE_CLASS( CNPC_Custom_Zombie, CAI_BlendingHost<CZombie> );

public:

	CNPC_Custom_Zombie( void );
	~CNPC_Custom_Zombie();


	virtual void NPCThink( void );
	virtual void Precache();
	virtual void Spawn( void );
	virtual Class_T Classify( void );
	virtual void Event_Killed( const CTakeDamageInfo &info );
	virtual int OnTakeDamage_Alive( const CTakeDamageInfo &inputInfo );

	virtual bool ShouldBecomeTorso( const CTakeDamageInfo &info, float flDamageThreshold ) { return false; }
	virtual bool CanBecomeLiveTorso() { return false; }
	virtual const char *GetLegsModel( void ) { return nullptr; }
	virtual const char *GetTorsoModel( void ) { return nullptr; }
	virtual const char *GetHeadcrabClassname( void ) { return nullptr; }
	virtual const char *GetHeadcrabModel( void ) { return nullptr; }

	CNPC_Custom_Zombie * GetEntity() { return this; }

	void InputEnable( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );

private:

	int m_intHealth;
	int m_intMaxHealth;
	int m_intBloodColor;
	int m_intCaps;
	Class_T m_intClassify;
	float m_floatFOV;
	string_t m_strHullName;
	string_t m_strClassName;
	bool m_startDisabled;
	bool m_boolIsBoss;
	COutputEvent m_outputOnKilled;

	bool m_bLosingBlood;
	EHANDLE m_hBleedAttacker;
	float m_flBleedTime;

	DECLARE_DATADESC();
};

BEGIN_DATADESC( CNPC_Custom_Zombie )
	DEFINE_KEYFIELD( m_intHealth, 		FIELD_INTEGER, 	"health" ),
	DEFINE_KEYFIELD( m_intMaxHealth, 	FIELD_INTEGER, 	"maxhealth" ),
	DEFINE_KEYFIELD( m_intBloodColor, 	FIELD_INTEGER, 	"bloodcolor" ),
	DEFINE_KEYFIELD( m_intCaps, 		FIELD_INTEGER, 	"capabilities" ),
	DEFINE_KEYFIELD( m_intClassify, 	FIELD_INTEGER, 	"classify" ),
	DEFINE_KEYFIELD( m_floatFOV, 		FIELD_FLOAT, 	"fieldofview" ),
	DEFINE_KEYFIELD( m_strHullName, 	FIELD_STRING, 	"hullname" ),
	DEFINE_KEYFIELD( m_strClassName, 	FIELD_STRING, 	"class_name" ),
	DEFINE_KEYFIELD( m_startDisabled, 	FIELD_BOOLEAN, 	"start_disabled" ),
	DEFINE_KEYFIELD( m_boolIsBoss, 		FIELD_BOOLEAN, 	"boss_npc" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", 	InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", 	InputDisable ),

	DEFINE_OUTPUT( m_outputOnKilled , "OnKilled" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( npc_custom_zombie, CNPC_Custom_Zombie );

//=========================================================
// Constructor
//=========================================================
CNPC_Custom_Zombie::CNPC_Custom_Zombie( void )
{
	m_startDisabled = false;
	m_boolIsBoss = false;

	m_bLosingBlood = false;
	m_hBleedAttacker = NULL;
	m_flBleedTime = 0.0f;
}

//=========================================================
// Destructor
//=========================================================
CNPC_Custom_Zombie::~CNPC_Custom_Zombie( void )
{
}

//=========================================================
// Precache - precaches all resources this NPC needs
//=========================================================
void CNPC_Custom_Zombie::Precache( void )
{
	PrecacheModel( STRING( GetModelName() ) );

	PrecacheParticleSystem( "blood_advisor_puncture_withdraw" );
	PrecacheScriptSound( "NPC_BaseZombie.HeadSquirt" );

	CZombie::Precache();
}

//=========================================================
// NPCThink
//=========================================================
void CNPC_Custom_Zombie::NPCThink( void )
{
	BaseClass::NPCThink();

	// Blood loss system
	if ( m_bLosingBlood && (gpGlobals->curtime >= m_flBleedTime) && IsAlive() )	// too much blood loss? not dead yet? well, prepare to die zombitch!
	{
		KillMe();	// IT'S OVER.

		// It's important we credit the player who caused the bleeding for their work in killing this zombie
		CTFPlayer *pBleedAttacker = ToTFPlayer( dynamic_cast<CBasePlayer*>( m_hBleedAttacker.Get() ) );
		if ( pBleedAttacker && pBleedAttacker->IsPlayer() )
		{
			pBleedAttacker->IncrementFragCount( 1 );
			GetGlobalTeam( pBleedAttacker->GetTeamNumber() )->AddScore( 1 );
		}

		// Reset all the variables that got us here
		m_bLosingBlood = false;
		m_hBleedAttacker = NULL;
		m_flBleedTime = 0.0f;
	}
}

//=========================================================
// Classify - indicates this NPC's place in the 
// relationship table.
//=========================================================
Class_T	CNPC_Custom_Zombie::Classify( void )
{
	if ( m_intClassify != NULL )
		return m_intClassify;

	return CLASS_ZOMBIE;
}

//=========================================================
// Spawn
//=========================================================
void CNPC_Custom_Zombie::Spawn( void )
{
	if ( m_strClassName != NULL_STRING )
		SetClassname( STRING( m_strClassName ) );

	char *szModel = (char *)STRING( GetModelName() );
	if ( !szModel || !*szModel )
	{
		Warning( "npc_custom_zombie at %.0f %.0f %0.f missing modelname\n", GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z );
		UTIL_Remove( this );
		return;
	}

	Precache();

	m_fIsHeadless = true;	// no headcrabs!

	SetModel( STRING( GetModelName() ) );

	CZombie::Spawn();

	if ( m_intBloodColor != NULL )
		m_bloodColor = m_intBloodColor;
	else
		m_bloodColor = BLOOD_COLOR_RED;

	if ( m_intHealth != NULL )
		m_iHealth = m_intHealth;
	else
		m_iHealth = 500;

	if ( m_intMaxHealth != NULL )
		m_iMaxHealth = m_intMaxHealth;
	else
		m_iMaxHealth = GetHealth();

	// indicates the width of this NPC's forward view cone ( as a dotproduct result )
	if ( m_floatFOV != NULL )
		m_flFieldOfView = m_floatFOV;
	else
		m_flFieldOfView = 0.2;
	
	m_NPCState = NPC_STATE_NONE;

	if ( m_intCaps != NULL )
		CapabilitiesAdd( m_intCaps );

	if ( m_strHullName != NULL_STRING )
		SetHullType( NAI_Hull::LookupId( STRING( m_strHullName ) ) );
	else
		SetHullType( HULL_HUMAN );

	if ( m_startDisabled )
	{
		AddFlag( EF_NODRAW );
		SetSolid( SOLID_NONE );
	}
}


//=========================================================
// InputEnable
//=========================================================
void CNPC_Custom_Zombie::InputEnable( inputdata_t &inputdata )
{
	RemoveFlag( EF_NODRAW );
	SetSolid( SOLID_VPHYSICS );
	m_startDisabled = false;
}

//=========================================================
// InputDisable
//=========================================================
void CNPC_Custom_Zombie::InputDisable( inputdata_t &inputdata )
{
	AddFlag( EF_NODRAW );
	SetSolid( SOLID_NONE );
	m_startDisabled = true;
}

//=========================================================
// On Take Damage Alive
//=========================================================
int CNPC_Custom_Zombie::OnTakeDamage_Alive( const CTakeDamageInfo &inputInfo )
{
	CTakeDamageInfo info = inputInfo;

	if ( info.GetDamageType() & (DMG_BULLET | DMG_BUCKSHOT) )	// creepers handle bullets and buckshot differently depending on where they're hit
	{
		if ( m_bHeadShot )
		{
			info.ScaleDamage( 2.0f );	// headshots hurt us a lot!

			// Blood loss system
			int shouldHeadExplode = random->RandomInt( 1, 10 );	// there's a 10% chance of our head bleeding when we get shot in the head
			if ( shouldHeadExplode == 1 )	// sorry zombie, but here it comes!
			{
				EmitSound( "NPC_BaseZombie.HeadSquirt" ); // the sound...

				DispatchParticleEffect( "blood_advisor_puncture_withdraw", PATTACH_POINT_FOLLOW, this, "eyes" );	// the blood...

				AddFlag( FL_ONFIRE );	// this could have some nasty side-effects, but oh well!
				RemoveSpawnFlags( SF_NPC_GAG );
				MoanSound( envMoanIgnited, ARRAYSIZE( envMoanIgnited ) );
				if ( m_pMoanSound )	// begin moaning like we're on fire since bleeding hurts :(
				{
					ENVELOPE_CONTROLLER.SoundChangePitch( m_pMoanSound, 120, 1.0 );
					ENVELOPE_CONTROLLER.SoundChangeVolume( m_pMoanSound, 1, 1.0 );
				}

				m_bLosingBlood = true;
				m_hBleedAttacker = info.GetAttacker();
				m_flBleedTime = gpGlobals->curtime + 2.0f;	// we're bleeding from our head, which means we should die pretty soon
			}
		}
		else
		{
			info.ScaleDamage( 0.5f );	// other things do not hurt as much (we can survive with damaged limbs and whatnot)
		}
	}

	// Throw some blood on the ground when we take damage (adds some realism)
	trace_t	tr;
	AI_TraceLine( GetAbsOrigin()+Vector(0,0,1), GetAbsOrigin()-Vector(0,0,64), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );
	UTIL_BloodDecalTrace( &tr, BloodColor() );

	return BaseClass::OnTakeDamage_Alive( info );
}

//=========================================================
// Event Killed
//=========================================================
void CNPC_Custom_Zombie::Event_Killed( const CTakeDamageInfo &info )
{
	if ( m_boolIsBoss )
		m_outputOnKilled.FireOutput( info.GetAttacker(), this );

	StopSound( "NPC_BaseZombie.HeadSquirt" );
	StopParticleEffects( this );	// make sure we end any attachment-based particle effects here
									// not doing so would make them float around in the air because we're about to become a ragdoll
									// actually, we may have become one already (not sure how it works), but better late than never

	BaseClass::Event_Killed( info );
}