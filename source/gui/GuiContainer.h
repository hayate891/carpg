#pragma once

//-----------------------------------------------------------------------------
#include "Control.h"

//-----------------------------------------------------------------------------
class TextBox;
class Button;

//-----------------------------------------------------------------------------
class GuiContainer
{
public:
	enum Flags
	{
		F_MOUSE_FOCUS = 1<<0,
		F_FOCUS = 1<<1,
		F_CLICK_TO_FOCUS = 1<<2
	};

	typedef std::pair<Control*, int> GuiItem;
	typedef vector<GuiItem>::iterator Iter;

	GuiContainer();
	void Draw();
	void Update(float dt);
	inline void Add(Control* ctrl, int flags)
	{
		assert(ctrl);
		items.push_back(GuiItem(ctrl, flags));
	}
	inline void Add(TextBox* textbox) { Add((Control*)textbox, F_FOCUS|F_CLICK_TO_FOCUS|F_MOUSE_FOCUS); }
	inline void Add(TextBox& textbox) { Add(&textbox); }
	inline void Add(Button* button) { Add((Control*)button, F_MOUSE_FOCUS); }
	inline void Add(Button& button) { Add(&button); }
	void GainFocus();
	void LostFocus();
	void Move(const INT2& global_pos);
	void CheckGiveFocus();
	
	vector<GuiItem> items;
	bool focus;
	Control* focus_ctrl, *give_focus;
};
