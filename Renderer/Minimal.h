#pragma once

#include "Base/Base.h"
#include "Math/Math.h"
#include <windows.h>

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)		{ if(p) { delete p; (p)=NULL; } }
#endif