#pragma once

//-----------------------------------------------------------------------------
// info o kompilacji PCH
#ifdef PRECOMPILED_INFO
#	pragma message("Compiling PCH...")
#endif

//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define STRICT
#ifdef _DEBUG
#	define D3D_DEBUG_INFO
#	include <vld.h>
#endif
#include <d3dx9.h>
#include <wininet.h>
#include <process.h>
#include <RakPeerInterface.h>
#include <RakNetTypes.h>
#include <MessageIdentifiers.h>
#define __BITSTREAM_NATIVE_END
#include <BitStream.h>
#include <fmod.hpp>
#include <btBulletCollisionCommon.h>
#include <BulletCollision\CollisionShapes\btHeightfieldTerrainShape.h>
#include <ctime>
#include <core/Core.h>

//-----------------------------------------------------------------------------
using namespace RakNet;
