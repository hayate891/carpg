#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define STRICT

#include <Common.h>
#include <RakPeerInterface.h>
#include <RakNetTypes.h>
#include <MessageIdentifiers.h>
#define __BITSTREAM_NATIVE_END
#include <PluginInterface2.h>
#include <BitStream.h>
#include <chrono>
#ifdef LINUX
#include <unistd.h>
#else
#include <thread>
#endif
using namespace RakNet;
