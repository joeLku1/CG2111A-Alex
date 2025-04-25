#include "arm_color.h"
#include "constants.h"

/**
 * ========================================================
 * Servo functions
 * ========================================================
 */

// Sets up the servos
void servoSetup() {
	// Attach the servos to the pins
	PRR1 = 0;															// Disable power reduction for Timer 5 (PRTIM5 = 0)

	// Set pins 44 and 45 as output
	DDRL |= (1 << DDL4) | (1 << DDL5); 		// PL4 and 5, corresponding to OC5B and OC5C

	// Set phase correct PWM mode
	// CS = 0b010, prescaler = 8
	// WGM = 0b1010, phase correct PWM mode
	TCCR5A |= (1 << COM5B1) | (1 << COM5C1) | (1 << WGM51);
	TCCR5B |= (1 << WGM53) | (1 << CS51);

	// Set ICR5 for period
	ICR5 = 20000; 												// Set TOP value for 20ms period

	// Set the initial position of the servos
	servoClose();
}

// Open the claw
void servoOpen() {
	OCR5B = (1000 * (openAngleLeft / 180)) + 1000; 				// Left servo
	OCR5C = (1000 * (openAngleRight / 180)) + 1000; 			// Right servo
}

// Close the claw
void servoClose() {
	OCR5B = (1000 * (closeAngleLeft1 / 180)) + 1000; 			// Left servo
	OCR5C = (1000 * (closeAngleRight1 / 180)) + 1000; 		// Right servo
	delay(500);
	OCR5B = (1000 * (closeAngleLeft2 / 180)) + 1000;		 	// Left servo
	OCR5C = (1000 * (closeAngleRight2 / 180)) + 1000; 		// Right servo
}


/**
 * ========================================================
 * Color sensor functions
 * ========================================================
 */


// Setup the color sensor
void colourSensorSetup(){
	// Set S0: 30 (PC7), S1: 32 (PC5), S2: 33 (PC4), S3: 31 (PC6) as output
	DDRC |= (1 << DDC7) | (1 << DDC5) | (1 << DDC4) | (1 << DDC6);

	// Set sensorOut: 35 (PC2) as input
	DDRC &= ~(1 << DDC2);
	
	// For 20% scaling, set S0: 30 (PC7) HIGH and S1: 32 (PC5) LOW
	PORTC |= (1 << PC7);  	// Set S0 HIGH
	PORTC &= ~(1 << PC5); 	// Set S1 LOW
}

/**
 * @brief Reads the color from the color sensor and evaluates it.
 * @return String: Returns the color detected ("red", "green", or "not green or red").
 */
String readColor() {
	// RED
	PORTC &= ~(1 << PC4); 	// Set S2 LOW
	PORTC &= ~(1 << PC6); 	// Set S3 LOW
	unsigned long redFrequency = pulseIn(sensorOut, LOW);
	
	// GREEN
	PORTC |= (1 << PC4);  	// Set S2 HIGH
	PORTC |= (1 << PC6);  	// Set S3 HIGH
	unsigned long greenFrequency = pulseIn(sensorOut, LOW);
	
	// BLUE
	PORTC &= ~(1 << PC4); 	// Set S2 LOW
	PORTC |= (1 << PC6);  	// Set S3 HIGH
	unsigned long blueFrequency = pulseIn(sensorOut, LOW);

	color raw_color = {(float) redFrequency, (float) greenFrequency, (float) blueFrequency, "unknown"};
	
	String s = predict_color(raw_color);
	return s;
}


const float MAX_FLOAT = 3.4028235e+38f;

// Red & green dataset, updated after normalizing
std::vector<color> dataset = {
	// Red values
	{240.7, 15.3, 10.2, "red"}, {232.3, 24.9, 18.6, "red"}, {250.1, 5.8, 4.3, "red"}, 
	{220.5, 35.6, 30.1, "red"}, {238.4, 17.5, 22.8, "red"}, {245.9, 12.7, 9.4, "red"},
	{210.2, 45.3, 40.7, "red"}, {215.8, 40.6, 38.2, "red"}, {230.4, 24.1, 19.8, "red"},
	{225.9, 30.6, 28.5, "red"}, {242.3, 12.8, 13.9, "red"}, {218.7, 36.4, 35.1, "red"},
	{234.6, 20.7, 22.6, "red"}, {226.9, 28.1, 27.5, "red"}, {248.2, 7.5, 8.3, "red"},
	{237.2, 18.9, 19.3, "red"}, {213.4, 42.5, 41.8, "red"}, {246.7, 8.9, 10.3, "red"},
	{222.3, 33.7, 32.5, "red"}, {208.6, 47.2, 45.9, "red"}, {251.8, 3.2, 6.4, "red"},
	{235.1, 20.4, 16.7, "red"}, {219.8, 35.2, 36.9, "red"}, {244.5, 11.6, 12.4, "red"},
	{231.2, 23.8, 21.5, "red"}, {216.9, 39.4, 37.5, "red"}, {249.3, 6.8, 7.1, "red"},
	{227.7, 27.9, 25.6, "red"}, {236.5, 19.5, 20.8, "red"}, {223.4, 32.1, 29.7, "red"},

	// Green values
	{24.3, 230.7, 20.4, "green"}, {19.8, 245.3, 18.9, "green"}, {30.2, 225.8, 29.7, "green"}, 
	{12.5, 242.6, 15.8, "green"}, {40.7, 210.3, 38.9, "green"}, {22.8, 232.5, 21.3, "green"},
	{35.6, 220.9, 33.4, "green"}, {15.3, 239.7, 14.2, "green"}, {45.8, 205.2, 43.6, "green"},
	{27.9, 227.3, 26.5, "green"}, {10.2, 249.8, 12.7, "green"}, {38.4, 215.6, 36.1, "green"},
	{20.5, 235.4, 19.7, "green"}, {42.3, 208.7, 40.5, "green"}, {16.7, 238.3, 15.9, "green"},
	{32.8, 222.6, 31.4, "green"}, {25.6, 229.8, 24.3, "green"}, {8.9, 251.1, 9.5, "green"},
	{37.2, 217.8, 35.8, "green"}, {13.6, 241.5, 14.3, "green"}, {44.5, 206.9, 42.8, "green"},
	{28.7, 226.4, 27.6, "green"}, {18.2, 237.9, 17.4, "green"}, {39.5, 214.2, 38.3, "green"},
	{21.9, 234.1, 20.8, "green"}, {47.3, 203.7, 45.9, "green"}, {14.8, 240.2, 13.7, "green"},
	{33.9, 221.5, 32.6, "green"}, {23.6, 231.3, 22.9, "green"}, {11.4, 248.6, 10.8, "green"}
};

// Hardcoded sensor black and white references
color black_ref = {929, 813 ,569};  	// Black reference
color white_ref = {121, 136, 123}; 		// White reference

/**
 * @brief Normalize input RGB using black and white references, scaling to 0-255 range
 * @param input: The raw RGB input from the color sensor
 * @param black_ref: The black reference color
 * @param white_ref: The white reference color
 * @return color: The normalized RGB color
 */
color normalize_rgb(const color& input, const color& black_ref, const color& white_ref) {
	color norm;
	
	// Normalize the RGB input based on the difference between black and white references
	norm.r = ((input.r - white_ref.r) / -(white_ref.r - black_ref.r)) * 255.0f;
	norm.g = ((input.g - white_ref.g) / -(white_ref.g - black_ref.g)) * 255.0f;
	norm.b = ((input.b - white_ref.b) / -(white_ref.b - black_ref.b)) * 255.0f;

	// Clamp values to ensure they stay within 0-255 range
	norm.r = cmax(0.0f, cmin(255.0f, norm.r));
	norm.g = cmax(0.0f, cmin(255.0f, norm.g));
	norm.b = cmax(0.0f, cmin(255.0f, norm.b));
	return norm;
}

/**
 * @brief Compute squared Euclidean distance between two colors
 * @param current: The current color
 * @param sample: The sample color to compare against
 * @return colordiff: The squared distance and the color string
 */
colordiff eucdistsqr(const color& current, const color& sample) {
	colordiff node;
	node.distsqr = ((current.r - sample.r) * (current.r - sample.r)) +
									((current.g - sample.g) * (current.g - sample.g)) +
									((current.b - sample.b) * (current.b - sample.b));
	node.colorstring = sample.colorstring;
	return node;
}

/**
 * @brief KNN classification for red and green colors
 * @param input_color: The input color to classify
 * @param k: The number of neighbors to consider
 * @return String: The predicted color ("red", "green", or "not green or red")
 */
String knn_classification_red_green(const color& input_color, int k) {
	std::vector<colordiff> distances;

	for (const auto& sample : dataset) {
		distances.push_back(eucdistsqr(input_color, sample));
	}

	std::sort(distances.begin(), distances.end(),
						[](const colordiff& a, const colordiff& b) {
								return a.distsqr < b.distsqr;
						});

	int red_count = 0, green_count = 0;

	for (int i = 0; i < cmin(k, (int)distances.size()); i++) {
		if (distances[i].colorstring == "red") red_count++;
		else if (distances[i].colorstring == "green") green_count++;
	}

	if (red_count > green_count) return "red";
	else if (green_count > red_count) return "green";
	else return "not green or red";
}

/**
 * @brief Predict color based on input and distance threshold
 * @param raw_input: The raw input color from the sensor
 * @return String: The predicted color ("red", "green", or "not green or red")
 */
String predict_color(const color& raw_input) {
	int k_neighbors = 3;
	float threshold = 200;  // distance threshold

	// Normalize input
	color input = normalize_rgb(raw_input, black_ref, white_ref);

	// Check if it is too far from known red/green
	float min_distance = MAX_FLOAT;
	for (const auto& sample : dataset) {
		float dist = ((input.r - sample.r) * (input.r - sample.r)) +
								((input.g - sample.g) * (input.g - sample.g)) +
								((input.b - sample.b) * (input.b - sample.b));
		if (dist < min_distance) min_distance = dist; 
	}

	if (min_distance > threshold) return "not green or red"; 

	String predicted = knn_classification_red_green(input, k_neighbors);

	return predicted;
}
