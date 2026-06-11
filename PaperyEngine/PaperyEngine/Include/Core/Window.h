#pragma once
#include "Prerequisites.h"

/**
 * @file Window.h
 * @brief Definición de la clase Window, un wrapper para la gestión de la ventana de renderizado de SFML.
 */

 /**
  * @class Window
  * @brief Encapsula una ventana de renderizado (sf::RenderWindow) y proporciona métodos para su gestión y actualización de frames.
  */
class
    Window {
public:
    /**
     * @brief Constructor por defecto.
     */
    Window() = default;

    /**
     * @brief Constructor que inicializa la ventana con dimensiones y título específicos.
     * @param width Ancho de la ventana en píxeles.
     * @param height Alto de la ventana en píxeles.
     * @param title Título que se mostrará en la barra de la ventana.
     */
    Window(int width, int height, const std::string& title);

    /**
     * @brief Destructor por defecto.
     */
    ~Window() = default;

    //void 
    //handleEvents(EngineGUI& engineGUI);

    /**
     * @brief Verifica si la ventana sigue abierta.
     * @return true si la ventana está abierta, false en caso contrario.
     */
    bool
        isOpen() const;

    /**
     * @brief Limpia el contenido de la ventana con un color específico.
     * @param color Color de fondo con el que se limpiará la ventana (negro por defecto).
     */
    void
        clear(const sf::Color& color = sf::Color(0, 0, 0, 255));

    /**
     * @brief Dibuja un objeto de SFML en la ventana.
     * @param drawable Objeto a dibujar (sf::Sprite, sf::Shape, etc.).
     * @param states Estados de renderizado a aplicar (transformaciones, shaders, etc.). Por defecto usa los estados base.
     */
    void
        draw(const sf::Drawable& drawable,
            const sf::RenderStates& states = sf::RenderStates::Default);

    /**
     * @brief Muestra en pantalla todo lo que ha sido dibujado en el buffer oculto durante el frame actual.
     */
    void
        display();

    /**
     * @brief Cierra la ventana actual y detiene el procesamiento de eventos.
     */
    void
        close();

    /**
     * @brief Actualiza la lógica interna de la ventana (como el cálculo del deltaTime).
     */
    void
        update();

    /**
     * @brief Maneja el proceso de renderizado base de la ventana.
     */
    void
        render();

    /**
     * @brief Destruye explícitamente los recursos asociados a la ventana.
     */
    void
        destroy();

public:
    /** @brief Puntero único a la ventana de renderizado subyacente de SFML. */
    std::unique_ptr<sf::RenderWindow> m_window = nullptr;
private:
    /** @brief Vista o cámara actual aplicada a la ventana. */
    sf::View m_view;
    /** @brief Tiempo transcurrido entre el frame anterior y el actual. */
    sf::Time deltaTime;
    /** @brief Reloj utilizado para calcular el tiempo entre frames (delta time). */
    sf::Clock clock;
};