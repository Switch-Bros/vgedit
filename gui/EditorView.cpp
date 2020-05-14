#include "EditorView.hpp"
#include "EKeyboard.hpp"

EditorView::EditorView(Editor* editor)
{
	// initialize the text view for the first time, using whatever's in the editor
	mainTextField = new TextInputElement(editor->contents());
	mainTextField->x = 10;
	mainTextField->y = 70;
	this->elements.push_back(mainTextField);

	// create a tool bar, but don't add it to the elements list
	// it will be manually drawn in later (above everything else)
	toolbar = new Toolbar(editor->filename);

	this->editor = editor;
}

void EditorView::render(Element* parent)
{
	SDL_SetRenderDrawColor(parent->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderFillRect(parent->renderer, NULL);

	super::render(parent);

	toolbar->render(parent);
}

void EditorView::reset_bounds()
{
	int selected_x = mainTextField->selected_x;
	int selected_y = mainTextField->selected_y;

	selected_y = selected_y < 0 ? 0 : selected_y;
	selected_y = selected_y > editor->lineCount() - 1 ? editor->lineCount() - 1 : selected_y;

	// loop around in x direction
	selected_x = selected_x < 0 ? editor->lineLength(selected_y) - 1 : selected_x;
	selected_x = selected_x > editor->lineLength(selected_y) - 1 ? 0 : selected_x;

	mainTextField->selected_x = selected_x;
	mainTextField->selected_y = selected_y;

	int selected_width = mainTextField->selected_width;
	int selected_height = mainTextField->selected_height;

	// adjust the bounds of the selection
	selected_height = selected_height < 1 ? 1 : selected_height;
	selected_height = selected_height > editor->lineCount() - selected_y ? editor->lineCount() - selected_y : selected_height;

	selected_width = selected_width < 1 ? 1 : selected_width;
	selected_width = selected_width > editor->lineLength(selected_y) - selected_x ? editor->lineLength(selected_y) - selected_x : selected_width;

	mainTextField->selected_width = selected_width;
	mainTextField->selected_height = selected_height;

	// always snap the cursor to be on screen and visible (by moving the screen)
	int h = mainTextField->fontHeight + 2;
	float cursor_y = (h * mainTextField->selected_y - 50) * -1;

	if (cursor_y > mainTextField->y + 50)
		mainTextField->y += h;

	if (cursor_y < mainTextField->y - 550)
		mainTextField->y -= h;

	// if it's still offscreen, and we're showing the keyboard
	if (mainTextField->insertMode)
		mainTextField->y = cursor_y;
}

bool EditorView::copySelection()
{
	// TODO: handle vertical selections (abstract into Selection class?)

	if (copiedText)
		delete copiedText;

	stringstream s;
	int sx = mainTextField->selected_x;
	int sy = mainTextField->selected_y;
	for (int x = sx; x < sx + mainTextField->selected_width; x++)
		s << editor->lines[sy][x];

	copiedText = new std::string(s.str());
	return true;
}

bool EditorView::pasteSelection()
{
	if (copiedText == NULL)
		return false;

	for (char& letter : *copiedText)
		editor->type(mainTextField->selected_y, mainTextField->selected_x++, letter);

	syncText();
	return true;
}

void EditorView::syncText()
{
	reset_bounds();
	mainTextField->updateText(editor->contents());
	toolbar->setModified(true);
}

bool EditorView::process(InputEvents* e)
{
	// move the cursoraround the editor
	if ((keyboard == NULL || keyboard->hidden) && e->pressed(LEFT_BUTTON | RIGHT_BUTTON | UP_BUTTON | DOWN_BUTTON))
	{
		if (e->pressed(LEFT_BUTTON))
			mainTextField->selected_x -= 1;
		if (e->pressed(RIGHT_BUTTON))
			mainTextField->selected_x += 1;
		if (e->pressed(UP_BUTTON))
			mainTextField->selected_y -= 1;
		if (e->pressed(DOWN_BUTTON))
			mainTextField->selected_y += 1;

		reset_bounds();
		return true;
	}

	// perform a save
	if (e->released(START_BUTTON))
	{
		toolbar->setModified(false);
		editor->save();
		return true;
	}

	// delete what's under the current selection (not backspace)
	if (e->pressed(B_BUTTON))
	{
		// in insert mode, delete is a "backspace-style" action, and moves the cursor left if it can
		if (mainTextField->insertMode)
		{
			if (mainTextField->selected_x > 0)
			{
				mainTextField->selected_x--;
			}
			else
			{
				// delete the last newline on the previous line (joining this line and the earlier one)
				if (mainTextField->selected_y > 0)
				{
					mainTextField->selected_y--;
					mainTextField->selected_x = editor->lineLength(mainTextField->selected_y) - 1;
				}
				else
				{
					// nothing to delete
					return false;
				}
			}
		}

		editor->del(mainTextField->selected_y, mainTextField->selected_x, mainTextField->selected_width);
		syncText();
		return true;
	}

	// bring up the keyboard
	if (e->released(A_BUTTON))
	{
		if (keyboard == NULL)
		{
			keyboard = new EKeyboard(this);
			this->elements.push_back(keyboard);
		}

		keyboard->hidden = false;

		// force selection to be width 1 (more than 1 doesn't make sense in insert mode)
		// (but it does make sense for vertical selections, to type on multiple lines)
		mainTextField->selected_width = 1;
		mainTextField->insertMode = true;
		toolbar->keyboardShowing = true;

		reset_bounds();

		return true;
	}

	if (keyboard == NULL || keyboard->hidden)
	{
		// copy and paste on X and Y
		if (e->released(X_BUTTON))
		{
			copySelection();
			return true;
		}
	}
	else
	{
		if (e->released(X_BUTTON))
		{
			// editor->overwriteMode = !editor->overwriteMode;
			// TODO: add overwrite mode back when hex editor is here

			keyboard->shiftOn = !keyboard->shiftOn;
			keyboard->updateSize();
			return true;
		}

		if (e->released(Y_BUTTON))
		{
			keyboard->hidden = true;
			mainTextField->insertMode = false;
			toolbar->keyboardShowing = false;
			return true;
		}
	}

	if (e->released(Y_BUTTON))
	{
		pasteSelection();
		return true;
	}

	if (e->pressed(L_BUTTON | R_BUTTON | ZL_BUTTON | ZR_BUTTON))
	{
		// move the bounds of the selection
		if (e->pressed(L_BUTTON))
			mainTextField->selected_width -= 1;
		if (e->pressed(R_BUTTON))
			mainTextField->selected_width += 1;
		// if (e->pressed(ZL_BUTTON))
		//   mainTextField->selected_height -= 1;
		// if (e->pressed(ZR_BUTTON))
		//   mainTextField->selected_height += 1;

		reset_bounds();
		return true;
	}

	return super::process(e) || toolbar->process(e);
}
