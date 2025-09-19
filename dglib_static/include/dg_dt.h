#pragma once
#include "dg_libcommon.h"

#if defined(_MSC_VER)
#define DG_DTCALL 
#elif defined(__GCC__) || defined(__clang__)
#define DG_DTCALL 
#endif

typedef struct dt_id_s {
	uint32_t familyid;
	uint32_t objectid;
} dt_id_t;

#define DT_DECL_BEGIN(type)\
	struct type##_s;\
	typedef struct type##_s type;\
	struct type##_s {\
		dt_id_t id;

#define DT_DECL_FUNCTION(return_type, funcname, ...)\
	return_type (DG_DTCALL *funcname)(__VA_ARGS__);

#define DT_DECL_END()\
		void *enddt;\
	};

#define DT_INIT_BEGIN(dt_type, varname, objfamilyid, objid)\
	static const dt_type varname = {\
		.id = {\
			.familyid = (objfamilyid),\
			.objectid = (objid)\
		},

#define DT_INIT_FUNCTION(fieldname, funcname)\
		.fieldname = funcname,

#define DT_INIT_END()\
		.enddt = NULL\
	};

#define DT_GET_FUNCS(pdt) ((void **)((uint8_t*)pdt + sizeof(dt_id_t)))

size_t dt_get_num_functions(const void *psrcdt, size_t maxfunctions);
size_t dt_copy(void **pdst, const void *psrc, size_t maxfunctions);

/**
* new object declaration
*/
#define DECL_OBJECT_BEGIN(obj_type_name, obj_dt_name)\
struct obj_type_name##_s;\
typedef struct obj_type_name##_s obj_type_name;\
struct obj_type_name##_s {\
	obj_dt_name *pdt;

#define DECL_OBJECT_END() };