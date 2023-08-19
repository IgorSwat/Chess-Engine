#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

namespace BoardTextures {
	extern std::unordered_map<std::string, sf::Texture> textures;

	void loadTextures();
}