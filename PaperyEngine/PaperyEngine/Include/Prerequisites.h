/**
 * @file Prerequisites.h
 * @brief Archivo de cabecera con inclusiones estándar, macros de utilidad y enumeraciones globales.
 */

#pragma once

 // Librerias STD
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <thread>
#include <map>
#include <fstream> 
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <limits>
#include <cassert>
#include <utility>
#include <tuple>
#include <cstddef>
#include <queue>

// Third Parties
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

/**
 * @brief Macro para la liberación segura de memoria de punteros.
 * * Verifica si el puntero no es nulo, libera la memoria asignada y lo establece a nullptr
 * para evitar punteros colgantes (dangling pointers).
 * * @param x Puntero que se desea liberar de forma segura.
 */
#define SAFE_PTR_RELEASE(x) if(x != nullptr) { delete x; x = nullptr; }

 /**
  * @brief Macro para registrar mensajes sobre la creación de recursos o cambios de estado.
  * * Construye y emite un mensaje formateado a la salida de error estándar (std::cerr).
  * * @param classObj Nombre de la clase u objeto que emite el mensaje.
  * @param method Nombre del método desde donde se llama la macro.
  * @param state Descripción del estado o del recurso que se está creando.
  */
#define MESSAGE(classObj, method, state)                      \
{                                                             \
    std::ostringstream os_;                                   \
    os_ << classObj << "::" << method << " : "                \
        << "[CREATION OF RESOURCE" << ": " << state "] \n";\
    std::cerr << os_.str();                                   \
}

  /**
   * @brief Macro para registrar errores críticos y terminar la ejecución del programa.
   * * Construye y emite un mensaje de error formateado a la salida de error estándar (std::cerr),
   * indicando un problema con los parámetros proporcionados, y luego aborta la ejecución con exit(1).
   * * @param classObj Nombre de la clase u objeto donde ocurrió el error.
   * @param method Nombre del método donde ocurrió el error.
   * @param errorMSG Mensaje descriptivo indicando el motivo del error.
   */
#define ERROR(classObj, method, errorMSG)                         \
{                                                                 \
    std::ostringstream os_;                                       \
    os_ << "ERROR : " << classObj << "::" << method << " : "      \
        << "  Error in data from params [" << errorMSG"] \n"; \
    std::cerr << os_.str();                                       \
    exit(1);                                                      \
}

   /**
    * @brief Enumeración que define los diferentes tipos de formas geométricas disponibles.
    * * Utilizada para identificar el tipo específico de una figura dentro del sistema.
    */
enum ShapeType {
    EMPTY = 0,      /**< Representa la ausencia de forma o una figura no inicializada. */
    CIRCLE = 1,     /**< Representa una forma circular. */
    RECTANGLE = 2,  /**< Representa una forma rectangular. */
    TRIANGLE = 3,   /**< Representa una forma triangular. */
    POLYGON = 4,    /**< Representa un polígono genérico de múltiples lados. */
    LINE = 5        /**< Representa una línea recta. */
};