#include <stdio.h>
#include <string.h>
#include <taihen.h>
#include <vitasdkkern.h>
#include "cmd56.h"
#include "cobra.h"
#include "format.h"

void _start() __attribute__((weak, alias("module_start")));
int module_start(SceSize argc, const void *args)
{
	cobra_patch(); // revert cobra blackfin patch
	cmd56_patch(); // patch cmd56 ..
	get_module_functions(); // get format stuff
	
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args)
{
	cobra_unpatch(); 
	cmd56_unpatch();
	return SCE_KERNEL_STOP_SUCCESS;
}
