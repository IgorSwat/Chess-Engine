#pragma once

#include "boardImage.h"
#include "navbar.h"
#include <memory>


namespace Evaluation { class Evaluator; }

void runGUI();


class BoardController
{
public:
	BoardController(BoardConfig* board, Evaluation::Evaluator* evaluator);
	~BoardController();

	void run();
	void update();	// Updates everything withing the gui

private:
	BoardConfig* config;
	std::unique_ptr<BoardImage> gui;
	std::unique_ptr<Navbar> navbar;
};