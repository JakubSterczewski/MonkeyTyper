#include "Button.h"

Button::Button(const sf::Vector2f& position, const sf::Vector2f& size, const std::string& textStr, const sf::Font& font)
    : position(position), size(size), text(font, textStr, 24) {
    rectangle.setPosition(position);
    rectangle.setSize(size);
    rectangle.setFillColor(sf::Color(45, 45, 45));
    rectangle.setOutlineThickness(2);
    rectangle.setOutlineColor(sf::Color(100, 100, 100));

    text.setFillColor(sf::Color(220, 220, 220));

    //https://stackoverflow.com/questions/67523148/centering-text-on-top-of-buttons-in-sfml
    text.setPosition(
        {position.x + (size.x - text.getGlobalBounds().size.x) / 2,
        position.y + (size.y - text.getGlobalBounds().size.y) / 2}
    );
}

auto Button::draw(sf::RenderWindow& window) const -> void {
    window.draw(rectangle);
    window.draw(text);
}

auto Button::setSelected(bool selected) -> void {
    isSelected = selected;
    if (isSelected) {
        rectangle.setFillColor(sf::Color(0, 120, 0));
        rectangle.setOutlineColor(sf::Color(0, 200, 0));
        text.setFillColor(sf::Color::White);
    } else {
        rectangle.setFillColor(sf::Color(45, 45, 45));
        rectangle.setOutlineColor(sf::Color(100, 100, 100));
        text.setFillColor(sf::Color(220, 220, 220));
    }
}

auto Button::getText() const -> std::string{
    return text.getString();
}

auto Button::updateAllButtons(std::vector<std::vector<Button>>& buttonsVectors, const sf::Font& font) -> void {
    for (auto& buttonsVector : buttonsVectors) {
        for (auto& button : buttonsVector) {
            button.text.setFont(font);
        }
    }
}

auto Button::drawButtons(const std::vector<Button>& buttons, sf::RenderWindow& renderWindow) -> void {
    for (const auto& button : buttons) {
        button.draw(renderWindow);
    }
}

auto Button::getPosition() const -> sf::Vector2f {
    return position;
}