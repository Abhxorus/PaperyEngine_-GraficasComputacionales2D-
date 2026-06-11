#include "Core/Window.h"

/**
 * @file Window.cpp
 * @brief Implementación de los métodos de la clase Window.
 */

 /**
  * @brief Constructor que inicializa la ventana de renderizado de SFML.
  * @details Crea la instancia de sf::RenderWindow con las dimensiones y el título especificados.
  * Establece un límite de 60 fotogramas por segundo y registra el estado de la creación mediante macros.
  * @param width Ancho de la ventana en píxeles.
  * @param height Alto de la ventana en píxeles.
  * @param title Título que aparecerá en la barra de la ventana.
  */
Window::Window(int width, int height, const std::string& title) {

	m_window = std::make_unique<sf::RenderWindow>(sf::VideoMode({ static_cast<unsigned int>(width),
																																static_cast<unsigned int>(height) }),
		title,
		sf::Style::Default);
	if (m_window) {
		m_window->setFramerateLimit(60);
		MESSAGE("Window", "Window", "Window created successfully");

	}
	else {
		ERROR("Window", "Window", "Failed to create window");

	}
}

/**
 * @brief Verifica de forma segura si la ventana está inicializada y abierta.
 * @return true si el puntero es válido y la ventana está abierta, false en caso contrario (registra error).
 */
bool
Window::isOpen() const {
	// Check that window is not null
	if (m_window) {
		return m_window && m_window->isOpen();
	}
	else {
		ERROR("Window", "isOpen", "Window is null");
		return false;
	}
}

/**
 * @brief Limpia el búfer de la ventana con el color especificado.
 * @param color Color de limpieza (negro por defecto según la cabecera).
 */
void
Window::clear(const sf::Color& color) {
	if (m_window) {
		m_window->clear(color);
	}
	else {
		ERROR("Window", "clear", "Window is null");
	}
}

/**
 * @brief Dibuja un objeto renderizable en el búfer oculto de la ventana.
 * @param drawable Objeto a dibujar (debe heredar de sf::Drawable).
 * @param states Estados adicionales de renderizado (sf::RenderStates).
 */
void
Window::draw(const sf::Drawable& drawable, const sf::RenderStates& states) {
	if (m_window) {
		m_window->draw(drawable, states);
	}
	else {
		ERROR("Window", "draw", "Window is null");
	}
}

/**
 * @brief Intercambia los búferes y muestra lo dibujado en el fotograma actual.
 */
void
Window::display() {
	if (m_window) {
		m_window->display();
	}
	else {
		ERROR("Window", "display", "Window is null");
	}
}

/**
 * @brief Cierra la ventana y detiene la recepción de eventos.
 */
void
Window::close()
{
	if (m_window) {
		m_window->close();
	}
	else {
		ERROR("Window", "close", "Window is null");
	}
}

/**
 * @brief Actualiza la lógica de tiempo de la ventana calculando el deltaTime del fotograma.
 */
void
Window::update() {
	// Almacena el deltaTime una sola vez
	deltaTime = clock.restart();
}

/**
 * @brief Método base para manejar la lógica de renderizado interno de la ventana si es necesario.
 */
void
Window::render() {
}

/**
 * @brief Destruye explícitamente el objeto sf::RenderWindow subyacente liberando la memoria del puntero único.
 */
void
Window::destroy() {
	m_window.reset();
}