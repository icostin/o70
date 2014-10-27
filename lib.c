#include "intern.h"

#if _DEBUG
#define L(...) O70_LOG(w, __VA_ARGS__)
#define A(...) O70_ASSERT(w, __VA_ARGS__)
//#define NZDBGCHK(_expr) if ((_expr)) ; else do { L("*** BUG *** $s:$i:$s: ($s) is zero\n", __FILE__, __LINE__, __FUNCTION__, #_expr); return O70S_BUG; } while (0)
#else
#define L(...)
#define A(...)
//#define NZDBGCHK(_expr)
#endif
//#define A NZDBGCHK

//#define L(...)

/* ics_key_cmp **************************************************************/
/**
 *  Internalised const string key compare function.
 *  This is a callback function used when finding an internalised const string.
 *  @param key [in] a pointer to an c42_u8an_t instance
 *  @param node [in] an o70_prop_node_t instance
 *  @param context [in] an o70_world_t instance
 */
static uint_fast8_t C42_CALL ics_key_cmp
(
    uintptr_t key,
    c42_rbtree_node_t * node,
    void * restrict context
);

/* ics_node_create **********************************************************/
/**
 *  Creates a node in the internalised const string tree.
 *  @note this increases the number of references to @a ctstr
 */
static o70_status_t C42_CALL ics_node_create
(
    o70_world_t * w,
    c42_rbtree_path_t * path,
    o70_ref_t ctstr
);

/* ics_node_delete **********************************************************/
/**
 *  Deletes a node from ics tree.
 */
static o70_status_t C42_CALL ics_node_delete
(
    o70_world_t * w,
    o70_ref_t ctstr
);

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

/* prop_bag_init ************************************************************/
/**
 *  Inits a property bag.
 *  Property bags are used for object fields, methods and maybe others.
 */
static void C42_CALL prop_bag_init
(
    o70_prop_bag_t * bag
);

/* prop_bag_finish **********************************************************/
static o70_status_t C42_CALL prop_bag_finish
(
    o70_world_t * w,
    o70_prop_bag_t * bag,
    int deref
);

/* prop_bag_peek *************************************************************/
/**
 *  Retrieves a property.
 *  @retval 0 found it
 *  @retval O70S_MISSING no property by that name
 *  @warning this DOES NOT increment the ref count for the value when found
 */
static o70_status_t C42_CALL prop_bag_peek
(
    o70_prop_bag_t * bag,
    o70_ref_t name,
    o70_ref_t * value
);

/* prop_bag_put *************************************************************/
/**
 *  Changes/adds a property.
 *  @note @a name will get the ref count incremented if it was not part of the
 *  bag.
 *  @note @a value will have the ref count incremented.
 *  @retval 0 ok
 *  @retval O70S_NO_MEM failed allocating memory to store the new property
 *  @retval O70S_BUG bug found; don't touch anything, exit asap
 */
static o70_status_t C42_CALL prop_bag_put
(
    o70_world_t * w,
    o70_prop_bag_t * bag,
    o70_ref_t name,
    o70_ref_t value
);

/* prop_key_cmp *************************************************************/
/**
 *  Compares property keys.
 *  @param key an object ref pointing to the internalised const string for
 *          property being looked for
 *  @param node pointer to struct o70_prop_node_t
 *  @param context unused
 */
static uint_fast8_t C42_CALL prop_key_cmp
(
    uintptr_t key,
    c42_rbtree_node_t * node,
    void * restrict context
);

/* dynobj_finish ************************************************************/
static o70_status_t C42_CALL dynobj_finish
(
    o70_world_t * w,
    o70_ref_t o
);

/* str_finish ***************************************************************/
static o70_status_t C42_CALL str_finish
(
    o70_world_t * w,
    o70_ref_t r
);

/* ictstr_finish ************************************************************/
static o70_status_t C42_CALL ictstr_finish
(
    o70_world_t * w,
    o70_ref_t r
);

/* func_finish **************************************************************/
static o70_status_t C42_CALL func_finish
(
    o70_world_t * w,
    o70_ref_t r
);

/* ifunc_exectx_finish ******************************************************/
static o70_status_t C42_CALL ifunc_exectx_finish
(
    o70_world_t * w,
    o70_ref_t obj
);

/* ifunc_exectx_exec ********************************************************/
static o70_status_t C42_CALL ifunc_exectx_exec
(
    o70_flow_t * flow,
    o70_exectx_t * e
);

/* ifunc_finish *************************************************************/
static o70_status_t C42_CALL ifunc_finish
(
    o70_world_t * w,
    o70_ref_t r
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
        X(O70S_EXC);
        X(O70S_MISSING);
        X(O70S_BAD_ARG);
        X(O70S_BAD_TYPE);
        X(O70S_NO_MEM);
        X(O70S_IO_ERROR);
        X(O70S_BAD_FMT);
        X(O70S_BAD_UTF8);
        X(O70S_CONV_ERROR);
        X(O70S_OTHER);
        X(O70S_BUG);
        X(O70S_TODO);
    }
    return (uint8_t const *) "O70S_unspecified";
#undef X
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

    A(O70_IS_OREF(pn->kv.key));
    cs = w->ot[O70_RTOX(pn->kv.key)];

    if (key_str->n != cs->data.n)
        return key_str->n < cs->data.n ? C42_RBTREE_LESS : C42_RBTREE_MORE;
    c = c42_u8a_cmp(key_str->a, cs->data.a, key_str->n);
    if (c) return c < 0 ? C42_RBTREE_LESS : C42_RBTREE_MORE;
    return C42_RBTREE_EQUAL;
}

/* ics_node_create **********************************************************/
static o70_status_t C42_CALL ics_node_create
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
    A(node);
    node->kv.val = node->kv.key = ctstr;
    c42_rbtree_insert(path, &node->rbtn);
    o70_ref_inc(w, ctstr);
    return 0;
}

/* ics_node_delete **********************************************************/
static o70_status_t C42_CALL ics_node_delete
(
    o70_world_t * w,
    o70_ref_t r
)
{
    c42_rbtree_path_t path;
    o70_ctstr_t * cs;
    o70_prop_node_t * n;

    uint_fast8_t rbte;
    uint_fast8_t mae;
    if (!(o70_model(w, r) & O70M_ICTSTR)) return O70S_BAD_TYPE;
    cs = w->ot[O70_RTOX(r)];
    rbte = c42_rbtree_find(&path, &w->ics.rbt, (uintptr_t) &cs->data);
    if (rbte != C42_RBTREE_FOUND) return O70S_MISSING;
#if _DEBUG > 2
    {
        unsigned int i;
        L("ics_node_delete: path ($d):", path.last + 1);
        for (i = 0; i <= path.last; ++i)
        {
            L(" $xp ($c)", path.nodes[i],
              path.sides[i] ? (path.sides[i] == 1 ? 'r' : 'e') : 'l');
        }
        L(".\n");
    }
#endif
    n = (o70_prop_node_t *) path.nodes[path.last];
    c42_rbtree_delete(&path);
    mae = C42_MA_VAR_FREE(&w->ma, n);
    if (mae) return O70S_BUG;
    return 0;
}

/* o70_world_init ***********************************************************/
O70_API o70_status_t C42_CALL o70_world_init
(
    o70_world_t * w,
    o70_init_t * ini
)
{
#define I(_x, _id, _str) \
        w->_id.data.a = (uint8_t *) (_str); w->_id.data.n = sizeof(_str) - 1; \
        rbte = c42_rbtree_find(&path, &w->ics.rbt, (uintptr_t) &w->_id.data); \
        if (rbte != C42_RBTREE_NOT_FOUND) { r = O70S_BUG; break; } \
        if ((r = ics_node_create(w, &path, O70_XTOR((_x))))) break;

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
        c42_dlist_init(&w->wfl);

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
        w->on = O70X__COUNT;
        w->ffx = 0;
        c42_upa_set(w->nfx + w->on, 0, w->om - w->on);

        w->mn = 0;
        w->mm = O70CFG_INIT_MOD_NUM;
        mae = C42_MA_ARRAY_ALLOC(&w->ma, w->mod, w->mm);
        if (mae) { r = mae == C42_MA_CORRUPT ? O70S_BUG : O70S_NO_MEM; break; }
        w->mod[0] = &w->mcore;

        c42_rbtree_init(&w->ics.rbt, ics_key_cmp, w);

        w->ohdr[O70X_NULL] = &w->null_obj.ohdr;
        w->null_obj.ohdr.nref = 1;
        w->null_obj.ohdr.class_ox = O70X_NULL_CLASS;

        w->ohdr[O70X_FALSE] = &w->false_obj.ohdr;
        w->false_obj.ohdr.nref = 1;
        w->false_obj.ohdr.class_ox = O70X_BOOL_CLASS;

        w->ohdr[O70X_TRUE] = &w->true_obj.ohdr;
        w->true_obj.ohdr.nref = 1;
        w->true_obj.ohdr.class_ox = O70X_BOOL_CLASS;

        w->ohdr[O70X_NULL_CLASS] = &w->null_class.ohdr;
        w->null_class.ohdr.nref = 1;
        w->null_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->null_class.isize = sizeof(o70_object_t);
        w->null_class.model = 0;
        w->null_class.name = O70_XTOR(O70X_NULL_ICTSTR);

        w->ohdr[O70X_BOOL_CLASS] = &w->bool_class.ohdr;
        w->bool_class.ohdr.nref = 1;
        w->bool_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->bool_class.isize = sizeof(o70_object_t);
        w->bool_class.model = 0;
        w->bool_class.name = O70_XTOR(O70X_BOOL_ICTSTR);

        w->ohdr[O70X_INT_CLASS] = &w->int_class.ohdr;
        w->int_class.ohdr.nref = 1;
        w->int_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->int_class.isize = sizeof(o70_object_t);
        w->int_class.model = 0;
        w->int_class.name = O70_XTOR(O70X_INT_ICTSTR);

        w->ohdr[O70X_DYNOBJ_CLASS] = &w->dynobj_class.ohdr;
        w->dynobj_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->dynobj_class.ohdr.nref = 1;
        w->dynobj_class.isize = sizeof(o70_dynobj_t);
        w->dynobj_class.model = O70M_DYNOBJ;
        w->dynobj_class.finish = dynobj_finish;
        w->dynobj_class.name = O70_XTOR(O70X_DYNOBJ_ICTSTR);

        w->ohdr[O70X_CLASS_CLASS] = &w->class_class.ohdr;
        w->class_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->class_class.ohdr.nref = 1;
        w->class_class.isize = sizeof(o70_class_t);
        w->class_class.model = O70M_DYNOBJ | O70M_CLASS;
        w->class_class.name = O70_XTOR(O70X_CLASS_ICTSTR);

        w->ohdr[O70X_ARRAY_CLASS] = &w->array_class.ohdr;
        w->array_class.ohdr.nref = 1;
        w->array_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->array_class.isize = sizeof(o70_array_t);
        w->array_class.model = O70M_ARRAY;
        w->array_class.name = O70_XTOR(O70X_ARRAY_ICTSTR);

        w->ohdr[O70X_FUNCTION_CLASS] = &w->function_class.ohdr;
        w->function_class.ohdr.nref = 1;
        w->function_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->function_class.isize = sizeof(o70_function_t);
        w->function_class.model = O70M_FUNCTION;
        w->function_class.name = O70_XTOR(O70X_FUNCTION_ICTSTR);
        w->function_class.finish = func_finish;

        w->ohdr[O70X_IFUNC_CLASS] = &w->ifunc_class.ohdr;
        w->ifunc_class.ohdr.nref = 1;
        w->ifunc_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->ifunc_class.isize = sizeof(o70_ifunc_t);
        w->ifunc_class.model = O70M_FUNCTION | O70M_IFUNC;
        w->ifunc_class.name = O70_XTOR(O70X_IFUNC_ICTSTR);
        w->ifunc_class.finish = ifunc_finish;

        w->ohdr[O70X_STR_CLASS] = &w->str_class.ohdr;
        w->str_class.ohdr.nref = 1;
        w->str_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->str_class.isize = sizeof(o70_str_t);
        w->str_class.model = O70M_STR;
        w->str_class.finish = str_finish;
        w->str_class.name = O70_XTOR(O70X_STR_ICTSTR);

        w->ohdr[O70X_CTSTR_CLASS] = &w->ctstr_class.ohdr;
        w->ctstr_class.ohdr.nref = 1;
        w->ctstr_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->ctstr_class.isize = sizeof(o70_ctstr_t);
        w->ctstr_class.model = O70M_SCTSTR;
        w->ctstr_class.name = O70_XTOR(O70X_CTSTR_ICTSTR);

        w->ohdr[O70X_ACTSTR_CLASS] = &w->actstr_class.ohdr;
        w->actstr_class.ohdr.nref = 1;
        w->actstr_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->actstr_class.isize = sizeof(o70_ctstr_t);
        w->actstr_class.model = O70M_ACTSTR;
        w->actstr_class.name = O70_XTOR(O70X_ACTSTR_ICTSTR);

        w->ohdr[O70X_ICTSTR_CLASS] = &w->ictstr_class.ohdr;
        w->ictstr_class.ohdr.nref = 1;
        w->ictstr_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->ictstr_class.isize = sizeof(o70_ctstr_t);
        w->ictstr_class.model = O70M_SCTSTR | O70M_ICTSTR;
        w->ictstr_class.finish = ictstr_finish;
        w->ictstr_class.name = O70_XTOR(O70X_ICTSTR_ICTSTR);

        w->ohdr[O70X_IACTSTR_CLASS] = &w->iactstr_class.ohdr;
        w->iactstr_class.ohdr.nref = 1;
        w->iactstr_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->iactstr_class.isize = sizeof(o70_ctstr_t);
        w->iactstr_class.model = O70M_ACTSTR | O70M_ICTSTR;
        w->iactstr_class.name = O70_XTOR(O70X_IACTSTR_ICTSTR);

        w->ohdr[O70X_EXCEPTION_CLASS] = &w->exception_class.ohdr;
        w->exception_class.ohdr.nref = 1;
        w->exception_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->exception_class.isize = sizeof(o70_exception_t);
        w->exception_class.model = O70M_EXCEPTION;
        w->exception_class.name = O70_XTOR(O70X_EXCEPTION_ICTSTR);

        w->ohdr[O70X_MODULE_CLASS] = &w->module_class.ohdr;
        w->module_class.ohdr.nref = 1;
        w->module_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->module_class.isize = sizeof(o70_module_t);
        w->module_class.model = O70M_MODULE;
        w->module_class.name = O70_XTOR(O70X_MODULE_ICTSTR);

        w->ohdr[O70X_NULL_ICTSTR] = &w->null_ictstr.ohdr;
        w->null_ictstr.ohdr.nref = 1;
        w->null_ictstr.ohdr.class_ox = O70X_ICTSTR_CLASS;
        I(O70X_NULL_ICTSTR, null_ictstr, "null");

        w->ohdr[O70X_FALSE_ICTSTR] = &w->false_ictstr.ohdr;
        w->false_ictstr.ohdr.nref = 1;
        w->false_ictstr.ohdr.class_ox = O70X_ICTSTR_CLASS;
        I(O70X_FALSE_ICTSTR, false_ictstr, "false");

        w->ohdr[O70X_TRUE_ICTSTR] = &w->true_ictstr.ohdr;
        w->true_ictstr.ohdr.nref = 1;
        w->true_ictstr.ohdr.class_ox = O70X_ICTSTR_CLASS;
        I(O70X_TRUE_ICTSTR, true_ictstr, "true");

        w->ohdr[O70X_NULL_CLASS_ICTSTR] = &w->null_class_ictstr.ohdr;
        w->null_class_ictstr.ohdr.nref = 1;
        w->null_class_ictstr.ohdr.class_ox = O70X_ICTSTR_CLASS;
        I(O70X_NULL_CLASS_ICTSTR, null_class_ictstr, "null_class");

        w->ohdr[O70X_BOOL_ICTSTR] = &w->bool_ictstr.ohdr;
        w->bool_ictstr.ohdr.nref = 1;
        w->bool_ictstr.ohdr.class_ox = O70X_ICTSTR_CLASS;
        I(O70X_BOOL_ICTSTR, bool_ictstr, "bool");

        w->ohdr[O70X_INT_ICTSTR] = &w->int_ictstr.ohdr;
        w->int_ictstr.ohdr.nref = 1;
        w->int_ictstr.ohdr.class_ox = O70X_ICTSTR_CLASS;
        I(O70X_INT_ICTSTR, int_ictstr, "int");

        w->ohdr[O70X_DYNOBJ_ICTSTR] = &w->dynobj_ictstr.ohdr;
        w->dynobj_ictstr.ohdr.nref = 1;
        w->dynobj_ictstr.ohdr.class_ox = O70X_ICTSTR_CLASS;
        I(O70X_DYNOBJ_ICTSTR, dynobj_ictstr, "dynobj");

        w->ohdr[O70X_CLASS_ICTSTR] = &w->class_ictstr.ohdr;
        w->class_ictstr.ohdr.nref = 1;
        w->class_ictstr.ohdr.class_ox = O70X_ICTSTR_CLASS;
        I(O70X_CLASS_ICTSTR, class_ictstr, "class");

        w->ohdr[O70X_ARRAY_ICTSTR] = &w->array_ictstr.ohdr;
        w->array_ictstr.ohdr.nref = 1;
        w->array_ictstr.ohdr.class_ox = O70X_ICTSTR_CLASS;
        I(O70X_ARRAY_ICTSTR, array_ictstr, "array");

        w->ohdr[O70X_FUNCTION_ICTSTR] = &w->function_ictstr.ohdr;
        w->function_ictstr.ohdr.nref = 1;
        w->function_ictstr.ohdr.class_ox = O70X_ICTSTR_CLASS;
        I(O70X_FUNCTION_ICTSTR, function_ictstr, "function");

        w->ohdr[O70X_IFUNC_ICTSTR] = &w->ifunc_ictstr.ohdr;
        w->ifunc_ictstr.ohdr.nref = 1;
        w->ifunc_ictstr.ohdr.class_ox = O70X_ICTSTR_CLASS;
        I(O70X_IFUNC_ICTSTR, ifunc_ictstr, "interpreted_function");

        w->ohdr[O70X_STR_ICTSTR] = &w->str_ictstr.ohdr;
        w->str_ictstr.ohdr.nref = 1;
        w->str_ictstr.ohdr.class_ox = O70X_ICTSTR_CLASS;
        I(O70X_STR_ICTSTR, str_ictstr, "str");

        w->ohdr[O70X_CTSTR_ICTSTR] = &w->ctstr_ictstr.ohdr;
        w->ctstr_ictstr.ohdr.nref = 1;
        w->ctstr_ictstr.ohdr.class_ox = O70X_ICTSTR_CLASS;
        I(O70X_CTSTR_ICTSTR, ctstr_ictstr, "ctstr");

        w->ohdr[O70X_ACTSTR_ICTSTR] = &w->actstr_ictstr.ohdr;
        w->actstr_ictstr.ohdr.nref = 1;
        w->actstr_ictstr.ohdr.class_ox = O70X_ICTSTR_CLASS;
        I(O70X_ACTSTR_ICTSTR, actstr_ictstr, "actstr");

        w->ohdr[O70X_ICTSTR_ICTSTR] = &w->ictstr_ictstr.ohdr;
        w->ictstr_ictstr.ohdr.nref = 1;
        w->ictstr_ictstr.ohdr.class_ox = O70X_ICTSTR_CLASS;
        I(O70X_ICTSTR_ICTSTR, ictstr_ictstr, "ictstr");

        w->ohdr[O70X_IACTSTR_ICTSTR] = &w->iactstr_ictstr.ohdr;
        w->iactstr_ictstr.ohdr.nref = 1;
        w->iactstr_ictstr.ohdr.class_ox = O70X_ICTSTR_CLASS;
        I(O70X_IACTSTR_ICTSTR, iactstr_ictstr, "iactstr");

        w->ohdr[O70X_EXCEPTION_ICTSTR] = &w->exception_ictstr.ohdr;
        w->exception_ictstr.ohdr.nref = 1;
        w->exception_ictstr.ohdr.class_ox = O70X_ICTSTR_CLASS;
        I(O70X_EXCEPTION_ICTSTR, exception_ictstr, "exception");

        w->ohdr[O70X_MODULE_ICTSTR] = &w->module_ictstr.ohdr;
        w->module_ictstr.ohdr.nref = 1;
        w->module_ictstr.ohdr.class_ox = O70X_ICTSTR_CLASS;
        I(O70X_MODULE_ICTSTR, module_ictstr, "module");

        w->empty_ehc.fehx = 0;
        w->empty_ehc.ehn = 0;

        r = 0;
    }
    while (0);

    if (r) w->aux_status = o70_world_finish(w);

    return r;
#undef I
}

/* o70_world_finish *********************************************************/
O70_API o70_status_t C42_CALL o70_world_finish
(
    o70_world_t * w
)
{
    uint_fast8_t mae;
    o70_status_t rs = 0;
    size_t i;//, j;

    L("o70_world_finish: starting\n");
    while (w->wfl.next != &w->wfl)
    {
        o70_flow_t * flow;
        o70_status_t os;
        flow = C42_STRUCT_FROM_FIELD_PTR(o70_flow_t, wfl, w->wfl.next);
        L("kill flow $xp\n", flow);
        os = o70_flow_destroy(flow);
        if (os) return os;
    }

    if (w->om)
    {
        w->fdx = 0;
        for (i = 0; i < w->on; ++i)
            if (!(w->nfx[i] & 1)) // (w->ot[i])
            {
                w->ohdr[i]->ndx = ~w->fdx;
                w->fdx = i;
            }
        rs = _o70_obj_destroy(w);
        if (rs) return rs;

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

    if (w->mactx.ts)
    {
        L("*** BUG *** memory leaks: $z bytes in $z blocks.\n",
          w->mactx.ts, w->mactx.nb);
        rs = O70S_BUG;
    }

    L("o70_world_finish: done with status $s=$b\n", o70_status_name(rs), rs);
    return rs;
}

/* ox_alloc *****************************************************************/
static o70_status_t C42_CALL ox_alloc
(
    o70_world_t * restrict w,
    o70_oidx_t * restrict ox
)
{
    uint_fast8_t mae;
    /* if there's a free index in the chain, return it and update the head */
    if (w->ffx)
    {
        w->ffx = w->nfx[*ox = w->ffx] >> 1;
        return 0;
    }
    /* if all object entries are used then double the table */
    if (w->on == w->om)
    {
        mae = C42_MA_ARRAY_DOUBLE(&w->ma, w->ot, w->om);
        if (mae)
        {
            L("ox_alloc: ma double error: $b\n", mae);
            return mae == C42_MA_CORRUPT ? O70S_BUG : O70S_NO_MEM;
        }
        w->om <<= 1;
    }
    /* return the first unused index */
    *ox = w->on++;
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

    O70_ASSERT(w, clox < w->on);
    O70_ASSERT(w, !(w->nfx[O70_RTOX(clox)] & 1));
    os = ox_alloc(w, out);
    if (os) return os;
    ox = *out;
    cls = w->ot[clox];
    mae = c42_ma_alloc(&w->ma, w->ot + ox, 1, cls->isize);
    L("obj_alloc: isize=$xz, mae=$i, ox=$xi, r=$xi\n",
      cls->isize, mae, ox, O70_XTOR(ox));
    if (mae) { return mae == C42_MA_CORRUPT ? O70S_BUG : O70S_NO_MEM; }
    oh = w->ohdr[ox];
    oh->nref = 1;
    oh->class_ox = clox;
    o70_ref_inc(w, O70_RTOX(clox));
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

    /* loop for calling finish() callbacks on objects to be destroyed;
     * while executing these destructors other objects may be queued up in
     * this chain of objects to be destroyed */
    fmx = 0;
    while ((dx = w->fdx))
    {
        //L("obj_destroy: finishing ox=$xd\n", dx);
        w->fdx = ~(oh = w->ohdr[dx])->ndx;
        //L("dx=$xd, oh: $xp; count: $xd\n", dx, oh, O70X__COUNT);
        //L("oh->class_ox: $xd\n", oh->class_ox);
        c = w->ot[oh->class_ox];
        //L("c: $xp\n", c);
        if (c->finish)
        {
            //L("calling finish...\n");
            st = c->finish(w, O70_XTOR(dx));
            if (st)
            {
                // L("obj_destroy: r$Xd.finish = $s = $b\n",
                //   O70_XTOR(dx), o70_status_name(st), st);
                return st;
            }
        }
        /* queue up the object for deallocation (only if it is not one of the
         * static objects hardcoded in the world) */
        if (dx >= O70X__COUNT)
        {
            // L("queuing up for freeing\n");
            oh->ndx = ~fmx;
            fmx = dx;
        }
        else
        {
            //L("no queuing\n");
        }
    }
    //L("---\n");
    /* free the object bodies - they're just useless shells at this stage;
     * their corpses are kept in the chain so that other objects being destroyed
     * can do o70_ref_dec() on objects already finished - this situation
     * happens only at the end of the world or when garbage collection finds
     * floating object cycles */
    for (dx = fmx; dx; dx = fmx)
    {
        //L("obj_destroy: freeing ox=$xd\n", dx);
        c = w->ot[(oh = w->ohdr[dx])->class_ox];
        w->nfx[dx] = (w->ffx << 1) | 1;
        w->ffx = dx;
        fmx = ~oh->ndx;
        mae = c42_ma_free(&w->ma, oh, 1, c->isize);
        if (mae)
        {
            //L("obj_destroy: ma_free error $b\n", mae);
            return O70S_BUG;
        }
    }
    //L("_o70_obj_destroy done\n");
    return 0;
}

/* o70_flow_create **********************************************************/
O70_API o70_status_t C42_CALL o70_flow_create
(
    o70_world_t * w,
    o70_flow_t * * flow_ptr,
    unsigned int max_depth
)
{
    o70_flow_t * flow;
    size_t size;
    int mae;
    if (max_depth > 0x10000) return O70S_BAD_ARG;
    size = sizeof(o70_flow_t) + max_depth * sizeof(o70_ref_t);
    mae = c42_ma_alloc(&w->ma, (void * *) flow_ptr, 1, size);
    if (mae) return mae == C42_MA_CORRUPT ? O70S_BUG : O70S_NO_MEM;
    flow = *flow_ptr;
    flow->world = w;
    flow->exc = O70R_NULL;
    flow->m = max_depth;
    flow->n = 0;
    flow->stack = (o70_ref_t *) (flow + 1);
    C42_DLIST_APPEND(w->wfl, flow, wfl);
    L("created flow $xp, wfl=$xp\n", flow, &flow->wfl);
    return 0;
}

/* o70_flow_destroy *********************************************************/
O70_API o70_status_t C42_CALL o70_flow_destroy (o70_flow_t * flow)
{
    o70_world_t * w = flow->world;
    size_t size;
    unsigned int i, mae;
    L("destroying flow $xp...\n", flow);

    c42_dlist_del(&flow->wfl);
    size = sizeof(o70_flow_t) + sizeof(o70_ref_t) * flow->m;
    O70_ASSERT(w, flow->stack);
    for (i = 0; i < flow->n; ++i)
    {
        o70_status_t os;
        os = o70_ref_dec(w, flow->stack[i]);
        if (os) return os;
    }
    mae = c42_ma_free(&w->ma, flow, 1, size);
    if (mae) return O70S_BUG;
    return 0;
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
        if (e)
        {
            L("o70_dump_icst: print failed: $b\n", e);
            return O70S_IO_ERROR;
        }
    }
    return 0;
}

/* o70_ctstr *********************************************************/
O70_API o70_status_t C42_CALL o70_ctstr
(
    o70_world_t * w,
    o70_ref_t * out,
    void const * ptr,
    size_t len
)
{
    o70_status_t os;
    o70_oidx_t ox;
    o70_ctstr_t * cs;

    os = obj_alloc(w, &ox, O70X_CTSTR_CLASS);
    if (os) return os;
    cs = w->ot[ox];
    cs->data.a = (uint8_t *) ptr;
    cs->data.n = len;
    *out = O70_XTOR(ox);
    return 0;
}

/* o70_ictstr **************************************************/
O70_API o70_status_t C42_CALL o70_ictstr
(
    o70_world_t * w,
    o70_ref_t * out,
    void const * ptr,
    size_t len
)
{
    c42_rbtree_path_t path;
    o70_prop_node_t * pn;
    c42_u8an_t ba;
    uint_fast8_t rbte;
    o70_status_t os, os2;
    o70_ref_t r;

    /* init a byte array to use it as lookup key in the tree of internalised
     * constant strings */
    ba.a = (void *) ptr;
    ba.n = len;

    /* now do the lookup */
    rbte = c42_rbtree_find(&path, &w->ics.rbt, (uintptr_t) &ba);
    if (rbte == C42_RBTREE_FOUND)
    {
        /* found the node; the key of the node is the reference to our ictstr */
        pn = (o70_prop_node_t *) path.nodes[path.last];
        *out = r = pn->kv.key;
        o70_ref_inc(w, r);
        return 0;
    }
    /* ctstr not found, must create one */
    os = o70_ctstr(w, out, ptr, len);
    if (os) return os;
    /* create the node in the internalised ctstr tree */
    os = ics_node_create(w, &path, r = *out);
    if (os)
    {
        /* failed to create the node */
        if (os == O70S_BUG) return os;
        /* on non-buggy state, free the ctstr */
        os2 = o70_ref_dec(w, r);
        if (os2 == O70S_BUG) return os2;
        return os;
    }
    /* replace the class */
    o70_ref_dec(w, O70_XTOR(O70X_CTSTR_CLASS));
    o70_ref_inc(w, O70_XTOR(O70X_ICTSTR_CLASS));
    w->ohdr[O70_RTOX(r)]->class_ox = O70X_ICTSTR_CLASS;

    return 0;
}

/* o70_ctstr_intern *********************************************************/
O70_API o70_status_t C42_CALL o70_ctstr_intern
(
    o70_world_t * w,
    o70_ref_t in,
    o70_ref_t * out
)
{
    c42_rbtree_path_t path;
    o70_prop_node_t * pn;
    uint_fast8_t rbte;
    o70_status_t os;
    o70_ref_t r, ncx;
    o70_ctstr_t * ics;
    o70_class_t * cls;

    if (!O70_IS_OREF(in)) return O70S_BAD_TYPE;
    ics = w->ot[O70_RTOX(in)];
    cls = w->ot[ics->ohdr.class_ox];
    if ((cls->model & (O70M_SCTSTR | O70M_ACTSTR)) == 0) return O70S_BAD_TYPE;

    if ((cls->model & O70M_ICTSTR)) r = in;
    else
    {
        rbte = c42_rbtree_find(&path, &w->ics.rbt, (uintptr_t) &ics->data);
        if (rbte == C42_RBTREE_FOUND)
        {
            /* found the node; the key of the node is the reference
             * to our ictstr */
            pn = (o70_prop_node_t *) path.nodes[path.last];
            r = pn->kv.key;
        }
        else
        {
            /* we must add current ctstr to the ics tree and change it to
             * ictstr/iactstr */
            r = in;
            /* create the node in the internalised ctstr tree */
            os = ics_node_create(w, &path, in);
            if (os) return os;
            if (ics->ohdr.class_ox == O70X_CTSTR_CLASS)
                ncx = O70X_ICTSTR_CLASS;
            else if (ics->ohdr.class_ox == O70X_ACTSTR_CLASS)
                ncx = O70X_IACTSTR_CLASS;
            else return O70S_TODO; /* in case we will have other types of ctstr derivates */
            o70_ref_dec(w, O70_XTOR(ics->ohdr.class_ox));
            o70_ref_inc(w, O70_XTOR(ncx));
            ics->ohdr.class_ox = ncx;
        }
    }

    *out = r;
    o70_ref_inc(w, r);
    return 0;
}

/* prop_key_cmp *************************************************************/
static uint_fast8_t C42_CALL prop_key_cmp
(
    uintptr_t key,
    c42_rbtree_node_t * node,
    void * restrict context
)
{
    o70_prop_node_t const * pn = (o70_prop_node_t const *) node;
    (void) context;
    if ((uint32_t) key == pn->kv.key) return C42_RBTREE_EQUAL;
    return ((uint32_t) key > pn->kv.key) ? C42_RBTREE_MORE : C42_RBTREE_LESS;
}

/* prop_bag_init ************************************************************/
static void C42_CALL prop_bag_init
(
    o70_prop_bag_t * bag
)
{
    c42_rbtree_init(&bag->rbt, prop_key_cmp, NULL);
}

/* prop_bag_finish **********************************************************/
static o70_status_t C42_CALL prop_bag_finish
(
    o70_world_t * w,
    o70_prop_bag_t * bag,
    int deref
)
{
    c42_rbtree_path_t path;
    c42_rbtree_node_t * rbtn;
    o70_prop_node_t * n;
    o70_status_t os;
    uint_fast8_t mae;

    rbtn = c42_rbtree_first(&path, &bag->rbt);
    while (rbtn)
    {
        n = (o70_prop_node_t *) rbtn;
        rbtn = c42_rbtree_np(&path, C42_RBTREE_MORE);
        if (deref)
        {
            os = o70_ref_dec(w, n->kv.key);
            if (os) return os;
            os = o70_ref_dec(w, n->kv.val);
            if (os) return os;
        }
        mae = C42_MA_VAR_FREE(&w->ma, n);
        if (mae) return O70S_BUG;
    }
    return 0;
}


/* prop_bag_peek ************************************************************/
static o70_status_t C42_CALL prop_bag_peek
(
    o70_prop_bag_t * bag,
    o70_ref_t name,
    o70_ref_t * value
)
{
    c42_rbtree_path_t path;
    uint_fast8_t rbte;
    o70_prop_node_t * n;
    rbte = c42_rbtree_find(&path, &bag->rbt, (uintptr_t) name);
#if _DEBUG
    if (!(rbte == C42_RBTREE_FOUND || rbte == C42_RBTREE_NOT_FOUND))
        return O70S_BUG;
#endif
    if (rbte == C42_RBTREE_NOT_FOUND) return O70S_MISSING;
    n = (o70_prop_node_t *) path.nodes[path.last];
#if _DEBUG
    if (n->kv.key != name) return O70S_BUG;
#endif
    *value = n->kv.val;
    return 0;
}

/* prop_bag_put *************************************************************/
static o70_status_t C42_CALL prop_bag_put
(
    o70_world_t * w,
    o70_prop_bag_t * bag,
    o70_ref_t name,
    o70_ref_t value
)
{
    c42_rbtree_path_t path;
    uint_fast8_t rbte;
    o70_prop_node_t * n;
    o70_status_t os;

    rbte = c42_rbtree_find(&path, &bag->rbt, (uintptr_t) name);
#if _DEBUG
    if (!(rbte == C42_RBTREE_FOUND || rbte == C42_RBTREE_NOT_FOUND))
        return O70S_BUG;
#endif
    if (rbte == C42_RBTREE_FOUND)
    {
        n = (o70_prop_node_t *) path.nodes[path.last];
        os = o70_ref_dec(w, n->kv.val);
        if (os) return os;
    }
    else
    {
        /* alloc one cell */
        int mae;
        mae = C42_MA_VAR_ALLOC(&w->ma, n);
        if (mae) return mae == C42_MA_CORRUPT ? O70S_BUG : O70S_NO_MEM;
        A(n);
        n->kv.key = name;
        c42_rbtree_insert(&path, &n->rbtn);
        o70_ref_inc(w, name);
    }
    n->kv.val = value;
    o70_ref_inc(w, value);
    return 0;
}

/* o70_dynobj_create ********************************************************/
O70_API o70_status_t C42_CALL o70_dynobj_create
(
    o70_world_t * w,
    o70_ref_t * out
)
{
    o70_status_t os;
    o70_oidx_t ox;
    o70_dynobj_t * obj;
    os = obj_alloc(w, &ox, O70X_DYNOBJ_CLASS);
    if (os)
    {
        L("o70_obj_create: obj_alloc failed: $s = $xd\n",
          o70_status_name(os), os);
        return os;
    }
    obj = w->ot[ox];
    prop_bag_init(&obj->fields);
    *out = O70_XTOR(ox);
    return 0;
}

/* o70_dynobj_raw_get *******************************************************/
O70_API o70_status_t C42_CALL o70_dynobj_raw_get
(
    o70_world_t * w,
    o70_ref_t obj,
    o70_ref_t name,
    o70_ref_t * value
)
{
    o70_dynobj_t * o;
    o70_class_t * c;
    o70_status_t os;
    if (!O70_IS_OREF(obj)) return O70S_BAD_TYPE;
    o = w->ot[O70_RTOX(obj)];
    c = w->ot[o->ohdr.class_ox];
    if (!(c->model & O70M_DYNOBJ)) return O70S_BAD_TYPE;
    os = prop_bag_peek(&o->fields, name, value);
    if (os) return os;
    o70_ref_inc(w, *value);
    return 0;
}

/* o70_dynobj_raw_put *******************************************************/
O70_API o70_status_t C42_CALL o70_dynobj_raw_put
(
    o70_world_t * w,
    o70_ref_t obj,
    o70_ref_t name,
    o70_ref_t value
)
{
    o70_dynobj_t * o;
    o70_status_t os, osd;
    o70_ref_t prop;

    if (!(o70_model(w, obj) & O70M_DYNOBJ)) return O70S_BAD_TYPE;
    o = w->ot[O70_RTOX(obj)];
    os = o70_ctstr_intern(w, name, &prop);
    if (os) return os;
    os = prop_bag_put(w, &o->fields, prop, value);
    osd = o70_ref_dec(w, prop);
    if (osd == O70S_BUG) return O70S_BUG;
    return os ? os : osd;
}

/* dynobj_finish ************************************************************/
static o70_status_t C42_CALL dynobj_finish
(
    o70_world_t * w,
    o70_ref_t r
)
{
    o70_dynobj_t * o;
    o70_status_t os;
    //L("dynobj_finish\n");
    if (!(o70_model(w, r) & O70M_DYNOBJ)) return O70S_BUG;
    o = w->ot[O70_RTOX(r)];
    os = prop_bag_finish(w, &o->fields, 1);
    return os;
}

/* ictstr_finish ************************************************************/
static o70_status_t C42_CALL ictstr_finish
(
    o70_world_t * w,
    o70_ref_t r
)
{
    o70_status_t os;
    //L("ictstr_finish: ref=$xd\n", r);
    if (!(o70_model(w, r) & O70M_ICTSTR)) return O70S_BUG;
    os = ics_node_delete(w, r);
    if (os)
    {
        L("*** BUG *** ictstr_finish: ics_node_delete(ref=$xd) = $s = $b\n",
          r, o70_status_name(os), os);
        return O70S_BUG;
    }
    //L("ictstr_finish: ref=$xd -> done\n", r);
    return 0;
}

/* o70_ctstr_len ************************************************************/
O70_API size_t C42_CALL o70_ctstr_len
(
    o70_world_t * w,
    o70_ref_t r
)
{
    o70_str_t * s;
    if (!(o70_model(w, r) & O70M_ANY_CTSTR)) return 0;
    s = w->ot[O70_RTOX(r)];
    return s->data.n;
}

/* o70_ctstr_data ***********************************************************/
O70_API uint8_t * C42_CALL o70_ctstr_data
(
    o70_world_t * w,
    o70_ref_t r
)
{
    o70_str_t * s;
    if (!(o70_model(w, r) & O70M_ANY_CTSTR)) return NULL;
    s = w->ot[O70_RTOX(r)];
    return s->data.a;
}

/* o70_str_create ***********************************************************/
O70_API o70_status_t C42_CALL o70_str_create
(
    o70_world_t * w,
    o70_ref_t * obj_p
)
{
    o70_status_t os;
    o70_oidx_t ox;
    o70_str_t * str;
    os = obj_alloc(w, &ox, O70X_STR_CLASS);
    if (os)
    {
        L("o70_str_create: obj_alloc failed: $s = $xd\n",
          o70_status_name(os), os);
        return os;
    }
    str = w->ot[ox];
    *obj_p = O70_XTOR(ox);
    str->data.a = NULL;
    str->data.n = 0;
    str->asize = 0;
    return 0;
}

struct str_vafmt_writer_s
{
    o70_world_t * w;
    o70_str_t * s;
};

/* str_vafmt_writer *********************************************************/
static size_t C42_CALL str_vafmt_writer
(
    uint8_t const * data,
    size_t len,
    void * ctx
)
{
    struct str_vafmt_writer_s * wctx = ctx;
    o70_world_t * w = wctx->w;
    o70_str_t * s = wctx->s;
    uint_fast8_t mae;
    size_t alen, nlen;

L("vafmt_writer: len=$xz\n", len);
    if (s->asize - s->data.n < len)
    {
        nlen = s->data.n + len;
        if (nlen < len || (ptrdiff_t) nlen < 0) return 0;
        if (s->asize)
        {
            for (alen = s->asize; alen < nlen; alen <<= 1);
            mae = c42_ma_realloc(&w->ma, (void * *) &s->data.a, 1,
                                 s->asize, alen);
            if (mae) return 0;
            s->asize = alen;
        }
        else
        {
            for (alen = O70CFG_INIT_STR_SIZE; alen < nlen; alen <<= 1);
            L("vafmt_writer: alen=$xz\n", alen);
            mae = C42_MA_ARRAY_ALLOC(&w->ma, s->data.a, alen);
            if (mae) return 0;
            s->asize = alen;
        }
    }
    c42_u8a_copy(s->data.a + s->data.n, data, len);
    s->data.n += len;
    return len;
}

/* o70_str_vafmt ************************************************************/
O70_API o70_status_t C42_CALL o70_str_vafmt
(
    o70_world_t * w,
    o70_ref_t r,
    char const * fmt,
    va_list va
)
{
    struct str_vafmt_writer_s wctx;
    uint_fast8_t fmte;

    if (!(o70_model(w, r) & O70M_STR)) return O70S_BAD_TYPE;
    wctx.w = w;
    wctx.s = w->ot[O70_RTOX(r)];
    fmte = c42_write_vfmt(str_vafmt_writer, &wctx, c42_utf8_term_width, NULL,
                          fmt, va);
    L("o70_str_vafmt: fmte=$b, s.len=$z\n", fmte, wctx.s->data.n);
    switch (fmte)
    {
    case 0:
        return 0;
    case C42_FMT_MALFORMED:
        return O70S_BAD_FMT;
    case C42_FMT_WIDTH_ERROR:
        return O70S_BAD_UTF8;
    case C42_FMT_WRITE_ERROR:
        return O70S_NO_MEM;
    case C42_FMT_CONV_ERROR:
        return O70S_CONV_ERROR;
    }
    return O70S_OTHER;
}

/* o70_str_afmt *************************************************************/
O70_API o70_status_t C42_CALL o70_str_afmt
(
    o70_world_t * w,
    o70_ref_t r,
    char const * fmt,
    ...
)
{
    va_list va;
    o70_status_t os;

    va_start(va, fmt);
    os = o70_str_vafmt(w, r, fmt, va);
    va_end(va);
    return os;
}

/* str_finish ***************************************************************/
static o70_status_t C42_CALL str_finish
(
    o70_world_t * w,
    o70_ref_t r
)
{
    o70_str_t * s;
    //L("str_finish\n");
    if (!(o70_model(w, r) & O70M_STR)) return O70S_BUG;
    s = w->ot[O70_RTOX(r)];
    if (s->asize)
    {
        if (C42_MA_ARRAY_FREE(&w->ma, s->data.a, s->asize))
            return O70S_BUG;
    }

    return 0;
}

/* o70_str_len **************************************************************/
O70_API size_t C42_CALL o70_str_len
(
    o70_world_t * w,
    o70_ref_t r
)
{
    o70_str_t * s;
    if (!(o70_model(w, r) & O70M_STR)) return 0;
    s = w->ot[O70_RTOX(r)];
    return s->data.n;
}

/* o70_str_data *************************************************************/
O70_API uint8_t * C42_CALL o70_str_data
(
    o70_world_t * w,
    o70_ref_t r
)
{
    o70_str_t * s;
    if (!(o70_model(w, r) & O70M_STR)) return NULL;
    s = w->ot[O70_RTOX(r)];
    return s->data.a;
}

/* o70_str_asd **************************************************************/
O70_API o70_status_t C42_CALL o70_str_append_obj_short_desc
(
    o70_world_t * w,
    o70_ref_t str,
    o70_ref_t obj
)
{
    o70_status_t os;
    if (O70_IS_FASTINT(obj))
    {
        os = o70_str_afmt(w, str, "$D", O70_RTOFI(obj));
    }
    else
    {
        os = o70_str_afmt(w, str, "r$d", obj);
    }
    return os;
}

/* o70_short_desc ***********************************************************/
O70_API o70_status_t C42_CALL o70_obj_short_desc
(
    o70_world_t * w,
    o70_ref_t obj,
    o70_ref_t * out
)
{
    o70_status_t os;
    os = o70_str_create(w, out);
    if (!os)
    {
        os = o70_str_append_obj_short_desc(w, *out, obj);
        if (os && os != O70S_BUG)
        {
            o70_status_t rdos = o70_ref_dec(w, *out);
            if (rdos) return O70S_BUG;
        }
    }
    return os;
}

/* o70_dump_object_map ******************************************************/
O70_API o70_status_t C42_CALL o70_dump_object_map
(
    o70_world_t * w,
    c42_io8_t * io
)
{
    o70_oidx_t i;
    if (c42_io8_fmt(io, "object map [world $xp]:\n", w))
        return O70S_IO_ERROR;
    for (i = 0; i < w->on; ++i)
    {
        o70_ref_t oref, class_name;
        int model;
        if ((w->nfx[i] & 1)) continue;
        oref = O70_XTOR(i);
        class_name = o70_obj_class_name(w, oref);
        if (c42_io8_fmt(io, "- <$.*es:$04Xd rc=$d",
                        o70_ctstr_len(w, class_name),
                        o70_ctstr_data(w, class_name),
                        O70_XTOR(i), w->ohdr[i]->nref))
            return O70S_IO_ERROR;
        model = o70_model(w, oref);
        if ((model & O70M_ANY_CTSTR))
        {
            if (c42_io8_fmt(io, " data=\"$.*es\"", o70_ctstr_len(w, oref),
                            o70_ctstr_data(w, oref)))
                return O70S_IO_ERROR;
        }
        else if ((model & O70M_STR))
        {
            o70_str_t * s = w->ot[i];
            if (c42_io8_fmt(io, " data=\"$.*es\" asize=$xz",
                            s->data.n, s->data.a, s->asize))
                return O70S_IO_ERROR;
        }
        else if ((model & O70M_CLASS))
        {
            o70_class_t * c = w->ot[i];
            if (c42_io8_fmt(io, " name=\"$.*es\"",
                            o70_ctstr_len(w, c->name),
                            o70_ctstr_data(w, c->name)))
                return O70S_IO_ERROR;
        }
        else if ((model & O70M_DYNOBJ))
        {
            c42_rbtree_path_t path;
            c42_rbtree_node_t * n;
            o70_dynobj_t * o = w->ot[i];
            for (n = c42_rbtree_first(&path, &o->fields.rbt);
                 n;
                 n = c42_rbtree_np(&path, C42_RBTREE_MORE))
            {
                o70_prop_node_t * prop = (o70_prop_node_t *) n;
                o70_ref_t value_class_name;

                value_class_name = o70_obj_class_name(w, prop->kv.val);
                if (c42_io8_fmt(io, " $.*es=<$.*es:$04Xd/>",
                                o70_ctstr_len(w, prop->kv.key),
                                o70_ctstr_data(w, prop->kv.key),
                                o70_ctstr_len(w, value_class_name),
                                o70_ctstr_data(w, value_class_name),
                                prop->kv.val))
                    return O70S_IO_ERROR;
            }
        }
        if (c42_io8_fmt(io, "/>\n")) return O70S_IO_ERROR;
    }
    if (c42_io8_fmt(io, "end object map [world $xp]\n", w))
        return O70S_IO_ERROR;
    return 0;
}

/* ifunc_exectx_finish ******************************************************/
static o70_status_t C42_CALL ifunc_exectx_finish
(
    o70_world_t * w,
    o70_ref_t obj
)
{
    o70_ifunc_exectx_t * ie;
    o70_ifunc_t * ifunc;
    ie = o70_ptr(w, obj);
    ifunc = w->ot[ie->exectx.ohdr.class_ox];
    L("ifunc_exectx_finish: ec$xd\n", obj);
    O70_ASSERT(w, (o70_model(w, O70_XTOR(ie->exectx.ohdr.class_ox)) & O70M_IFUNC));
    O70_ASSERT(w, (o70_model(w, obj) & O70M_EXECTX));
    if (ie->exectx.lv) // this can be NULL if some error happens during the
        //creation of the context
    {
        unsigned int i;
        O70_ASSERT(w, ie->exectx.lv == &ie->lv[0]);
        for (i = 0; i < ifunc->func.sn; ++i)
        {
            o70_status_t os = o70_ref_dec(w, ie->lv[i]);
            if (os) return os;
        }
    }
    L("ifunc_exectx_finish done: ec$xd\n", obj);

    return 0;
}

/* ifunc_exectx_exec ********************************************************/
static o70_status_t C42_CALL ifunc_exectx_exec
(
    o70_flow_t * flow,
    o70_exectx_t * e
)
{
    o70_world_t * w = flow->world;
    (void) w;
    (void) flow;
    (void) e;
    L("ifunc_exectx_exec: todo\n");
    return O70S_TODO;
}

/* ifunc_exectx_init ********************************************************/
static o70_status_t C42_CALL ifunc_exectx_init
(
    o70_flow_t * flow,
    o70_exectx_t * e
)
{
    o70_ifunc_exectx_t * ie = (o70_ifunc_exectx_t *) e;
    (void) flow;
    e->lv = &ie->lv[0];
    ie->c = 0;
    return 0;
}

/* o70_ifunc_create *********************************************************/
O70_API o70_status_t C42_CALL o70_ifunc_create
(
    o70_world_t * w,
    o70_ref_t * out,
    size_t sn
)
{
    o70_ifunc_t * f;
    o70_oidx_t ox;
    o70_status_t os;

    if (sn > (PTRDIFF_MAX - sizeof(o70_exectx_t)) / sizeof(o70_ref_t))
    {
        L("ifunc_create: too many slots: $xz\n", sn);
        return O70S_BAD_ARG;
    }
    os = obj_alloc(w, &ox, O70X_IFUNC_CLASS);
    if (os) return os;
    *out = O70_XTOR(ox);
    f = w->ot[ox];

    prop_bag_init(&f->func.cls.fields);
    prop_bag_init(&f->func.cls.methods);
    f->func.cls.finish = ifunc_exectx_finish;
    f->func.cls.finish_context = NULL;
    f->func.cls.get_prop = NULL;
    f->func.cls.set_prop = NULL;
    f->func.cls.prop_context = NULL;
    f->func.cls.model = O70M_EXECTX;
    f->func.cls.name = O70R_NULL;
    f->func.cls.isize = sizeof(o70_ifunc_exectx_t) + sizeof(o70_ref_t) * sn;
    L("ifunc_create: isize=$xd\n", f->func.cls.isize);
    f->func.parent = O70R_NULL;

    f->func.an = 0;
    f->func.ant = NULL;
    f->func.axt = NULL;

    f->func.isx = NULL;
    f->func.ivt = NULL;
    f->func.ivn = 0;

    f->func.init = ifunc_exectx_init;
    f->func.exec = ifunc_exectx_exec;
    f->it = NULL; f->in = 0; f->im = 0;
    f->at = NULL; f->an = 0; f->am = 0;
    f->ct = NULL; f->cn = 0; f->cm = 0;
    f->eht = NULL; f->ehn = 0; f->ehm = 0;
    f->ect = &w->empty_ehc; f->ecn = 1; f->ecm = 0;

    L("ifunc_create: r$Xd\n", *out);
    return 0;
}

/* func_finish **************************************************************/
static o70_status_t C42_CALL func_finish
(
    o70_world_t * w,
    o70_ref_t r
)
{
    o70_function_t * func;
    o70_status_t os;
    size_t i;
    unsigned int mae;
    O70_ASSERT(w, (o70_model(w, r) & O70M_FUNCTION));
    func = o70_ptr(w, r);
    os = o70_ref_dec(w, func->parent);
    if (os) return os;
    for (i = 0; i < func->an; ++i)
    {
        os = o70_ref_dec(w, func->ant[i]);
        if (os) return os;
    }
    mae = C42_MA_ARRAY_FREE(&w->ma, func->ant, func->an);
    if (mae) return O70S_BUG;
    mae = C42_MA_ARRAY_FREE(&w->ma, func->axt, func->an);
    if (mae) return O70S_BUG;

    for (i = 0; i < func->ivn; ++i)
    {
        os = o70_ref_dec(w, func->ivt[i]);
        if (os) return os;
    }
    mae = C42_MA_ARRAY_FREE(&w->ma, func->ivt, func->ivn);
    if (mae) return O70S_BUG;

    return 0;
}

/* ifunc_finish *************************************************************/
static o70_status_t C42_CALL ifunc_finish
(
    o70_world_t * w,
    o70_ref_t r
)
{
    o70_ifunc_t * ifunc;
    unsigned int mae;
    unsigned int i;
    o70_status_t os;

    L("ifunc_finish r=$xd\n", r);
    ifunc = o70_ptr(w, r);
    O70_ASSERT(w, (o70_model(w, r) & O70M_IFUNC));

    mae = C42_MA_ARRAY_FREE(&w->ma, ifunc->it, ifunc->im);
    if (mae) return O70S_BUG;
    mae = C42_MA_ARRAY_FREE(&w->ma, ifunc->at, ifunc->am);
    if (mae) return O70S_BUG;
    for (i = 0; i < ifunc->cn; ++i)
    {
        os = o70_ref_dec(w, ifunc->ct[i]);
        if (os) return os;
    }

    mae = C42_MA_ARRAY_FREE(&w->ma, ifunc->ct, ifunc->cm);
    if (mae) return O70S_BUG;

    mae = C42_MA_ARRAY_FREE(&w->ma, ifunc->eht, ifunc->ehm);
    if (mae) return O70S_BUG;

    mae = C42_MA_ARRAY_FREE(&w->ma, ifunc->ect, ifunc->ecm);
    if (mae) return O70S_BUG;

    return func_finish(w, r);
}

/* o70_ifunc_ita ************************************************************/
O70_API o70_status_t C42_CALL o70_ifunc_ita
(
    o70_world_t * w,
    o70_ifunc_t * ifunc,
    unsigned int im
)
{
    unsigned int mae;
    mae = C42_MA_ARRAY_REALLOC(&w->ma, ifunc->it, ifunc->im, im);
    if (mae)
    {
        if (mae == C42_MA_CORRUPT) return O70S_BUG;
        return O70S_NO_MEM;
    }
    ifunc->im = im;
    return 0;
}

/* o70_ifunc_ata ************************************************************/
O70_API o70_status_t C42_CALL o70_ifunc_ata
(
    o70_world_t * w,
    o70_ifunc_t * ifunc,
    unsigned int am
)
{
    unsigned int mae;
    mae = C42_MA_ARRAY_REALLOC(&w->ma, ifunc->at, ifunc->am, am);
    if (mae)
    {
        if (mae == C42_MA_CORRUPT) return O70S_BUG;
        return O70S_NO_MEM;
    }
    return 0;
}

/* o70_ifunc_cta ************************************************************/
O70_API o70_status_t C42_CALL o70_ifunc_cta
(
    o70_world_t * w,
    o70_ifunc_t * ifunc,
    unsigned int cm
)
{
    unsigned int mae;
    mae = C42_MA_ARRAY_REALLOC(&w->ma, ifunc->ct, ifunc->cm, cm);
    if (mae)
    {
        if (mae == C42_MA_CORRUPT) return O70S_BUG;
        return O70S_NO_MEM;
    }
    return 0;
}

/* o70_ifunc_eha ************************************************************/
O70_API o70_status_t C42_CALL o70_ifunc_eha
(
    o70_world_t * w,
    o70_ifunc_t * ifunc,
    unsigned int ehm
)
{
    unsigned int mae;
    mae = C42_MA_ARRAY_REALLOC(&w->ma, ifunc->eht, ifunc->ehm, ehm);
    if (mae)
    {
        if (mae == C42_MA_CORRUPT) return O70S_BUG;
        return O70S_NO_MEM;
    }
    return 0;
}

/* o70_ifunc_eca ************************************************************/
O70_API o70_status_t C42_CALL o70_ifunc_eca
(
    o70_world_t * w,
    o70_ifunc_t * ifunc,
    unsigned int ecm
)
{
    unsigned int mae;
    mae = C42_MA_ARRAY_REALLOC(&w->ma, ifunc->ect, ifunc->ecm, ecm);
    if (mae)
    {
        if (mae == C42_MA_CORRUPT) return O70S_BUG;
        return O70S_NO_MEM;
    }
    return 0;
}

/* o70_ifunc_add_const ******************************************************/
O70_API o70_status_t C42_CALL o70_ifunc_add_const
(
    o70_world_t * w,
    o70_ifunc_t * ifunc,
    o70_ref_t value,
    uint16_t * ax
)
{
    unsigned int i, cm;
    o70_status_t os;
    for (i = 0; i < ifunc->cn; ++i)
    {
        if (ifunc->ct[i] == value) { *ax = i; return 0; }
    }
    cm = ifunc->cm ? ifunc->cm << 1 : O70CFG_IFUNC_CTAB_COUNT_INIT;
    os = o70_ifunc_cta(w, ifunc, cm);
    if (os) return os;
    ifunc->cm = cm;
    o70_ref_inc(w, value);
    ifunc->ct[*ax = ifunc->cn++] = value;
    return 0;
}

/* o70_ifunc_append_ret_const ***********************************************/
O70_API o70_status_t C42_CALL o70_ifunc_append_ret_const
(
    o70_world_t * w,
    o70_ifunc_t * ifunc,
    o70_ref_t value
)
{
    o70_status_t os;

    if (ifunc->in >= ifunc->im)
    {
        unsigned int im;
        im = ifunc->im ? ifunc->im << 1 : O70CFG_IFUNC_INSN_COUNT_INIT;
        os = o70_ifunc_ita(w, ifunc, im);
        if (os) return os;
        ifunc->im = im;
    }
    if (ifunc->an >= ifunc->am)
    {
        unsigned int am;
        am = ifunc->am ? ifunc->am << 1 : O70CFG_IFUNC_IARG_COUNT_INIT;
        os = o70_ifunc_ata(w, ifunc, am);
        if (os) return os;
        ifunc->am = am;
    }

    os = o70_ifunc_add_const(w, ifunc, value, ifunc->at + ifunc->an);
    if (os) return os;
    ifunc->it[ifunc->in].opcode = O70O_RET_CONST;
    ifunc->it[ifunc->in].ecx = 0;
    ifunc->it[ifunc->in].ax = ifunc->an;
    ifunc->in++;
    ifunc->an++;
    return 0;
}

/* o70_push_call ************************************************************/
O70_API o70_status_t C42_CALL o70_push_call
(
    o70_flow_t * flow,
    o70_ref_t func_ref
)
{
    o70_world_t * w = flow->world;
    o70_function_t * func;
    o70_exectx_t * e;
    o70_oidx_t ecx;
    o70_ref_t r;
    o70_status_t os, os2;

    if (!(o70_model(w, func_ref) & O70M_FUNCTION)) return O70S_BAD_TYPE;
    if (flow->n == flow->m)
    {
        L("execution stack full\n");
        return O70S_STACK_FULL;
    }

    os = obj_alloc(w, &ecx, O70_RTOX(func_ref));
    if (os) return os;
    func = w->ot[O70_RTOX(func_ref)];
    e = w->ot[ecx];
    flow->stack[flow->n++] = r = O70_XTOR(ecx);
    e->lv = NULL;
    os = func->init(flow, e);
    if (os)
    {
        os2 = o70_ref_dec(w, r);
        return os2 ? os2 : os;
    }
    return 0;
}

/* o70_exec *****************************************************************/
O70_API o70_status_t C42_CALL o70_exec
(
    o70_flow_t * flow,
    unsigned int min_depth,
    unsigned int steps_limit
)
{
    o70_world_t * w = flow->world;
    unsigned int steps;

    (void) w;
    /* min_depth must be at least one to ensure something is waiting to be
     * executed */
    if (!min_depth) return O70S_BAD_ARG;
    for (steps = 0; flow->n >= min_depth && steps < steps_limit; )
    {
        L("exec (steps $i)\n", steps);
        if (flow->exc)
        {
            /* unwinding stack for handling the exception being thrown
             * TODO: call excception handler for current exectx and 
             * if the exception got handled resume normal execution, 
             * otherwise decrement the stack depth */
        }
        else
        {
            o70_exectx_t * ectx;
            o70_function_t * func;
            o70_ref_t ecr;
            o70_oidx_t fx;
            o70_status_t os;
            ecr = flow->stack[flow->n - 1];
            /* make sure we have an execution context at the stack top */
            O70_ASSERT(w, o70_is_valid_oref(w, ecr));
            O70_ASSERT(w, o70_model(w, ecr) & O70M_EXECTX);
            /* fetch the execution context; its class is the function */
            ectx = o70_ptr(w, ecr);
            fx = ectx->ohdr.class_ox;
            /* check that we have indeed a function as its class */
            O70_ASSERT(w, o70_model(w, O70_XTOR(fx)) & O70M_FUNCTION);
            /* fetch the function */
            func = (o70_function_t *) w->ot[fx];
            /* run the execution handler */
            L("EXEC: flow=$xp, depth=$xd, ectx_$Xd, func_$Xd\n", 
              flow, flow->n, ecr, O70_XTOR(fx));
            os = func->exec(flow, ectx);
            steps += flow->steps;
            L("EXEC: flow=$xp, depth=$xd, ectx_$Xd, func_$Xd"
              " => status=$s, steps=$xd\n", //, steps=$xd\n", 
              flow, flow->n, ecr, O70_XTOR(fx), o70_status_name(os), steps);
            switch (os)
            {
            case O70S_OK:
                flow->n -= 1;
                break;
            case O70S_PENDING:
                break;
            case O70S_EXC:
            default:
                return O70S_BUG;
            }
        }

        return O70S_BUG;
    }

    return flow->n < min_depth ? (flow->exc ? O70S_EXC : 0) : O70S_PENDING;
}

