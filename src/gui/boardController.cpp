#include "boardController.h"
#include "../logic/boardConfig.h"
#include "../engine/evaluation.h"

namespace {
	constexpr int BOARD_SIZE = 800;
	constexpr int NAVBAR_HEIGHT = 60;
	const sf::Vector2f NAVBAR_POS = { 0.f, BOARD_SIZE };
	const sf::Vector2f NAVBAR_SIZE = { BOARD_SIZE, NAVBAR_HEIGHT };

	constexpr int ANTIALIASING_LEVEL = 2;

	const std::string STARTING_POS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
}


void runGUI()
{
	BoardTextures::loadTextures();
	BoardConfig config;

	//config.loadFromFen("4k3/2r1p2p/6p1/8/8/6PP/1R2PPK1/8 w - - 0 1");		// Rook & 4 pawns vs rook & 3 pawns drawish endgame (+0.5)
	//config.loadFromFen("8/3k2p1/3p3p/8/8/B7/4KPP1/8 w - - 0 1");		// Bishop vs pawn endgame (+3.5)
	//config.loadFromFen("4k3/5pp1/4n3/8/8/4P3/R2K2P1/8 w - - 0 1");		// Rook vs Knight endgame (+2.2)
	config.loadFromFen("8/4pkp1/3p4/8/8/8/4NK2/8 w - - 0 1");		// Knight vs 3 pawn drawish endgam (-0.5)
	// config.loadFromFen("3kb3/2p5/1p6/pP4p1/P5Pp/7P/1P2B3/2K5 w - - 0 1");
	std::unique_ptr<Evaluation::Evaluator> evaluator = std::make_unique<Evaluation::Evaluator>(&config);
	BoardController controller = { &config, evaluator.get()};
	controller.run();
}




BoardController::BoardController(BoardConfig* board, Evaluation::Evaluator* evaluator)
	: config(board), gui(std::make_unique<BoardImage>(this, evaluator)),
	  navbar(std::make_unique<Navbar>(NAVBAR_POS, NAVBAR_SIZE))
{
	gui->loadPosition(config);
	navbar->addButton(Button(ButtonType::BACK, 40.f, BoardTextures::getTexture("back")));
	navbar->addButton(Button(ButtonType::RESET, 40.f, BoardTextures::getTexture("refresh")));

	sf::ContextSettings settings;
	settings.antialiasingLevel = ANTIALIASING_LEVEL;
	window = new sf::RenderWindow(sf::VideoMode(BOARD_SIZE, BOARD_SIZE + NAVBAR_HEIGHT), "Chess", sf::Style::Close | sf::Style::Titlebar, settings);
	window->setFramerateLimit(60);

	sf::Cursor cursor;
	if (cursor.loadFromSystem(sf::Cursor::Hand))
		window->setMouseCursor(cursor);
}

BoardController::~BoardController()
{
	delete window;
}

void BoardController::run()
{
	sf::Event event;
	while (window->isOpen())
	{
		while (window->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window->close();
		}
		window->clear(sf::Color(245, 220, 152, 128));
		update();
		window->draw(*gui.get());
		window->draw(*navbar.get());
		window->display();
	}
}

void BoardController::update()
{
	if (gui->isWaitingForPromotion()) {
		gui->updatePromotion(config);
		return;
	}
	gui->updatePieces(config);
	ButtonType buttonClicked = navbar->updateButtonsStates();
	if (buttonClicked == ButtonType::BACK) {
		config->undoLastMove();
		gui->loadPosition(config);
	}
	else if (buttonClicked == ButtonType::RESET) {
		config->loadFromFen(STARTING_POS);
		gui->loadPosition(config);
	}
	mouseLeftButtonClicked = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
}