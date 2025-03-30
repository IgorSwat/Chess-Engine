#include "controller.h"
#include <sstream>


namespace GUI {

	// --------------
	// GUI parameters
	// --------------

	// Board tiles
	// - Together define board size
	const float TILE_SIZE = 80.f;
	const float BOARD_SIZE = TILE_SIZE * 8;

	// First (position) navbar
	// - Total navbar height includes bottom-top margines
	const float POS_NAVBAR_HEIGHT = 60.f;
	const float POS_NAVBAR_TOTAL_HEIGHT = POS_NAVBAR_HEIGHT;

	const float NAVBAR_BUTTON_SIZE = 36.f;
	const float NAVBAR_BUTTON_ROUND = 0.4f;

	// First (position) input bar
	// - This one does not include margin, so we need to add margin manually
	const float POS_INPUT_WIDTH = TILE_SIZE * 8 * 0.8f;
	const float POS_INPUT_HEIGHT = 26.f;
	const float POS_INPUT_BOTTOM_MARGIN = 20.f;
	const float POS_INPUT_TOTAL_HEIGHT = POS_INPUT_HEIGHT + POS_INPUT_BOTTOM_MARGIN;
	const unsigned POS_INPUT_FONT_SIZE = 13;

	// Second (engine) navbar
	// - Total navbar width includes bottom-top margines
	// - Button with the same size as in position navbar
	const float ENGINE_NAVBAR_HEIGHT = NAVBAR_BUTTON_SIZE;
	const float ENGINE_NAVBAR_WIDTH = NAVBAR_BUTTON_SIZE;

	const float ENGINE_NAVBAR_LEFT_MARGIN = 140.f;

	// Second (engine's depth) input bar
	const float DEPTH_INPUT_WIDTH = 40.f;
	const float DEPTH_INPUT_HEIGHT = 32.f;
	const float DEPTH_INPUT_LEFT_MARGIN = 20.f;
	const float DEPTH_INPUT_BOTTOM_MARGIN = 10.f;
	const unsigned DEPTH_INPUT_FONT_SIZE = 14;

	// Evaluation display
	const unsigned EVAL_FONT_SIZE = 20;
	const float EVAL_TEXT_LEFT_MARGIN = 40;

	// Global window parameters
	const float WINDOW_TOTAL_WIDTH = TILE_SIZE * 8;
	const float WINDOW_TOTAL_HEIGHT = TILE_SIZE * 8 + POS_NAVBAR_TOTAL_HEIGHT + POS_INPUT_TOTAL_HEIGHT +
													  POS_INPUT_TOTAL_HEIGHT;

	const sf::Color WINDOW_BACKGROUND_COLOR = sf::Color(245, 220, 152, 128);


	// ------------------------------
	// Controller methods - init
	// ------------------------------

	Controller::Controller(Board* board, Engine* engine)
		: board(board), engine(engine),
		  window(sf::VideoMode(WINDOW_TOTAL_WIDTH, WINDOW_TOTAL_HEIGHT), "Chess", sf::Style::Close | sf::Style::Titlebar,
		  		 sf::ContextSettings(0, 0, 2, 1, 1, 0, false)),
		  boardImage(TILE_SIZE), 
		  pos_nav(sf::Vector2f(0.f, BOARD_SIZE), sf::Vector2f(WINDOW_TOTAL_WIDTH, POS_NAVBAR_HEIGHT)),
		  pos_input(sf::Vector2f((WINDOW_TOTAL_WIDTH - POS_INPUT_WIDTH) / 2.f, BOARD_SIZE + POS_NAVBAR_TOTAL_HEIGHT),
		  		   sf::Vector2f(POS_INPUT_WIDTH, POS_INPUT_HEIGHT), POS_INPUT_FONT_SIZE),
		  engine_nav(sf::Vector2f(ENGINE_NAVBAR_LEFT_MARGIN, BOARD_SIZE + POS_NAVBAR_TOTAL_HEIGHT + POS_INPUT_TOTAL_HEIGHT),
		  			 sf::Vector2f(ENGINE_NAVBAR_WIDTH, ENGINE_NAVBAR_HEIGHT)),
		  depth_input(sf::Vector2f(ENGINE_NAVBAR_LEFT_MARGIN + ENGINE_NAVBAR_WIDTH + DEPTH_INPUT_LEFT_MARGIN, 
								   BOARD_SIZE + POS_NAVBAR_TOTAL_HEIGHT + POS_INPUT_TOTAL_HEIGHT),
		  			  sf::Vector2f(DEPTH_INPUT_WIDTH, DEPTH_INPUT_HEIGHT), DEPTH_INPUT_FONT_SIZE),
		  eval_text("Best move:", font, EVAL_FONT_SIZE)
	{
		window.setFramerateLimit(60);

		pos_nav.addButton(RoundedButton(ButtonType::BACK, NAVBAR_BUTTON_SIZE, NAVBAR_BUTTON_ROUND));
		pos_nav.addButton(RoundedButton(ButtonType::RESET, NAVBAR_BUTTON_SIZE, NAVBAR_BUTTON_ROUND));
		pos_nav.addButton(RoundedButton(ButtonType::LOAD, NAVBAR_BUTTON_SIZE, NAVBAR_BUTTON_ROUND));
		pos_nav.addButton(RoundedButton(ButtonType::X, NAVBAR_BUTTON_SIZE, NAVBAR_BUTTON_ROUND));

		engine_nav.addButton(RoundedButton(ButtonType::PLAY, NAVBAR_BUTTON_SIZE, NAVBAR_BUTTON_ROUND));

		font.loadFromFile("resource/octosquares.ttf");
		eval_text.setStyle(sf::Text::Bold);
    	eval_text.setPosition(ENGINE_NAVBAR_LEFT_MARGIN + ENGINE_NAVBAR_WIDTH + DEPTH_INPUT_LEFT_MARGIN + DEPTH_INPUT_WIDTH + EVAL_TEXT_LEFT_MARGIN, 
							  BOARD_SIZE + POS_NAVBAR_TOTAL_HEIGHT + POS_INPUT_TOTAL_HEIGHT + (DEPTH_INPUT_HEIGHT - EVAL_FONT_SIZE) / 3);
    	eval_text.setFillColor(sf::Color::Black);

		sf::Cursor cursor;
		if (cursor.loadFromSystem(sf::Cursor::Hand))
			window.setMouseCursor(cursor);

		boardImage.loadPosition(board);
	}


	// ------------------------------
	// Controller methods - main loop
	// ------------------------------

	void Controller::run()
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
				auto [move, afterPromotion] = boardImage.update(event, mousePos, board);
				if (move != Moves::null) {
					bool legal = board->is_legal_f(move);
					if (legal && move.is_promotion() && !afterPromotion)
						boardImage.setupPromotionScreen(move);
					else if (legal) {
						board->make_move(move);
						boardImage.loadPosition(board);
						eval_text.setString("Best move:");
					}
					else
						boardImage.loadPosition(board);
				}

				// Update pos_nav
				ButtonType clicked = pos_nav.update(event, mousePos);
				switch (clicked) {
					case ButtonType::BACK:
						if (board->last_move() != Moves::null) {
							board->undo_move();
							boardImage.loadPosition(board);
							eval_text.setString("Best move:");
						}
						break;
					case ButtonType::RESET:
						board->load_position();
						boardImage.loadPosition(board);
						eval_text.setString("Best move:");
						break;
					case ButtonType::LOAD:
						if (!pos_input.getInput().empty()) {
							board->load_position(pos_input.getInput());
							boardImage.loadPosition(board);
							eval_text.setString("Best move:");
						}
						break;
					case ButtonType::X:
						pos_input.clear();
						break;
					default:
						break;
				}

				// Update input bar
				pos_input.update(event, mousePos);

				// Update engine navbar
				if (engine_nav.update(event, mousePos) == ButtonType::PLAY) {
					eval_text.setString("Best move:");

					// Read depth from input
					std::string depth_str = depth_input.getInput();
					Search::Depth depth = -1;

					try {
						depth = Search::Depth(std::stoi(depth_str));
					} catch (std::invalid_argument&) {}

					// Perform search and display results
					if (depth >= 0) {
						engine->set_position(*board);
						auto [score, best_move] = engine->evaluate(depth);

						std::ostringstream stream;
						stream << "Best move: " << best_move.from() << best_move.to();
						if (best_move.is_promotion()) {
							PieceType promote_to = best_move.promotion_type();
							std::string notation = promote_to == QUEEN ? "Q" :
												promote_to == ROOK ? "R" :
												promote_to == BISHOP ? "B" : "N";
							stream << "=" << notation;
						}
						stream << "  " << (score >= 0 ? "+" : "") << (float(score) / 100.f);

						eval_text.setString(stream.str());
					}
				}

				// Update depth input
				depth_input.update(event, mousePos);
			}

			// Periodic update
			pos_input.updateTimer();
			depth_input.updateTimer();

			// Render
			window.clear(WINDOW_BACKGROUND_COLOR);
			window.draw(boardImage);
			window.draw(pos_nav);
			window.draw(pos_input);
			window.draw(engine_nav);
			window.draw(depth_input);
			window.draw(eval_text);
			window.display();
		}
	}

}