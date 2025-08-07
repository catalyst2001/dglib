#include "dg_list.h"
#include "dg_alloc.h"

dg_llnode_t* list_new_node(
  dg_llnode_t* plast,
  dg_llnode_t* pnext,
  size_t size)
{
  dg_llnode_t* pnode = (dg_llnode_t*)calloc(1, sizeof(dg_llnode_t) + size);
  if (!pnode)
    return NULL;

  pnode->plast = plast;
  pnode->pnext = pnext;
  return pnode;
}

dg_llnode_t* list_add_node_ex(dg_list_t* plist, bool back)
{
  dg_llnode_t* pnewnode = list_new_node(
    back ? plist->pend : NULL,
    back ? NULL : plist->pbegin,
    plist->elemsize);
  if (!pnewnode)
    return NULL;

  if (back) {
    if (plist->pend) {
      dg_llnode_t* pendnode = plist->pend;
      assert(pendnode->pnext == NULL && "pendnode->pnext must be NULL for node in back of list");
      pendnode->pnext = pnewnode;
    }
    else {
      plist->pbegin = pnewnode;
    }
    plist->pend = pnewnode;
  }
  else {
    if (plist->pbegin) {
      dg_llnode_t* pbeginnode = plist->pbegin;
      assert(pbeginnode->plast == NULL && "pbeginnode->plast must be NULL for node in front of list");
      pbeginnode->plast = pnewnode;
    }
    else {
      plist->pend = pnewnode;
    }
    plist->pbegin = pnewnode;
  }
  return pnewnode;
}
