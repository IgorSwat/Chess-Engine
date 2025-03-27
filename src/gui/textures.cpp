#include "textures.h"
#include "button.h"
#include <iostream>
#include <unordered_map>
#include <cstdlib>
#include <exception>

namespace GUI {
	namespace Textures
	{
		// Texture container
		std::unordered_map<std::string, sf::Texture> textures;

		// --------------------
		// Texture path defines
		// --------------------

		const std::string TEXTURES_ROOT = "resource/";

		const std::string BACK = TEXTURES_ROOT + "back.png";
		const std::string REFRESH = TEXTURES_ROOT + "refresh.png";
		const std::string UPLOAD = TEXTURES_ROOT + "upload.png";
		const std::string X = TEXTURES_ROOT + "x.png";
		const std::string PLAY = TEXTURES_ROOT + "play.png";

		const std::string SQUARES = TEXTURES_ROOT + "squares.png";
		const std::string WKING = TEXTURES_ROOT + "Wking.png";
		const std::string BKING = TEXTURES_ROOT + "Bking.png";
		const std::string WKNIGHT = TEXTURES_ROOT + "Wknight.png";
		const std::string BKNIGHT = TEXTURES_ROOT + "Bknight.png";
		const std::string WBISHOP = TEXTURES_ROOT + "Wbishop.png";
		const std::string BBISHOP = TEXTURES_ROOT + "Bbishop.png";
		const std::string WROOK = TEXTURES_ROOT + "Wrook.png";
		const std::string BROOK = TEXTURES_ROOT + "Brook.png";
		const std::string WQUEEN = TEXTURES_ROOT + "Wqueen.png";
		const std::string BQUEEN = TEXTURES_ROOT + "Bqueen.png";
		const std::string WPAWN = TEXTURES_ROOT + "Wpawn.png";
		const std::string BPAWN = TEXTURES_ROOT + "Bpawn.png";

		const std::string PIECE_TEXTURE_NAMES[PIECE_RANGE] = {
			"", "Wpawn", "Wknight", "Wbishop", "Wrook", "Wqueen", "Wking", "",
			"", "Bpawn", "Bknight", "Bbishop", "Brook", "Bqueen", "Bking", ""
		};

		const std::string BUTTON_TEXTURE_NAMES[int(ButtonType::BUTTON_TYPE_RANGE)] = {
			"", "back", "refresh", "upload", "x", "play"
		};

		// ---------------
		// Texture loading
		// ---------------

		// Load given texture and check for correctness
		void load_texture(const std::string &name, const std::string &path)
		{
			textures.insert({name, {}});
			if (!textures[name].loadFromFile(path)) {
				std::cerr << "Unable to load texture from: " << path << std::endl;
				exit(EXIT_FAILURE);
			}
		}

		void load_textures()
		{
			// !!!
			// Here is a complete list of texture names, which act as references to textures in get_texture() function
			// !!!
			load_texture("back", BACK);
			load_texture("refresh", REFRESH);
			load_texture("upload", UPLOAD);
			load_texture("x", X);
			load_texture("play", PLAY);

			load_texture("squares", SQUARES);
			load_texture("Wking", WKING);
			load_texture("Bking", BKING);
			load_texture("Wknight", WKNIGHT);
			load_texture("Bknight", BKNIGHT);
			load_texture("Wbishop", WBISHOP);
			load_texture("Bbishop", BBISHOP);
			load_texture("Wrook", WROOK);
			load_texture("Brook", BROOK);
			load_texture("Wqueen", WQUEEN);
			load_texture("Bqueen", BQUEEN);
			load_texture("Wpawn", WPAWN);
			load_texture("Bpawn", BPAWN);
		}

		// ---------------
		// Texture getters
		// ---------------

		const sf::Texture& get_texture(const std::string &textureName)
		{
			if (textures.find(textureName) == textures.end())
				throw std::invalid_argument("No texture with name: " + textureName);
			return textures[textureName];
		}

		const sf::Texture& get_texture(Piece piece)
		{
			return get_texture(PIECE_TEXTURE_NAMES[piece]);
		}

		const sf::Texture& get_texture(ButtonType bType)
		{
			return get_texture(BUTTON_TEXTURE_NAMES[int(bType)]);
		}

	}
}
