/*
  ESPIR_Menu.h - Menu for ST7735
  Copyright (c) 2021 Luke Mills.  All right reserved.
*/

#include "ESPIR_Menu.h"

// Constructor for Keyboard
ESPIR_Menu::ESPIR_Menu(Adafruit_ST7735* display, int btn_count, 
		char** button_values) {
	tft = display;
	
	button_count = btn_count;
	values = button_values;
	
	buttons = (Button*) malloc(sizeof(Button) * button_count);

	for (int i = 0; i < button_count; i++){
		buttons[i] = Button(tft, 0, 4 + i * 14, tft -> width(), 12, 
			button_values[i]);
	}
	
	selected_button_index = 0;
}

// Performs the action of the currently selected key
char* ESPIR_Menu::press() {
	return buttons[selected_button_index].action;
}

// Display the menu on the screen
void ESPIR_Menu::display() {
	tft -> fillRect(0, 0, tft -> width(), tft -> height(), BLACK);
	tft -> setTextColor(ST77XX_WHITE);
	tft -> setTextSize(1);
	for (int i = 0; i < button_count; i++){
		if (i == selected_button_index){
			buttons[i].displaySelected();
		} else {
			buttons[i].display();
		}
	}
}

// Move down to the next button
void ESPIR_Menu::moveDown() {
	buttons[selected_button_index].display();
	selected_button_index = (selected_button_index + 1) % button_count;
	buttons[selected_button_index].displaySelected();
}

// Move up to the above button
void ESPIR_Menu::moveUp() {
	buttons[selected_button_index].display();
	selected_button_index = selected_button_index == 0 ? 
		button_count - 1 : selected_button_index - 1;
	buttons[selected_button_index].displaySelected();
}

// Get the list of buttons
Button* ESPIR_Menu::getButtons() {
	return buttons;
}




// Constructor for Button
Button::Button(Adafruit_ST7735* display, int x_pos, int y_pos, int width, 
		int height, char* act) {
	tft = display;
	x = x_pos;
	y = y_pos;
	w = width;
	h = height;
	action = act;
	selector_count = 0;
	selectors = (Selector*) malloc(sizeof(Selector) * MAX_SELECTORS);
	current_selector = 0;
}

// Display the Button
void Button::display() {
	tft -> fillRoundRect(x, y, w, h, 2, DARK_GREY);
	tft -> setCursor(x + 8, y + (h - 8)/2);
	tft -> setTextColor(WHITE);
	tft -> print(action);
}

// Display the Button as selected
void Button::displaySelected() {
	tft -> fillRoundRect(x, y, w, h, 2, GRAY);
	tft -> setCursor(x + 8, y + (h - 8)/2);
	tft -> setTextColor(BLACK);
	tft -> print(action);
}

// Add a selector to the submenu of the button
void Button::addSelector(char* prompt, char** options, 
		int window_size, int max, int vals) {
	if (selector_count < MAX_SELECTORS){
		selector_count++;
		selectors[selector_count - 1] = Selector(tft, 0, 
			((selector_count) * 28) - 25, prompt, options, window_size, max, vals);
	}
}

// Draw the buttons sub menu
void Button::drawSubMenu() {
	tft -> fillScreen(BLACK);
	
	for (int i = 0; i < selector_count; i++)
		selectors[i].display();
}

// Press the currently selected button on the sub menu
char* Button::pressSubMenu() {
	selectors[current_selector].press();
}

// Flash the current selected option
void Button::flashSelectedSelector() {
	selectors[current_selector].flashSelected();
}

// Move down to the next component in the sub menu
void Button::subMenuDown(){
	if (selectors[current_selector].atBottom() == 1){
		if (current_selector != selector_count - 1){
			current_selector++;
		} else {
			current_selector = 0;
		}
	} else if (selectors[current_selector].atBottom() == 0){
		selectors[current_selector].moveDown();
	}
}

// Move up to the above component in the sub menu
void Button::subMenuUp() {
	if (selectors[current_selector].atTop() == 1){
		if (current_selector != 0){
			current_selector--;
		} else {
			current_selector = selector_count-1;
		}
	} else {
		selectors[current_selector].moveUp();
	}
}

// Selected the option on the left (if possible) in sub menu
void Button::subMenuLeft() {
	selectors[current_selector].moveLeft();
}

// Selected the option on the right (if possible) in sub menu
void Button::subMenuRight() {
	selectors[current_selector].moveRight();
}

// Constructor for selector
Selector::Selector(Adafruit_ST7735* display, int x_pos, int y_pos, 
		char* act, char** opt, int size, int max, int count) {
	tft = display;
	x = x_pos;
	y = y_pos;
	prompt = act;
	options = opt;
	selected_index = 0;
	window_size = size;
	max_selected = max;
	current_changing_index = 0;
	value_count = count;
	
	// Selected index init
	selected_indexes = (int*) malloc(sizeof(int) * max);
	selected_indexes[0] = 0; // First selected by default

	for (int i = 1; i < max; i++)
		selected_indexes[i] = -1;
}

// Redraw the buttons of the selector
void Selector::cycleButtons() {
	int j = 0;
	for (int i = 0; i < value_count; i++){
		drawItem(i);
	}
}

// Indicate if the currently selected index is at the top of the selector
int Selector::atTop(){
	return current_changing_index < window_size ? 1 : 0;
}

// Indicate if the currently selected index is at the bottom of the selector
int Selector::atBottom(){
	return current_changing_index >= value_count -  window_size ? 1 : 0;
}

void Selector::drawItem(int index){
	tft -> setCursor(map(index%window_size, 0, window_size - 1, 
		3, tft -> width() - (tft -> width()/window_size)) - 3, y + 5 + 
		13*floor(index/window_size));
	
	int selected_test = 0;
	for (int j = 0; j < max_selected; j++){
		if (selected_indexes[j] == index){
			selected_test = 1;
			break;
		}
	}
	
	if (selected_test == 0){ // Not selected so display red background
		unselectIndex(index);
	} else { // Selected so display green background
		selectIndex(index);
	}
}

// Display the selector
void Selector::display() {
	tft->setTextColor(WHITE);
	tft -> setCursor(x + 5, y);
	tft -> fillRect(x, y - 3, tft -> width(), 31 + 13 * 
		(ceil(value_count/window_size) - 1), ST77XX_BLACK);
	tft -> print(prompt);
	
	this -> cycleButtons();
}

// Display the element at the passed index with a green background
void Selector::selectIndex(int index){
	tft -> setTextColor(WHITE);
	
	tft -> fillRoundRect(map(index%window_size, 0, window_size - 1, 3, 
		tft -> width() - (tft -> width()/window_size)), 
		y + 10 + 13*floor(index/window_size), 
		0.9 * (tft -> width() / window_size), 12, 2, GREEN);
		
	tft -> setCursor(map(index%window_size, 0, window_size - 1, 3, 
		tft -> width() - (tft -> width()/window_size)) + 2, 
		y + 12 + 13*floor(index/window_size));
	
	tft -> print(options[index]);
}

// Display the element at the passed index with a red background
void Selector::unselectIndex(int index){
	tft -> setTextColor(WHITE);
	
	tft -> fillRoundRect(map(index%window_size, 0, window_size - 1, 3, 
		tft -> width() - (tft -> width()/window_size)), 
		y + 10 + 13*floor(index/window_size), 
		0.9 * (tft -> width() / window_size), 12, 2, RED);
		
	tft -> setCursor(map(index%window_size, 0, window_size - 1, 3, 
		tft -> width() - (tft -> width()/window_size)) + 2, 
		y + 12 + 13*floor(index/window_size));
	
	tft -> print(options[index]);
}

// Flash the selected option
void Selector::flashSelected() {
	tft -> setCursor(map(current_changing_index%window_size, 0, 
		window_size - 1, FIRST_BUTTON_X, tft -> width() - 
		LAST_BUTTON_X) - 3, y + 13 + 13*floor(current_changing_index/window_size));
	
	int selected_test = 0;
	for (int j = 0; j < max_selected; j++){
		if (selected_indexes[j] == current_changing_index){
			selected_test = 1;
			break;
		}
	}
	
	tft -> fillRoundRect(map(current_changing_index%window_size, 0, 
		window_size - 1, 3, tft -> width() - (tft -> width()/window_size)), 
		y + 10 + 13*floor(current_changing_index/window_size), 
		0.9 * (tft -> width() / window_size), 12, 2, DARK_GREY);
		
	tft -> setCursor(map(current_changing_index%window_size, 0, 
	window_size - 1, 3, tft -> width() - (tft -> width()/window_size)) + 2, 
		y + 12 + 13*floor(current_changing_index/window_size));

	tft -> print(options[current_changing_index]);

	delay(100);

	if (selected_test == 0){ // Not selected so display red background
		unselectIndex(current_changing_index);
	} else { // Selected so display green background
		selectIndex(current_changing_index);
	}
	
	delay(100);
}

// Move to the option on the left
void Selector::moveLeft() {
	drawItem(current_changing_index);
	if (current_changing_index > 0){
		current_changing_index--;
	} else {
		current_changing_index = value_count-1;;
	}
	drawItem(current_changing_index);
}

// Move to the option on the right
void Selector::moveRight() {
	drawItem(current_changing_index);
	if (current_changing_index < value_count-1){
		current_changing_index++;
	} else {
		current_changing_index = 0;
	}
	drawItem(current_changing_index);
}

// Move to the option below
void Selector::moveDown(){
	current_changing_index = 
		current_changing_index + window_size > value_count ? 
		current_changing_index : 
		current_changing_index + window_size;
}

// Move to the option above
void Selector::moveUp(){
	current_changing_index = 
		current_changing_index - window_size < 0 ? 
		current_changing_index : 
		current_changing_index - window_size;
}

// Press the current selected button
void Selector::press() {
	if (max_selected == 1){
		unselectIndex(selected_indexes[0]);
		selected_indexes[0] = current_changing_index;
	} else {
		for (int i = 0; i < max_selected; i++){
			if (selected_indexes[i] == current_changing_index){
				// Make sure at least 1 thing pressed
				if (i == 0 && selected_indexes[1] == -1)
					return;
				
				for (int j = i; j < max_selected-1; j++)
					selected_indexes[j] = selected_indexes[j+1];

				selected_indexes[max_selected-1] = -1;
				
				return;
			}
		}
		
		if (selected_indexes[max_selected-1] != -1)
			unselectIndex(selected_indexes[max_selected-1]);
		
		for (int i = max_selected - 1; i > 0; i--)
			selected_indexes[i] = selected_indexes[i-1];

		selected_indexes[0] = current_changing_index;
	}

	selectIndex(current_changing_index);
}

// Return the indexes of the selected elements
int* Selector::getSelected() {
	return selected_indexes;
}

void Selector::setSelected(int index, int selected_index){
	selected_indexes[index] = selected_index;
}