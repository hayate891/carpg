#include "Pch.h"
#include "Common.h"
#include "Control.h"
#include "Overlay.h"

void Control::TakeFocus()
{
	GUI.GetOverlay()->SetFocus(this);
}
