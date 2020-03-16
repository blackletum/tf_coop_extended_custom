//====== Copyright � 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef C_PLAYERATTACHEDMODEL_H
#define C_PLAYERATTACHEDMODEL_H
#ifdef _WIN32
#pragma once
#endif

#define PAM_PERMANENT			-1

// Flags
#define PAM_SPIN_Z				(1<<0)
#define PAM_ROTATE_RANDOMLY		(1<<1)
#define PAM_SCALEUP				(1<<2)
#define PAM_ANIMATE_RANDOMLY	(1<<3)

//-----------------------------------------------------------------------------
// Purpose: A clientside, visual only model that's attached to players
//-----------------------------------------------------------------------------
class C_PlayerAttachedModel : public C_BaseAnimating
{
	DECLARE_CLASS( C_PlayerAttachedModel, C_BaseAnimating );
public:
	static C_PlayerAttachedModel *Create( const char *pszModelName, C_BaseEntity *pParent, int iAttachment, const Vector &vecOffset, float flLifetime = 0.2, int iFlags = 0 );
	static C_PlayerAttachedModel *Create( const char *pszModelName, C_BaseEntity *pParent, float flLifetime = PAM_PERMANENT, int iFlags = 0 );

	bool	Initialize( const char *pszModelName, C_BaseEntity *pParent,
		int iAttachment = -1, bool bBoneMerge = true, const Vector &vecOffset = vec3_origin, float flLifetime = PAM_PERMANENT, int iFlags = 0 );

	void	SetLifetime( float flLifetime );
	void	ClientThink( void );
	void	ApplyBoneMatrixTransform( matrix3x4_t& transform );

	virtual bool ShouldDraw( void );

private:
	float	m_flExpiresAt;
	int		m_iFlags;
	float	m_flRotateAt;
	float	m_flAnimateAt;
	float	m_flScale;
};

#define WPN_PAM_PERMANENT			-1

// Flags
#define WPN_PAM_SPIN_Z				(1<<0)
#define WPN_PAM_ROTATE_RANDOMLY		(1<<1)
#define WPN_PAM_SCALEUP				(1<<2)
#define WPN_PAM_ANIMATE_RANDOMLY	(1<<3)

//-----------------------------------------------------------------------------
// Purpose: A clientside, visual only model that's attached to players
//-----------------------------------------------------------------------------
class C_TFWeaponAttachmentModel : public C_BaseAnimating
{
	DECLARE_CLASS( C_TFWeaponAttachmentModel, C_BaseAnimating );
public:
	static C_TFWeaponAttachmentModel *Create( const char *pszModelName, C_BaseEntity *pParent, int iAttachment, const Vector &vecOffset, float flLifetime = 0.2, int iFlags = 0 );
	static C_TFWeaponAttachmentModel *Create( const char *pszModelName, C_BaseEntity *pParent, float flLifetime = WPN_PAM_PERMANENT, int iFlags = 0 );

	bool	Initialize( const char *pszModelName, C_BaseEntity *pParent,
		int iAttachment = -1, bool bBoneMerge = true, const Vector &vecOffset = vec3_origin, float flLifetime = WPN_PAM_PERMANENT, int iFlags = 0 );

	void	SetLifetime( float flLifetime );
	void	ClientThink( void );
	void	ApplyBoneMatrixTransform( matrix3x4_t& transform );

	virtual bool ShouldDraw( void );

	friend class C_ViewmodelAttachmentModel;
private:
	float	m_flExpiresAt;
	int		m_iFlags;
	float	m_flRotateAt;
	float	m_flAnimateAt;
	float	m_flScale;
};

#endif // C_PLAYERATTACHEDMODEL_H
