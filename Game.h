#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>
#include "components/Button.h"
#include "components/Word.h"
#include "enums/GameState.h"
#include "enums/Difficulty.h"
#include "enums/WordPackage.h"

class Game {
public:
    Game();
    auto run() -> void;

private:
    auto processEvents() -> void;
    auto update() -> void;
    auto render() -> void;
    auto resetGame() -> void;
    auto decreaseHealth() -> void;
    auto checkWord() -> void;
    auto spawnWord() -> void;

    auto renderMenuScreen() -> void;
    auto renderGameScreen() -> void;
    auto renderGameOverScreen() -> void;
    auto renderPauseScreen() -> void;
    auto renderSettingsScreen() -> void;
    auto renderDifficultySettingsScreen() -> void;
    auto renderWordPackageSettingsScreen() -> void;
    auto renderFontSettingsScreen() -> void;
    auto renderLeaderboardScreen() -> void;

    auto loadBackground() -> void;
    auto loadFont(const std::string& fontName) -> bool;
    auto loadWordPackage() -> void;
    auto loadLeaderboard() -> void;
    auto loadGame() -> bool;

    auto saveGame() -> void;
    auto saveScore() -> void;

    auto createButtons(std::vector<Button>& buttonsVector,
                      const std::vector<std::string>& buttonTexts,
                      const float& marginTop,
                      const float& buttonWidth,
                      const float& buttonHeight,
                      const float& spacing) -> void;
    auto updateAllTexts() -> void;

    auto handleMenuSelection(int index) -> void;
    auto handlePauseSelection(int index) -> void;
    auto handleGameOverSelection(int index) -> void;
    auto handleSettingsSelection(int index) -> void;
    auto handleDifficultySelection(int index) -> void;
    auto handleWordPackageSelection(int index) -> void;
    auto handleFontSelection(int index) -> void;

    auto setupText(const std::string& content, 
                  const int& size, 
                  const sf::Color& color,
                  const sf::Vector2f& position, 
                  const bool& centerX = false) const -> sf::Text;
    auto getWordSpeed() const -> float;
    auto getSpawnInterval() const -> float;
    auto getScoreMultiplier() const -> float;
    auto getMaxHealth() const -> int;
    auto getDifficultyString() const -> std::string;
    auto getWordPackageString() const -> std::string;

    sf::RenderWindow renderWindow;
    sf::Font font;
    sf::Texture* backgroundTexture;
    sf::Sprite* background;
    std::vector<Button> menuButtons;
    std::vector<Button> gameOverButtons;
    std::vector<Button> settingsButtons;
    std::vector<Button> pauseButtons;
    std::vector<Button> difficultyButtons;
    std::vector<Button> wordPackageButtons;
    std::vector<Button> fontButtons;
    std::vector<Word> words;
    std::string currentInput;
    int score;
    GameState currentState;
    std::chrono::steady_clock::time_point lastFrame;
    std::chrono::steady_clock::time_point lastWordSpawn;
    std::vector<std::vector<std::string>> leaderboard;
    Difficulty currentDifficulty;
    int health;
    std::vector<std::string> wordList;
    WordPackage currentWordPackage;
    std::string currentFont;
    int selectedButtonIndex = 0;
    sf::SoundBuffer* soundBuffer;
    sf::Sound* sound;
}; 