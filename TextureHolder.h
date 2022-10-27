#pragma once
#ifndef TEXTURE_HOLDER_H
#define TEXTURE_HOLDER_H
#include <SFML/Graphics.hpp>
#include <map>
using namespace sf;
using namespace std;

class TextureHolder {
private:
	map<string, Texture> m_Textures;//map container
	static TextureHolder* m_s_Instance;//static pointer to the same type as the class itself -> one and only instance

public:
	TextureHolder();
	static Texture& GetTexture(string const& filename);
};
#endif // !TEXTURE_HOLDER_H
