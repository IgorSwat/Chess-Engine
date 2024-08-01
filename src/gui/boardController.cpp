#include "boardController.h"


namespace GUI {

	// --------------
	// GUI parameters
	// --------------

	constexpr float TILE_SIZE = 60.f;
	constexpr float NAVBAR_HEIGHT = 60.f;
	constexpr float NAV_BUTTON_SIZE = 30.f;
	constexpr float NAV_BUTTON_ROUND = 0.4f;

	const sf::Color WINDOW_BACKGROUND_COLOR = sf::Color(245, 220, 152, 128);


	// ------------------------------
	// BoardController methods - init
	// ------------------------------

	BoardController::BoardController()
		: board(), 
		  window(sf::VideoMode(TILE_SIZE * 8, TILE_SIZE * 8 + NAVBAR_HEIGHT), "Chess", sf::Style::Close | sf::Style::Titlebar),
		  boardImage(TILE_SIZE), 
		  navbar(sf::Vector2f(0.f, TILE_SIZE * 8), sf::Vector2f(TILE_SIZE * 8, NAVBAR_HEIGHT))
	{
		window.setFramerateLimit(60);

		navbar.addButton(RoundedButton(ButtonType::BACK, NAV_BUTTON_SIZE, NAV_BUTTON_ROUND));
		navbar.addButton(RoundedButton(ButtonType::RESET, NAV_BUTTON_SIZE, NAV_BUTTON_ROUND));

		sf::Cursor cursor;
		if (cursor.loadFromSystem(sf::Cursor::Hand))
			window.setMouseCursor(cursor);

		boardImage.loadPosition(&board);
	}


	// -----------------------------------
	// BoardController methods - main loop
	// -----------------------------------

	// BUG 1 - Ruy Lopez Exchange, pawn capture on c6
	// BUG 2 - Short castle

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
					}
					else
						boardImage.loadPosition(&board);
				}

				// Update navbar
				ButtonType clicked = navbar.update(event, mousePos);
				switch (clicked) {
					case ButtonType::BACK:
						board.undoLastMove();
						// TODO: perform a check if undoLastMove() changed anything
						boardImage.loadPosition(&board);
						break;
					case ButtonType::RESET:
						board.loadPosition();
						boardImage.loadPosition(&board);
						break;
					default:
						break;
				}
			}

			// Render
			window.clear(WINDOW_BACKGROUND_COLOR);
			window.draw(boardImage);
			window.draw(navbar);
			window.display();
		}
	}

}