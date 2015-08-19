#pragma once

template<typename T>
class Resource2
{
public:
	enum class Type
	{
		Mesh,
		Texture,
		Sound,
		Music
	};

	string id, path;
	Type type;
	T data;
	int refs;
};

typedef Resource2<void*> AnyResource;
typedef Resource2<TEX> TextureResource;
typedef Resource2<Animesh*> MeshResource;
typedef Resource2<SOUND> SoundResource;

class ResourceManager
{
public:
	AnyResource* Find(cstring id);

private:
	std::map<cstring, AnyResource*> resources;
	AnyResource* last_resource;
};
