#pragma once
#include "Prerequisites.h"
class Window;

/**
 * @file CShape.h
 * @brief Definición de la clase CShape, un contenedor para gestionar figuras geométricas de SFML.
 */

 /**
  * @class CShape
  * @brief Encapsula un objeto sf::Shape de SFML, permitiendo su creación y renderizado basado en un ShapeType.
  */
class
    CShape {
public:
    /**
     * @brief Constructor por defecto.
     */
    CShape() = default;

    /**
     * @brief Constructor que inicializa la forma geométrica según el tipo especificado.
     * @param shapeType El tipo de figura a crear (ej. CIRCLE, RECTANGLE, etc.).
     */
    explicit CShape(ShapeType shapeType);

    /**
     * @brief Destructor por defecto.
     */
    ~CShape() = default;

    /**
     * @brief Dibuja la figura geométrica en la ventana proporcionada.
     * @param window Referencia a la ventana de renderizado (Window) donde se dibujará la forma.
     */
    void
        draw(Window& window);

    /**
     * @brief Obtiene un puntero a la figura geométrica subyacente.
     * @return Puntero sin procesar (raw pointer) al objeto sf::Shape.
     */
    sf::Shape* getShape();

private:
    /**
     * @brief Método de fábrica interno para instanciar la figura de SFML correspondiente.
     * @param shapeType El tipo de figura que se desea crear.
     * @return Un puntero único (unique_ptr) gestionando la nueva instancia de sf::Shape.
     */
    static std::unique_ptr<sf::Shape>
        createShape(ShapeType shapeType);

private:
    /** @brief Puntero único que gestiona el ciclo de vida de la figura geométrica de SFML. */
    std::unique_ptr<sf::Shape> m_shape = nullptr;

    /** @brief Identificador del tipo de figura que representa esta instancia. */
    ShapeType m_shapeType;
};