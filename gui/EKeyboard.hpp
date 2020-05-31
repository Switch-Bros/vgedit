#include "../libs/chesto/src/Element.hpp"
#include "../libs/chesto/src/TextElement.hpp"
#include <SDL2/SDL_image.h>
#pragma once

class EditorView;

class EKeyboard : public Element
{
public:
	EKeyboard(EditorView* editorView);
	~EKeyboard();
	void render(Element* parent);
	bool process(InputEvents* event);

	// setup field variables
	void updateSize();
	void just_type(const char input);

	EditorView* editorView = NULL;
	std::function<void(char)> typeAction = NULL;

	// information representing a default qwerty EKeyboard, lower and upper
	// alongside default EKeyboards will also be: tab, caps lock, return, delete, and shifts
	const char* lower_keys = "`1234567890-=qwertyuiop[]\\asdfghjkl;'zxcvbnm,./";
	const char* upper_keys = "~!@#$\%^&*()_+QWERTYUIOP{}|ASDFGHJKL:\"ZXCVBNM<>?";
	int breaks[4] = { 13, 13, 11, 10 };

	// the rest of the keys will be dynamically drawn by going through hex iterations
	// these EKeyboards will fill the extra space with more characters

	std::vector<std::string*> rows;

	inline int rowCount() {
		return (int)rows.size();
	}

	inline int rowLength(int row) {
		return (int)rows[row]->size() / 2;
	}

	bool shiftOn = false;
	bool capsOn = false;
	int mode = 0; // the mode of which EKeyboard type we're on (first is special and has shift)

	// the currently selected row and index
	int curRow = -1;
	int index = -1;

	// the below variables are stored to be used in processing touch events
	// and rendering the drawings to screen

	// attributes of each key
	int keyWidth = 0;
	int padding = 0;
	int textSize = 0;

	// attributes of delete and backspace keys
	int dPos = 0;
	int dHeight = 0;
	int sPos = 0;
	int enterPos = 0;
	int enterHeight = 0;
	int dWidth = 0;
	int sWidth = 0;
	int enterWidth = 0;

	// positions of key location offset information
	int kXPad = 0;
	int kXOff = 0;
	int yYOff = 0;
	int kYPad = 0;
	int ySpacing = 0;

	bool touchMode = false;

	void space();
	void backspace();
	void type(int y, int x);
	void updateView();
	void generateEKeyboard();
};
