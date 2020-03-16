#include "cbase.h"
#include "c_zombie.h"


IMPLEMENT_CLIENTCLASS_DT( C_TFZombie, DT_TFZombie, CTFZombie )
	RecvPropInt( RECVINFO( m_nSkeletonType ) ),
	RecvPropInt( RECVINFO( m_nZombieClass ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
C_TFZombie::C_TFZombie()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
C_TFZombie::~C_TFZombie()
{
	if ( m_pCosmetic )
		m_pCosmetic->Release();
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_TFZombie::Spawn( void )
{
	BaseClass::Spawn();

	SetNextClientThink( gpGlobals->curtime + 1.0f );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_TFZombie::ClientThink( void )
{
	if ( m_nZombieClass > TF_CLASS_UNDEFINED )
	{
		if ( !m_pCosmetic )
		{
			m_pCosmetic = new C_BaseAnimating;

			const char *pszZombieModel = "models/player/items/scout/scout_zombie.mdl";
			switch ( m_nZombieClass )
			{
			case TF_CLASS_SCOUT:
				pszZombieModel = "models/player/items/scout/scout_zombie.mdl";
				break;

			case TF_CLASS_SNIPER:
				pszZombieModel = "models/player/items/sniper/sniper_zombie.mdl";
				break;

			case TF_CLASS_SOLDIER:
				pszZombieModel = "models/player/items/soldier/soldier_zombie.mdl";
				break;

			case TF_CLASS_DEMOMAN:
				pszZombieModel = "models/player/items/demo/demo_zombie.mdl";
				break;

			case TF_CLASS_MEDIC:
				pszZombieModel = "models/player/items/medic/medic_zombie.mdl";
				break;

			case TF_CLASS_HEAVYWEAPONS:
				pszZombieModel = "models/player/items/heavy/heavy_zombie.mdl";
				break;

			case TF_CLASS_PYRO:
				pszZombieModel = "models/player/items/pyro/pyro_zombie.mdl";
				break;

			case TF_CLASS_SPY:
				pszZombieModel = "models/player/items/spy/spy_zombie.mdl";
				break;

			case TF_CLASS_ENGINEER:
				pszZombieModel = "models/player/items/engineer/engineer_zombie.mdl";
				break;
			}

			if ( m_pCosmetic->InitializeAsClientEntity( pszZombieModel, RENDER_GROUP_OPAQUE_ENTITY ) == false )
			{	
				m_pCosmetic->Remove();
			}
			else
			{
				m_pCosmetic->FollowEntity( this );
				SetNextClientThink( CLIENT_THINK_NEVER );
			}
		}
	}
	else
	{
		SetNextClientThink( CLIENT_THINK_NEVER );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_TFZombie::FireEvent( const Vector& origin, const QAngle& angle, int event, const char *options )
{
	BaseClass::FireEvent( origin, angle, event, options );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void C_TFZombie::BuildTransformations( CStudioHdr *pStudioHdr, Vector *pos, Quaternion q[], const matrix3x4_t &cameraTransform, int boneMask, CBoneBitList &boneComputed )
{
	BaseClass::BuildTransformations( pStudioHdr, pos, q, cameraTransform, boneMask, boneComputed );

	if ( GetModelScale() <= 0.5f )
	{
		int iBone = LookupBone( "bip_head" );
		if ( iBone != -1 )
			MatrixScaleBy( 1.4f, GetBoneForWrite( iBone ) );

		iBone = LookupBone( "prp_helmet" );
		if ( iBone != -1 )
			MatrixScaleBy( 1.4f, GetBoneForWrite( iBone ) );

		iBone = LookupBone( "prp_hat" );
		if ( iBone != -1 )
			MatrixScaleBy( 1.4f, GetBoneForWrite( iBone ) );

		iBone = LookupBone( "bip_neck" );
		if ( iBone != -1 )
			MatrixScaleBy( 1.4f, GetBoneForWrite( iBone ) );

		m_BoneAccessor.SetWritableBones( BONE_USED_BY_ANYTHING );
	}
}

bool C_TFZombie::ShouldCollide( int collisionGroup, int contentsMask ) const
{
	if ( collisionGroup == COLLISION_GROUP_PLAYER_MOVEMENT )
		return false;

	return BaseClass::ShouldCollide( collisionGroup, contentsMask );
}