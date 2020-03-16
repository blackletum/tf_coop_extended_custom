#ifndef RTIME_H
#define RTIME_H

#ifdef _WIN32
#pragma once
#endif

#include "time.h"

namespace CRTime
{
	static uint32 sm_nTimeCur;
	static void UpdateRealTime() { sm_nTimeCur = time( 0 ); }
	static uint32 RTime32TimeCur() { return sm_nTimeCur; }
};

#endif // !RTIME_H
