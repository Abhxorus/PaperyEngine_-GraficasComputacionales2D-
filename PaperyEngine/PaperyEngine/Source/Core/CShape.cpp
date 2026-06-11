#include "Core/CShape.h"
#include "Core/Window.h"

/**
 * @file CShape.cpp
 * @brief Implementación de los métodos de la clase CShape.
 */

 /**
  * @brief Constructor que inicializa el tipo de forma y crea la instancia de sf::Shape correspondiente.
  * @param shapeType Tipo de forma geométrica a inicializar.
  */
CShape::CShape(ShapeType shapeType) : m_shapeType(shapeType), m_shape(createShape(shapeType)) {
}

/**
 * @brief Dibuja la figura en la ventana proporcionada, verificando primero que la figura exista.
 * @param window Referencia a la ventana de renderizado (Window) donde se dibujará la forma.
 */
void
CShape::draw(Window& window) {
	if (m_shape) {
		window.draw(*m_shape);
	}
}

/**
 * @brief Obtiene el puntero a la figura geométrica subyacente de SFML.
 * @return Puntero sin procesar (raw pointer) al objeto sf::Shape.
 */
sf::Shape* CShape::getShape()
{
	return m_shape.get();
}

/**
 * @brief Método de fábrica que construye e inicializa la figura geométrica solicitada.
 * @details Dependiendo del ShapeType, genera un círculo, rectángulo, triángulo, polígono o línea,
 * asignándole dimensiones, color blanco y una posición predeterminada en pantalla.
 * @param shapeType El identificador del tipo de figura que se instanciará.
 * @return Un puntero único (unique_ptr) que gestiona la nueva instancia de sf::Shape, o nullptr si el tipo es EMPTY.
 */
std::unique_ptr<sf::Shape>
CShape::createShape(ShapeType shapeType) {

	switch (shapeType)
	{
	case EMPTY:
	{
		return nullptr;
	}
	case CIRCLE:
	{
		auto circle = std::make_unique<sf::CircleShape>(50.f);
		circle->setFillColor(sf::Color::White);
		circle->setPosition({ 100.0f, 100.0f });
		return circle;
	}
	case RECTANGLE:
	{
		auto rectangle = std::make_unique<sf::RectangleShape>(sf::Vector2f(100.f, 50.f));
		rectangle->setFillColor(sf::Color::White);
		rectangle->setPosition({ 200.0f, 200.0f });
		return rectangle;
	}
	case TRIANGLE:
	{
		auto triangle = std::make_unique<sf::ConvexShape>(3);
		triangle->setPoint(0, sf::Vector2f(0.f, 0.f));
		triangle->setPoint(1, sf::Vector2f(100.f, 0.f));
		triangle->setPoint(2, sf::Vector2f(50.f, 100.f));
		triangle->setFillColor(sf::Color::White);
		triangle->setPosition({ 300.0f, 300.0f });
		return triangle;
	}
	case POLYGON:
	{
		auto polygon = std::make_unique<sf::ConvexShape>(5);
		polygon->setPoint(0, sf::Vector2f(0.f, 0.f));
		polygon->setPoint(1, sf::Vector2f(100.f, 0.f));
		polygon->setPoint(2, sf::Vector2f(120.f, 50.f));
		polygon->setPoint(3, sf::Vector2f(50.f, 100.f));
		polygon->setPoint(4, sf::Vector2f(-20.f, 50.f));
		polygon->setFillColor(sf::Color::White);
		polygon->setPosition({ 400.0f, 400.0f });
		return polygon;
	}
	case LINE:
	{
		auto line = std::make_unique<sf::RectangleShape>(sf::Vector2f(200.f, 1.0f));
		line->setFillColor(sf::Color::White);
		line->setPosition({ 500.0f, 500.0f });
		return line;
	}
	default:
		return nullptr;
	}
}