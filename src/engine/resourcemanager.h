#pragma once

#include <raylib.h>
#include <string>
#include <string_view>
#include <unordered_map>

class ResourceManager {
public:
	ResourceManager() = default;
	~ResourceManager();

	ResourceManager(const ResourceManager&) = delete;
	ResourceManager& operator=(const ResourceManager&) = delete;

	// Loading
	bool LoadTexture(std::string_view id, std::string_view filepath);
	bool LoadSound(std::string_view id, std::string_view filepath);
	bool LoadMusic(std::string_view id, std::string_view filepath);

	// Access
	const Texture2D& GetTexture(std::string_view id) const;
	const Sound& GetSound(std::string_view id) const;
	const Music& GetMusic(std::string_view id) const;

	// Cleanup
	void UnloadTexture(std::string_view id);
	void UnloadSound(std::string_view id);
	void UnloadMusic(std::string_view id);
	void UnloadAll();

	// Utility
	bool HasTexture(std::string_view id) const;
	bool HasSound(std::string_view id) const;
	bool HasMusic(std::string_view id) const;

private:
	std::unordered_map<std::string, Texture2D> textures;
	std::unordered_map<std::string, Sound> sounds;
	std::unordered_map<std::string, Music> musics;
};