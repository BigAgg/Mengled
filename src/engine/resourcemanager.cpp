#include "resourcemanager.h"

#include <cassert>

ResourceManager::~ResourceManager() {
	UnloadAll();
}

bool ResourceManager::LoadTexture(std::string_view id, std::string_view filepath){
	// Prevent duplicates
	if (textures.contains(std::string(id)))
		return false;

	Texture2D texture = ::LoadTexture(filepath.data());

	if (texture.id == 0)
		return false;

	textures.emplace(std::string(id), texture);
	return true;
}

bool ResourceManager::LoadSound(std::string_view id, std::string_view filepath){
	// Prevent duplicates
	if (sounds.contains(std::string(id)))
		return false;

	Sound sound = ::LoadSound(filepath.data());

	if (sound.frameCount == 0)
		return false;

	sounds.emplace(std::string(id), sound);
	return true;
}

bool ResourceManager::LoadMusic(std::string_view id, std::string_view filepath){
	// Prevent duplicates
	if (sounds.contains(std::string(id)))
		return false;

	Music music = ::LoadMusicStream(filepath.data());

	if (music.frameCount == 0)
		return false;

	musics.emplace(std::string(id), music);
	return true;
}

const Texture2D& ResourceManager::GetTexture(std::string_view id) const{
	auto it = textures.find(std::string(id));
	assert(it != textures.end() && "Texture not found!");
	return it->second;
}

const Sound& ResourceManager::GetSound(std::string_view id) const{
	auto it = sounds.find(std::string(id));
	assert(it != sounds.end() && "Sound not found!");
	return it->second;
}

const Music& ResourceManager::GetMusic(std::string_view id) const{
	auto it = musics.find(std::string(id));
	assert(it != musics.end() && "Music not found!");
	return it->second;
}

void ResourceManager::UnloadTexture(std::string_view id){
	auto it = textures.find(std::string(id));

	if (it == textures.end())
		return;

	::UnloadTexture(it->second);
	textures.erase(it);
}

void ResourceManager::UnloadSound(std::string_view id){
	auto it = sounds.find(std::string(id));

	if (it == sounds.end())
		return;

	::UnloadSound(it->second);
	sounds.erase(it);
}

void ResourceManager::UnloadMusic(std::string_view id){
	auto it = musics.find(std::string(id));

	if (it == musics.end())
		return;

	::UnloadMusicStream(it->second);
	musics.erase(it);
}

void ResourceManager::UnloadAll(){
	for (auto& [name, texture] : textures) {
		::UnloadTexture(texture);
	}
	textures.clear();
	for (auto& [name, sound] : sounds) {
		::UnloadSound(sound);
	}
	sounds.clear();
	for (auto& [name, music] : musics) {
		::UnloadMusicStream(music);
	}
	musics.clear();
}

bool ResourceManager::HasTexture(std::string_view id) const{
	return textures.contains(std::string(id));
}

bool ResourceManager::HasSound(std::string_view id) const{
	return sounds.contains(std::string(id));
}

bool ResourceManager::HasMusic(std::string_view id) const{
	return musics.contains(std::string(id));
}
