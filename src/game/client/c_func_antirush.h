//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: func_antirush clientside thing header
//
//=============================================================================//

#ifndef C_FUNC_ANTIRUSH_H
#define C_FUNC_ANTIRUSH_H
#ifdef _WIN32
#pragma once
#endif

class C_FuncAntirush : public C_BaseEntity
{
public:
	DECLARE_CLASS(C_FuncAntirush, C_BaseEntity);
	DECLARE_CLIENTCLASS();

	void OnDataChanged(DataUpdateType_t type);

private:
};

#endif	// C_FUNC_ANTIRUSH_H

