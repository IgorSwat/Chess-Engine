#pragma once

#include "boardImage.h"
#include "navbar.h"
#include "inputBar.h"
#include "../logic/boardConfig.h"
#include "../test/guiTester.h"
#include <memory>


namespace GUI {

	// ---------------------
	// BoardController class
	// ---------------------

	using GuiTesterPtr = std::unique_ptr<Testing::GuiTester>;

	// A middleware class between view (boardImage & navbar) and data (board & other stuff))
	class BoardController
	{
	public:
		BoardController();

		// GUI rendering and update
		void run();

		// Testing environment handling
		void addTester(GuiTesterPtr tester);
		void runTesters(bool init = false);

	private:
		// Logical board
		BoardConfig board;

		// GUI elements
		sf::RenderWindow window;
		BoardImage boardImage;
		Navbar navbar;
		InputBar inputBar;

		// Testing environment
		std::vector<GuiTesterPtr> testers;
	};

}