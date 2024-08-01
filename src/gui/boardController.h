#pragma once

#include "boardImage.h"
#include "navbar.h"
#include "../logic/boardConfig.h"
#include <memory>


namespace GUI {

	// ---------------------
	// BoardController class
	// ---------------------

	// A middleware class between view (boardImage & navbar) and data (board & other stuff))
	class BoardController
	{
	public:
		BoardController();

		// GUI rendering and update
		void run();

	private:
		// Logical board
		BoardConfig board;

		// GUI elements
		sf::RenderWindow window;
		BoardImage boardImage;
		Navbar navbar;
	};

}