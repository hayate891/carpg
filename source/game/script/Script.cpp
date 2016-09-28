#include "Pch.h"
#include "Common.h"
#include <cas/cas.h>

void print_int(int a)
{
	a += 1;
}

void cas_test()
{
	cas::Initialize();
	cas::IModule* module = cas::CreateModule();
	module->AddFunction("void print_int(int a)", print_int);
	module->ParseAndRun("print_int(7);");
	cas::Shutdown();

	exit(0);
}
