#include "Game.h"
#include <random>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <fmt/ostream.h>

#include "SFML/Audio/Sound.hpp"
#include "SFML/Audio/SoundBuffer.hpp"

Game::Game() : renderWindow(sf::VideoMode(sf::Vector2u(800, 600)), "Monkey Typer") {
    if (!loadFont("arial.ttf")) {
        renderWindow.close();
    }

    loadBackground();

    soundBuffer = new sf::SoundBuffer();
    if (soundBuffer->loadFromFile("assets/sounds/score.mp3")) {
        sound = new sf::Sound(*soundBuffer);
        sound->setVolume(50.0f);
    }

    auto buttonWidth = 200.0f;
    auto buttonHeight = 50.0f;
    auto buttonSpacing = 20.0f;

    createButtons(menuButtons,
                  {"Play", "Settings", "Leaderboard", "Load Game"},
                  200, buttonWidth, buttonHeight, buttonSpacing);

    createButtons(gameOverButtons,
                  {"Play Again", "Main Menu"},
                  300, buttonWidth, buttonHeight, buttonSpacing);

    createButtons(pauseButtons,
                  {"Continue", "Save Game", "Main Menu"},
                  200, buttonWidth, buttonHeight, buttonSpacing);

    createButtons(settingsButtons,
                  {"Difficulty", "Word Package", "Font", "Back to Menu"},
                  200, buttonWidth, buttonHeight, buttonSpacing);

    createButtons(difficultyButtons,
                  {"Easy", "Medium", "Hard", "Back"},
                  200, buttonWidth, buttonHeight, buttonSpacing);

    createButtons(wordPackageButtons,
                  {"English Words", "Polish Words", "Back"},
                  200, buttonWidth, buttonHeight, buttonSpacing);

    createButtons(fontButtons,
                  {"Arial", "Calibri", "Consolas", "Back"},
                  200, buttonWidth, buttonHeight, buttonSpacing);

    currentState = GameState::Menu;
    renderWindow.setFramerateLimit(60);
    lastWordSpawn = std::chrono::steady_clock::now();
    score = 0;
    currentDifficulty = Difficulty::Easy;
    health = getMaxHealth();
    loadLeaderboard();
    currentWordPackage = WordPackage::English;
    loadWordPackage();
    currentFont = "arial.ttf";
}

auto Game::run() -> void {
    while (renderWindow.isOpen()) {
        processEvents();
        update();
        render();
    }
}

auto Game::processEvents() -> void {
    while (auto event = renderWindow.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            renderWindow.close();
        }

        if (event->is<sf::Event::KeyPressed>()) {
            auto keyEvent = event->getIf<sf::Event::KeyPressed>();
            if (keyEvent) {
                if (keyEvent->code == sf::Keyboard::Key::Escape) {
                    if (currentState == GameState::Game) {
                        currentState = GameState::Pause;
                    }
                    else if (currentState == GameState::Pause) {
                        currentState = GameState::Game;
                    }
                    else if (currentState != GameState::Menu) {
                        currentState = GameState::Menu;
                        resetGame();
                    }
                }
                else if (currentState == GameState::Game) {
                    if (keyEvent->code == sf::Keyboard::Key::Enter) {
                        if (!currentInput.empty()) {
                            checkWord();
                            currentInput.clear();
                        }
                    }
                    else if (keyEvent->code == sf::Keyboard::Key::Backspace) {
                        if (!currentInput.empty()) {
                            currentInput.pop_back();
                        }
                    }
                    else if (keyEvent->code >= sf::Keyboard::Key::A && keyEvent->code <= sf::Keyboard::Key::Z) {
                        char c = static_cast<char>('a' + (static_cast<int>(keyEvent->code) - static_cast<int>(sf::Keyboard::Key::A)));
                        currentInput += c;
                    }
                }
                else if (currentState != GameState::Game) {
                    std::vector<Button>* currentButtons = nullptr;
                    switch (currentState) {
                        case GameState::Menu:
                            currentButtons = &menuButtons;
                            break;
                        case GameState::Pause:
                            currentButtons = &pauseButtons;
                            break;
                        case GameState::GameOver:
                            currentButtons = &gameOverButtons;
                            break;
                        case GameState::Settings:
                            currentButtons = &settingsButtons;
                            break;
                        case GameState::SettingsDifficulty:
                            currentButtons = &difficultyButtons;
                            break;
                        case GameState::SettingsWordPackage:
                            currentButtons = &wordPackageButtons;
                            break;
                        case GameState::SettingsFont:
                            currentButtons = &fontButtons;
                            break;
                        default:
                            break;
                    }

                    if (currentButtons) {
                        if (keyEvent->code == sf::Keyboard::Key::Up) {
                            selectedButtonIndex = (selectedButtonIndex - 1 + currentButtons->size()) % currentButtons->size();
                        }
                        else if (keyEvent->code == sf::Keyboard::Key::Down) {
                            selectedButtonIndex = (selectedButtonIndex + 1) % currentButtons->size();
                        }
                        else if (keyEvent->code == sf::Keyboard::Key::Enter) {
                            switch (currentState) {
                                case GameState::Menu:
                                    handleMenuSelection(selectedButtonIndex);
                                    break;
                                case GameState::Pause:
                                    handlePauseSelection(selectedButtonIndex);
                                    break;
                                case GameState::GameOver:
                                    handleGameOverSelection(selectedButtonIndex);
                                    break;
                                case GameState::Settings:
                                    handleSettingsSelection(selectedButtonIndex);
                                    break;
                                case GameState::SettingsDifficulty:
                                    handleDifficultySelection(selectedButtonIndex);
                                    break;
                                case GameState::SettingsWordPackage:
                                    handleWordPackageSelection(selectedButtonIndex);
                                    break;
                                case GameState::SettingsFont:
                                    handleFontSelection(selectedButtonIndex);
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                }
            }
        }
    }
}

auto Game::update() -> void {
    if (currentState == GameState::Game) {
        auto currentTime = std::chrono::steady_clock::now();

        if (std::chrono::duration<float>(currentTime - lastWordSpawn).count() > getSpawnInterval()) {
            spawnWord();
            lastWordSpawn = currentTime;
        }

        for (auto& word : words) {
            word.update();
        }

        const auto windowWidth = renderWindow.getSize().x;
        words.erase(
            std::remove_if(words.begin(), words.end(),
                [this, windowWidth](const Word& word) {
                    if (word.isOffScreen(windowWidth)) {
                        decreaseHealth();
                        return true;
                    }
                    return false;
                }),
            words.end()
        );
    }

    std::vector<Button>* currentButtons = nullptr;
    switch (currentState) {
        case GameState::Menu:
            currentButtons = &menuButtons;
            break;
        case GameState::Pause:
            currentButtons = &pauseButtons;
            break;
        case GameState::GameOver:
            currentButtons = &gameOverButtons;
            break;
        case GameState::Settings:
            currentButtons = &settingsButtons;
            break;
        case GameState::SettingsDifficulty:
            currentButtons = &difficultyButtons;
            break;
        case GameState::SettingsWordPackage:
            currentButtons = &wordPackageButtons;
            break;
        case GameState::SettingsFont:
            currentButtons = &fontButtons;
            break;
        default:
            break;
    }

    if (currentButtons) {
        for (auto i = 0; i < currentButtons->size(); i++) {
            auto& button = (*currentButtons)[i];
            button.setSelected(i == selectedButtonIndex);
        }
    }
}

auto Game::render() -> void {
    renderWindow.clear(sf::Color(30, 30, 30));

    if (background) {
        renderWindow.draw(*background);
    }

    switch (currentState) {
        case GameState::Menu:
            renderMenuScreen();
            break;
        case GameState::Game:
            renderGameScreen();
            break;
        case GameState::Pause:
            renderPauseScreen();
            break;
        case GameState::GameOver:
            renderGameOverScreen();
            break;
        case GameState::Settings:
            renderSettingsScreen();
            break;
        case GameState::SettingsDifficulty:
            renderDifficultySettingsScreen();
            break;
        case GameState::SettingsWordPackage:
            renderWordPackageSettingsScreen();
            break;
        case GameState::SettingsFont:
            renderFontSettingsScreen();
            break;
        case GameState::Leaderboard:
            renderLeaderboardScreen();
            break;
    }

    renderWindow.display();
}

auto Game::resetGame() -> void {
    words.clear();
    currentInput.clear();
    score = 0;
    health = getMaxHealth();
    lastWordSpawn = std::chrono::steady_clock::now();
    lastFrame = std::chrono::steady_clock::now();
}

auto Game::decreaseHealth() -> void {
    health--;
    if (health <= 0) {
        currentState = GameState::GameOver;
        saveScore();
    }
}

auto Game::checkWord() -> void {
    auto iterator = std::find_if(words.begin(), words.end(),
        [this](const Word& word) {return word.getText() == currentInput;});

    if (iterator != words.end()) {
        words.erase(iterator);
        score += 10 * getScoreMultiplier();

        if (sound) {
            sound->play();
        }
    }
    else {
        decreaseHealth();
    }
}

auto Game::spawnWord() -> void {
    //https://stackoverflow.com/questions/7560114/random-number-c-in-some-range
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> wordDist(0, wordList.size() - 1);
    std::uniform_int_distribution<> y(50, 500);
    float yDist = y(gen);

    words.push_back({wordList[wordDist(gen)], 0, yDist, getWordSpeed(), font});
}

auto Game::renderMenuScreen() -> void {
    auto const texture = sf::Texture("assets/logo.png");
    auto logoImage = sf::Sprite(texture);

    auto const textureSize = texture.getSize();
    auto const width = 300.0f;
    auto const scale = width / textureSize.x;
    logoImage.setScale({scale, scale});

    logoImage.setPosition({(renderWindow.getSize().x - logoImage.getGlobalBounds().size.x) / 2, 10});
    renderWindow.draw(logoImage);

    Button::drawButtons(menuButtons, renderWindow);
}

auto Game::renderGameScreen() -> void {
    for (auto& word : words) {
        word.draw(renderWindow);
    }

    renderWindow.draw(setupText(currentInput, 24, sf::Color::Green, sf::Vector2f(10, 550)));
    renderWindow.draw(setupText(fmt::format("Score: {}", score), 24, sf::Color::White, sf::Vector2f(650, 550)));
    renderWindow.draw(setupText(fmt::format("Health: {}", health), 24, sf::Color::Red, sf::Vector2f(10, 20)));
    renderWindow.draw(setupText(fmt::format("Difficulty: {}", getDifficultyString()), 24, sf::Color::Yellow, sf::Vector2f(650, 20)));
}

auto Game::renderGameOverScreen() -> void {
    renderWindow.draw(setupText("Game Over!", 60, sf::Color::Red, sf::Vector2f(0, 150), true));
    renderWindow.draw(setupText(fmt::format("Achieved score: {}", score), 30, sf::Color::White, sf::Vector2f(0, 220), true));

    Button::drawButtons(gameOverButtons, renderWindow);
}

auto Game::renderPauseScreen() -> void {
    renderGameScreen();

    sf::RectangleShape darkenLayer(sf::Vector2f(800, 600));
    darkenLayer.setFillColor(sf::Color(0, 0, 0, 150));
    renderWindow.draw(darkenLayer);

    renderWindow.draw(setupText("Game Paused", 60, sf::Color::White, sf::Vector2f(0, 100), true));

    Button::drawButtons(pauseButtons, renderWindow);
}

auto Game::renderSettingsScreen() -> void {
    renderWindow.draw(setupText("Settings", 60, sf::Color::White, sf::Vector2f(0, 100), true));

    Button::drawButtons(settingsButtons, renderWindow);

    renderWindow.draw(setupText(
        fmt::format("Current: {}, {}", getDifficultyString(), getWordPackageString()),
        24, sf::Color::Yellow, sf::Vector2f(0, 500), true
    ));
}

auto Game::renderDifficultySettingsScreen() -> void {
    renderWindow.draw(setupText("Select Difficulty", 60, sf::Color::White, sf::Vector2f(0, 100), true));

    Button::drawButtons(difficultyButtons, renderWindow);

    renderWindow.draw(setupText(
        "Easy: 3 health, slow speed, multiplier 1x\n"
        "Medium: 2 health, medium speed, multiplier 1.3x\n"
        "Hard: 1 health, fast speed, multiplier 1.5x",
        20, sf::Color::Yellow, sf::Vector2f(0, 475), true
    ));
}

auto Game::renderWordPackageSettingsScreen() -> void {
    renderWindow.draw(setupText("Select Word Package", 60, sf::Color::White, sf::Vector2f(0, 100), true));

    Button::drawButtons(wordPackageButtons, renderWindow);
}

auto Game::renderFontSettingsScreen() -> void {
    renderWindow.draw(setupText("Select Font", 60, sf::Color::White, sf::Vector2f(0, 100), true));

    Button::drawButtons(fontButtons, renderWindow);

    renderWindow.draw(setupText(
        fmt::format("Current: {}", currentFont),
        24, sf::Color::Yellow, sf::Vector2f(0, 475), true
    ));
}

auto Game::renderLeaderboardScreen() -> void {
    loadLeaderboard();

    renderWindow.draw(setupText("Leaderboard", 60, sf::Color::White, sf::Vector2f(0, 50), true));
    renderWindow.draw(setupText("Rank", 24, sf::Color::Yellow, sf::Vector2f(100, 120)));
    renderWindow.draw(setupText("Score", 24, sf::Color::Yellow, sf::Vector2f(250, 120)));
    renderWindow.draw(setupText("Date", 24, sf::Color::Yellow, sf::Vector2f(400, 120)));

    auto yPosition = 170.0f;
    auto spacing = 40.0f;
    for (auto i = 0; i < 10 && i < leaderboard.size(); i++) {
        const auto& entry = leaderboard[i];
        renderWindow.draw(setupText(std::to_string(i + 1), 24, sf::Color::White, {100, yPosition}));
        renderWindow.draw(setupText(entry[0], 24, sf::Color::White, {250, yPosition}));
        renderWindow.draw(setupText(entry[1], 24, sf::Color::White, {400, yPosition}));
        yPosition += spacing;
    }

    renderWindow.draw(setupText("Press ESC to return to menu", 20, sf::Color(200, 200, 200), sf::Vector2f(0, 550), true));
}

auto Game::loadBackground() -> void {
    //https://www.youtube.com/watch?v=tXfdP3pcppI
    backgroundTexture = new sf::Texture();
    if (backgroundTexture->loadFromFile("assets/background.png")) {
        background = new sf::Sprite(*backgroundTexture);

        auto textureSize = backgroundTexture->getSize();
        auto windowSize = renderWindow.getSize();

        //https://www.geeksforgeeks.org/casting-operators-in-cpp/
        float scaleX = static_cast<float>(windowSize.x) / textureSize.x;
        float scaleY = static_cast<float>(windowSize.y) / textureSize.y;
        auto scale = std::max(scaleX, scaleY);

        background->setScale({scale, scale});
    }
}

auto Game::loadFont(const std::string& fontName) -> bool {
    //https://www.sfml-dev.org/tutorials/3.0/graphics/text/
    if (font.openFromFile("assets/fonts/" + fontName)) {
        currentFont = fontName;
        return true;
    }
    return false;
}

auto Game::loadWordPackage() -> void {
    wordList.clear();
    auto filename = "assets/packages/words_english.txt";

    switch (currentWordPackage) {
        case WordPackage::English:
            filename = "assets/packages/words_english.txt";
        break;
        case WordPackage::Polish:
            filename = "assets/packages/words_polish.txt";
        break;
    }

    auto file = std::fstream(filename);
    auto word = std::string();

    while (file >> word) {
        wordList.push_back(word);
    }
}

auto Game::loadLeaderboard() -> void {
    leaderboard.clear();
    std::ifstream file("assets/data/leaderboard.csv");

    if (file.is_open()) {
        auto line = std::string();
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::vector<std::string> entry;

            auto value = std::string();
            while (std::getline(ss, value, ';')) {
                entry.push_back(value);
            }

            if (entry.size() == 2) {
                leaderboard.push_back(entry);
            }
        }
        file.close();

        std::sort(leaderboard.begin(), leaderboard.end(),
            [](const std::vector<std::string>& a, const std::vector<std::string>& b) {
        try {
            int scoreA = std::stoi(a[0]);
            int scoreB = std::stoi(b[0]);
            return scoreA > scoreB;
        } catch (const std::exception& e) {
            return false;
        }
    });
    }
}

auto Game::loadGame() -> bool {
    std::ifstream file("assets/data/savegame.txt");

    if (file.is_open()) {
        try {
            std::string line;
            std::string key, value;

            while (std::getline(file, line)) {
                std::stringstream ss(line);
                std::getline(ss, key, ':');
                std::getline(ss, value);

                if (key == "Score") {
                    score = std::stoi(value);
                }
                else if (key == "Health") {
                    health = std::stoi(value);
                }
                else if (key == "Difficulty") {
                    currentDifficulty = static_cast<Difficulty>(std::stoi(value));
                }
                else if (key == "WordPackage") {
                    currentWordPackage = static_cast<WordPackage>(std::stoi(value));
                }
                else if (key == "Words") {
                    int wordCount = std::stoi(value);
                    words.clear();

                    for (int i = 0; i < wordCount; i++) {
                        if (std::getline(file, line)) {
                            std::stringstream wordSS(line);

                            std::vector<std::string> entry;
                            std::string s;
                            while (std::getline(wordSS, s, ';')) {
                                entry.push_back(s);
                            }

                            auto wordText = entry[0];
                            auto x = std::stof(entry[1]);
                            auto y = std::stof(entry[2]);
                            auto speed = std::stof(entry[3]);

                            words.push_back({wordText, x, y, speed, font});
                        }
                    }
                }
            }

            file.close();
            return true;
        }
        catch (const std::exception& e) {
            file.close();
            return false;
        }
    }
    return false;
}

auto Game::saveGame() -> void {
    //https://stackoverflow.com/questions/8357240/how-to-automatically-convert-strongly-typed-enum-into-int
    std::ofstream file("assets/data/savegame.txt");
    if (file.is_open()) {
        file << "Score:" << score << "\n"
        << "Health:" << health << "\n"
        << "Difficulty:" << static_cast<int>(currentDifficulty) << "\n"
        << "WordPackage:" << static_cast<int>(currentWordPackage) << "\n";

        file << "Words:" << words.size() << "\n";
        for (const auto& word : words) {
            auto pos = word.getPosition();
            file << word.getText() << ";"
                 << pos.x << ";"
                 << pos.y << ";"
                 << word.getSpeed() << "\n";
        }

        file.close();
    }
}

auto Game::saveScore() -> void {
    // https://stackoverflow.com/questions/997512/string-representation-of-time-t
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    char buffer[32];
    std::strftime(buffer, 32, "%Y-%m-%d %H:%M:%S", std::localtime(&time_t));

    leaderboard.push_back({std::to_string(score), buffer});

    std::ofstream file("assets/data/leaderboard.csv");
    if (file.is_open()) {
        for (const auto& entry : leaderboard) {
            file << entry[0] << ";" << entry[1] << "\n";
        }
        file.close();
    }
}

auto Game::createButtons(std::vector<Button>& buttonsVector,
                        const std::vector<std::string>& buttonTexts,
                        const float& marginTop,
                        const float& buttonWidth,
                        const float& buttonHeight,
                        const float& spacing) -> void {
    for (auto i = 0; i < buttonTexts.size(); i++) {
        buttonsVector.push_back(
            {
                {(renderWindow.getSize().x - buttonWidth) / 2, marginTop + i * (buttonHeight + spacing)},
                {buttonWidth, buttonHeight},
                buttonTexts[i],
                font
            }
        );
    }
}

auto Game::updateAllTexts() -> void {
    auto allButtons = std::vector<std::vector<Button>>{
        menuButtons, settingsButtons, difficultyButtons,
        wordPackageButtons, fontButtons, gameOverButtons, pauseButtons
    };
    Button::updateAllButtons(allButtons, font);
}

auto Game::handleMenuSelection(int index) -> void {
    auto selected = menuButtons[index].getText();
    selectedButtonIndex = 0;

    if (selected == "Play") {
        currentState = GameState::Game;
        resetGame();
    } else if (selected == "Settings") {
        currentState = GameState::Settings;
    } else if (selected == "Leaderboard") {
        currentState = GameState::Leaderboard;
    } else if (selected == "Load Game") {
        if (loadGame()) {
            currentState = GameState::Game;
        }
    }
}

auto Game::handlePauseSelection(int index) -> void {
    auto selected = pauseButtons[index].getText();
    selectedButtonIndex = 0;

    if (selected == "Continue") {
        currentState = GameState::Game;
    } else if (selected == "Save Game") {
        saveGame();
        currentState = GameState::Menu;
    } else if (selected == "Main Menu") {
        currentState = GameState::Menu;
        resetGame();
    }
}

auto Game::handleGameOverSelection(int index) -> void {
    auto selected = gameOverButtons[index].getText();
    selectedButtonIndex = 0;

    if (selected == "Play Again") {
        currentState = GameState::Game;
        resetGame();
    } else if (selected == "Main Menu") {
        currentState = GameState::Menu;
        resetGame();
    }
}

auto Game::handleSettingsSelection(int index) -> void {
    auto selected = settingsButtons[index].getText();
    selectedButtonIndex = 0;

    if (selected == "Difficulty") {
        currentState = GameState::SettingsDifficulty;
    } else if (selected == "Word Package") {
        currentState = GameState::SettingsWordPackage;
    } else if (selected == "Font") {
        currentState = GameState::SettingsFont;
    } else if (selected == "Back to Menu") {
        currentState = GameState::Menu;
    }
}

auto Game::handleDifficultySelection(int index) -> void {
    auto selected = difficultyButtons[index].getText();
    selectedButtonIndex = 0;

    if (selected == "Easy") {
        currentDifficulty = Difficulty::Easy;
    } else if (selected == "Medium") {
        currentDifficulty = Difficulty::Medium;
    } else if (selected == "Hard") {
        currentDifficulty = Difficulty::Hard;
    }
    currentState = GameState::Settings;
}

auto Game::handleWordPackageSelection(int index) -> void {
    auto selected = wordPackageButtons[index].getText();
    selectedButtonIndex = 0;

    if (selected == "English Words") {
        currentWordPackage = WordPackage::English;
        loadWordPackage();
    } else if (selected == "Polish Words") {
        currentWordPackage = WordPackage::Polish;
        loadWordPackage();
    }
    currentState = GameState::Settings;
}

auto Game::handleFontSelection(int index) -> void {
    auto selected = fontButtons[index].getText();
    selectedButtonIndex = 0;

    if (selected == "Arial") {
        if (loadFont("arial.ttf")) {
            updateAllTexts();
        }
    } else if (selected == "Calibri") {
        if (loadFont("calibri.ttf")) {
            updateAllTexts();
        }
    } else if (selected == "Consolas") {
        if (loadFont("consolas.ttf")) {
            updateAllTexts();
        }
    }
    currentState = GameState::Settings;
}

auto Game::setupText(const std::string& content, 
                    const int& size, 
                    const sf::Color& color,
                    const sf::Vector2f& position, 
                    const bool& centerX) const -> sf::Text {
    sf::Text textObj(font, content, size);
    textObj.setFillColor(color);
    textObj.setOutlineThickness(2);

    if (centerX) {
        auto bounds = textObj.getLocalBounds();
        textObj.setPosition({(renderWindow.getSize().x - bounds.size.x) / 2, position.y});
    } else {
        textObj.setPosition(position);
    }

    return textObj;
}

auto Game::getWordSpeed() const -> float {
    switch (currentDifficulty) {
        case Difficulty::Easy: return 2.0f;
        case Difficulty::Medium: return 3.0f;
        case Difficulty::Hard: return 4.0f;
        default: return 2.0f;
    }
}

auto Game::getSpawnInterval() const -> float {
    switch (currentDifficulty) {
        case Difficulty::Easy: return 2.0f;
        case Difficulty::Medium: return 1.5f;
        case Difficulty::Hard: return 1.0f;
        default: return 2.0f;
    }
}

auto Game::getScoreMultiplier() const -> float {
    switch (currentDifficulty) {
        case Difficulty::Easy: return 1.0f;
        case Difficulty::Medium: return 1.3f;
        case Difficulty::Hard: return 1.5f;
        default: return 1.0f;
    }
}

auto Game::getMaxHealth() const -> int {
    switch (currentDifficulty) {
        case Difficulty::Easy: return 3;
        case Difficulty::Medium: return 2;
        case Difficulty::Hard: return 1;
        default: return 3;
    }
}

auto Game::getDifficultyString() const -> std::string {
    switch (currentDifficulty) {
        case Difficulty::Easy: return "Easy";
        case Difficulty::Medium: return "Medium";
        case Difficulty::Hard: return "Hard";
        default: return "Easy";
    }
}

auto Game::getWordPackageString() const -> std::string {
    switch (currentWordPackage) {
        case WordPackage::English: return "English";
        case WordPackage::Polish: return "Polish";
        default: return "English";
    }
}