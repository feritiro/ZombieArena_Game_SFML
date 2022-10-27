#include <SFML/Graphics.hpp>
#include "Player.h"
#include "ZombieArena.h"
#include "TextureHolder.h"
#include <fstream>
#include "Bullet.h"
#include "Pickup.h"
#include <sstream>
#include <SFML/Audio.hpp>
using namespace sf;

int main() {
	TextureHolder holder;

	enum class State{PAUSED, LEVELING_UP,GAME_OVER,PLAYING};
	State state = State::GAME_OVER;

	Vector2f resolution;
	resolution.x = VideoMode::getDesktopMode().width;
	resolution.y = VideoMode::getDesktopMode().height;
	RenderWindow window(VideoMode(resolution.x, resolution.y), "Zombie Arena", Style::Fullscreen);
	View mainView(sf::FloatRect(0, 0, resolution.x, resolution.y));

	Clock clock;
	Time gameTimeTotal;

	Vector2f mouseWorldPosition;
	Vector2i mouseScreenPosition;

	Player player;
	IntRect arena;

	VertexArray background;
	Texture textureBackground = TextureHolder::GetTexture("graphics/background_sheet.png");

	int numZombies;
	int numZombiesAlive;
	Zombie* zombies = nullptr;
	Bullet bullets[100];
	int currentBullets = 0;
	int bulletsSpare = 24;
	int bulletsInClip = 6;
	int clipSize = 6;
	float fireRate = 1;
	Time lastPressed;

	window.setMouseCursorVisible(true);
	Sprite spriteCrosshair;
	Texture textureCrosshair = TextureHolder::GetTexture("graphics/crosshair.png");
	spriteCrosshair.setTexture(textureCrosshair);
	spriteCrosshair.setOrigin(12, 12);

	Pickup healthPickup(1);
	Pickup ammoPickup(2);

	int score = 0;
	int hiScore = 0;

	Sprite spriteGameOver;
	Texture textureGameOver = TextureHolder::GetTexture("graphics/background.png");
	spriteGameOver.setTexture(textureGameOver);
	spriteGameOver.setPosition(0, 0);
	View hudView(sf::FloatRect(0, 0, resolution.x, resolution.y));

	Sprite spriteAmmoIcon;
	Texture textureAmmoIcon = TextureHolder::GetTexture("graphics/ammo_icon.png");
	spriteAmmoIcon.setTexture(textureAmmoIcon);
	spriteAmmoIcon.setPosition(10, 460);

	Font font;
	font.loadFromFile("fonts/zombiecontrol.ttf");

	Text pausedText;
	pausedText.setFont(font);
	pausedText.setCharacterSize(80);
	pausedText.setFillColor(Color::White);
	pausedText.setPosition(200, 200);
	pausedText.setString("Press Enter \nto continue");

	Text gameOverText;
	gameOverText.setFont(font);
	gameOverText.setCharacterSize(80);
	gameOverText.setFillColor(Color::White);
	gameOverText.setPosition(125, 400);
	gameOverText.setString("Press Enter to play");

	Text levelUpText;
	levelUpText.setFont(font);
	levelUpText.setCharacterSize(40);
	levelUpText.setFillColor(Color::White);
	levelUpText.setPosition(75, 125);
	std::stringstream levelUpStream;
	levelUpStream <<
		"1 - increased rate of fire" <<
		"\n2 - increased clip size" <<
		"\n3 - increased max health" <<
		"\n4 - increased run speed" <<
		"\n5 - more and better health pickups" <<
		"\n6 - more and better ammo pickups";
	levelUpText.setString(levelUpStream.str());

	Text ammoText;
	ammoText.setFont(font);
	ammoText.setCharacterSize(30);
	ammoText.setFillColor(Color::White);
	ammoText.setPosition(100, 440);

	Text scoreText;
	scoreText.setFont(font);
	scoreText.setCharacterSize(30);
	scoreText.setFillColor(Color::White);
	scoreText.setPosition(10, 0);

	std::ifstream inputFile("gamedata/scores.txt");
	if (inputFile.is_open()) {
		inputFile >> hiScore;
		inputFile.close();
	}

	Text hiScoreText;
	hiScoreText.setFont(font);
	hiScoreText.setCharacterSize(30);
	hiScoreText.setFillColor(Color::White);
	hiScoreText.setPosition(700, 0);
	std::stringstream s;
	s << "Hi Score" << hiScore;
	hiScoreText.setString(s.str());

	Text zombiesRemainingText;
	zombiesRemainingText.setFont(font);
	zombiesRemainingText.setCharacterSize(30);
	zombiesRemainingText.setFillColor(Color::White);
	zombiesRemainingText.setPosition(720, 440);
	zombiesRemainingText.setString("Zombies: 100");

	int wave = 0;
	Text waveNumberText;
	waveNumberText.setFont(font);
	waveNumberText.setCharacterSize(30);
	waveNumberText.setFillColor(Color::White);
	waveNumberText.setPosition(720, 430);
	waveNumberText.setString("Wave: 0");

	RectangleShape healthBar;
	healthBar.setFillColor(Color::Red);
	healthBar.setPosition(220, 430);

	Text debugText;
	debugText.setFont(font);
	debugText.setCharacterSize(20);
	debugText.setFillColor(Color::White);
	debugText.setPosition(10, 120);
	std::ostringstream ss;
	int framesSinceLastHUDUpdate = 0;
	int fpsMeasurementFrameInterval = 1000;

	SoundBuffer hitBuffer;
	hitBuffer.loadFromFile("sound/hit.wav");
	Sound hit;
	hit.setBuffer(hitBuffer);
	SoundBuffer splatBuffer;
	splatBuffer.loadFromFile("sound/splat.wav");
	Sound splat;
	splat.setBuffer(splatBuffer);
	SoundBuffer shootBuffer;
	shootBuffer.loadFromFile("sound/shoot.wav");
	Sound shoot;
	shoot.setBuffer(shootBuffer);
	SoundBuffer reloadBuffer;
	reloadBuffer.loadFromFile("sound/reload.wav");
	Sound reload;
	reload.setBuffer(reloadBuffer);
	SoundBuffer reloadFailedBuffer;
	reloadFailedBuffer.loadFromFile("sound/reload_failed.wav");
	Sound reloadFailed;
	reloadFailed.setBuffer(reloadFailedBuffer);
	SoundBuffer powerupBuffer;
	powerupBuffer.loadFromFile("sound/powerup.wav");
	Sound powerup;
	powerup.setBuffer(powerupBuffer);
	SoundBuffer pickupBuffer;
	pickupBuffer.loadFromFile("sound/pickup.wav");
	Sound pickup;
	pickup.setBuffer(pickupBuffer);





	while (window.isOpen()) {

		Event event;
		while (window.pollEvent(event)) {
			if (event.type == Event::KeyPressed) {
				if (event.key.code == Keyboard::Return && state == State::PLAYING)
					state = State::PAUSED;
				else if (event.key.code == Keyboard::Return && state == State::PAUSED) {
					state = State::PLAYING;
					clock.restart();
				}
				else if (event.key.code == Keyboard::Return && state == State::GAME_OVER) {
					state = State::LEVELING_UP;
					wave = 0;
					score = 0;
					currentBullets = 0;
					bulletsSpare = 24;
					bulletsInClip = 6;
					clipSize = 6;
					fireRate = 1;
					player.resetPlayerStatus();
				}
				if (state == State::PLAYING){
					if (event.key.code == Keyboard::R) {
						if (bulletsSpare >= clipSize) {
							bulletsInClip = clipSize;
							bulletsSpare -= clipSize;
							reload.play();
						}
						else if (bulletsSpare > 0) {
							bulletsInClip = bulletsSpare;
							bulletsSpare = 0;
							reload.play();
						}
						else {
							reloadFailed.play();
						}
					}
				}
			}
		}
		if (Keyboard::isKeyPressed(Keyboard::Escape))
			window.close();

		if (state == State::PLAYING) {
			if (Keyboard::isKeyPressed(Keyboard::W))
				player.moveUp();
			else
				player.stopUp();
			if (Keyboard::isKeyPressed(Keyboard::S))
				player.moveDown();
			else
				player.stopDown();
			if (Keyboard::isKeyPressed(Keyboard::A))
				player.moveLeft();
			else
				player.stopLeft();
			if (Keyboard::isKeyPressed(Keyboard::D))
				player.moveRight();
			else
				player.stopRight();
			if (Mouse::isButtonPressed(sf::Mouse::Left)) {
				if (gameTimeTotal.asMilliseconds() - lastPressed.asMilliseconds() > 1000 / fireRate && bulletsInClip > 0) {
					bullets[currentBullets].shoot(player.getCenter().x, player.getCenter().y, mouseWorldPosition.x, mouseWorldPosition.y);
					currentBullets++;
					if (currentBullets > 99)
						currentBullets = 0;
					lastPressed = gameTimeTotal;
					shoot.play();
					bulletsInClip--;
				}
			}
		}
		if (state == State::LEVELING_UP) {
			if (event.key.code == Keyboard::Num1) {
				fireRate++;
				state = State::PLAYING;
			}
			if (event.key.code == Keyboard::Num2) {
				clipSize += clipSize;
				state = State::PLAYING;
			}
			if (event.key.code == Keyboard::Num3) {
				player.upgradeHealth();
				state = State::PLAYING;
			}
			if (event.key.code == Keyboard::Num4) {
				player.upgradeSpeed();
				state = State::PLAYING;
			}
			if (event.key.code == Keyboard::Num5) {
				healthPickup.upgrade();
				state = State::PLAYING;
			}
			if (event.key.code == Keyboard::Num6) {
				ammoPickup.upgrade();
				state = State::PLAYING;
			}

			if (state == State::PLAYING) {
				wave++;
				arena.width = 500 * wave;
				arena.height = 500 * wave;
				arena.left = 0;
				arena.top = 0;
				int tileSize = createBackground(background, arena);
				player.spawn(arena, resolution, tileSize);
				healthPickup.setArena(arena);
				ammoPickup.setArena(arena);
				numZombies = 5 * wave;
				delete[] zombies;
				zombies = createHorde(numZombies, arena);
				numZombiesAlive = numZombies;
				powerup.play();
				clock.restart();
			}
		}
		if (state == State::PLAYING) {
			Time dt = clock.restart();
			gameTimeTotal += dt;
			float dtAsSeconds = dt.asSeconds();
			mouseScreenPosition = Mouse::getPosition();
			mouseWorldPosition = window.mapPixelToCoords(Mouse::getPosition(), mainView);
			spriteCrosshair.setPosition(mouseWorldPosition);
			player.update(dtAsSeconds, Mouse::getPosition());
			Vector2f playerPosition(player.getCenter());
			mainView.setCenter(player.getCenter());
			for (int i = 0; i < numZombies; i++) 
				if (zombies[i].isAlive())
					zombies[i].update(dt.asSeconds(), playerPosition);

			for (int i = 0; i < 100; i++)
				if (bullets[i].isInFlight())
					bullets[i].update(dtAsSeconds);

			healthPickup.update(dtAsSeconds);
			ammoPickup.update(dtAsSeconds);

			for (int i = 0; i < 100; i++) {
				for (int j = 0; j < numZombies; j++) {
					if (bullets[i].isInFlight() && zombies[j].isAlive()) {
						if (bullets[i].getPosition().intersects(zombies[j].getPosition())) {
							bullets[i].stop();
							if (zombies[j].hit()) {
								score+=10;
								if (score >= hiScore)
									hiScore = score;
								numZombiesAlive--;
								if (numZombiesAlive == 0)
									state = State::LEVELING_UP;
								splat.play();
							}
						}
					}
				}
			}
			for (int i = 0; i < numZombies; i++) {
				if (player.getPosition().intersects(zombies[i].getPosition()) && zombies[i].isAlive()) {
					if (player.hit(gameTimeTotal)){
						hit.play();
					}
					if (player.getHealth() <= 0) {
						state = State::GAME_OVER;
						std::ofstream outputFile("gamedata/scores.txt");
						outputFile << hiScore;
						outputFile.close();
					}
				}
				if (player.getPosition().intersects(healthPickup.getPosition()) && healthPickup.isSpawned()) {
					player.increaseHealthLevel(healthPickup.gotIt());
					pickup.play();
				}

				if (player.getPosition().intersects(ammoPickup.getPosition()) && ammoPickup.isSpawned()) {
					bulletsSpare += ammoPickup.gotIt();
					pickup.play();
				}

				healthBar.setSize(Vector2f(player.getHealth() * 3, 25));
				framesSinceLastHUDUpdate++;
				if (framesSinceLastHUDUpdate > fpsMeasurementFrameInterval) {
					std::stringstream ssAmmo;
					std::stringstream ssScore;
					std::stringstream ssHiScore;
					std::stringstream ssWave;
					std::stringstream ssZombiesAlive;
					ssAmmo << bulletsInClip << "/" << bulletsSpare;
					ammoText.setString(ssAmmo.str());
					ssScore << "Score: " << score;
					scoreText.setString(ssScore.str());
					ssHiScore << "HiScore: " << hiScore;
					hiScoreText.setString(ssHiScore.str());
					ssWave << "Wave: " << wave;
					waveNumberText.setString(ssWave.str());
					ssZombiesAlive << "Zombies: " << numZombiesAlive;
					zombiesRemainingText.setString(ssZombiesAlive.str());
					framesSinceLastHUDUpdate = 0;
				}
			}

		}

		if (state == State::PLAYING) {
			window.clear();
			window.setView(mainView);
			window.draw(background, &textureBackground);

			for (int i = 0; i < numZombies; i++)
				window.draw(zombies[i].getSprite());

			for (int i = 0; i < 100; i++)
				if (bullets[i].isInFlight())
					window.draw(bullets[i].getShape());

			window.draw(player.getSprite());

			if (ammoPickup.isSpawned())
				window.draw(ammoPickup.getSprite());

			if (healthPickup.isSpawned())
				window.draw(healthPickup.getSprite());

			window.draw(spriteCrosshair);
			window.setView(hudView);
			window.draw(spriteAmmoIcon);
			window.draw(ammoText);
			window.draw(scoreText);
			window.draw(hiScoreText);
			window.draw(healthBar);
			window.draw(waveNumberText);
			window.draw(zombiesRemainingText);
		}
		if (state == State::LEVELING_UP) {
			window.draw(spriteGameOver);
			window.draw(levelUpText);
		}
		if (state == State::PAUSED) {
			window.draw(pausedText);
		}
		if (state == State::GAME_OVER) {
			window.draw(spriteGameOver);
			window.draw(gameOverText);
			window.draw(scoreText);
			window.draw(hiScoreText);
		}
		window.display();
	}
	delete[] zombies;
	return 0;
}