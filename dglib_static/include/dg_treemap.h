// dg_treemap.h — header-only C99 ordered map based on a Red-Black Tree
// Public Domain / Unlicense. Single-file: in ONE C file, do
//   #define DG_TMAP_IMPLEMENTATION
// before including this header to emit function bodies.
//
// Goals vs hash map:
// - Ordered keys, deterministic in-order iteration
// - O(log N) worst-case insert/find/erase
// - Erase-during-iteration is trivial and doesn't trigger rehash/moves
// - Stable iterators unless you erase the pointed node
//
// API is macro-generated per (K,V) pair with a user-supplied comparator CMP(a,b):
//   <0 if a<b, 0 if equal, >0 if a>b.
// Optional KEYFREE/VALFREE hooks via _EX variant.
//
// Example:
//   typedef struct { uint64_t id; } Key; // comparator by id
//   static int key_cmp(Key a, Key b){ return (a.id<b.id)?-1:(a.id> b.id); }
//   DG_TMAP_DECL(u64_to_ptr, Key, void*);
//   DG_TMAP_IMPL(u64_to_ptr, Key, void*, key_cmp);
//
//   u64_to_ptr m; if(!u64_to_ptr_init(&m)) {/*oom*/}
//   void *p=(void*)123; u64_to_ptr_insert(&m,(Key){42}, &p, NULL);
//   void **pref = u64_to_ptr_get_ref(&m,(Key){42});
//   for (u64_to_ptr_it it=u64_to_ptr_iter_begin(&m); it; ) {
//       if (*u64_to_ptr_iter_val(&m,it)==p) it = u64_to_ptr_erase_at(&m,it); else it=u64_to_ptr_iter_next(it);
//   }
//   u64_to_ptr_destroy(&m);
//
#ifndef DG_TREE_MAP_H
#define DG_TREE_MAP_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef DG_TMAP_MALLOC
#  define DG_TMAP_MALLOC(sz) malloc(sz)
#endif
#ifndef DG_TMAP_FREE
#  define DG_TMAP_FREE(p) free(p)
#endif

#define DG__NOOP_FREE(p) ((void)0)

// Declarations generator
#define DG_TMAP_DECL(NAME, K, V) \
    typedef struct NAME NAME; \
    typedef struct NAME##_node NAME##_node; \
    typedef NAME##_node* NAME##_it; \
    struct NAME { NAME##_node* root; size_t size; }; \
    /* basic ops */ \
    int  NAME##_init(NAME* m); \
    void NAME##_clear(NAME* m); \
    void NAME##_destroy(NAME* m); \
    size_t NAME##_size(const NAME* m); \
    /* CRUD */ \
    int  NAME##_insert(NAME* m, K key, const V* val, int* replaced); \
    int  NAME##_get(const NAME* m, K key, V* out); \
    V*   NAME##_get_ref(NAME* m, K key); \
    int  NAME##_erase(NAME* m, K key); \
    /* iterators (in-order) */ \
    NAME##_it NAME##_iter_begin(const NAME* m); \
    NAME##_it NAME##_iter_end(const NAME* m); \
    NAME##_it NAME##_iter_next(NAME##_it it); \
    NAME##_it NAME##_iter_prev(NAME##_it it); \
    K* NAME##_iter_key(NAME##_it it); \
    V* NAME##_iter_val(NAME##_it it); \
    NAME##_it NAME##_erase_at(NAME* m, NAME##_it it); \
    /* bounds */ \
    NAME##_it NAME##_lower_bound(const NAME* m, K key); \
    NAME##_it NAME##_upper_bound(const NAME* m, K key);

// Implementation generator (default free hooks = no-op)
#define DG_TMAP_IMPL(NAME, K, V, CMPFN) \
    DG_TMAP_IMPL_EX(NAME, K, V, CMPFN, DG__NOOP_FREE, DG__NOOP_FREE)

#define DG_TMAP_IMPL_EX(NAME, K, V, CMPFN, KEYFREE, VALFREE) \
    struct NAME##_node { \
        K key; V val; \
        struct NAME##_node *left, *right, *parent; \
        unsigned char red; /* 1=red, 0=black */ \
    }; \
    static inline NAME##_it NAME##__min_(NAME##_it n){ if(!n) return NULL; while(n->left) n=n->left; return n; } \
    static inline NAME##_it NAME##__max_(NAME##_it n){ if(!n) return NULL; while(n->right) n=n->right; return n; } \
    static inline NAME##_it NAME##__succ_(NAME##_it n){ if(!n) return NULL; if(n->right) return NAME##__min_(n->right); \
        NAME##_it p=n->parent; while(p && n==p->right){ n=p; p=p->parent; } return p; } \
    static inline NAME##_it NAME##__pred_(NAME##_it n){ if(!n) return NULL; if(n->left) return NAME##__max_(n->left); \
        NAME##_it p=n->parent; while(p && n==p->left){ n=p; p=p->parent; } return p; } \
    static inline void NAME##__rotate_left_(NAME* m, NAME##_it x){ \
        NAME##_it y=x->right; x->right=y->left; if(y->left) y->left->parent=x; \
        y->parent=x->parent; \
        if(!x->parent) m->root=y; else if(x==x->parent->left) x->parent->left=y; else x->parent->right=y; \
        y->left=x; x->parent=y; } \
    static inline void NAME##__rotate_right_(NAME* m, NAME##_it x){ \
        NAME##_it y=x->left; x->left=y->right; if(y->right) y->right->parent=x; \
        y->parent=x->parent; \
        if(!x->parent) m->root=y; else if(x==x->parent->right) x->parent->right=y; else x->parent->left=y; \
        y->right=x; x->parent=y; } \
    static void NAME##__insert_fix_(NAME* m, NAME##_it z){ \
        while(z->parent && z->parent->red){ \
            NAME##_it p=z->parent; NAME##_it g=p->parent; \
            if(p==g->left){ NAME##_it y=g->right; \
                if(y && y->red){ p->red=0; y->red=0; g->red=1; z=g; } \
                else { if(z==p->right){ z=p; NAME##__rotate_left_(m,z); p=z->parent; g=p->parent; } \
                       p->red=0; g->red=1; NAME##__rotate_right_(m,g); } \
            } else { NAME##_it y=g->left; \
                if(y && y->red){ p->red=0; y->red=0; g->red=1; z=g; } \
                else { if(z==p->left){ z=p; NAME##__rotate_right_(m,z); p=z->parent; g=p->parent; } \
                       p->red=0; g->red=1; NAME##__rotate_left_(m,g); } \
            } \
        } \
        m->root->red=0; \
    } \
    static void NAME##__transplant_(NAME* m, NAME##_it u, NAME##_it v){ \
        if(!u->parent) m->root=v; else if(u==u->parent->left) u->parent->left=v; else u->parent->right=v; \
        if(v) v->parent=u->parent; \
    } \
    static void NAME##__erase_fix_(NAME* m, NAME##_it x, NAME##_it xparent){ \
        while( (x!=m->root) && (!x || !x->red) ){ \
            if(x == (xparent?xparent->left:NULL)){ NAME##_it w = xparent?xparent->right:NULL; \
                if(w && w->red){ w->red=0; if(xparent) xparent->red=1; NAME##__rotate_left_(m,xparent); w = xparent?xparent->right:NULL; } \
                if( (!w->left || !w->left->red) && (!w->right || !w->right->red) ){ if(w) w->red=1; x=xparent; xparent=xparent?xparent->parent:NULL; } \
                else { if(!w->right || !w->right->red){ if(w->left) w->left->red=0; if(w){ w->red=1; NAME##__rotate_right_(m,w);} w=xparent?xparent->right:NULL; } \
                       if(w) w->red = xparent?xparent->red:0; if(xparent) xparent->red=0; if(w && w->right) w->right->red=0; NAME##__rotate_left_(m,xparent); x=m->root; xparent=NULL; } \
            } else { NAME##_it w = xparent?xparent->left:NULL; \
                if(w && w->red){ w->red=0; if(xparent) xparent->red=1; NAME##__rotate_right_(m,xparent); w = xparent?xparent->left:NULL; } \
                if( (!w->right || !w->right->red) && (!w->left || !w->left->red) ){ if(w) w->red=1; x=xparent; xparent=xparent?xparent->parent:NULL; } \
                else { if(!w->left || !w->left->red){ if(w->right) w->right->red=0; if(w){ w->red=1; NAME##__rotate_left_(m,w);} w=xparent?xparent->left:NULL; } \
                       if(w) w->red = xparent?xparent->red:0; if(xparent) xparent->red=0; if(w && w->left) w->left->red=0; NAME##__rotate_right_(m,xparent); x=m->root; xparent=NULL; } \
            } \
        } \
        if(x) x->red=0; \
    } \
    int NAME##_init(NAME* m){ if(!m) return 0; m->root=NULL; m->size=0; return 1; } \
    void NAME##_clear(NAME* m){ if(!m) return; \
        NAME##_it it = NAME##_iter_begin(m); \
        while(it){ NAME##_it nxt = NAME##_iter_next(it); KEYFREE(&it->key); VALFREE(&it->val); DG_TMAP_FREE(it); it = nxt; } \
        m->root=NULL; m->size=0; \
    } \
    void NAME##_destroy(NAME* m){ if(!m) return; NAME##_clear(m); } \
    size_t NAME##_size(const NAME* m){ return m?m->size:0; } \
    static NAME##_it NAME##__find_node_(const NAME* m, K key){ NAME##_it cur=m?m->root:NULL; while(cur){ int c=CMPFN(key,cur->key); if(c==0) return cur; cur = (c<0)?cur->left:cur->right; } return NULL; } \
    int NAME##_get(const NAME* m, K key, V* out){ NAME##_it n=NAME##__find_node_(m,key); if(!n) return 0; if(out) *out=n->val; return 1; } \
    V* NAME##_get_ref(NAME* m, K key){ NAME##_it n=NAME##__find_node_(m,key); return n?&n->val:NULL; } \
    int NAME##_insert(NAME* m, K key, const V* val, int* replaced){ \
        if(!m) return 0; \
        NAME##_it y=NULL, x=m->root; \
        while(x){ y=x; int c=CMPFN(key,x->key); if(c==0){ if(replaced) *replaced=1; VALFREE(&x->val); x->val=*val; return 1; } x = (c<0)?x->left:x->right; } \
        NAME##_it z=(NAME##_it)DG_TMAP_MALLOC(sizeof(*z)); if(!z) return 0; \
        z->key=key; z->val=*val; z->left=z->right=NULL; z->parent=y; z->red=1; \
        if(!y) m->root=z; else { if(CMPFN(key,y->key)<0) y->left=z; else y->right=z; } \
        NAME##__insert_fix_(m,z); m->size++; if(replaced) *replaced=0; return 1; \
    } \
    static NAME##_it NAME##__erase_node_(NAME* m, NAME##_it z){ \
        NAME##_it y=z, x=NULL; unsigned char y_red=y->red; NAME##_it xparent=NULL; \
        if(!z->left){ x=z->right; xparent=z->parent; NAME##__transplant_(m,z,z->right); } \
        else if(!z->right){ x=z->left; xparent=z->parent; NAME##__transplant_(m,z,z->left); } \
        else { y=NAME##__min_(z->right); y_red=y->red; x=y->right; \
            if(y->parent==z){ xparent=y; if(x) x->parent=y; } \
            else { NAME##__transplant_(m,y,y->right); y->right=z->right; if(y->right) y->right->parent=y; xparent=y->parent; } \
            NAME##__transplant_(m,z,y); y->left=z->left; if(y->left) y->left->parent=y; y->red=z->red; \
        } \
        if(!y_red) NAME##__erase_fix_(m,x,xparent); \
        return z; /* caller frees z */ \
    } \
    int NAME##_erase(NAME* m, K key){ if(!m) return 0; NAME##_it z=NAME##__find_node_(m,key); if(!z) return 0; \
        NAME##_it victim = NAME##__erase_node_(m,z); KEYFREE(&victim->key); VALFREE(&victim->val); DG_TMAP_FREE(victim); m->size--; return 1; } \
    NAME##_it NAME##_iter_begin(const NAME* m){ return m?NAME##__min_(m->root):NULL; } \
    NAME##_it NAME##_iter_end(const NAME* m){ (void)m; return NULL; } \
    NAME##_it NAME##_iter_next(NAME##_it it){ return NAME##__succ_(it); } \
    NAME##_it NAME##_iter_prev(NAME##_it it){ return NAME##__pred_(it); } \
    K* NAME##_iter_key(NAME##_it it){ return it?&it->key:NULL; } \
    V* NAME##_iter_val(NAME##_it it){ return it?&it->val:NULL; } \
    NAME##_it NAME##_erase_at(NAME* m, NAME##_it it){ if(!m || !it) return NULL; NAME##_it nxt=NAME##__succ_(it); \
        NAME##_it victim = NAME##__erase_node_(m,it); KEYFREE(&victim->key); VALFREE(&victim->val); DG_TMAP_FREE(victim); m->size--; return nxt; } \
    NAME##_it NAME##_lower_bound(const NAME* m, K key){ NAME##_it cur=m?m->root:NULL, res=NULL; while(cur){ int c=CMPFN(key,cur->key); if(c<=0){ res=cur; cur=cur->left; } else cur=cur->right; } return res; } \
    NAME##_it NAME##_upper_bound(const NAME* m, K key){ NAME##_it cur=m?m->root:NULL, res=NULL; while(cur){ int c=CMPFN(key,cur->key); if(c<0){ res=cur; cur=cur->left; } else cur=cur->right; } return res; }

#endif /* DG_TREE_MAP_H */
