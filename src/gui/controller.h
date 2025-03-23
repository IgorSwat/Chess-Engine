#pragma once

#include "boardimg.h"
#include "navbar.h"
#include "inputBar.h"
#include "../engine/board.h"
#include "../engine/engine.h"
#include <memory>


namespace GUI {

	// ----------------
	// Controller class
	// ----------------

	// A middleware class between view (boardImage & navbar) and data (board & other stuff))
	class Controller
	{
	public:
		Controller(Board* board, Engine* engine);

		// GUI rendering and update
		void run();

	private:
		// Logical board connection
		Board* board;

		// Engine connection
		Engine* engine;

		// GUI elements - window
		sf::RenderWindow window;

		// Gui elements - board
		BoardImage boardImage;

		// GUI elements - board change navbar
		Navbar pos_nav;
		InputBar pos_input;

		// GUI elements - engine evaluation navbar
		Navbar engine_nav;
		InputBar depth_input;
		sf::Font font;
		sf::Text eval_text;
	};

}