#include "boardController.h"


namespace GUI {

	// --------------
	// GUI parameters
	// --------------

	const float TILE_SIZE = 100.f;
	const float NAVBAR_HEIGHT = 80.f;
	const float NAV_BUTTON_SIZE = 40.f;
	const float NAV_BUTTON_ROUND = 0.4f;
	const float INPUT_BAR_WIDTH = TILE_SIZE * 8 * 0.8f;
	const float INPUT_BAR_HEIGHT = 30.f;
	const float INPUT_BOTTOM_MARGIN = 20.f;
	const unsigned INPUT_FONT_SIZE = 14;

	const float WINDOW_WIDTH = TILE_SIZE * 8;
	const float WINDOW_HEIGHT = TILE_SIZE * 8 + NAVBAR_HEIGHT + INPUT_BAR_HEIGHT;

	const sf::Color WINDOW_BACKGROUND_COLOR = sf::Color(245, 220, 152, 128);


	// ------------------------------
	// BoardController methods - init
	// ------------------------------

	BoardController::BoardController()
		: board(), 
		  window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT + INPUT_BOTTOM_MARGIN), "Chess", sf::Style::Close | sf::Style::Titlebar,
		  		 sf::ContextSettings(0, 0, 2, 1, 1, 0, false)),
		  boardImage(TILE_SIZE), 
		  navbar(sf::Vector2f(0.f, WINDOW_WIDTH), sf::Vector2f(WINDOW_WIDTH, NAVBAR_HEIGHT)),
		  inputBar(sf::Vector2f((WINDOW_WIDTH - INPUT_BAR_WIDTH) / 2.f, WINDOW_HEIGHT - INPUT_BAR_HEIGHT),
		  		   sf::Vector2f(INPUT_BAR_WIDTH, INPUT_BAR_HEIGHT), INPUT_FONT_SIZE)   
	{
		board.loadPosition("rn1q1rk1/ppp1bpp1/5n1p/4p1P1/4p1bP/1PN5/PBPPQP2/2KR1BNR w - - 1 10");
		window.setFramerateLimit(60);

		navbar.addButton(RoundedButton(ButtonType::BACK, NAV_BUTTON_SIZE, NAV_BUTTON_ROUND));
		navbar.addButton(RoundedButton(ButtonType::RESET, NAV_BUTTON_SIZE, NAV_BUTTON_ROUND));
		navbar.addButton(RoundedButton(ButtonType::LOAD, NAV_BUTTON_SIZE, NAV_BUTTON_ROUND));

		sf::Cursor cursor;
		if (cursor.loadFromSystem(sf::Cursor::Hand))
			window.setMouseCursor(cursor);

		boardImage.loadPosition(&board);
	}


	// -----------------------------------
	// BoardController methods - main loop
	// -----------------------------------

	void BoardController::run()
	{
		sf::Event event;

		while (window.isOpen()) {
			// Event processing
			while (window.pollEvent(event)) {
				// Close the window
				if (event.type == sf::Event::Closed)
					window.close();

				// Get the correct mouse position
				sf::Vector2i mousePos = sf::Mouse::getPosition(window);
				if (event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::MouseButtonReleased)
					mousePos = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
				else if (event.type == sf::Event::MouseMoved)
					mousePos = sf::Vector2i(event.mouseMove.x, event.mouseMove.y);

				// Update board image
				auto [move, afterPromotion] = boardImage.update(event, mousePos, &board);
				if (move != Move::null()) {
					bool legal = board.legalityCheckFull(move);
					if (legal && move.isPromotion() && !afterPromotion)
						boardImage.setupPromotionScreen(move);
					else if (legal) {
						board.makeMove(move);
						boardImage.loadPosition(&board);
						runTesters(false, true);
					}
					else
						boardImage.loadPosition(&board);
				}

				// Update navbar
				ButtonType clicked = navbar.update(event, mousePos);
				switch (clicked) {
					case ButtonType::BACK:
						if (board.lastMove() != Move::null()) {
							board.undoLastMove();
							boardImage.loadPosition(&board);
							runTesters(false, false);
						}
						break;
					case ButtonType::RESET:
						board.loadPosition();
						boardImage.loadPosition(&board);
						runTesters(true);
						break;
					case ButtonType::LOAD:
						if (!inputBar.getInput().empty()) {
							board.loadPosition(inputBar.getInput());
							boardImage.loadPosition(&board);
							runTesters(true);
						}
						break;
					default:
						break;
				}

				// Update input bar
				inputBar.update(event, mousePos);
			}

			// Periodic update
			inputBar.updateTimer();

			// Render
			window.clear(WINDOW_BACKGROUND_COLOR);
			window.draw(boardImage);
			window.draw(navbar);
			window.draw(inputBar);
			window.display();
		}
	}


	// ---------------------------------------------
	// BoardController methods - testing environment
	// ---------------------------------------------

	void BoardController::addTester(GuiTesterPtr tester)
	{
		testers.push_back(std::move(tester));
	}

	void BoardController::runTesters(bool init, bool forward)
	{
		for (GuiTesterPtr& tester : testers) {
			if (init)
				tester->initialTest(&board);
			else
				tester->nextTest(&board, forward);
		}
	}

}