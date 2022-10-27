#pragma once
#include <SFML/Graphics.hpp>
using namespace sf;
#include "Zombie.h"

int createBackground(VertexArray& rVA, IntRect arena);
Zombie* createHorde(int numZombies, IntRect arena);
