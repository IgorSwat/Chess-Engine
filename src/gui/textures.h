#pragma once

#include "../logic/misc.h"
#include <unordered_map>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

extern sf::RenderWindow* window;
extern bool mouseLeftButtonClicked;

namespace BoardTextures {
	extern std::unordered_map<std::string, sf::Texture> textures;

	void loadTextures();
	const sf::Texture& getTexture(const std::string& textureName);
	const sf::Texture& pieceTexture(Piece piece);
}