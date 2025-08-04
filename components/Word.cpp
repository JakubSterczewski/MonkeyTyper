#include "Word.h"

Word::Word(const std::string& text, float x, float y, float speed, const sf::Font& font)
    : speed(speed), textObj(font, text, 30) {
    textObj.setPosition({x, y});
    textObj.setOutlineThickness(2);
}

auto Word::draw(sf::RenderWindow& window) -> void{
    auto screenPercentage = textObj.getPosition().x / window.getSize().x;
    if (screenPercentage >= 0.75) {
        textObj.setFillColor(sf::Color::Red);
    } else if (screenPercentage >= 0.50) {
        textObj.setFillColor(sf::Color::Yellow);
    } else {
        textObj.setFillColor(sf::Color::Green);
    }

    window.draw(textObj);
}

auto Word::update() -> void{
    auto pos = textObj.getPosition();
    pos.x += speed;
    textObj.setPosition(pos);
}

auto Word::isOffScreen(const int& width) const -> bool {
    return textObj.getPosition().x > width;
}

auto Word::getText() const -> std::string {
    return textObj.getString().toAnsiString();
}

auto Word::getPosition() const -> sf::Vector2f {
    return textObj.getPosition();
}

auto Word::getSpeed() const -> float {
    return speed;
}