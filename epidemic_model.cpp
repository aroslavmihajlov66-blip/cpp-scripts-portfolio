#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <optional>
#include <cmath>

using namespace std;

const int CELL_SIZE = 5;
const int WIDTH = 200;
const int HEIGHT = 130;
const int WIN_W = WIDTH * CELL_SIZE;
const int WIN_H = HEIGHT * CELL_SIZE;

enum State { HEALTHY, SICK, RECOVERED };
enum Terrain { OCEAN, PLAINS, FOREST, MOUNTAIN, DESERT, CITY, VILLAGE, ROAD };

struct Cell {
    State state;
    Terrain terrain;
    int sickDays;
    float population;      // плотность населения
    float spreadModifier;  // модификатор распространения
    
    Cell() : state(HEALTHY), terrain(OCEAN), sickDays(0), population(0), spreadModifier(0) {}
};

// Генератор континента (используем шум Перлина упрощенно)
class ContinentGenerator {
public:
    static vector<vector<Terrain>> generate(int width, int height) {
        vector<vector<Terrain>> map(height, vector<Terrain>(width, OCEAN));
        
        // Создаем несколько континентов
        
        // ===== КОНТИНЕНТ 1: Большой материк (центр) =====
        int centerX = width / 2;
        int centerY = height / 2;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                // Эллиптическая форма материка
                float dx = (x - centerX) / 45.0;
                float dy = (y - centerY) / 35.0;
                float dist = dx*dx + dy*dy + sin(x*0.02) * 0.1 + cos(y*0.02) * 0.1;
                
                if (dist < 1.1) {
                    map[y][x] = PLAINS;
                }
                else if (dist < 1.3 && rand() % 100 < 70) {
                    map[y][x] = PLAINS;
                }
            }
        }
        
        // ===== КОНТИНЕНТ 2: Северный остров =====
        for (int y = 10; y < 45; y++) {
            for (int x = 30; x < 70; x++) {
                float dx = (x - 50) / 20.0;
                float dy = (y - 27) / 15.0;
                if (dx*dx + dy*dy < 0.9) {
                    if (map[y][x] == OCEAN) map[y][x] = FOREST;
                }
            }
        }
        
        // ===== КОНТИНЕНТ 3: Южный остров =====
        for (int y = 90; y < 120; y++) {
            for (int x = 120; x < 170; x++) {
                float dx = (x - 145) / 22.0;
                float dy = (y - 105) / 15.0;
                if (dx*dx + dy*dy < 0.8) {
                    if (map[y][x] == OCEAN) map[y][x] = PLAINS;
                }
            }
        }
        
        // ===== КОНТИНЕНТ 4: Восточный архипелаг =====
        for (int island = 0; island < 8; island++) {
            int ix = 160 + rand() % 30;
            int iy = 20 + rand() % 100;
            for (int y = iy-4; y <= iy+4; y++) {
                for (int x = ix-4; x <= ix+4; x++) {
                    if (x >= 0 && x < width && y >= 0 && y < height) {
                        float dx = (x - ix) / 4.0;
                        float dy = (y - iy) / 4.0;
                        if (dx*dx + dy*dy < 0.8 && map[y][x] == OCEAN) {
                            map[y][x] = (rand() % 2 == 0) ? FOREST : PLAINS;
                        }
                    }
                }
            }
        }
        
        // ===== КОНТИНЕНТ 5: Западный полуостров =====
        for (int y = 50; y < 85; y++) {
            for (int x = 5; x < 40; x++) {
                float dx = (x - 20) / 18.0;
                float dy = (y - 67) / 16.0;
                if (dx*dx + dy*dy < 0.7) {
                    if (map[y][x] == OCEAN) map[y][x] = PLAINS;
                }
            }
        }
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (map[y][x] != OCEAN) {
                    // Случайные города
                    if (rand() % 200 < 3) {
                        map[y][x] = CITY;
                        // Разрастание города
                        for (int dy = -2; dy <= 2; dy++) {
                            for (int dx = -2; dx <= 2; dx++) {
                                int nx = x + dx;
                                int ny = y + dy;
                                if (nx >= 0 && nx < width && ny >= 0 && ny < height && map[ny][nx] != OCEAN) {
                                    if (abs(dx) + abs(dy) <= 2 && rand() % 100 < 60) {
                                        map[ny][nx] = CITY;
                                    }
                                }
                            }
                        }
                    }
                    // Деревни
                    else if (rand() % 50 < 2) {
                        map[y][x] = VILLAGE;
                    }
                    // Горы
                    else if (rand() % 100 < 5) {
                        map[y][x] = MOUNTAIN;
                    }
                    // Пустыни
                    else if (rand() % 100 < 8 && abs(y - height/2) < 30) {
                        map[y][x] = DESERT;
                    }
                }
            }
        }
        
        // Добавляем дороги между городами (упрощенно)
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (map[y][x] == CITY) {
                    for (int dy = -1; dy <= 1; dy++) {
                        for (int dx = -1; dx <= 1; dx++) {
                            int nx = x + dx;
                            int ny = y + dy;
                            if (nx >= 0 && nx < width && ny >= 0 && ny < height && map[ny][nx] == PLAINS) {
                                if (rand() % 100 < 30) map[ny][nx] = ROAD;
                            }
                        }
                    }
                }
            }
        }
        
        return map;
    }
    
    static void setupTerrain(Cell& cell, Terrain t) {
        cell.terrain = t;
        switch(t) {
            case OCEAN:
                cell.population = 0;
                cell.spreadModifier = 0;
                break;
            case PLAINS:
                cell.population = 0.5;
                cell.spreadModifier = 1.0;
                break;
            case FOREST:
                cell.population = 0.2;
                cell.spreadModifier = 0.5;
                break;
            case MOUNTAIN:
                cell.population = 0.05;
                cell.spreadModifier = 0.2;
                break;
            case DESERT:
                cell.population = 0.1;
                cell.spreadModifier = 0.4;
                break;
            case CITY:
                cell.population = 0.9;
                cell.spreadModifier = 1.5;
                break;
            case VILLAGE:
                cell.population = 0.3;
                cell.spreadModifier = 0.8;
                break;
            case ROAD:
                cell.population = 0;
                cell.spreadModifier = 0;
                break;
        }
    }
    
    static sf::Color getColor(Cell& cell) {
        // Сначала состояние эпидемии
        if (cell.state == SICK) return sf::Color(255, 50, 50);
        if (cell.state == RECOVERED) return sf::Color(160, 100, 200);
        
        // Цвет ландшафта
        switch(cell.terrain) {
            case OCEAN:     return sf::Color(40, 80, 150);
            case PLAINS:    return sf::Color(100, 180, 80);
            case FOREST:    return sf::Color(40, 120, 40);
            case MOUNTAIN:  return sf::Color(120, 100, 80);
            case DESERT:    return sf::Color(210, 180, 100);
            case CITY:      return sf::Color(80, 80, 100);
            case VILLAGE:   return sf::Color(150, 130, 80);
            case ROAD:      return sf::Color(70, 70, 70);
            default:        return sf::Color(100, 100, 100);
        }
    }
};

int main() {
    srand(time(nullptr));
    
    sf::VideoMode videoMode(sf::Vector2u(WIN_W, WIN_H));
    sf::RenderWindow window(videoMode, "World Epidemic - Continental Map");
    window.setFramerateLimit(60);
    
    // Генерация мира
    auto terrainMap = ContinentGenerator::generate(WIDTH, HEIGHT);
    vector<vector<Cell>> grid(HEIGHT, vector<Cell>(WIDTH));
    
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            ContinentGenerator::setupTerrain(grid[y][x], terrainMap[y][x]);
            grid[y][x].state = HEALTHY;
            grid[y][x].sickDays = 0;
        }
    }
    
    bool paused = false;
    int step = 0;
    
    cout << "\n========================================" << endl;
    cout << "     МИРОВАЯ ЭПИДЕМИЯ - КОНТИНЕНТЫ" << endl;
    cout << "========================================" << endl;
    cout << "\n ЛЕГЕНДА КАРТЫ МИРА:" << endl;
    cout << "   Океан - не заселен" << endl;
    cout << "   Равнины - средняя плотность" << endl;
    cout << "   Леса - низкая плотность" << endl;
    cout << "   Горы - очень низкая плотность" << endl;
    cout << "   Пустыня - низкая плотность" << endl;
    cout << "   Город - ВЫСОКАЯ ПЛОТНОСТЬ" << endl;
    cout << "   Деревня - средняя плотность" << endl;
    cout << "   Дороги - ускоряют распространение" << endl;
    cout << "\n ЦВЕТА ЭПИДЕМИИ:" << endl;
    cout << "   🟢 Зеленый - здоровые" << endl;
    cout << "   🔴 КРАСНЫЙ - БОЛЬНЫЕ" << endl;
    cout << "   🟣 Сиреневый - переболевшие" << endl;
    cout << "\n📌 ВАЖНО:" << endl;
    cout << "   ОКЕАН не заражается и не распространяет болезнь" << endl;
    cout << "   Эпидемия НЕ ПЕРЕПЛЫВЕТ через океан!" << endl;
    cout << "   Каждый континент живет своей эпидемией" << endl;
    cout << "\nИНСТРУКЦИЯ:" << endl;
    cout << "   КЛИКНИТЕ МЫШКОЙ на любом континенте" << endl;
    cout << "   (можно кликнуть несколько раз на разных континентах)" << endl;
    cout << "\nУПРАВЛЕНИЕ:" << endl;
    cout << "   Пробел - пауза" << endl;
    cout << "   R - перезапуск всего мира" << endl;
    cout << "   C - очистить болезнь (оставить карту)" << endl;
    cout << "   Esc - выход" << endl;
    cout << "\n========================================\n" << endl;
    
    sf::Clock clock;
    int frameDelay = 50;
    
    while (window.isOpen()) {
        while (const optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape) window.close();
                else if (key->code == sf::Keyboard::Key::Space) {
                    paused = !paused;
                    cout << (paused ? "⏸ ПАУЗА" : "▶ ЗАПУЩЕНО") << endl;
                }
                else if (key->code == sf::Keyboard::Key::R) {
                    for (int y = 0; y < HEIGHT; y++)
                        for (int x = 0; x < WIDTH; x++)
                            grid[y][x].state = HEALTHY;
                    step = 0;
                    cout << "🌍 ПЕРЕЗАПУСК МИРА" << endl;
                }
                else if (key->code == sf::Keyboard::Key::C) {
                    for (int y = 0; y < HEIGHT; y++)
                        for (int x = 0; x < WIDTH; x++)
                            if (grid[y][x].terrain != OCEAN)
                                grid[y][x].state = HEALTHY;
                    step = 0;
                    cout << "🧹 ОЧИСТКА БОЛЕЗНИ" << endl;
                }
            }
            else if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouse->button == sf::Mouse::Button::Left) {
                    int x = mouse->position.x / CELL_SIZE;
                    int y = mouse->position.y / CELL_SIZE;
                    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
                        if (grid[y][x].terrain != OCEAN) {
                            grid[y][x].state = SICK;
                            grid[y][x].sickDays = 0;
                            cout << " ОЧАГ НА КОНТИНЕНТЕ (" << x << ", " << y << ")" << endl;
                        } else {
                            cout << "💧 НЕЛЬЗЯ - ЭТО ОКЕАН!" << endl;
                        }
                    }
                }
            }
        }
        
        // Симуляция
        if (!paused && clock.getElapsedTime().asMilliseconds() >= frameDelay) {
            auto newState = grid;
            int sickCount = 0;
            
            for (int y = 0; y < HEIGHT; y++) {
                for (int x = 0; x < WIDTH; x++) {
                    if (grid[y][x].terrain == OCEAN) continue;
                    
                    if (grid[y][x].state == SICK) {
                        sickCount++;
                        grid[y][x].sickDays++;
                        
                        // Выздоровление
                        double recoverChance = 0.04;
                        if (grid[y][x].terrain == CITY) recoverChance = 0.06;
                        if ((double)rand() / RAND_MAX < recoverChance) {
                            newState[y][x].state = RECOVERED;
                        }
                        
                        // Заражение соседей (только по суше!)
                        int radius = 2;
                        for (int dy = -radius; dy <= radius; dy++) {
                            for (int dx = -radius; dx <= radius; dx++) {
                                if (dx == 0 && dy == 0) continue;
                                int nx = x + dx;
                                int ny = y + dy;
                                if (nx >= 0 && nx < WIDTH && ny >= 0 && ny < HEIGHT) {
                                    // КЛЮЧЕВОЕ УСЛОВИЕ: не распространяемся через океан
                                    if (grid[ny][nx].terrain != OCEAN && newState[ny][nx].state == HEALTHY) {
                                        double dist = sqrt(dx*dx + dy*dy);
                                        double prob = 0.35 / (dist + 0.5);
                                        
                                        // Модификаторы от местности
                                        prob *= grid[ny][nx].spreadModifier;
                                        if (grid[ny][nx].terrain == CITY) prob *= 1.5;
                                        if (grid[ny][nx].terrain == ROAD) prob *= 1.3;
                                        if (grid[ny][nx].terrain == MOUNTAIN) prob *= 0.3;
                                        
                                        if ((double)rand() / RAND_MAX < prob) {
                                            newState[ny][nx].state = SICK;
                                            newState[ny][nx].sickDays = 0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            grid = newState;
            
            if (sickCount > 0) {
                step++;
                
                int healthy = 0, sick = 0, recovered = 0;
                for (int y = 0; y < HEIGHT; y++) {
                    for (int x = 0; x < WIDTH; x++) {
                        if (grid[y][x].terrain == OCEAN) continue;
                        if (grid[y][x].state == HEALTHY) healthy++;
                        else if (grid[y][x].state == SICK) sick++;
                        else recovered++;
                    }
                }
                
                cout << "Шаг " << step 
                     << " | 🟢 Здоровых: " << healthy 
                     << " | 🔴 Больных: " << sick 
                     << " | 🟣 Переболевших: " << recovered << endl;
                
                if (sick == 0 && step > 0) {
                    cout << "\n=== ЭПИДЕМИЯ ЗАВЕРШЕНА ===" << endl;
                }
            }
            
            clock.restart();
        }
        
        // Отрисовка
        window.clear(sf::Color::Black);
        sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 0.5f, CELL_SIZE - 0.5f));
        
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                cell.setFillColor(ContinentGenerator::getColor(grid[y][x]));
                cell.setPosition(sf::Vector2f(x * CELL_SIZE, y * CELL_SIZE));
                window.draw(cell);
            }
        }
        
        window.display();
    }
    
    return 0;
}
