#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <queue>
#include <thread>
#include <chrono>

using namespace std;
using namespace sf;

int start_i = -1, start_k = -1;
int counted = 0;

bool isValid(int i, int k) {
    return i >= 0 && i < 4 && k >= 0 && k < 4;
}

bool isConnected(int map[][4]) {
    bool visited[4][4] = { false };

    for (int i = 0; i < 4; i++) {
        for (int k = 0; k < 4; k++) {
            if (map[i][k] == 1) {
                start_i = i;
                start_k = k;
                break;
            }
        }
        if (start_i != -1) break;
    }
    if (start_i == -1) return false;

    queue<pair<int, int>> q;
    q.push({ start_i, start_k });
    visited[start_i][start_k] = true;

    while (!q.empty()) {
        auto current = q.front();
        q.pop();
        int i = current.first;
        int k = current.second;

        if (isValid(i - 1, k) && map[i - 1][k] == 1 && !visited[i - 1][k]) {
            q.push({ i - 1, k });
            visited[i - 1][k] = true;
        }
        if (isValid(i + 1, k) && map[i + 1][k] == 1 && !visited[i + 1][k]) {
            q.push({ i + 1, k });
            visited[i + 1][k] = true;
        }
        if (isValid(i, k - 1) && map[i][k - 1] == 1 && !visited[i][k - 1]) {
            q.push({ i, k - 1 });
            visited[i][k - 1] = true;
        }
        if (isValid(i, k + 1) && map[i][k + 1] == 1 && !visited[i][k + 1]) {
            q.push({ i, k + 1 });
            visited[i][k + 1] = true;
        }
    }

    for (int i = 0; i < 4; i++) {
        for (int k = 0; k < 4; k++) {
            if (map[i][k] == 1 && !visited[i][k]) {
                return false;
            }
        }
    }

    return true;
}

void genMap(int map[][4]) {
    srand(static_cast<unsigned int>(time(0)));

    for (int i = 0; i < 4; i++) {
        for (int k = 0; k < 4; k++) {
            map[i][k] = 0;
        }
    }

    int countRooms = (rand() % 9) + 5;
    for (int i = 0; i < countRooms; i++) {
        int a = rand() % 4;
        int b = rand() % 4;
        map[a][b] = 1;
    }
}


struct Bullet {
    Vector2f velocity;
    CircleShape bulletShape;
};

struct Block {
    Vector2f pos;
    RectangleShape block;
    string name;
};

struct Player {
    RectangleShape player;
    int health;
    int damage;
    float damageCooldown;
    Vector2f direction;
};

class Enemy {
public:
    Enemy() : health(1), position({ 550, 350 }), damage(1),mob(Vector2f(10,20)) {
    }

    int getHealth() const { return health; }
    Vector2f getPosition() const { return position; }
    int getDamage() const { return damage; }
    RectangleShape getShape() const { return mob; }

    void setHealth(int h) { health = h; }
    void setPosition(sf::Vector2f pos) { position = pos; mob.setPosition(pos); }
    void setDamage(int dmg) { damage = dmg; }

    void move(Player& player, vector<Block>& walls, float deltaTime) {
        Vector2f playerPos = player.player.getPosition() + Vector2f(48, 48);
        Vector2f direction = playerPos - position - Vector2f(50, 50);
        float length = sqrt(direction.x * direction.x + direction.y * direction.y);

        if (length != 0) {
            direction /= length;
        }

        float speed = 50.0f;
        Vector2f step = direction * speed * deltaTime;

        if (!mob.getGlobalBounds().intersects(player.player.getGlobalBounds())) {
            mob.move(step.x, 0);
            bool collisionX = false;
            for (const auto& wall : walls) {
                if (mob.getGlobalBounds().intersects(wall.block.getGlobalBounds())) {
                    collisionX = true;
                    break;
                }
            }
            if (collisionX) {
                mob.move(-step.x, 0);
            }

            mob.move(0, step.y);
            bool collisionY = false;
            for (const auto& wall : walls) {
                if (mob.getGlobalBounds().intersects(wall.block.getGlobalBounds())) {
                    collisionY = true;
                    break;
                }
            }
            if (collisionY) {
                mob.move(0, -step.y);
            }

            position = mob.getPosition();
        }
    }

    char getAttackSide(Player& player) {
        Vector2f playerPos = player.player.getPosition();
        Vector2f enemyPos = position;

        float dx = enemyPos.x - playerPos.x;
        float dy = enemyPos.y - playerPos.y;

        if (abs(dx) > abs(dy)) {
            if (dx > 0) {
                return 'l';
            }
            else {
                return 'r';
            }
        }
        else {
            if (dy > 0) {
                return 't';
            }
            else {
                return 'b';
            }
        }
    }

    void attack(Player& player, const vector<Block>& walls, vector<Enemy>& enemies, float deltaTime) {
        if (checkEnemyCollisionPlayer(player.player)) {
            player.health--;
            
            char attackSide = getAttackSide(player);
            for (int i = 0; i < 10; i++) {
                if (attackSide == 't') {
                    mob.move(Vector2f(0, i));
                }
                if (attackSide == 'b') {
                    mob.move(Vector2f(0, -i));
                }
                if (attackSide == 'l') {
                    mob.move(Vector2f(i, 0));
                }
                if (attackSide == 'r') {
                    mob.move(Vector2f(-i, 0));
                }
            }
        }

        for (auto& enemy : enemies) {
            if (this != &enemy && mob.getGlobalBounds().intersects(enemy.getShape().getGlobalBounds())) {
                Vector2f enemyPos = enemy.getPosition();
                Vector2f direction = position - enemyPos;
                float length = sqrt(direction.x * direction.x + direction.y * direction.y);
                if (length != 0) {
                    direction /= length;
                }
                mob.move(direction * 5.0f);
            }
        }
    }



    void takeDamage(Bullet& bullet) {
        if (checkBulletCollision(bullet)) {
            health--;
        }
    }

    bool isDead() const {
        if (health <= 0) {
            counted += 10;
        }
        return health <= 0;
    }

    void draw(RenderWindow& window) {
        window.draw(mob);
    }

    bool checkBulletCollision(Bullet& bullet) {
        if (bullet.bulletShape.getGlobalBounds().intersects(mob.getGlobalBounds())) {
            return true;
        }
        return false;
    }

    static void createEnemies(int map[][4], vector<Enemy>& enemies, float roomSize, const vector<Block>& walls) {
        srand(static_cast<unsigned int>(time(0)));

        for (int i = 0; i < 4; i++) {
            for (int k = 0; k < 4; k++) {
                if (map[i][k] == 1) {
                    int enemyCount = rand() % 4 + 1;
                    for (int q = 0; q < enemyCount; q++) {
                        Enemy mob;
                        Vector2f pos;
                        bool validPosition = false;

                        while (!validPosition) {
                            float x = (rand() % static_cast<int>(roomSize - 50)) + (k * roomSize) + 25;
                            float y = (rand() % static_cast<int>(roomSize - 50)) + (i * roomSize) + 25;
                            pos = Vector2f(x, y);
                            mob.setPosition(pos);
                            validPosition = true;

                            for (const auto& wall : walls) {
                                if (mob.mob.getGlobalBounds().intersects(wall.block.getGlobalBounds())) {
                                    validPosition = false;
                                    break;
                                }
                            }
                            for (const auto& enemy : enemies) {
                                if (mob.mob.getGlobalBounds().intersects(enemy.mob.getGlobalBounds())) {
                                    validPosition = false;
                                    break;
                                }
                            }
                        }
                        enemies.push_back(mob);
                    }
                }
            }
        }
    }
private:
    int health;
    Vector2f position;
    int damage;
    RectangleShape mob;

    bool checkEnemyCollisionPlayer(sf::RectangleShape& player) {
        if (mob.getGlobalBounds().intersects(player.getGlobalBounds())) {
            return true;
        }
        return false;
    }

    bool checkEnemyCollisionWalls(const vector<Block>& walls) {
        for (const auto& wall : walls) {
            if (mob.getGlobalBounds().intersects(wall.block.getGlobalBounds())) {
                return true;
            }
        }
        return false;
    }
};



void controlPlayer(Player& player, float time, Vector2f& direction, const vector<Block>& walls, vector<Enemy>& enemies, vector <Block>& passages);
void createBullet(RectangleShape& player, vector<Bullet>& bullets, Vector2f direction);
void createWalls(vector<Block>& walls, float windowWidth, float windowHeight, int map[][4], Player& player, vector <Block>& passages, View& view);
bool checkPlayerCollision(Player player, const vector<Block>& walls, const Vector2f& movement, vector<Enemy>& enemies);
bool checkBulletCollision(Bullet& bullet, const vector<Block>& walls);
void shooting(RectangleShape& player, vector<Bullet>& bullets, Vector2f& direction, float& bulletDelay);
void PlayerCollisionPassages(Player& player, vector <Block>& passages);

int main() {
    int map[4][4];

    do {
        genMap(map);
    } while (!isConnected(map));

    VideoMode desktop = VideoMode::getDesktopMode();
    RenderWindow window(desktop, "wow", Style::Fullscreen);

    View view;
    view.setSize(Vector2f(350.f, 350.f));
    View miniMapView;
    miniMapView.setSize(Vector2f(1400.f, 1400.f));
    miniMapView.setViewport(FloatRect(0.f, 0.f, 1.f, 0.25f));

    Texture texPlayer;
    texPlayer.loadFromFile("C:/Users/Msi/Downloads/Pixel Crawler - FREE - 1.8/Pixel Crawler - FREE - 1.8/Heroes/Wizzard/Idle/Idle-Sheet.png");
    int frameWidth = 32;
    int frameHeight = 32;
    int numFrames = 6;

    vector<IntRect> frames;
    for (int i = 0; i < numFrames; ++i) {
        frames.push_back(IntRect(i * frameWidth, 0, frameWidth, frameHeight));
    }

    Font font;
    font.loadFromFile("Inky-Thin-Pixels_0.ttf");
    Text text;
    text.setFont(font);
    text.setCharacterSize(25);
    text.setFillColor(Color::Yellow);
    text.setScale(1.f, 2.f);

    Text gameOver;
    gameOver.setFont(font);
    gameOver.setCharacterSize(50);
    gameOver.setFillColor(Color::Red);
    gameOver.setPosition(Vector2f(300, 700));
    gameOver.setScale(1.f, 2.f);

    Text win;
    win.setFont(font);
    win.setCharacterSize(70);
    win.setFillColor(Color::Red);
    win.setString("YOU WIN!!!");
    win.setPosition(Vector2f(300, 1200));
    win.setScale(1.f, 2.f);

    Player player;
    player.health = 5;
    player.damage = 10;
    player.damageCooldown = 0.0f;
    player.player.setSize(Vector2f(16, 20));

    player.player.setTexture(&texPlayer);
    player.player.setTextureRect(frames[0]);


    Clock clock;
    Time moveDelay = milliseconds(1);
    vector<Bullet> bullets;
    vector<Block> walls;
    vector <Block> passages;
    player.direction = Vector2f(0.f, 0.f);
    float bulletDelay = 0.0;
    float frameTime = 0.1f;
    int currentFrame = 0;

    Texture heart;
    heart.loadFromFile("C:/Users/Msi/Downloads/Hearts/PNG/basic/heart.png");
    Sprite hearts(heart);
    hearts.setScale(2, 10);


    createWalls(walls, window.getSize().x, window.getSize().y, map, player, passages, view);

    vector<Enemy> enemies;
    Enemy enemyTemplate;
    enemies.emplace_back();
    enemyTemplate.createEnemies(map, enemies, 350.f, walls);


    Texture tileset;
    tileset.loadFromFile("C:/Users/Msi/Downloads/32rogues/tiles.png");
    Sprite backgroundTile;
    backgroundTile.setTexture(tileset);
    backgroundTile.setTextureRect(IntRect(0, 0, 16, 16));
    backgroundTile.setScale(Vector2f(350.f / 16.f, 350.f / 16.f));

    while (window.isOpen()) {
        Time elapsedTime = clock.restart();
        float deltaTime = elapsedTime.asSeconds();
        Event event;

        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
        }

        if (player.damageCooldown > 0) {
            player.damageCooldown -= deltaTime;
        }

        if (clock.getElapsedTime().asSeconds() > frameTime) {
            currentFrame = (currentFrame + 1) % numFrames;
            player.player.setTextureRect(frames[currentFrame]);
            clock.restart();
        }

        controlPlayer(player, deltaTime, player.direction, walls, enemies, passages);

        float blockSize = 350.f;
        int roomX = static_cast<int>(player.player.getPosition().x / blockSize);
        int roomY = static_cast<int>(player.player.getPosition().y / blockSize);
        view.setCenter(roomX * blockSize + blockSize / 2, roomY * blockSize + blockSize / 2);

        for (auto& enemy : enemies) {
            enemy.move(player, walls, deltaTime);
            enemy.attack(player, walls, enemies,deltaTime);
        }

        for (int i = 0; i < bullets.size(); i++) {
            for (auto& enemy : enemies) {
                enemy.takeDamage(bullets[i]);
            }
        }

        if (bulletDelay <= 0) {
            shooting(player.player, bullets, player.direction, bulletDelay);
        }
        else {
            bulletDelay -= deltaTime;
        }

        for (auto& bullet : bullets) {
            bullet.bulletShape.move(bullet.velocity * deltaTime);
        }

        bullets.erase(remove_if(bullets.begin(), bullets.end(),
            [&walls, &enemies](Bullet& bullet) {
                return checkBulletCollision(bullet, walls) || any_of(enemies.begin(), enemies.end(),
                    [&bullet](Enemy& enemy) {
                        return enemy.checkBulletCollision(bullet);
                    });
            }), bullets.end());

        enemies.erase(remove_if(enemies.begin(), enemies.end(),
            [](Enemy& enemy) {
                return enemy.isDead();
            }), enemies.end());
        text.setString("Points : " + to_string(counted));

        window.clear();
        window.setView(view);

        for (int y = 0; y < window.getSize().y; y += 350) {
            for (int x = 0; x < window.getSize().x; x += 350) {
                backgroundTile.setPosition(x, y);
                window.draw(backgroundTile);
            }
        }

        for (auto& passage : passages) {
            window.draw(passage.block);
        }
        for (auto& bullet : bullets) {
            window.draw(bullet.bulletShape);
        }
        window.draw(player.player);
        for (auto& enemy : enemies) {
            enemy.draw(window);
        }

        for (auto& wall : walls) {
            wall.block.setTexture(&tileset);
            wall.block.setTextureRect(IntRect(0, 16, 16, 16));
            window.draw(wall.block);
        }

        if (player.health > 0 && enemies.size() == 0) {
            window.clear(Color::Black);
            window.draw(win);
        }

        window.setView(miniMapView);
        window.draw(text);

        int i = 0;
        int a = 650;
        switch (player.health) {
        case 1:
            i = 1;
            break;
        case 2:
            i = 2;
            break;
        case 3:
            i = 3;
            break;
        case 4:
            i = 4;
            break;
        case 5:
            i = 5;
            break;
        }
        for (int k = 0; k < i; k++) {
            hearts.setPosition(Vector2f(a += 22, 0));
            window.draw(hearts);
        }

        if (player.health <= 0) {
            window.clear(Color::Black);
            gameOver.setString("GAME OVER'\n'TOTAL POINTS " + to_string(counted));
            window.draw(gameOver);
        }

        window.display();
    }
}

void controlPlayer(Player& player, float time, Vector2f& direction, const vector<Block>& walls, vector<Enemy>& enemies, vector <Block>& passages) {
    float speed = 100.0f;

    Vector2f playerMovement(0.f, 0.f);
    direction = { 0.f, 0.f };
    if (Keyboard::isKeyPressed(Keyboard::W)) {
        playerMovement.y -= speed * time;
        direction.y = -1.f;
    }
    if (Keyboard::isKeyPressed(Keyboard::S)) {
        playerMovement.y += speed * time;
        direction.y = 1.f;
    }
    if (Keyboard::isKeyPressed(Keyboard::A)) {
        playerMovement.x -= speed * time;
        direction.x = -1.f;
    }
    if (Keyboard::isKeyPressed(Keyboard::D)) {
        playerMovement.x += speed * time;
        direction.x = 1.f;
    }

    if (!checkPlayerCollision(player, walls, playerMovement, enemies)) {
        player.player.move(playerMovement);
    }

    PlayerCollisionPassages(player, passages);
}

void createBullet(RectangleShape& player, vector<Bullet>& bullets, Vector2f direction) {
    const float speed = 150.0f;

    Bullet bullet;
    bullet.bulletShape.setRadius(2.f);
    bullet.bulletShape.setFillColor(Color::Red);
    bullet.bulletShape.setPosition(player.getPosition() + Vector2f(7, 13));
    bullet.velocity = direction * speed;
    bullets.push_back(bullet);
}

void createWalls(vector<Block>& walls, float windowWidth, float windowHeight, int map[][4], Player& player, vector<Block>& passages, View& view) {
    float thickness = 50.0f;
    float blockSize = 350.0f;

    for (int i = 0; i < 4; i++) {
        for (int k = 0; k < 4; k++) {
            if (map[i][k] == 1) {

                if (i == start_i && k == start_k) {
                    player.player.setPosition(blockSize * k + blockSize / 2 - player.player.getSize().x / 2, blockSize * i + blockSize / 2 - player.player.getSize().y / 2);
                    view.setCenter(blockSize * k + blockSize / 2 - player.player.getSize().x / 2, blockSize * i + blockSize / 2 - player.player.getSize().y / 2);
                }

                Block topWall;
                topWall.block.setSize(Vector2f(blockSize, thickness));
                topWall.block.setPosition(blockSize * k, blockSize * i);
                topWall.block.setFillColor(Color::Blue);
                walls.push_back(topWall);

                Block bottomWall;
                bottomWall.block.setSize(Vector2f(blockSize, thickness));
                bottomWall.block.setPosition(blockSize * k, blockSize * i + blockSize - thickness);
                bottomWall.block.setFillColor(Color::Blue);
                walls.push_back(bottomWall);

                Block leftWall;
                leftWall.block.setSize(Vector2f(thickness, blockSize));
                leftWall.block.setPosition(blockSize * k, blockSize * i);
                leftWall.block.setFillColor(Color::Blue);
                walls.push_back(leftWall);

                Block rightWall;
                rightWall.block.setSize(Vector2f(thickness, blockSize));
                rightWall.block.setPosition(blockSize * k + blockSize - thickness, blockSize * i);
                rightWall.block.setFillColor(Color::Blue);
                walls.push_back(rightWall);

                if (i > 0 && map[i - 1][k] == 1) {
                    Block topPassage;
                    topPassage.block.setSize(Vector2f(thickness, thickness * 2 + 10));
                    topPassage.block.setPosition(blockSize * k + blockSize / 2 - thickness / 2, blockSize * i - thickness);
                    topPassage.block.setFillColor(Color::White);
                    topPassage.name = "top";
                    passages.push_back(topPassage);
                }
                if (i < 3 && map[i + 1][k] == 1) {
                    Block bottomPassage;
                    bottomPassage.block.setSize(Vector2f(thickness, thickness * 2 + 10));
                    bottomPassage.block.setPosition(blockSize * k + blockSize / 2 - thickness / 2, (blockSize * i + blockSize - thickness) - 5);
                    bottomPassage.block.setFillColor(Color::White);
                    bottomPassage.name = "bottom";
                    passages.push_back(bottomPassage);
                }
                if (k > 0 && map[i][k - 1] == 1) {
                    Block leftPassage;
                    leftPassage.block.setSize(Vector2f(thickness * 2 + 10, thickness));
                    leftPassage.block.setPosition(blockSize * k - thickness, blockSize * i + blockSize / 2 - thickness / 2);
                    leftPassage.block.setFillColor(Color::White);
                    leftPassage.name = "left";
                    passages.push_back(leftPassage);
                }
                if (k < 3 && map[i][k + 1] == 1) {
                    Block rightPassage;
                    rightPassage.block.setSize(Vector2f(thickness * 2 + 10, thickness));
                    rightPassage.block.setPosition((blockSize * k + blockSize - thickness) - 5, blockSize * i + blockSize / 2 - thickness / 2);
                    rightPassage.block.setFillColor(Color::White);
                    rightPassage.name = "right";
                    passages.push_back(rightPassage);
                }
            }
        }
    }
}


bool checkPlayerCollision(Player player, const std::vector<Block>& walls, const Vector2f& movement, vector<Enemy>& enemies) {
    FloatRect playerBounds = player.player.getGlobalBounds();
    playerBounds.left += movement.x;
    playerBounds.top += movement.y;

    for (const auto& wall : walls) {
        if (playerBounds.intersects(wall.block.getGlobalBounds())) {
            return true;
        }
    }
    for (auto& enemy : enemies) {
        if (playerBounds.intersects(enemy.getShape().getGlobalBounds())) {
            return true;
        }
    }

    return false;
}

bool checkBulletCollision(Bullet& bullet, const vector<Block>& walls) {
    for (const auto& wall : walls) {
        if (bullet.bulletShape.getGlobalBounds().intersects(wall.block.getGlobalBounds())) {
            return true;
        }
    }
    return false;
}

void PlayerCollisionPassages(Player& player, vector<Block>& passages) {
    for (const auto& passage : passages) {
        if (player.player.getGlobalBounds().intersects(passage.block.getGlobalBounds())) {
            if (Keyboard::isKeyPressed(sf::Keyboard::W)) {
                if (passage.name == "top") {
                    player.player.move(Vector2f(0, -50));
                }
            }
            if (Keyboard::isKeyPressed(sf::Keyboard::S)) {
                if (passage.name == "bottom") {
                    player.player.move(Vector2f(0, 50));
                }
            }
            if (Keyboard::isKeyPressed(sf::Keyboard::A)) {
                if (passage.name == "left") {
                    player.player.move(Vector2f(-50, 0));
                }
            }
            if (Keyboard::isKeyPressed(sf::Keyboard::D)) {
                if (passage.name == "right") {
                    player.player.move(Vector2f(50, 0));
                }
            }
        }
    }
}

void shooting(RectangleShape& player, vector<Bullet>& bullets, Vector2f& direction, float& bulletDelay) {
    if (Keyboard::isKeyPressed(Keyboard::Up) && Keyboard::isKeyPressed(Keyboard::Right)) {
        direction = { 1.f, -1.f };
        createBullet(player, bullets, direction);
        bulletDelay = 0.3;
    }
    else if (Keyboard::isKeyPressed(Keyboard::Down) && Keyboard::isKeyPressed(Keyboard::Right)) {
        direction = { 1.f, 1.f };
        createBullet(player, bullets, direction);
        bulletDelay = 0.3;
    }
    else if (Keyboard::isKeyPressed(Keyboard::Up) && Keyboard::isKeyPressed(Keyboard::Left)) {
        direction = { -1.f, -1.f };
        createBullet(player, bullets, direction);
        bulletDelay = 0.3;
    }
    else if (Keyboard::isKeyPressed(Keyboard::Down) && Keyboard::isKeyPressed(Keyboard::Left)) {
        direction = { -1.f, 1.f };
        createBullet(player, bullets, direction);
        bulletDelay = 0.3;
    }
    else if (Keyboard::isKeyPressed(Keyboard::Up)) {
        direction = { 0.f, -1.f };
        createBullet(player, bullets, direction);
        bulletDelay = 0.3;
    }
    else if (Keyboard::isKeyPressed(Keyboard::Down)) {
        direction = { 0.f, 1.f };
        createBullet(player, bullets, direction);
        bulletDelay = 0.3;
    }
    else if (Keyboard::isKeyPressed(Keyboard::Left)) {
        direction = { -1.f, 0.f };
        createBullet(player, bullets, direction);
        bulletDelay = 0.3;
    }
    else if (Keyboard::isKeyPressed(Keyboard::Right)) {
        direction = { 1.f, 0.f };
        createBullet(player, bullets, direction);
        bulletDelay = 0.3;
    }
}