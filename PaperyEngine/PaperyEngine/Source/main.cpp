#include "Prerequisites.h"
#include "Core/Window.h"
#include "Core/CShape.h"

/**
 * @file main.cpp
 * @brief Punto de entrada principal de la aplicación y ciclo de ejecución del motor.
 */

 /** @brief Instancia global de la ventana de la aplicación. */
Window g_window(Window(800, 600, "Papery Engine"));

/** @brief Instancia de prueba para una figura geométrica de tipo círculo. */
CShape Circle(ShapeType::CIRCLE);

/** @brief Instancia de prueba para una figura geométrica de tipo línea. */
CShape line(ShapeType::LINE);

/**
 * @brief Libera los recursos globales o realiza tareas de limpieza antes de cerrar la aplicación.
 */
void destroy() {
	//SAFE_PTR_RELEASE(g_window);
}

/**
 * @brief Función principal del programa.
 * @details Inicializa la ventana y los objetos, y mantiene activo el bucle principal (Game Loop)
 * donde se procesan eventos, se actualiza la lógica y se renderizan los gráficos en pantalla.
 * @return Código de salida del programa (0 indica ejecución exitosa).
 */
int
main() {
	// create the window
	//g_window = new Window(800, 600, "My window");
	// set the shape color to green
	Circle.getShape()->setFillColor(sf::Color(100, 250, 50));

	// run the program as long as the window is open
	while (g_window.isOpen()) {
		// check all the window's events that were triggered since the last iteration of the loop
		while (const std::optional event = g_window.m_window->pollEvent()) {
			// "close requested" event: we close the window
			if (event->is<sf::Event::Closed>()) {
				g_window.close();
			}
		}

		// clear the window with black color
		g_window.clear(sf::Color::Black);

		// draw everything here...
		Circle.draw(g_window);
		line.draw(g_window);

		// end the current frame
		g_window.display();
	}
	destroy();
	return 0;
}