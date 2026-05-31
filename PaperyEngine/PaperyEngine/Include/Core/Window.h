#pragma once
#include "Prerequisites.h"

class 
Window {
public:
	Window() = default;
	Window(int width, int height, const std::string& title);
	~Window() = default;

	//void 
	//handleEvents(EngineGUI& engineGUI);

	bool 
	isOpen() const;

	void 
	clear(const sf::Color& color = sf::Color(0, 0, 0, 255));

	void 
	draw(const sf::Drawable & drawable, 
			 const sf::RenderStates& states = sf::RenderStates::Default);

	void 
	display();

	void
	close();

	void
	update();

	void
	render();

	void
	destroy();

public:
	std::unique_ptr<sf::RenderWindow> m_window = nullptr;
private:
	sf::View m_view;
	sf::Time deltaTime;
	sf::Clock clock;
};