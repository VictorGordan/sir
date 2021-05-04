// Victor Gordan
#include <iostream>
#include <vector>
#include "SFML/Graphics.hpp"



const unsigned WIDTH_WINDOW = 1600;
const unsigned HEIGHT_WINDOW = 900;
const sf::String NAME_WINDOW = "SRI Model";
const unsigned int SPEED = 60;

const unsigned NUM_CELLS = 500;
const char* TOPOGRAPHY_FILE = "maps/england.png";
const float INITIAL_INFECTIOUS = 0.1f;
const float INFECTION_CHANCE = 0.5f;

const unsigned INFECTIOUS_TIME = 5; // MAYBE STUDY THESE?
const unsigned RESISTANT_TIME = 25; // MAYBE STUDY THESE?


const sf::Color COLOR_SUSCEPTIBLE = sf::Color(86, 178, 114, 255);
const sf::Color COLOR_INFECTIOUS = sf::Color(216, 36, 58, 255);
const sf::Color COLOR_RESISTANT = sf::Color(43, 118, 112, 255);

enum States
{
	Susceptible,
	Infectious,
	Resistant
};

struct Cell
{
	unsigned char state;
	float popDensity;
	unsigned int time;
	Cell* NW;
	Cell* N;
	Cell* NE;				//   Neighbours
	Cell* W;				//   NW  N   NE
	Cell* E;				//   W  cell E 
	Cell* SW;				//   SW  S   SE
	Cell* S;
	Cell* SE;
};

// Cell that will be the border to the field of cells
Cell resistantForPointer = { Resistant, 0.0f, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
Cell* RESISTANT = &resistantForPointer;

// The field that was displayed on the last frame
Cell prevField[NUM_CELLS][NUM_CELLS];
// The field that will be displayed on this frame
Cell crntField[NUM_CELLS][NUM_CELLS];

// Generates a random float number between 0 and 1
float randf()
{
	return (float)(rand() / (float)RAND_MAX);
}

// Initializes the field of cells
void initializeFields(sf::Texture topography)
{
	// Make Image of topography to read easily
	sf::Image topographyCopy = topography.copyToImage();
	// Initialize states, population density, and times
	for (unsigned int i = 0; i < NUM_CELLS; i++)
	{
		for (unsigned int j = 0; j < NUM_CELLS; j++)
		{
			if (randf() < INITIAL_INFECTIOUS && (float)topographyCopy.getPixel(i, j).r / 255.0f != 0.0f)
			{
				prevField[i][j] = Cell{ Infectious, (float)topographyCopy.getPixel(i, j).r / 255.0f, 0, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT };
				crntField[i][j] = Cell{ Infectious, (float)topographyCopy.getPixel(i, j).r / 255.0f, 0, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT };
			}
			else if ((float)topographyCopy.getPixel(i, j).r / 255.0f == 0.0f)
			{
				prevField[i][j] = Cell{ Resistant, 0.0f, 0, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT };
				crntField[i][j] = Cell{ Resistant, 0.0f, 0, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT };
			}
			else
			{
				prevField[i][j] = Cell{ Susceptible, (float)topographyCopy.getPixel(i, j).r / 255.0f, 0, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT };
				crntField[i][j] = Cell{ Susceptible, (float)topographyCopy.getPixel(i, j).r / 255.0f, 0, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT, RESISTANT };
			}
		}
	}
	// Initialize the neighbours
	for (unsigned int i = 0; i < NUM_CELLS; i++)
	{
		for (unsigned int j = 0; j < NUM_CELLS; j++)
		{
			if (i >= 1 && j >= 1)
			{
				prevField[i][j].NW = &prevField[i - 1][j - 1];
				crntField[i][j].NW = &crntField[i - 1][j - 1];
			}
			if (j >= 1)
			{
				prevField[i][j].N = &prevField[i][j - 1];
				crntField[i][j].N = &crntField[i][j - 1];
			}
			if (i + 1 < NUM_CELLS && j >= 1)
			{
				prevField[i][j].NE = &prevField[i + 1][j - 1];
				crntField[i][j].NE = &crntField[i + 1][j - 1];
			}
			if (i >= 1)
			{
				prevField[i][j].W = &prevField[i - 1][j];
				crntField[i][j].W = &crntField[i - 1][j];
			}
			if (i + 1 < NUM_CELLS)
			{
				prevField[i][j].E = &prevField[i + 1][j];
				crntField[i][j].E = &crntField[i + 1][j];
			}
			if (i >= 1 && j + 1 < NUM_CELLS)
			{
				prevField[i][j].SW = &prevField[i - 1][j + 1];
				crntField[i][j].SW = &crntField[i - 1][j + 1];
			}
			if (j + 1 < NUM_CELLS)
			{
				prevField[i][j].S = &prevField[i][j + 1];
				crntField[i][j].S = &crntField[i][j + 1];
			}
			if (i + 1 < NUM_CELLS && j + 1 < NUM_CELLS)
			{
				prevField[i][j].SE = &prevField[i + 1][j + 1];
				crntField[i][j].SE = &crntField[i + 1][j + 1];
			}
		}
	}
}

// Set the colors of an image to correspond with the properties of a field of cells
void imprintField(sf::Image* image)
{
	for (unsigned int i = 0; i < NUM_CELLS; i++)
	{
		for (unsigned int j = 0; j < NUM_CELLS; j++)
		{
			if (crntField[i][j].state == Susceptible)
			{
				image->setPixel(i, j, COLOR_SUSCEPTIBLE);
			}
			else if (crntField[i][j].state == Infectious)
			{
				image->setPixel(i, j, COLOR_INFECTIOUS);
			}
			else
			{
				image->setPixel(i, j, COLOR_RESISTANT);
			}
		}
	}
}

bool checkInfectious(unsigned int i, unsigned int j)
{
	if (prevField[i][j].NW->state == Infectious)
		return true;
	else if (prevField[i][j].N->state == Infectious)
		return true;
	else if (prevField[i][j].NE->state == Infectious)
		return true;
	else if (prevField[i][j].W->state == Infectious)
		return true;
	else if (prevField[i][j].E->state == Infectious)
		return true;
	else if (prevField[i][j].SW->state == Infectious)
		return true;
	else if (prevField[i][j].S->state == Infectious)
		return true;
	else if (prevField[i][j].SE->state == Infectious)
		return true;
	else
		return false;
}
// Handle updating of cells
void updateField()
{
	for (unsigned int i = 0; i < NUM_CELLS; i++)
	{
		for (unsigned int j = 0; j < NUM_CELLS; j++)
		{
			if (prevField[i][j].state == Susceptible && checkInfectious(i, j) && randf() < INFECTION_CHANCE * prevField[i][j].popDensity)
			{
				crntField[i][j].state = Infectious;
			}
			else if (prevField[i][j].state == Infectious)
			{
				if (prevField[i][j].time == INFECTIOUS_TIME)
				{
					crntField[i][j].state = Resistant;
					crntField[i][j].time = 0;
				}
				else if (randf() > prevField[i][j].popDensity)
				{
					crntField[i][j].time++;
				}
			}
			else if (prevField[i][j].state == Resistant && prevField[i][j].popDensity != 0.0f)
			{
				if (prevField[i][j].time == RESISTANT_TIME)
				{
					crntField[i][j].state = Susceptible;
					crntField[i][j].time = 0;
				}
				else
				{
					crntField[i][j].time++;
				}
			}
		}
	}
}



int main()
{
	// Create Window object
	sf::RenderWindow window;
	// Assign window object properties
	window.create(sf::VideoMode(WIDTH_WINDOW, HEIGHT_WINDOW), NAME_WINDOW);
	// Enable VSync
	window.setFramerateLimit(SPEED);





	// Create Texture object
	sf::Texture topography;
	// Load Texture
	if (!topography.loadFromFile(TOPOGRAPHY_FILE))
	{
		std::cout << "Failed to load the topography texture" << std::endl;
		return -1;
	}
	// Disable smoothing
	topography.setSmooth(false);


	// Create Sprite object to draw the texture
	sf::Sprite topographySprite;
	// Assign the texture to the sprite object
	topographySprite.setTexture(topography);
	// Center sprite
	topographySprite.setPosition(sf::Vector2f(WIDTH_WINDOW * 4.5 / 16, HEIGHT_WINDOW * 1 / 9));
	// Resize sprite
	float topographyScaleResize = ((float)WIDTH_WINDOW * 7.0f) / (16.0f * (float)topography.getSize().x);
	topographySprite.setScale(topographyScaleResize, topographyScaleResize);



	// Initialize the fields
	initializeFields(topography);

	// Create Image object
	sf::Image fieldImage;
	// Make the field white
	fieldImage.create(NUM_CELLS, NUM_CELLS, sf::Color(0, 0, 0));
	// Imprint the crntField on the fieldImage
	imprintField(&fieldImage);

	// Create Texture object
	sf::Texture fieldTex;
	// Make the Texture blank
	if (!fieldTex.create(NUM_CELLS, NUM_CELLS))
	{
		std::cout << "Failed to create the texture of the field" << std::endl;
		return -1;
	}
	// Disable smoothing
	fieldTex.setSmooth(false);
	// Impose the field onto the texture
	fieldTex.update(fieldImage);


	// Create Sprite object to draw the texture
	sf::Sprite fieldSprite;
	// Assign the texture to the sprite object
	fieldSprite.setTexture(fieldTex);
	// Center sprite
	fieldSprite.setPosition(sf::Vector2f(WIDTH_WINDOW * 4.5 / 16, HEIGHT_WINDOW * 1 / 9));
	// Resize sprite
	float fieldScaleResize = ((float)WIDTH_WINDOW * 7.0f) / (16.0f * (float)fieldTex.getSize().x);
	fieldSprite.setScale(fieldScaleResize, fieldScaleResize);



	// Main loop
	while (window.isOpen())
	{
		// Event handling
		sf::Event event;
		while (window.pollEvent(event))
		{
			// Handle closing of window
			if (event.type == sf::Event::Closed)
				window.close();
		}


		updateField();
		// Imprint the crntField on the fieldImage
		imprintField(&fieldImage);
		// Impose the field onto the texture
		fieldTex.update(fieldImage);



		// Clear window
		window.clear(sf::Color(25, 25, 30));
		// Draw topography
		window.draw(topographySprite);
		// Draw field
		window.draw(fieldSprite);
		// Swap buffers
		window.display();
		// Swap fields
		for (unsigned int i = 0; i < NUM_CELLS; i++)
		{
			for (unsigned int j = 0; j < NUM_CELLS; j++)
			{
				prevField[i][j] = crntField[i][j];
			}
		}
	}

	return 0;
}