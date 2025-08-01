#pragma once
#include "dg_libcommon.h"

typedef struct dg_llnode_s {
	struct  dg_llnode_s* plast;
	struct  dg_llnode_s* pnext;
	uint8_t data[];
} dg_llnode_t;

dg_llnode_t* list_new_node(
	dg_llnode_t *plast,
	dg_llnode_t *pnext,
	size_t size);

static inline void list_node_insert_before(
	dg_llnode_t* target,
	dg_llnode_t* node)
{
	node->plast = target->plast;
	node->pnext = target;
	if (target->plast)
		target->plast->pnext = node;
	target->plast = node;
}

static inline void list_node_insert_after(
	dg_llnode_t* target,
	dg_llnode_t* node)
{
	node->pnext = target->pnext;
	node->plast = target;
	if (target->pnext)
		target->pnext->plast = node;
	target->pnext = node;
}

static inline dg_llnode_t* list_node_tail(dg_llnode_t* pnode) {
	while (pnode->pnext)
		pnode = pnode->pnext;

	return pnode;
}

static inline dg_llnode_t* list_node_head(dg_llnode_t* pnode) {
	while (pnode->plast)
		pnode = pnode->plast;

	return pnode;
}

typedef struct dg_list_s {
	size_t       elemsize;
	dg_llnode_t* pbegin;
	dg_llnode_t* pend;
} dg_list_t;

#define list_init(type) { .elemsize = sizeof(type), .pbegin = NULL, .pend = NULL }

dg_llnode_t *list_add_node_ex(dg_list_t* plist, bool back);

static inline bool list_add_back(dg_list_t* plist, const void* psrc) {
	dg_llnode_t* pnode = list_add_node_ex(plist, true);
	if (!pnode)
		return false;
	memcpy(pnode->data, psrc, plist->elemsize);
	return true;
}

static inline bool list_add_front(dg_list_t* plist, const void* psrc) {
	dg_llnode_t* pnode = list_add_node_ex(plist, false);
	if (!pnode)
		return false;
	memcpy(pnode->data, psrc, plist->elemsize);
	return true;
}

static inline dg_llnode_t* list_get_at(dg_list_t* plist, size_t idx) {
	dg_llnode_t* pnode = plist->pbegin;
	for (size_t i = 0; pnode; pnode = pnode->pnext, i++)
		if (i == idx)
			return pnode;

	return NULL;
}

static inline bool list_free(dg_list_t* plist) {
	dg_llnode_t* pnode = plist->pbegin, *pcurnode;
	if (!pnode)
		return false;

	while (pnode) {
		pcurnode = pnode;
		pnode = pnode->pnext;
		free(pcurnode);
	}
	plist->pbegin = plist->pend = NULL;
	return true;
}