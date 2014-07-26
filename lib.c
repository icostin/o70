#include "intern.h"

#define L(...) (c42_io8_fmt(w->err, __VA_ARGS__))
//#define L(...)

/* ox_alloc *****************************************************************/
/**
 *  Object index alloc.
 */
static o70_status_t C42_CALL ox_alloc
(
    o70_world_t * restrict w,
    o70_oidx_t * restrict ox
);

/* obj_alloc ****************************************************************/
/**
 *  Allocates an object.
 */
static o70_status_t C42_CALL obj_alloc
(
    o70_world_t * restrict w,
    o70_oidx_t * restrict out,
    o70_oidx_t clox
);

/* o70_lib_name *************************************************************/
O70_API uint8_t const * C42_CALL o70_lib_name ()
{
    return (uint8_t const *) "ostropel-v0000"
#if C42_ARM32
        "-arm32"
#elif C42_ARM64
        "-arm64"
#elif C42_MIPS
        "-mips"
#elif C42_AMD64
        "-amd64"
#elif C42_IA32
        "-ia32"
#else
        "-unknown_arch"
#endif

#if C42_BSLE
        "-bsle"
#elif C42_BSBE
        "-bsbe"
#elif C42_WSLE
        "-wsle"
#elif C42_WSBE
        "-wsbe"
#endif

#if O70_STATIC
        "-static"
#else
        "-dynamic"
#endif

#if _DEBUG
        "-debug"
#else
        "-release"
#endif
        ;
}

/* o70_status_name **********************************************************/
O70_API uint8_t const * C42_CALL o70_status_name (o70_status_t sc)
{
#define X(_s) case _s: return (uint8_t const *) #_s
    switch (sc)
    {
        X(O70S_OK);
        X(O70S_PENDING);
        X(O70S_BAD_ARG);
        X(O70S_NO_MEM);

        X(O70S_BUG);
        X(O70S_TODO);
    }
    return (uint8_t const *) "O70S_unspecified";
}

/* ics_key_cmp **************************************************************/
static uint_fast8_t C42_CALL ics_key_cmp
(
    uintptr_t key,
    c42_rbtree_node_t * node,
    void * restrict context
)
{
    c42_u8an_t * restrict key_str = (c42_u8an_t *) key;
    o70_prop_node_t * pn = (o70_prop_node_t *) node;
    o70_ctstr_t * cs;
    o70_world_t * w = context;
    int c;

    //L("cmp_in k:$xp, node:$xp\n", key, node);
    //L("key: '$.*es'\n", key_str->n, key_str->a);
    //L("node obj ref: $xd\n", pn->kv.key);
    //pn = C42_STRUCT_FROM_FIELD_PTR(o70_prop_node_t, rbtn, node);
    cs = w->ot[O70_RTOX(pn->kv.key)];

    if (key_str->n != cs->data.n)
        return key_str->n < cs->data.n ? C42_RBTREE_LESS : C42_RBTREE_MORE;
    c = c42_u8a_cmp(key_str->a, cs->data.a, key_str->n);
    if (c) return c < 0 ? C42_RBTREE_LESS : C42_RBTREE_MORE;
    return C42_RBTREE_EQUAL;
}

/* ics_node_create **********************************************************/
//static
    o70_status_t C42_CALL ics_node_create
(
    o70_world_t * w,
    c42_rbtree_path_t * path,
    o70_ref_t ctstr
)
{
    o70_prop_node_t * node;
    int mae;
    mae = C42_MA_VAR_ALLOC(&w->ma, node);
    //L("prop_node alloc: $xp (mae=$i)\n", node, mae);
    if (mae) return mae == C42_MA_CORRUPT ? O70S_BUG : O70S_NO_MEM;
    if (!node) return O70S_BUG;
    node->kv.val = node->kv.key = ctstr;
    c42_rbtree_insert(path, &node->rbtn);
    return 0;
}

/* o70_world_init ***********************************************************/
O70_API o70_status_t C42_CALL o70_world_init
(
    o70_world_t * w,
    o70_init_t * ini
)
{
    c42_rbtree_path_t path;
    uint_fast8_t rbte;
    uint_fast8_t mae;
    o70_status_t r;

    C42_VAR_CLEAR(*w);

    do
    {
        w->in = ini->in;
        w->out = ini->out;
        w->err = ini->err;
        //L("world init\n");

        L("t=$x/4z, b=$x/4z, c=$x/4z\n", 
                       ini->ma_total_limit && ini->ma_total_limit < SIZE_MAX
                       ? (size_t) ini->ma_total_limit : SIZE_MAX,
                       ini->ma_block_limit && ini->ma_block_limit < SIZE_MAX
                       ? (size_t) ini->ma_block_limit : SIZE_MAX,
                       ini->ma_count_limit && ini->ma_count_limit < SIZE_MAX
                       ? (size_t) ini->ma_count_limit : SIZE_MAX);
        c42_malim_init(&w->ma, &w->mactx, ini->ma,
                       ini->ma_total_limit && ini->ma_total_limit < SIZE_MAX
                       ? (size_t) ini->ma_total_limit : SIZE_MAX,
                       ini->ma_block_limit && ini->ma_block_limit < SIZE_MAX
                       ? (size_t) ini->ma_block_limit : SIZE_MAX,
                       ini->ma_count_limit && ini->ma_count_limit < SIZE_MAX
                       ? (size_t) ini->ma_count_limit : SIZE_MAX);
        w->om = 1 << c42_u32_bit_width(O70X__COUNT);
        mae = C42_MA_ARRAY_ALLOC(&w->ma, w->ohdr, w->om);
        if (mae) { r = mae == C42_MA_CORRUPT ? O70S_BUG : O70S_NO_MEM; break; }
        w->ffx = w->on = O70X__COUNT;
        c42_upa_set(w->nfx + w->on, 0, w->om - w->on); /* CRASH */

        w->mn = 0;
        w->mm = O70CFG_INIT_MOD_NUM;
        mae = C42_MA_ARRAY_ALLOC(&w->ma, w->mod, w->mm);
        if (mae) { r = mae == C42_MA_CORRUPT ? O70S_BUG : O70S_NO_MEM; break; }
        w->mod[0] = &w->mcore;

        c42_rbtree_init(&w->ics.rbt, ics_key_cmp, w);

        w->ohdr[O70X_NULL            ] = &w->        null_obj.ohdr;
        w->ohdr[O70X_FALSE           ] = &w->       false_obj.ohdr;
        w->ohdr[O70X_TRUE            ] = &w->        true_obj.ohdr;
        w->ohdr[O70X_NULL_CLASS      ] = &w->      null_class.ohdr;
        w->ohdr[O70X_BOOL_CLASS      ] = &w->      bool_class.ohdr;
        w->ohdr[O70X_OBJECT_CLASS    ] = &w->    object_class.ohdr;
        w->ohdr[O70X_CLASS_CLASS     ] = &w->     class_class.ohdr;
        w->ohdr[O70X_ARRAY_CLASS     ] = &w->     array_class.ohdr;
        w->ohdr[O70X_FUNCTION_CLASS  ] = &w->  function_class.ohdr;
        w->ohdr[O70X_STR_CLASS       ] = &w->       str_class.ohdr;
        w->ohdr[O70X_CTSTR_CLASS     ] = &w->     ctstr_class.ohdr;
        w->ohdr[O70X_ACTSTR_CLASS    ] = &w->    actstr_class.ohdr;
        w->ohdr[O70X_EXCEPTION_CLASS ] = &w-> exception_class.ohdr;
        w->ohdr[O70X_MODULE_CLASS    ] = &w->    module_class.ohdr;
        w->ohdr[O70X_NULL_CTSTR      ] = &w->      null_ctstr.ohdr;
        w->ohdr[O70X_FALSE_CTSTR     ] = &w->     false_ctstr.ohdr;
        w->ohdr[O70X_TRUE_CTSTR      ] = &w->      true_ctstr.ohdr;
        w->ohdr[O70X_NULL_CLASS_CTSTR] = &w->null_class_ctstr.ohdr;
        w->ohdr[O70X_BOOL_CTSTR      ] = &w->      bool_ctstr.ohdr;
        w->ohdr[O70X_INT_CTSTR       ] = &w->       int_ctstr.ohdr;
        w->ohdr[O70X_OBJECT_CTSTR    ] = &w->    object_ctstr.ohdr;
        w->ohdr[O70X_CLASS_CTSTR     ] = &w->     class_ctstr.ohdr;
        w->ohdr[O70X_ARRAY_CTSTR     ] = &w->     array_ctstr.ohdr;
        w->ohdr[O70X_FUNCTION_CTSTR  ] = &w->  function_ctstr.ohdr;
        w->ohdr[O70X_STR_CTSTR       ] = &w->       str_ctstr.ohdr;
        w->ohdr[O70X_CTSTR_CTSTR     ] = &w->     ctstr_ctstr.ohdr;
        w->ohdr[O70X_ACTSTR_CTSTR    ] = &w->    actstr_ctstr.ohdr;
        w->ohdr[O70X_EXCEPTION_CTSTR ] = &w-> exception_ctstr.ohdr;
        w->ohdr[O70X_MODULE_CTSTR    ] = &w->    module_ctstr.ohdr;

        w->        null_obj.ohdr.nref = 1;
        w->       false_obj.ohdr.nref = 1;
        w->        true_obj.ohdr.nref = 1;
        w->      null_class.ohdr.nref = 1;
        w->      bool_class.ohdr.nref = 1;
        w->       int_class.ohdr.nref = 1;
        w->    object_class.ohdr.nref = 1;
        w->     class_class.ohdr.nref = 1;
        w->     array_class.ohdr.nref = 1;
        w->  function_class.ohdr.nref = 1;
        w->       str_class.ohdr.nref = 1;
        w->     ctstr_class.ohdr.nref = 1;
        w-> exception_class.ohdr.nref = 1;
        w->    module_class.ohdr.nref = 1;
        w->      null_ctstr.ohdr.nref = 1;
        w->     false_ctstr.ohdr.nref = 1;
        w->      true_ctstr.ohdr.nref = 1;
        w->null_class_ctstr.ohdr.nref = 1;
        w->      bool_ctstr.ohdr.nref = 1;
        w->       int_ctstr.ohdr.nref = 1;
        w->    object_ctstr.ohdr.nref = 1;
        w->     class_ctstr.ohdr.nref = 1;
        w->     array_ctstr.ohdr.nref = 1;
        w->  function_ctstr.ohdr.nref = 1;
        w->       str_ctstr.ohdr.nref = 1;
        w->     ctstr_ctstr.ohdr.nref = 1;
        w->    actstr_ctstr.ohdr.nref = 1;
        w-> exception_ctstr.ohdr.nref = 1;
        w->    module_ctstr.ohdr.nref = 1;

        w->        null_obj.ohdr.class_ox = O70X_NULL_CLASS;
        w->       false_obj.ohdr.class_ox = O70X_BOOL_CLASS;
        w->        true_obj.ohdr.class_ox = O70X_BOOL_CLASS;
        w->      null_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->      bool_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->       int_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->    object_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->     class_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->     array_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->  function_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->       str_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->     ctstr_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w-> exception_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->    module_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->      null_ctstr.ohdr.class_ox = O70X_CTSTR_CLASS;
        w->     false_ctstr.ohdr.class_ox = O70X_CTSTR_CLASS;
        w->      true_ctstr.ohdr.class_ox = O70X_CTSTR_CLASS;
        w->null_class_ctstr.ohdr.class_ox = O70X_CTSTR_CLASS;
        w->      bool_ctstr.ohdr.class_ox = O70X_CTSTR_CLASS;
        w->       int_ctstr.ohdr.class_ox = O70X_CTSTR_CLASS;
        w->    object_ctstr.ohdr.class_ox = O70X_CTSTR_CLASS;
        w->     class_ctstr.ohdr.class_ox = O70X_CTSTR_CLASS;
        w->     array_ctstr.ohdr.class_ox = O70X_CTSTR_CLASS;
        w->  function_ctstr.ohdr.class_ox = O70X_CTSTR_CLASS;
        w->       str_ctstr.ohdr.class_ox = O70X_CTSTR_CLASS;
        w->     ctstr_ctstr.ohdr.class_ox = O70X_CTSTR_CLASS;
        w->    actstr_ctstr.ohdr.class_ox = O70X_CTSTR_CLASS;
        w-> exception_ctstr.ohdr.class_ox = O70X_CTSTR_CLASS;
        w->    module_ctstr.ohdr.class_ox = O70X_CTSTR_CLASS;

#define A(_x, _id, _str) \
        w->_id.data.a = (uint8_t *) (_str); w->_id.data.n = sizeof(_str) - 1; \
        rbte = c42_rbtree_find(&path, &w->ics.rbt, (uintptr_t) &w->_id.data); \
        if (rbte != C42_RBTREE_NOT_FOUND) { r = O70S_BUG; break; } \
        if ((r = ics_node_create(w, &path, O70_XTOR((_x))))) break;

        A(O70X_NULL_CTSTR      ,       null_ctstr, "null"         );
        A(O70X_FALSE_CTSTR     ,      false_ctstr, "false"        );
        A(O70X_TRUE_CTSTR      ,       true_ctstr, "true"         );
        A(O70X_NULL_CLASS_CTSTR, null_class_ctstr, "null_class"   );
        A(O70X_BOOL_CTSTR      ,       bool_ctstr, "bool"         );
        A(O70X_INT_CTSTR       ,        int_ctstr, "int"          );
        A(O70X_OBJECT_CTSTR    ,     object_ctstr, "object"       );
        A(O70X_CLASS_CTSTR     ,      class_ctstr, "class"        );
        A(O70X_ARRAY_CTSTR     ,      array_ctstr, "array"        );
        A(O70X_FUNCTION_CTSTR  ,   function_ctstr, "function"     );
        A(O70X_STR_CTSTR       ,        str_ctstr, "str"          );
        A(O70X_CTSTR_CTSTR     ,      ctstr_ctstr, "ctstr"        );
        A(O70X_ACTSTR_CTSTR    ,     actstr_ctstr, "actstr"       );
        A(O70X_EXCEPTION_CTSTR ,  exception_ctstr, "exception"    );
        A(O70X_MODULE_CTSTR    ,     module_ctstr, "module"       );
#undef A

        w->      null_class.isize = sizeof(o70_slim_obj_t   );
        w->      bool_class.isize = sizeof(o70_slim_obj_t   );
        w->       int_class.isize = sizeof(o70_slim_obj_t   );
        w->    object_class.isize = sizeof(o70_slim_obj_t   );
        w->     class_class.isize = sizeof(o70_class_t      );
        w->     array_class.isize = sizeof(o70_array_t      );
        w->  function_class.isize = sizeof(o70_function_t   );
        w->       str_class.isize = sizeof(o70_str_t        );
        w->     ctstr_class.isize = sizeof(o70_ctstr_t      );
        w-> exception_class.isize = sizeof(o70_exception_t  );
        w->    module_class.isize = sizeof(o70_module_t     );

        r = 0;
    }
    while (0);

    if (r) w->aux_status = o70_world_finish(w);

    return r;
}

/* o70_world_finish *********************************************************/
O70_API o70_status_t C42_CALL o70_world_finish
(
    o70_world_t * w
)
{
    uint_fast8_t mae;
    o70_status_t rs = 0;

    if (w->om)
    {
        mae = C42_MA_ARRAY_FREE(&w->ma, w->ohdr, w->om);
        if (mae) return O70S_BUG;
        w->om = 0;
    }

    if (w->mm)
    {
        mae = C42_MA_ARRAY_FREE(&w->ma, w->mod, w->mm);
        if (mae) return O70S_BUG;
        w->mm = 0;
    }

    return rs;
}

/* ox_alloc *****************************************************************/
static o70_status_t C42_CALL ox_alloc
(
    o70_world_t * restrict w,
    o70_oidx_t * restrict ox
)
{
    if (!w->ffx)
    {
        L("ox_alloc: need to realloc obj table\n");
        return O70S_TODO;
    }
    w->ffx = w->nfx[*ox = w->ffx];
    return 0;
}

/* obj_alloc ****************************************************************/
static o70_status_t C42_CALL obj_alloc
(
    o70_world_t * restrict w,
    o70_oidx_t * restrict out,
    o70_oidx_t clox
)
{
    o70_ohdr_t * oh;
    o70_class_t * cls;
    o70_status_t os;
    uint_fast8_t mae;
    size_t ox;
    os = ox_alloc(w, out);
    if (os) return os;
    ox = *out;
    cls = w->ot[clox];
    mae = c42_ma_alloc(&w->ma, w->ot + ox, 1, cls->isize);
    L("obj_alloc: isize=$xz, mae=$i\n", cls->isize, mae);
    if (mae) { return mae == C42_MA_CORRUPT ? O70S_BUG : O70S_NO_MEM; }
    oh = w->ohdr[ox];
    oh->nref = 1;
    oh->class_ox = clox;
    return 0;
}

/* _o70_obj_destroy *********************************************************/
O70_API o70_status_t C42_CALL _o70_obj_destroy (o70_world_t * w)
{
    uint32_t dx, fmx;
    o70_class_t * c;
    o70_ohdr_t * oh;
    o70_status_t st;
    uint_fast8_t mae;

    fmx = 0;
    while ((dx = w->fdx))
    {
        L("obj_destroy: finishing ox=$xd\n", dx);
        w->fdx = (oh = w->ohdr[dx])->ndx;
        c = w->ot[oh->class_ox];
        if (c->finish)
        {
            st = c->finish(w, dx);
            if (st) return st;
        }
        oh->ndx = fmx;
        fmx = dx;
    }
    for (dx = fmx; dx; dx = fmx)
    {
        L("obj_destroy: freeing 0x=$xd\n", dx);
        c = w->ot[(oh = w->ohdr[dx])->class_ox];
        fmx = oh->ndx;
        mae = c42_ma_free(&w->ma, oh, 1, c->isize);
        if (mae)
        {
            L("obj_destroy: ma_free error $b\n", mae);
            return O70S_BUG;
        }
    }

    return 0;
}

/* o70_flow_create **********************************************************/
O70_API o70_status_t C42_CALL o70_flow_create
(
    o70_world_t * w,
    o70_flow_t * * flow_ptr,
    uint32_t max_stack_depth
)
{
    (void) w;
    (void) flow_ptr;
    (void) max_stack_depth;
    return O70S_TODO;
}

/* o70_flow_destroy *********************************************************/
O70_API o70_status_t C42_CALL o70_flow_destroy (o70_flow_t * flow)
{
    (void) flow;
    return O70S_TODO;
}

/* o70_dump_icst ************************************************************/
O70_API o70_status_t C42_CALL o70_dump_icst
(
    o70_world_t * w,
    c42_io8_t * o
)
{
    uint_fast8_t e;
    c42_rbtree_path_t path;
    c42_rbtree_node_t * n;
    o70_prop_node_t * pn;
    o70_ctstr_t * cs;

    for (n = c42_rbtree_first(&path, &w->ics.rbt);
         n;
         n = c42_rbtree_np(&path, C42_RBTREE_MORE))
    {
        pn = (o70_prop_node_t *) n;
        cs = w->ot[O70_RTOX(pn->kv.key)];
        e = c42_io8_fmt(o, "$.*es\n", cs->data.n, cs->data.a);
        if (e) L("o70_dump_icst: print failed: $b\n", e);
        if (e) return O70S_IO_ERROR;
    }
    return 0;
}

/* o70_static_ctstr *********************************************************/
O70_API o70_status_t C42_CALL o70_static_ctstr
(
    o70_world_t * w,
    o70_ref_t * out,
    void const * ptr,
    size_t len
)
{
    o70_status_t os;
    o70_oidx_t x;
    o70_ctstr_t * cs;
    os = obj_alloc(w, &x, O70X_CTSTR_CLASS);
    if (os) return os;
    cs = w->ot[x];
    cs->data.a = (uint8_t *) ptr;
    cs->data.n = len;
    *out = O70_XTOR(x);
    return 0;
}

/* o70_static_ctstr_intern **************************************************/
O70_API o70_status_t C42_CALL o70_static_ctstr_intern
(
    o70_world_t * w,
    o70_ref_t * out,
    void const * ptr,
    size_t len
)
{
    // c42_rbtree_path_t path;
    // c42_u8an_t ba;
    // uint_fast8_t rbte;
    (void) w;
    (void) out;
    (void) ptr;
    (void) len;
    // /* init a byte array to use it as lookup key in the tree of internalised
    //  * constant strings */
    // ba.a = (void *) ptr;
    // ba.n = len;

    // /* now do the lookup */
    // rbte = c42_rbtree_find(&path, &w->ics.rbt, (uintptr_t) &ba);
    // if (rbte == C42_RBTREE_FOUND)
    //     w->_id.data.a = (uint8_t *) (_str); w->_id.data.n = sizeof(_str) - 1;
    //     rbte = c42_rbtree_find(&path, &w->ics.rbt, (uintptr_t) &w->_id.data);
    //     if (rbte != C42_RBTREE_NOT_FOUND) { r = O70S_BUG; break; }
    //     if ((r = ics_node_create(w, &path, O70_XTOR((_x))))) break;
    return O70S_TODO;
}

