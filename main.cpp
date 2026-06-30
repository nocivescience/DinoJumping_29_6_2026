#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <iostream>
#include <variant>
#include <random>
#include <string_view>

constexpr std::string_view ROJO_TEXTO = "\033[31m";
constexpr std::string_view RESET_COLOR = "\033[0m";

constexpr int ANCHO_VENTANA = 800;
constexpr int ALTO_VENTANA = 600;

float obtenerAlturaTerreno(float x) {
    return 450.0f + std::sin(x * 0.005f) * 50.0f + std::cos(x * 0.02f) * 20.0f;
}

struct Jugador {
    float x = 150.0f;
    float y = 0.0f;
    float ancho = 40.0f;
    float alto = 40.0f;
    float velY = 0.0f;
    bool enSuelo = false;
};

struct Obstaculo {
    float x;
    float ancho;
    float alto;
};

int main() {
    sf::RenderWindow window(sf::VideoMode({ANCHO_VENTANA, ALTO_VENTANA}), "Rectangulo Runner en C++ y SFML 3");
    window.setFramerateLimit(60);

    Jugador jugador;
    std::vector<Obstaculo> obstaculos = {
        {500.0f, 30.0f, 50.0f},
        {900.0f, 30.0f, 60.0f},
        {1400.0f, 40.0f, 40.0f},
        {1900.0f, 25.0f, 70.0f}
    };

    // --- BUENA PRÁCTICA: El motor aleatorio se inicializa UNA SOLA VEZ aquí fuera ---
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distribucion_distancia(300, 600); // Rango de separación entre obstáculos

    const float gravedad = 0.6f;
    const float fuerzaSalto = -12.0f;
    const float velocidadAvance = 4.0f;

    sf::RectangleShape rectJugador;
    rectJugador.setFillColor(sf::Color::Blue);

    sf::RectangleShape rectObstaculo;
    rectObstaculo.setFillColor(sf::Color::Red);

    while (window.isOpen()) {
        // --- 1. MANEJO DE EVENTOS ---
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Space && jugador.enSuelo) {
                    jugador.velY = fuerzaSalto;
                    jugador.enSuelo = false;
                }
            }
        }

        // --- 2. ACTUALIZAR FÍSICAS ---
        jugador.x += velocidadAvance;
        jugador.velY += gravedad;
        jugador.y += jugador.velY;

        float desplazamientoCamara = jugador.x - 150.0f;

        // --- PERPETUAR OBSTÁCULOS (Reciclaje en C++) ---
        for (auto& obs : obstaculos) {
            // Si el obstáculo ya salió completamente de la pantalla por la izquierda
            if (obs.x < desplazamientoCamara - obs.ancho) {
                
                // Buscamos cuál es el obstáculo que está más adelante en el mapa actualmente
                float mas_adelante = jugador.x;
                for (const auto& o : obstaculos) {
                    if (o.x > mas_adelante) {
                        mas_adelante = o.x;
                    }
                }

                // Generamos una distancia aleatoria usando tu motor
                int separacion_aleatoria = distribucion_distancia(gen);
                
                // Movemos el obstáculo viejo al frente de la fila
                obs.x = mas_adelante + separacion_aleatoria;

                // Opcional: Imprime en consola usando tu color rojo solo cuando se recicla uno
                std::cout << ROJO_TEXTO << "Obstaculo recolocado a la distancia: " << obs.x << RESET_COLOR << std::endl;
            }
        }

        // Colisión con el suelo estable
        float alturaSueloActual = obtenerAlturaTerreno(jugador.x + jugador.ancho / 2.0f) - jugador.alto;
        if (jugador.y >= alturaSueloActual) {
            jugador.y = alturaSueloActual; // Quitamos el "numero_aleatorio" de aquí para evitar saltos raros en el suelo
            jugador.velY = 0.0f;
            jugador.enSuelo = true;
        }

        // --- 3. RENDERIZADO ---
        window.clear(sf::Color(211, 211, 211));

        // Dibujar terreno irregular
        float inicioPantalla = desplazamientoCamara;
        float finPantalla = desplazamientoCamara + ANCHO_VENTANA;

        for (float xTerreno = inicioPantalla; xTerreno < finPantalla; xTerreno += 2.0f) {
            float yTerreno = obtenerAlturaTerreno(xTerreno);
            sf::Vertex linea[] = {
                sf::Vertex{sf::Vector2f{xTerreno - desplazamientoCamara, yTerreno}, sf::Color(105, 105, 105)},
                sf::Vertex{sf::Vector2f{xTerreno - desplazamientoCamara, (float)ALTO_VENTANA}, sf::Color(105, 105, 105)}
            };
            window.draw(linea, 2, sf::PrimitiveType::Lines);
        }

        // Dibujar obstáculos y colisiones
        bool colisionDetectada = false;
        for (const auto &obs : obstaculos) {
            float obsY = obtenerAlturaTerreno(obs.x) - obs.alto;

            rectObstaculo.setPosition({obs.x - desplazamientoCamara, obsY});
            rectObstaculo.setSize({obs.ancho, obs.alto});
            window.draw(rectObstaculo);

            if (jugador.x < obs.x + obs.ancho &&
                jugador.x + jugador.ancho > obs.x &&
                jugador.y < obsY + obs.alto &&
                jugador.y + jugador.alto > obsY) {
                colisionDetectada = true;
            }
        }

        // Dibujar Jugador
        rectJugador.setPosition({jugador.x - desplazamientoCamara, jugador.y});
        rectJugador.setSize({jugador.ancho, jugador.alto});
        window.draw(rectJugador);

        if (colisionDetectada) {
            std::cout << "¡COLISIÓN CON OBSTÁCULO!" << std::endl;
        }

        window.display();
    }

    return 0;
}