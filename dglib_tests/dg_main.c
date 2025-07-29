#include "dg_main.h"
#include "dg_libcommon.h"

int dg_initialize()
{
	if (!initialize_memory()) {
		DG_ERROR("dg_initialize(): failed to init memory allocator");
		return 0;
	}
	if (!initialize_sysui()) {
		DG_ERROR("dg_initialize(): failed to init sysui");
		return 0;
	}
	if (!initialize_filesystem()) {
		DG_ERROR("dg_initialize(): failed to init filesystem");
		return 0;
	}
	if (!initialize_threads()) {
		DG_ERROR("dg_initialize(): failed to init threads");
		return 0;
	}
	return 1;
}

void dg_deinitialize()
{
	deinitialize_sysui();
	deinitialize_filesystem();
	deinitialize_threads();
}
