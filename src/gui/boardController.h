#pragma once

#include "boardImage.h"
#include "navbar.h"
#include <memory>


void runGUI();


class BoardController
{
public:
	BoardController(BoardConfig* board);
	~BoardController();

	void run();
	void update();	// Updates everything withing the gui

private:
	BoardConfig* config;
	std::unique_ptr<BoardImage> gui;
	std::unique_ptr<Navbar> navbar;
};