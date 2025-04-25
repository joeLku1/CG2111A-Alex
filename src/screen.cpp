#include "screen.h"

// Setup the LCD, I2C address 0x27, 16 columns and 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Array of messages to be displayed on the LCD screen
// Note: the message has to be less than 16 characters
const char* message_array[5] = {
  "Can you speak?",
  "What is yr name?",
  "Are you hurt?",
  "Are you alone?",
  "We are coming!"
};

/**
 * @brief Setup the LCD screen
 */
void screenSetup() {
	lcd.init();
	lcd.backlight(); 								// Turn on the backlight
	lcd.setCursor(0, 0); 						// Set cursor to the first column of the first row
	lcd.print("Rescue Mode ON!"); 	// Display message
}

/**
 * @brief Show a message on the LCD screen
 * @param message_no: The message number to display (0-4)
 */
void showMessage(uint32_t message_no) {
	lcd.clear(); 														// Clear the LCD screen
	lcd.setCursor(0, 0); 										// Move to the first column of the first row
	lcd.print("Reply by Voice:"); 					// Display message
	lcd.setCursor(0, 1); 										// Set cursor to the first column of the second row
	lcd.print(message_array[message_no]); 	// Display the message based on the parameter   
}