#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <iostream>
#include <variant>

// Configuración de la pantalla
const int ANCHO_VENTANA = 800;
const int ALTO_VENTANA = 600;

// Función para obtener la altura del terreno irregular en una coordenada X dada
float obtenerAlturaTerreno(float x)
{
    return 450.0f + std::sin(x * 0.005f) * 50.0f + std::cos(x * 0.02f) * 20.0f;
}

struct Jugador
{
    float x = 150.0f;
    float y = 0.0f;
    float ancho = 40.0f;
    float alto = 40.0f;
    float velY = 0.0f;
    bool enSuelo = false;
};

struct Obstaculo
{
    float x;
    float ancho;
    float alto;
};

int main()
{
    // SFML 3: Se mantiene la creación clásica de la ventana, usando sf::VideoMode
    sf::RenderWindow window(sf::VideoMode({ANCHO_VENTANA, ALTO_VENTANA}), "Rectangulo Runner en C++ y SFML 3");
    window.setFramerateLimit(60);

    // Inicializar entidades
    Jugador jugador;
    std::vector<Obstaculo> obstaculos = {
        {500.0f, 30.0f, 50.0f},
        {900.0f, 30.0f, 60.0f},
        {1400.0f, 40.0f, 40.0f},
        {1900.0f, 25.0f, 70.0f}};

    // Constantes físicas
    const float gravedad = 0.6f;
    const float fuerzaSalto = -12.0f;
    const float velocidadAvance = 4.0f;

    sf::RectangleShape rectJugador;
    rectJugador.setFillColor(sf::Color::Blue);

    sf::RectangleShape rectObstaculo;
    rectObstaculo.setFillColor(sf::Color::Red);

    while (window.isOpen())
    {
        // --- 1. MANEJO DE EVENTOS (NUEVO EN SFML 3) ---
        // SFML 3 usa std::optional y std::variant. Desaparece window.pollEvent(event).
        while (const std::optional<sf::Event> event = window.pollEvent())
        {

            // Comprobamos el tipo de evento usando std::holds_alternative o sus structs dedicados
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }

            // Nueva forma de detectar pulsaciones de teclas en SFML 3
            if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->code == sf::Keyboard::Key::Space && jugador.enSuelo)
                {
                    jugador.velY = fuerzaSalto;
                    jugador.enSuelo = false;
                }
            }
        }

        // --- 2. ACTUALIZAR FÍSICAS ---
        jugador.x += velocidadAvance;
        jugador.velY += gravedad;
        jugador.y += jugador.velY;

        float alturaSueloActual = obtenerAlturaTerreno(jugador.x + jugador.ancho / 2.0f) - jugador.alto;

        if (jugador.y >= alturaSueloActual)
        {
            jugador.y = alturaSueloActual;
            jugador.velY = 0.0f;
            jugador.enSuelo = true;
        }

        float desplazamientoCamara = jugador.x - 150.0f;

        // --- 3. RENDERIZADO ---
        window.clear(sf::Color(211, 211, 211));

        // Dibujar terreno irregular
        float inicioPantalla = desplazamientoCamara;
        float finPantalla = desplazamientoCamara + ANCHO_VENTANA;

        for (float xTerreno = inicioPantalla; xTerreno < finPantalla; xTerreno += 2.0f)
        {
            float yTerreno = obtenerAlturaTerreno(xTerreno);

            // SFML 3: Se interactúa igual con sf::Vertex, pero se suelen usar sf::Vector2f de forma más estricta
            sf::Vertex linea[] = {
                sf::Vertex{sf::Vector2f{xTerreno - desplazamientoCamara, yTerreno}, sf::Color(105, 105, 105)},
                sf::Vertex{sf::Vector2f{xTerreno - desplazamientoCamara, (float)ALTO_VENTANA}, sf::Color(105, 105, 105)}};
            window.draw(linea, 2, sf::PrimitiveType::Lines); // SFML 3 usa la versión fuertemente tipada de PrimitiveType
        }

        // Dibujar obstáculos y colisiones
        bool colisionDetectada = false;
        for (const auto &obs : obstaculos)
        {
            float obsY = obtenerAlturaTerreno(obs.x) - obs.alto;

            rectObstaculo.setPosition({obs.x - desplazamientoCamara, obsY});
            rectObstaculo.setSize({obs.ancho, obs.alto});
            window.draw(rectObstaculo);

            if (jugador.x < obs.x + obs.ancho &&
                jugador.x + jugador.ancho > obs.x &&
                jugador.y < obsY + obs.alto &&
                jugador.y + jugador.alto > obsY)
            {
                colisionDetectada = true;
            }
        }

        // Dibujar Jugador
        rectJugador.setPosition({jugador.x - desplazamientoCamara, jugador.y});
        rectJugador.setSize({jugador.ancho, jugador.alto});
        window.draw(rectJugador);

        if (colisionDetectada)
        {
            std::cout << "¡COLISIÓN CON OBSTÁCULO!" << std::endl;
        }

        window.display();
    }

    return 0;
}