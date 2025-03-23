#pragma once

#include "../engine/types.h"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>


namespace GUI {

	enum class ButtonType;
	
	namespace Textures {
		
		void load_textures();

		const sf::Texture& get_texture(const std::string& textureName);
		const sf::Texture& get_texture(Piece piece);
		const sf::Texture& get_texture(ButtonType bType);

	}
}