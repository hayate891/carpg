#pragma once

#include "Item.h"

namespace guinew
{
	class Control
	{
	public:
		Control() : dock(false)
		{

		}

		bool dock;
	};

	class Container : public Control
	{
	public:
		inline void Add(Control* ctrl)
		{
			assert(ctrl);
			controls.push_back(ctrl);
		}

		vector<Control*> controls;
	};

	class Panel : public Container
	{
	public:
	};

	class SplitPanelContainer : public Control
	{
	public:
		SplitPanelContainer() : horizontal(true)
		{
			panel1 = new Panel;
			panel2 = new Panel;
		}

		bool horizontal;
		Panel* panel1;
		Panel* panel2;
		int panelSize;
	};

	class Form : public Panel
	{
	public:
	};

	class Label : public Control
	{
	public:
		string text;
	};

	class TextBox : public Control
	{
	public:
	};

	class ListBox : public Control
	{
	public:
	};

	class ListMenu : public Control
	{
	public:
		class Item
		{
		public:
			string text;
			int value;
		};

		ListMenu() : selected(-1)
		{

		}

		void Add(cstring text, int value)
		{
			Item* item = new Item;
			item->text = text;
			item->value = id;
			items.push_back(item);
		}

		vector<Item*> items;
		int selected;
	};

	class GroupBox : public Container
	{
	public:
		GroupBox(cstring text) : text(text)
		{

		}

		string text;
	};

	template<typename T>
	class TiledControl : public Control
	{
	public:
		TiledControl(cstring text)
		{
			label = new Label;
			label->text = text;
			control = new T;
		}

		Label* label;
		T* control;
	};

	typedef TiledControl<TextBox> TiledTextBox;
	typedef TiledControl<ListMenu> TiledListMenu;

	class GridLayout : public Control
	{
	public:
		GridLayout(int w, int h)
		{

		}

		void Add(Control* ctrl, int x, int y)
		{

		}
	};

	class ListBox : public Control
	{
	public:
	};

	class CheckBox : public Control
	{
	public:
		string text;
		bool checked;
	};

	class CheckBoxField : public Control
	{
	public:
	};

	//======================================================
	class ItemEditor : public Form
	{
	public:
		void Init()
		{
			splitPanelContainer1 = new SplitPanelContainer;
			splitPanelContainer1->horizontal = false;
			splitPanelContainer1->dock = true;
			splitPanelContainer1->panelSize = 100;
			Add(splitPanelContainer1);

			gridLayout1 = new GridLayout(3, 1);
			splitPanelContainer1->panel1->Add(gridLayout1);

			lmItemType = new TiledListMenu("Type:");
			PopulateItemType(lmItemType->control, true);
			gridLayout1->Add(lmItemType, 0, 0);

			lItemType = new Label;
			gridLayout1->Add(lItemType, 1, 0);

			tbFilter = new TiledTextBox("Filter:");
			gridLayout1->Add(tcFilter, 2, 0);

			splitPanelContainer2 = new SplitPanelContainer;
			splitPanelContainer2->dock = true;
			splitPanelContainer2->panelSize = 200;
			splitPanelContainer1->panel2->Add(splitPanelContainer2);

			lbItems = new ListBox;
			lbItems->dock = true;
			splitPanelContainer2->panel1->Add(lbItems);

			// common fields
			gbCommon = new GroupBox("Common");

			lmItemType2 = new TiledListMenu("Type:");
			PopulateItemType(lmItemType2->control, false);
			gbCommon->Add(lmItemType2);

			tbId = new TiledTextBox("Id:");
			gbCommon->Add(tbId);

			tbMesh = new TiledTextBox("Mesh:");
			gbCommon->Add(tbMesh);

			tbWeight = new TiledTextBox("Weight:");
			gbCommon->Add(tbWeight);

			tbValue = new TiledTextBox("Value:");
			gbCommon->Add(tbValue);

			cbfFlags = new CheckBoxField("Flags:");
			cbfFlags->Add("Not chest")

			/*ITEM_NOT_CHEST = 1<<0,
	ITEM_NOT_SHOP = 1<<1,
	ITEM_NOT_ALCHEMIST = 1<<2,
	ITEM_QUEST = 1<<3,
	ITEM_NOT_BLACKSMITH = 1<<4,
	ITEM_MAGE = 1<<5,
	ITEM_DONT_DROP = 1<<6, // can't drop when in dialog
	ITEM_SECRET = 1<<7,
	ITEM_BACKSTAB = 1<<8,
	ITEM_POWER_1 = 1<<9,
	ITEM_POWER_2 = 1<<10,
	ITEM_POWER_3 = 1<<11,
	ITEM_POWER_4 = 1<<12,
	ITEM_MAGIC_RESISTANCE_10 = 1<<13,
	ITEM_MAGIC_RESISTANCE_25 = 1<<14,
	ITEM_GROUND_MESH = 1<<15, // when on ground is displayed as mesh not as bag
	ITEM_CRYSTAL_SOUND = 1<<16,
	ITEM_IMPORTANT = 1<<17, // drawn on map as gold bag in minimap
	ITEM_TEX_ONLY = 1<<18,
	ITEM_NOT_MERCHANT = 1<<19,
	ITEM_NOT_RANDOM = 1<<20,
	ITEM_HQ = 1<<21, // high quality item icon
	ITEM_MAGICAL = 1<<23, // magic quality item icon
	ITEM_UNIQUE = 1<<24, // unique quality item icon*/
		}

		void PopulateItemType(ListMenu* menu, bool any)
		{
			if(any)
				menu->Add("(Any)", -1);
			menu->Add("Weapon", IT_WEAPON);
			menu->Add("Bow", IT_BOW);
			menu->Add("Shield", IT_SHIELD);
			menu->Add("Armor", IT_ARMOR);
			menu->Add("Other", IT_OTHER);
			menu->Add("Consumeable", IT_CONSUMEABLE);
		}

	private:
		SplitPanelContainer* splitPanelContainer1;
		SplitPanelContainer* splitPanelContainer2;
		GridLayout* gridLayout1;
		Label* lItemType;
		TiledListMenu* lmItemType;
		TiledTextBox* tbFilter;
		ListBox* lbItems;

		// common fields
		GroupBox* gbCommon;
		TiledControl<ListMenu>* lmItemType2;
		TiledTextBox* tbId;
		TiledTextBox* tbMesh;
		TiledTextBox* tbWeight;
		TiledTextBox* tbValue;
		CheckBoxField* cbfFlags;
	};
}
