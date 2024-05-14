#include <U8glib.h>
#include <limits.h> // Include to use INT_MAX

// Define display dimensions
const int screenWidth = 240;
const int screenHeight = 128;

// Define object sizes
const int objectSize = 2;

// Define maximum number of segments for the snake
const int maxSegments = 100;

// Initialize the U8glib display object with the specified pinout
U8GLIB_T6963_240X128 u8g(8, 9, 10, 11, 4, 5, 6, 7, 14, 15, 17, 18, 16); // Pins: CS=8, A0=9, WR=10, RD=11, RESET=4, DB0=5, DB1=6, DB2=7, DB3=14, DB4=15, DB5=17, DB6=18, DB7=16

// Define snake segments
struct Segment {
  int x;
  int y;
};

Segment snake[maxSegments];

// Define food position
int foodX, foodY;

// Define snake length
int snakeLength = 1;

// Flag to track if the snake has eaten food
bool ateFood = false;

// Flag to track if the game is over
bool gameOver = false;

// Counter for food pellets eaten
int foodEaten = 0;

// Define current movement direction of the snake
int direction = 1; // 0: UP, 1: RIGHT, 2: DOWN, 3: LEFT

// Function prototypes
void drawSnake();
void moveSnake();
bool isFoodOnSnake();
void placeFood();
int heuristic(int x1, int y1, int x2, int y2);
int findNextDirection();

void setup() {
  // Initialize the display
  u8g.begin();
  // Seed random number generator
  randomSeed(analogRead(0));
  // Initialize the snake
  snake[0].x = 3;
  snake[0].y = 3;
  // Place initial food
  placeFood();
}

void loop() {
  if (!gameOver) {
    // Clear the display
    u8g.firstPage();
    do {
      // Draw the snake
      drawSnake();
      // Draw the food
      u8g.setColorIndex(1); // White color for food
      u8g.drawBox(foodX * objectSize, foodY * objectSize, objectSize, objectSize);
      // Draw the food counter
      u8g.setFont(u8g_font_helvB08); // Set font size
      u8g.setColorIndex(1); // White color for text
      char counterText[20];
      sprintf(counterText, "Food Eaten: %d", foodEaten);
      int textWidth = u8g.getStrWidth(counterText);
      int xPos = (screenWidth - textWidth) / 2;
      int yPos = screenHeight - 10;
      u8g.drawStr(xPos, yPos, counterText); // Draw the text
      // Move the snake
      moveSnake();
    } while (u8g.nextPage());
  } else {
    // Game over, display message
    u8g.firstPage();
    do {
      u8g.setFont(u8g_font_helvB08); // Set font size
      u8g.setColorIndex(1); // White color for text
      // Calculate text position to center it
      int textWidth = u8g.getStrWidth("The AI Lost!");
      int xPos = (screenWidth - textWidth) / 2;
      int yPos = screenHeight / 2;
      u8g.drawStr(xPos, yPos, "The AI Lost!"); // Draw the text
    } while (u8g.nextPage());
    delay(2000); // Display message for 2 seconds
    // Reset the game
    snakeLength = 1;
    snake[0].x = 3;
    snake[0].y = 3;
    placeFood();
    gameOver = false;
    foodEaten = 0; // Reset food eaten counter
  }
}

// Function to draw the snake
void drawSnake() {
  u8g.setColorIndex(1); // White color for snake
  for (int i = 0; i < snakeLength; i++) {
    u8g.drawBox(snake[i].x * objectSize, snake[i].y * objectSize, objectSize, objectSize);
  }
}

// Function to move the snake
void moveSnake() {
  // Find the next direction using A* algorithm
  int nextDirection = findNextDirection();

  // Update the current movement direction
  direction = nextDirection;

  // Move the snake head
  int newX = snake[0].x;
  int newY = snake[0].y;
  switch (direction) {
    case 0: // UP
      newY -= 1;
      break;
    case 1: // RIGHT
      newX += 1;
      break;
    case 2: // DOWN
      newY += 1;
      break;
    case 3: // LEFT
      newX -= 1;
      break;
  }

  // Check if the new position of the snake's head is valid
  if (newX < 0 || newX >= screenWidth / objectSize || newY < 0 || newY >= screenHeight / objectSize) {
    // Set game over flag
    gameOver = true;
    return;
  }
  // Check if the snake's head collides with its own body
  for (int i = 1; i < snakeLength; i++) {
    if (newX == snake[i].x && newY == snake[i].y) {
      // Set game over flag
      gameOver = true;
      return;
    }
  }

  // Check if the head has reached the food
  if (newX == foodX && newY == foodY) {
    // Set ateFood flag
    ateFood = true;
    // Increment food eaten counter
    foodEaten++;
    // Place new food
    placeFood();
  }

  // Move the snake body
  for (int i = snakeLength - 1; i > 0; i--) {
    snake[i] = snake[i - 1];
  }
  snake[0].x = newX;
  snake[0].y = newY;

  // If snake has eaten food in the previous iteration, grow the tail
  if (ateFood) {
    snakeLength++;
    ateFood = false;
  }

  // No delay for snake movement speed
  delay(0);
}

// Function to calculate the Manhattan distance heuristic
int heuristic(int x1, int y1, int x2, int y2) {
  return abs(x1 - x2) + abs(y1 - y2);
}

// Function to find the next direction using A* algorithm
int findNextDirection() {
  int currentX = snake[0].x;
  int currentY = snake[0].y;

  int minDistance = INT_MAX;
  int nextDirection = direction; // Initialize with current direction

  // Try all possible directions and choose the one with the minimum distance to food
  for (int d = 0; d < 4; d++) {
    int newX = currentX;
    int newY = currentY;
    switch (d) {
      case 0: // UP
        newY--;
        break;
      case 1: // RIGHT
        newX++;
        break;
      case 2: // DOWN
        newY++;
        break;
      case 3: // LEFT
        newX--;
        break;
    }
    int distance = heuristic(newX, newY, foodX, foodY);
    if (distance < minDistance) {
      minDistance = distance;
      nextDirection = d;
    }
  }

  // Prevent snake from doubling back
  if (nextDirection == (direction + 2) % 4) {
    // If the next direction is opposite to the current direction, try to turn right or left instead
    int rightDirection = (direction + 1) % 4;
    int leftDirection = (direction + 3) % 4;
    if (heuristic(currentX, currentY, snake[0].x, snake[0].y - 1) < heuristic(currentX, currentY, snake[0].x, snake[0].y + 1)) {
      nextDirection = rightDirection;
    } else {
      nextDirection = leftDirection;
    }
  }

  return nextDirection;
}

// Function to place the food at a random position
void placeFood() {
  do {
    foodX = random(screenWidth / objectSize);
    foodY = random(screenHeight / objectSize);
  } while (isFoodOnSnake());
}

// Function to check if food is placed on the snake's body
bool isFoodOnSnake() {
  for (int i = 0; i < snakeLength; i++) {
    if (foodX == snake[i].x && foodY == snake[i].y) {
      return true;
    }
  }
  return false;
}
