#pragma once

//-----------------------------------------------------------------------------
#ifdef _DEBUG
extern HRESULT _d_hr;
#	define V(x) assert(SUCCEEDED(_d_hr = (x)))
#else
#	define V(x) (x)
#endif

//-----------------------------------------------------------------------------
// Inne typy
#ifndef COMMON_ONLY
typedef FMOD::Sound* SOUND;
#endif
struct Animesh;
struct AnimeshInstance;

//-----------------------------------------------------------------------------
// Typy zmiennych directx
typedef ID3DXFont* FONT;
typedef LPDIRECT3DINDEXBUFFER9 IB;
typedef IDirect3DTexture9* TEX;
typedef IDirect3DSurface9* SURFACE;
typedef LPDIRECT3DVERTEXBUFFER9 VB;

//-----------------------------------------------------------------------------
// funkcja do zwalniania obiektów directx
template<typename T>
inline void SafeRelease(T& x)
{
	if(x)
	{
		x->Release();
		x = nullptr;
	}
}

//-----------------------------------------------------------------------------
// Funkcje dla bullet physics
//-----------------------------------------------------------------------------
inline VEC2 ToVEC2(const btVector3& v)
{
	return VEC2(v.x(), v.z());
}

inline VEC3 ToVEC3(const btVector3& v)
{
	return VEC3(v.x(), v.y(), v.z());
}

inline btVector3 ToVector3(const VEC2& v)
{
	return btVector3(v.x, 0, v.y);
}

inline btVector3 ToVector3(const VEC3& v)
{
	return btVector3(v.x, v.y, v.z);
}

// Function for calculating rotation around point for physic nodes
// from: http://bulletphysics.org/Bullet/phpBB3/viewtopic.php?t=5182
inline void RotateGlobalSpace(btTransform& out, const btTransform& T, const btMatrix3x3& rotationMatrixToApplyBeforeTGlobalSpace,
	const btVector3& centerOfRotationRelativeToTLocalSpace)
{
	// Note:  - centerOfRotationRelativeToTLocalSpace = TRelativeToCenterOfRotationLocalSpace (LocalSpace is relative to the T.basis())
	// Distance between the center of rotation and T in global space
	const btVector3 TRelativeToTheCenterOfRotationGlobalSpace = T.getBasis() * (-centerOfRotationRelativeToTLocalSpace);
	// Absolute position of the center of rotation = Absolute position of T + PositionOfTheCenterOfRotationRelativeToT
	const btVector3 centerOfRotationAbsolute = T.getOrigin() - TRelativeToTheCenterOfRotationGlobalSpace;
	out = btTransform(rotationMatrixToApplyBeforeTGlobalSpace*T.getBasis(),
		centerOfRotationAbsolute + rotationMatrixToApplyBeforeTGlobalSpace * TRelativeToTheCenterOfRotationGlobalSpace);
}

inline void RotateGlobalSpace(btTransform& out, const btTransform& T, const btQuaternion& rotationToApplyBeforeTGlobalSpace,
	const btVector3& centerOfRotationRelativeToTLocalSpace)
{
	RotateGlobalSpace(out, T, btMatrix3x3(rotationToApplyBeforeTGlobalSpace), centerOfRotationRelativeToTLocalSpace);
}
