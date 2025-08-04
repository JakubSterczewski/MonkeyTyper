#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class Button {
public:
    Button(const sf::Vector2f& position, const sf::Vector2f& size, const std::string& textStr, const sf::Font& font);
    auto draw(sf::RenderWindow& window) const -> void;
    auto setSelected(bool selected) -> void;
    auto getText() const -> std::string;
    static auto updateAllButtons(std::vector<std::vector<Button>>& buttonsVectors, const sf::Font& font) -> void;
    static auto drawButtons(const std::vector<Button>& buttons, sf::RenderWindow& renderWindow) -> void;
    auto getPosition() const -> sf::Vector2f;
private:
    sf::RectangleShape rectangle;
    sf::Text text;
    sf::Vector2f position;
    sf::Vector2f size;
    bool isSelected = false;
}; 