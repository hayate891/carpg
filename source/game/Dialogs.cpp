#include "Pch.h"
#include "Base.h"
#include "Dialog.h"
#include "Game.h"
#include "Crc.h"
#include "Quest2.h"

extern string g_system_dir;
extern string g_lang_prefix;
typedef std::map<cstring, GameDialog*, CstringComparer> DialogsMap;
DialogsMap dialogs;
Tokenizer dialogs_tokenizer;
bool initialized_dialogs_tokenizer;
vector<DialogFunction> function_code;

//=================================================================================================
void CheckText(cstring text, bool talk2)
{
	bool have_format = (strchr(text, '$') != nullptr);
	if(talk2 != have_format)
		WARN(Format("Invalid dialog type for text \"%s\".", text));
}

enum Group
{
	G_TOP,
	G_KEYWORD
};

enum TopKeyword
{
	T_DIALOG
};

enum Keyword
{
	K_CHOICE,
	K_TRADE,
	K_TALK,
	K_TALK2,
	K_RESTART,
	K_END,
	K_END2,
	K_SHOW_CHOICES,
	K_SPECIAL,
	K_SET_QUEST_PROGRESS,
	K_IF,
	K_QUEST_TIMEOUT,
	K_RAND,
	K_ELSE,
	K_CHECK_QUEST_TIMEOUT,
	K_HAVE_QUEST_ITEM,
	K_DO_QUEST,
	K_DO_QUEST_ITEM,
	K_QUEST_PROGRESS,
	K_NEED_TALK,
	K_ESCAPE,
	K_ONCE,
	K_CHOICES,
	K_DO_QUEST2,
	K_HAVE_ITEM,
	K_QUEST_PROGRESS_RANGE,
	K_QUEST_EVENT,
	K_DO_ONCE,
	K_NOT_ACTIVE,
	K_QUEST_SPECIAL,
	K_SCRIPT
};

enum IfState
{
	IFS_IF,
	IFS_INLINE_IF,
	IFS_ELSE,
	IFS_INLINE_ELSE,
	IFS_CHOICE,
	IFS_INLINE_CHOICE,
	IFS_ESCAPE
};

//=================================================================================================
bool LoadDialog(Tokenizer& t, CRC32& crc, Quest2* quest)
{
	GameDialog* dialog = new GameDialog;
	vector<IfState> if_state;
	bool line_block = false;
	dialog->max_index = -1;

	try
	{
		dialog->id = t.MustGetItemKeyword();
		crc.Update(dialog->id);
		t.Next();

		t.AssertSymbol('{');
		t.Next();

		while(true)
		{
			if(t.IsSymbol('}'))
			{
				t.Next();
				if(if_state.empty())
					break;
				switch(if_state.back())
				{
				case IFS_INLINE_CHOICE:
				case IFS_INLINE_IF:
				case IFS_INLINE_ELSE:
					t.Unexpected();
					break;
				case IFS_IF:
					if(t.IsKeyword(K_ELSE, G_KEYWORD))
					{
						// if { ... } else
						t.Next();
						dialog->code.push_back({ DT_ELSE, nullptr });
						crc.Update(DT_ELSE);
						if(t.IsSymbol('{'))
						{
							if_state.back() = IFS_ELSE;
							t.Next();
						}
						else
							if_state.back() = IFS_INLINE_ELSE;
					}
					else
					{
						dialog->code.push_back({ DT_END_IF, nullptr });
						if_state.pop_back();
						crc.Update(DT_END_IF);
					}
					break;
				case IFS_ELSE:
					dialog->code.push_back({ DT_END_IF, nullptr });
					if_state.pop_back();
					crc.Update(DT_END_IF);
					break;
				case IFS_CHOICE:
					dialog->code.push_back({ DT_END_CHOICE, nullptr });
					if_state.pop_back();
					crc.Update(DT_END_CHOICE);
					if(!if_state.empty() && if_state.back() == IFS_ESCAPE)
					{
						dialog->code.push_back({ DT_ESCAPE_CHOICE, nullptr });
						if_state.pop_back();
						crc.Update(DT_ESCAPE_CHOICE);
					}
					break;
				}
			}
			else if(t.IsKeywordGroup(G_KEYWORD))
			{
				Keyword k = (Keyword)t.GetKeywordId(G_KEYWORD);
				t.Next();

				switch(k)
				{
				case K_CHOICE:
				case K_ESCAPE:
					{
						if(k == K_ESCAPE)
						{
							crc.Update(DT_ESCAPE_CHOICE);
							t.AssertKeyword(K_CHOICE, G_KEYWORD);
							t.Next();
							if_state.push_back(IFS_ESCAPE);
						}

						if(!quest)
						{
							int index = t.MustGetInt();
							if(index < 0)
								t.Throw("Invalid text index %d.", index);
							t.Next();
							dialog->code.push_back({ DT_CHOICE, (cstring)index });
							crc.Update(DT_CHOICE);
							crc.Update(index);

							++index;
							if(index > dialog->max_index)
							{
								dialog->texts.resize(index, { -1, -1, false });
								dialog->max_index = index;
							}
							dialog->texts[index - 1].exists = true;
						}
						else
						{
							const string& id = t.MustGetItemKeyword();
							dialog->code.push_back({ DT_CHOICE, (cstring)dialog->strs.size() });
							dialog->strs.push_back(id);
							crc.Update(DT_CHOICE);
							crc.Update(id);
							t.Next();
						}
						
						if(t.IsSymbol('{'))
						{
							if_state.push_back(IFS_CHOICE);
							t.Next();
						}
						else
							if_state.push_back(IFS_INLINE_CHOICE);
						line_block = true;
					}
					break;
				case K_TRADE:
					dialog->code.push_back({ DT_TRADE, nullptr });
					crc.Update(DT_TRADE);
					break;
				case K_TALK:
				case K_TALK2:
					if(!quest)
					{
						int index = t.MustGetInt();
						if(index < 0)
							t.Throw("Invalid text index %d.", index);
						t.Next();
						dialog->code.push_back({ k == K_TALK ? DT_TALK : DT_TALK2, (cstring)index });
						++index;
						if(index > dialog->max_index)
						{
							dialog->texts.resize(index, { -1, -1, false });
							dialog->max_index = index;
						}
						dialog->texts[index - 1].exists = true;
						crc.Update(k == K_TALK ? DT_TALK : DT_TALK2);
						crc.Update(index);
					}
					else
					{
						if(k == K_TALK2)
							t.Throw("Talk2 is not available in new quests.");
						const string& id = t.MustGetItemKeyword();
						dialog->code.push_back({ DT_TALK, (cstring)dialog->strs.size() });
						dialog->strs.push_back(id);
						crc.Update(DT_TALK);
						crc.Update(id);
						t.Next();
					}
					break;
				case K_RESTART:
					dialog->code.push_back({ DT_RESTART, nullptr });
					crc.Update(DT_RESTART);
					break;
				case K_END:
					dialog->code.push_back({ DT_END, nullptr });
					crc.Update(DT_END);
					break;
				case K_END2:
					dialog->code.push_back({ DT_END2, nullptr });
					crc.Update(DT_END2);
					break;
				case K_SHOW_CHOICES:
					dialog->code.push_back({ DT_SHOW_CHOICES, nullptr });
					crc.Update(DT_SHOW_CHOICES);
					break;
				case K_SPECIAL:
					if(!quest)
					{
						int index = dialog->strs.size();
						dialog->strs.push_back(t.MustGetString());
						t.Next();
						dialog->code.push_back({ DT_SPECIAL, (cstring)index });
						crc.Update(DT_SPECIAL);
						crc.Update(dialog->strs.back());
					}
					else
						t.Throw("Special is not available in new quests.");
					break;
				case K_SET_QUEST_PROGRESS:
					{
						int prog;
						if(!quest)
						{
							prog = t.MustGetInt();
							if(prog < 0)
								t.Throw("Invalid quest progress %d.", prog);
							
						}
						else
						{
							const string& id = t.MustGetItemKeyword();
							prog = quest->FindProgress(id);
							if(prog == -1)
								t.Throw("Invalid quest progress '%s'.", id.c_str());
						}
						t.Next();
						dialog->code.push_back({ DT_SET_QUEST_PROGRESS, (cstring)prog });
						crc.Update(DT_SET_QUEST_PROGRESS);
						crc.Update(prog);
					}
					break;
				case K_IF:
					{
						k = (Keyword)t.MustGetKeywordId(G_KEYWORD);
						t.Next();

						switch(k)
						{
						case K_QUEST_TIMEOUT:
							if(!quest)
							{
								dialog->code.push_back({ DT_IF_QUEST_TIMEOUT, nullptr });
								crc.Update(DT_IF_QUEST_TIMEOUT);
							}
							else
								t.Throw("If quest_timeout is not available in new quests.");
							break;
						case K_RAND:
							{
								int chance = t.MustGetInt();
								if(chance <= 0 || chance >= 100)
									t.Throw("Invalid chance %d.", chance);
								t.Next();
								dialog->code.push_back({ DT_IF_RAND, (cstring)chance });
								crc.Update(DT_IF_RAND);
								crc.Update(chance);
							}
							break;
						case K_HAVE_QUEST_ITEM:
							if(!quest)
							{
								if(t.IsKeyword(K_NOT_ACTIVE, G_KEYWORD))
								{
									t.Next();
									dialog->code.push_back({ DT_NOT_ACTIVE, nullptr });
									crc.Update(DT_NOT_ACTIVE);
								}
								int index = dialog->strs.size();
								dialog->strs.push_back(t.MustGetString());
								t.Next();
								dialog->code.push_back({ DT_IF_HAVE_QUEST_ITEM, (cstring)index });
								crc.Update(DT_IF_HAVE_QUEST_ITEM);
								crc.Update(dialog->strs.back());
							}
							else
								t.Throw("If have_quest_item is not available in new quests.");
							break;
						case K_QUEST_PROGRESS:
							{
								int prog;
								if(!quest)
								{
									prog = t.MustGetInt();
									if(prog < 0)
										t.Throw("Invalid quest progress %d.", prog);

								}
								else
								{
									const string& id = t.MustGetItemKeyword();
									int prog = quest->FindProgress(id);
									if(prog == -1)
										t.Throw("Invalid quest progress '%s'.", id.c_str());
								}
								t.Next();
								dialog->code.push_back({ DT_IF_QUEST_PROGRESS, (cstring)prog });
								crc.Update(DT_IF_QUEST_PROGRESS);
								crc.Update(prog);
							}
							break;
						case K_NEED_TALK:
							{
								int index = dialog->strs.size();
								dialog->strs.push_back(t.MustGetString());
								t.Next();
								dialog->code.push_back({ DT_IF_NEED_TALK, (cstring)index });
								crc.Update(DT_IF_NEED_TALK);
								crc.Update(dialog->strs.back());
							}
							break;
						case K_SPECIAL:
							if(!quest)
							{
								int index = dialog->strs.size();
								dialog->strs.push_back(t.MustGetString());
								t.Next();
								dialog->code.push_back({ DT_IF_SPECIAL, (cstring)index });
								crc.Update(DT_IF_SPECIAL);
								crc.Update(dialog->strs.back());
							}
							else
								t.Throw("If special is not available in new quests.");
							break;
						case K_ONCE:
							dialog->code.push_back({ DT_IF_ONCE, nullptr });
							crc.Update(DT_IF_ONCE);
							break;
						case K_HAVE_ITEM:
							if(!quest)
							{
								const string& id = t.MustGetItemKeyword();
								ItemListResult result;
								const Item* item = FindItem(id.c_str(), false, &result);
								if(item && result.lis == nullptr)
								{
									t.Next();
									dialog->code.push_back({ DT_IF_HAVE_ITEM, (cstring)item });
								}
								else
									t.Throw("Invalid item '%s'.", id.c_str());
								crc.Update(DT_SPECIAL);
								crc.Update(item->id);
							}
							else
								t.Throw("If have_item is not available in new quests.");
							break;
						case K_QUEST_EVENT:
							if(!quest)
							{
								dialog->code.push_back({ DT_IF_QUEST_EVENT, nullptr });
								crc.Update(DT_IF_QUEST_EVENT);
							}
							else
								t.Throw("If quest_event is not available in new quests.");
							break;
						case K_QUEST_PROGRESS_RANGE:
							{
								int prog_a, prog_b;
								if(!quest)
								{
									prog_a = t.MustGetInt();
									t.Next();
									prog_b = t.MustGetInt();
									t.Next();
									if(prog_a < 0 || prog_a >= prog_b)
										t.Throw("Invalid quest progress range {%d %d}.", prog_a, prog_b);
								}
								else
								{
									const string& id_a = t.MustGetItemKeyword();
									prog_a = quest->FindProgress(id_a);
									if(prog_a == -1)
										t.Throw("Invalid quest progress '%s'.", id_a.c_str());
									LocalString id_a_copy = id_a;
									t.Next();
									const string& id_b = t.MustGetItemKeyword();
									prog_b = quest->FindProgress(id_b);
									if(prog_b == -1)
										t.Throw("Invalid quest progress '%s'.", id_b.c_str());
									if(prog_a >= prog_b)
										t.Throw("Invalid quest progress range {%s(%d) %s(%d)}.", id_a_copy.c_str(), prog_a, id_b.c_str(), prog_b);
									t.Next();
								}
								int prog = ((prog_a & 0xFFFF) | ((prog_b & 0xFFFF0000) >> 16));
								dialog->code.push_back({ DT_IF_QUEST_PROGRESS_RANGE, (cstring)prog });
								crc.Update(DT_IF_QUEST_PROGRESS_RANGE);
								crc.Update(prog);
							}
							break;
						case K_CHOICES:
							{
								int count = t.MustGetInt();
								if(count < 0)
									t.Throw("Invalid choices count %d.", count);
								t.Next();
								dialog->code.push_back({ DT_IF_CHOICES, (cstring)count });
								crc.Update(DT_IF_CHOICES);
								crc.Update(count);
							}
							break;
						case K_QUEST_SPECIAL:
							if(!quest)
							{
								int index = dialog->strs.size();
								dialog->strs.push_back(t.MustGetString());
								t.Next();
								dialog->code.push_back({ DT_IF_QUEST_SPECIAL, (cstring)index });
								crc.Update(DT_IF_QUEST_SPECIAL);
								crc.Update(dialog->strs.back());
							}
							else
								t.Throw("If quest_special is not available in new quests.");
							break;
						case K_SCRIPT:
							{
								int index = function_code.size();
								const string& code = t.MustGetString();
								string* str = StringPool.Get();
								*str = code;
								function_code.push_back({ str, true });
								t.Next();
								dialog->code.push_back({ DT_IF_SCRIPT, (cstring)index });
								crc.Update(DT_IF_SCRIPT);
								crc.Update(*str);
							}
							break;
						default:
							t.Unexpected();
							break;
						}

						if(t.IsSymbol('{'))
						{
							if_state.push_back(IFS_IF);
							t.Next();
						}
						else
							if_state.push_back(IFS_INLINE_IF);
						line_block = true;
					}
					break;
				case K_CHECK_QUEST_TIMEOUT:
					if(!quest)
					{
						int type = t.MustGetInt();
						if(type < 0 || type > 2)
							t.Throw("Invalid quest type %d.", type);
						t.Next();
						dialog->code.push_back({ DT_CHECK_QUEST_TIMEOUT, (cstring)type });
						crc.Update(DT_CHECK_QUEST_TIMEOUT);
						crc.Update(type);
					}
					else
						t.Throw("Check_quest_timeout is not available in new quests.");
					break;
				case K_DO_QUEST:
					if(!quest)
					{
						int index = dialog->strs.size();
						dialog->strs.push_back(t.MustGetString());
						t.Next();
						dialog->code.push_back({ DT_DO_QUEST, (cstring)index });
						crc.Update(DT_DO_QUEST);
						crc.Update(dialog->strs.back());
					}
					else
						t.Throw("Do_quest is not available in new quests.");
					break;
				case K_DO_QUEST_ITEM:
					if(!quest)
					{
						int index = dialog->strs.size();
						dialog->strs.push_back(t.MustGetString());
						t.Next();
						dialog->code.push_back({ DT_DO_QUEST_ITEM, (cstring)index });
						crc.Update(DT_DO_QUEST_ITEM);
						crc.Update(dialog->strs.back());
					}
					else
						t.Throw("Do_quest_item is not available in new quests.");
					break;
				case K_DO_QUEST2:
					if(!quest)
					{
						int index = dialog->strs.size();
						dialog->strs.push_back(t.MustGetString());
						t.Next();
						dialog->code.push_back({ DT_DO_QUEST2, (cstring)index });
						crc.Update(DT_DO_QUEST2);
						crc.Update(dialog->strs.back());
					}
					else
						t.Throw("Do_quest2 is not available in new quests.");
					break;
				case K_DO_ONCE:
					dialog->code.push_back({ DT_DO_ONCE, nullptr });
					crc.Update(DT_DO_ONCE);
					break;
				case K_QUEST_SPECIAL:
					if(!quest)
					{
						int index = dialog->strs.size();
						dialog->strs.push_back(t.MustGetString());
						t.Next();
						dialog->code.push_back({ DT_QUEST_SPECIAL, (cstring)index });
						crc.Update(DT_QUEST_SPECIAL);
						crc.Update(dialog->strs.back());
					}
					else
						t.Throw("Quest_special is not available in new quests.");
					break;
				case K_SCRIPT:
					{
						int index = function_code.size();
						const string& code = t.MustGetString();
						string* str = StringPool.Get();
						*str = code;
						function_code.push_back({ str, false });
						t.Next();
						dialog->code.push_back({ DT_SCRIPT, (cstring)index });
						crc.Update(DT_SCRIPT);
						crc.Update(*str);
					}
					break;
				default:
					t.Unexpected();
					break;
				}
			}
			else
				t.Unexpected();

			if(line_block)
				line_block = false;
			else
			{
				while(!if_state.empty())
				{
					bool b = false;
					switch(if_state.back())
					{
					case IFS_IF:
					case IFS_ELSE:
					case IFS_CHOICE:
						b = true;
						break;
					case IFS_INLINE_IF:
						if(t.IsKeyword(K_ELSE, G_KEYWORD))
						{
							dialog->code.push_back({ DT_ELSE, nullptr });
							crc.Update(DT_ELSE);
							t.Next();
							if(t.IsSymbol('{'))
							{
								if_state.back() = IFS_ELSE;
								t.Next();
							}
							else
								if_state.back() = IFS_INLINE_ELSE;
						}
						else
						{
							dialog->code.push_back({ DT_END_IF, nullptr });
							if_state.pop_back();
						}
						break;
					case IFS_INLINE_ELSE:
						dialog->code.push_back({ DT_END_IF, nullptr });
						if_state.pop_back();
						crc.Update(DT_END_IF);
						break;
					case IFS_INLINE_CHOICE:
						dialog->code.push_back({ DT_END_CHOICE, nullptr });
						crc.Update(DT_END_CHOICE);
						if_state.pop_back();
						if(!if_state.empty() && if_state.back() == IFS_ESCAPE)
						{
							dialog->code.push_back({ DT_ESCAPE_CHOICE, nullptr });
							if_state.pop_back();
							crc.Update(DT_ESCAPE_CHOICE);
						}
						break;
					}

					if(b)
						break;
				}
			}
		}

		std::pair<DialogsMap::iterator, bool>& result = dialogs.insert(std::pair<cstring, GameDialog*>(dialog->id.c_str(), dialog));
		if(!result.second)
			t.Throw("Dialog with that id already exists.");

		return true;
	}
	catch(Tokenizer::Exception& e)
	{
		ERROR(Format("Failed to parse dialog '%s': %s", dialog->id.c_str(), e.ToString()));
		delete dialog;
		return false;
	}
}

//=================================================================================================
void InitializeDialogsTokenizer()
{
	if(initialized_dialogs_tokenizer)
		return;

	Tokenizer& t = dialogs_tokenizer;

	t.AddKeyword("dialog", T_DIALOG, G_TOP);

	t.AddKeywords(G_KEYWORD, {
		{ "choice", K_CHOICE },
		{ "trade", K_TRADE },
		{ "talk", K_TALK },
		{ "talk2", K_TALK2 },
		{ "restart", K_RESTART },
		{ "end", K_END },
		{ "end2", K_END2 },
		{ "show_choices", K_SHOW_CHOICES },
		{ "special", K_SPECIAL },
		{ "set_quest_progress", K_SET_QUEST_PROGRESS },
		{ "if", K_IF },
		{ "quest_timeout", K_QUEST_TIMEOUT },
		{ "rand", K_RAND },
		{ "else", K_ELSE },
		{ "check_quest_timeout", K_CHECK_QUEST_TIMEOUT },
		{ "have_quest_item", K_HAVE_QUEST_ITEM },
		{ "do_quest", K_DO_QUEST },
		{ "do_quest_item", K_DO_QUEST_ITEM },
		{ "quest_progress", K_QUEST_PROGRESS },
		{ "need_talk", K_NEED_TALK },
		{ "escape", K_ESCAPE },
		{ "once", K_ONCE },
		{ "choices", K_CHOICES },
		{ "do_quest2", K_DO_QUEST2 },
		{ "have_item", K_HAVE_ITEM },
		{ "quest_progress_range", K_QUEST_PROGRESS_RANGE },
		{ "quest_event", K_QUEST_EVENT },
		{ "do_once", K_DO_ONCE },
		{ "not_active", K_NOT_ACTIVE },
		{ "quest_special", K_QUEST_SPECIAL },
		{ "script", K_SCRIPT }
	});

	initialized_dialogs_tokenizer = true;
}

//=================================================================================================
uint LoadDialogs(uint& out_crc)
{
	InitializeDialogsTokenizer();

	Tokenizer& t = dialogs_tokenizer;
	if(!t.FromFile(Format("%s/dialogs.txt", g_system_dir.c_str())))
		throw "Failed to open dialogs.txt.";

	int errors = 0;
	CRC32 crc;

	try
	{
		t.Next();

		while(!t.IsEof())
		{
			bool skip = false;

			if(t.IsKeywordGroup(G_TOP))
			{
				t.Next();

				if(!LoadDialog(t, crc, nullptr))
				{
					skip = true;
					++errors;
				}
			}
			else
			{
				int group = G_TOP;
				ERROR(t.FormatUnexpected(Tokenizer::T_KEYWORD_GROUP, &group));
				skip = true;
				++errors;
			}

			if(skip)
				t.SkipToKeywordGroup(G_TOP);
		}
	}
	catch(const Tokenizer::Exception& e)
	{
		ERROR(Format("Failed to load dialogs: %s", e.ToString()));
		++errors;
	}

	if(errors > 0)
		throw Format("Failed to load dialogs (%d errors), check log for details.", errors);

	out_crc = crc.Get();
	return dialogs.size();
}

//=================================================================================================
bool LoadQuestDialog(Tokenizer& t, Quest2* quest, vector<DialogFunction>*& _function_code)
{
	InitializeDialogsTokenizer();

	dialogs_tokenizer.FromTokenizer(t);

	// should be in format "id { ... }
	CRC32 crc;
	bool result = LoadDialog(dialogs_tokenizer, crc, quest);
	t.FromTokenizer(dialogs_tokenizer);

	if(_function_code)
		_function_code = &function_code;
	else
	{
		for(auto& f : function_code)
			StringPool.Free(f.code);
		function_code.clear();
	}

	return result;
}

//=================================================================================================
bool LoadDialogText(Tokenizer& t)
{
	GameDialog* dialog = nullptr;

	try
	{
		const string& id = t.MustGetItemKeyword();
		dialog = FindDialog(id.c_str());
		if(!dialog)
			t.Throw("Missing dialog '%s'.", id.c_str());
		
		t.Next();
		t.AssertSymbol('{');
		t.Next();

		while(!t.IsSymbol('}'))
		{
			int index = t.MustGetInt();
			if(index < 0 || index >= dialog->max_index)
				t.Throw("Invalid text index %d.", index);
			t.Next();

			if(t.IsSymbol('{'))
			{
				t.Next();
				int prev = -1;
				while(!t.IsSymbol('}'))
				{
					int str_idx = dialog->strs.size();
					dialog->strs.push_back(t.MustGetString());
					t.Next();
					if(prev == -1)
					{
						dialog->texts[index].id = str_idx;
						dialog->texts[index].exists = true;
						prev = index;
					}
					else
					{
						index = dialog->texts.size();
						dialog->texts[prev].next = index;
						dialog->texts.push_back({ str_idx, -1, true });
						prev = index;
					}
				}
			}
			else
			{
				int str_idx = dialog->strs.size();
				dialog->strs.push_back(t.MustGetString());
				dialog->texts[index].id = str_idx;
				dialog->texts[index].exists = true;
			}

			t.Next();
		}

		t.Next();

		bool ok = true;
		int index = 0;
		for(GameDialog::Text& t : dialog->texts)
		{
			if(!t.exists)
			{
				ERROR(Format("Dialog '%s' is missing text for index %d.", dialog->id.c_str(), index));
				ok = false;
			}
			++index;
		}

		return ok;
	}
	catch(Tokenizer::Exception& e)
	{
		if(dialog)
			ERROR(Format("Failed to load dialog '%s' texts: %s", dialog->id.c_str(), e.ToString()));
		else
			ERROR(Format("Failed to load dialog texts: %s", e.ToString()));
		return false;
	}
}

//=================================================================================================
void LoadDialogTexts()
{
	Tokenizer t;
	cstring path = Format("%s/lang/%s/dialogs.txt", g_system_dir.c_str(), g_lang_prefix.c_str());

	if(!t.FromFile(path))
	{
		ERROR(Format("Failed to load language file '%s'.", path));
		return;
	}

	t.AddKeyword("dialog", 0);

	int errors = 0;

	try
	{
		t.Next();

		while(!t.IsEof())
		{
			bool skip = false;

			if(t.IsKeyword(0))
			{
				t.Next();

				if(!LoadDialogText(t))
				{
					skip = true;
					++errors;
				}
			}
			else
			{
				int id = 0;
				ERROR(t.FormatUnexpected(Tokenizer::T_KEYWORD, &id));
				skip = true;
				++errors;
			}

			if(skip)
				t.SkipToKeywordGroup(Tokenizer::EMPTY_GROUP);
		}
	}
	catch(const Tokenizer::Exception& e)
	{
		ERROR(Format("Failed to load dialogs: %s", e.ToString()));
		++errors;
	}

	if(errors > 0)
		ERROR(Format("Failed to load dialog texts (%d errors), check log for details.", errors));
}

//=================================================================================================
cstring DialogContext::GetText(int index)
{
	GameDialog* d = (GameDialog*)dialog;
	GameDialog::Text& text = d->texts[index];
	if(text.next == -1)
		return d->strs[text.id].c_str();
	else
	{
		int count = 1;
		GameDialog::Text* t = &d->texts[index];
		while(t->next != -1)
		{
			++count;
			t = &d->texts[t->next];
		}
		int id = rand2() % count;
		t = &d->texts[index];
		for(int i = 0; i <= id; ++i)
		{
			if(i == id)
				return d->strs[t->id].c_str();
			t = &d->texts[t->next];
		}
	}
	return d->strs[d->texts[index].id].c_str();
}

//=================================================================================================
GameDialog* FindDialog(cstring id)
{
	auto it = dialogs.find(id);
	if(it == dialogs.end())
		return nullptr;
	else
		return it->second;
}
