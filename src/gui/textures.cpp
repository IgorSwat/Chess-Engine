#pragma once

#include "textures.h"
#include <iostream>
#include <cstdlib>

namespace {
	const std::string TEXTURES_ROOT = "resource/";
	
	const std::string BACK = TEXTURES_ROOT + "back.png";
	const std::string REFRESH = TEXTURES_ROOT + "refresh.png";
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
}