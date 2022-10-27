#include "TextureHolder.h"
#include <assert.h>

TextureHolder* TextureHolder::m_s_Instance = nullptr;

TextureHolder::TextureHolder() {
	assert(m_s_Instance == nullptr);
		m_s_Instance = this;
}
Texture& TextureHolder::GetTexture(string const& filename) {
	auto& m = m_s_Instance -> m_Textures;// return a reference to m_Textures using m_s_Instance
	auto keyValuePair = m.find(filename);// auto = map<string,Texture>::iterator
	
	if (keyValuePair != m.end()){
		return keyValuePair->second;//return the Texture
	}
	else {//filename not found
		auto& texture = m[filename];
		texture.loadFromFile(filename);
		return texture;
	}
}