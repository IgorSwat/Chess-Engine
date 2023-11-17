#include "textures.h"
#include <iostream>
#include <cstdlib>
#include <exception>


// Global graphical variables
sf::RenderWindow* window = nullptr;
bool mouseLeftButtonClicked = false;


namespace {
	const std::string TEXTURES_ROOT = "resource/";

	const std::string BACK = TEXTURES_ROOT + "back.png";
	const std::string REFRESH = TEXTURES_ROOT + "refresh.png";

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

	const std::string TEXTURE_NAMES[PIECE_RANGE] = {
		"", "Wpawn", "Wknight", "Wbishop", "Wrook", "Wqueen", "Wking", "",
		"", "Bpawn", "Bknight", "Bbishop", "Brook", "Bqueen", "Bking", ""
	};
}

namespace BoardTextures {
	std::unordered_map<std::string, sf::Texture> textures;

	void loadTexture(const std::string& name, const std::string& path)
	{
		textures.insert({ name, {} });
		if (!textures[name].loadFromFile(path))
		{
			std::cerr << "Unable to load texture from: " << path << std::endl;
			exit(EXIT_FAILURE);
		}
	}

	void loadTextures()
	{
		loadTexture("back", BACK);
		loadTexture("refresh", REFRESH);

		loadTexture("squares", SQUARES);
		loadTexture("Wking", WKING);
		loadTexture("Bking", BKING);
		loadTexture("Wknight", WKNIGHT);
		loadTexture("Bknight", BKNIGHT);
		loadTexture("Wbishop", WBISHOP);
		loadTexture("Bbishop", BBISHOP);
		loadTexture("Wrook", WROOK);
		loadTexture("Brook", BROOK);
		loadTexture("Wqueen", WQUEEN);
		loadTexture("Bqueen", BQUEEN);
		loadTexture("Wpawn", WPAWN);
		loadTexture("Bpawn", BPAWN);
	}

	const sf::Texture& getTexture(const std::string& textureName)
	{
		if (textures.find(textureName) == textures.end())
			throw std::invalid_argument("No texture with name: " + textureName);
		return textures[textureName];
	}

	const sf::Texture& pieceTexture(Piece piece)
	{
		return getTexture(TEXTURE_NAMES[piece]);
	}
}