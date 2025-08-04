#pragma once

#include <SFML/Graphics.hpp>
#include <string>

class Word {
public:
    Word(const std::string& text, float x, float y, float speed, const sf::Font& font);
    auto update() -> void;
    auto draw(sf::RenderWindow& window) -> void;
    auto isOffScreen(const int& width) const -> bool;
    auto getText() const -> std::string;
    auto getPosition() const -> sf::Vector2f;
    auto getSpeed() const -> float;
private:
    float speed;
    sf::Text textObj;
}; 