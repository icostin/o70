#include "intern.h"

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

/* id_key_cmp ***************************************************************/
static uint_fast8_t C42_CALL id_key_cmp
(
    uintptr_t key,
    c42_rbtree_node_t * node,
    void * restrict context
)
{
    c42_u8an_t * restrict key_str = (c42_u8an_t *) key;
    o70_id_t * restrict id = C42_STRUCT_FROM_FIELD_PTR(o70_id_t, rbtn, node);
    int c;
    (void) context;

    if (key_str->n != id->ctstr.data.n) 
        return key_str->n < id->ctstr.data.n 
            ? C42_RBTREE_LESS : C42_RBTREE_MORE;
    c = c42_u8a_cmp(key_str->a, id->ctstr.data.a, key_str->n);
    if (c) return c < 0 ? C42_RBTREE_LESS : C42_RBTREE_MORE;
    return C42_RBTREE_EQUAL;
}

/* o70_world_init ***********************************************************/
O70_API o70_status_t C42_CALL o70_world_init
(
    o70_world_t * w,
    o70_init_t * ini
)
{
    c42_rbtree_path_t path;
    c42_u8an_t ban;
    uint_fast8_t mae, rbte;
    o70_status_t r;

    C42_VAR_CLEAR(*w);

    do
    {
        w->ma = ini->ma;
        w->in = ini->in;
        w->out = ini->out;
        w->err = ini->err;

        w->om = 1 << c42_u32_bit_width(O70X__COUNT);
        mae = C42_MA_ARRAY_ALLOC(w->ma, w->ohdr, w->om);
        if (mae) { r = mae == C42_MA_CORRUPT ? O70S_BUG : O70S_NO_MEM; break; }
        w->ffx = w->on = O70X__COUNT;
        c42_upa_set(w->nfx + w->on, 0, w->om - w->on); /* CRASH */

        w->mn = 0;
        w->mm = O70CFG_INIT_MOD_NUM;
        mae = C42_MA_ARRAY_ALLOC(w->ma, w->mod, w->mm);
        if (mae) { r = mae == C42_MA_CORRUPT ? O70S_BUG : O70S_NO_MEM; break; }
        w->mod[0] = &w->mcore;

        c42_rbtree_init(&w->idt, id_key_cmp, NULL);

        w->ohdr[O70X_NULL           ] = &w->null_obj.ohdr;
        w->ohdr[O70X_FALSE          ] = &w->false_obj.ohdr;
        w->ohdr[O70X_TRUE           ] = &w->true_obj.ohdr;
        w->ohdr[O70X_NULL_CLASS     ] = &w->null_class.ohdr;
        w->ohdr[O70X_BOOL_CLASS     ] = &w->bool_class.ohdr;
        w->ohdr[O70X_OBJECT_CLASS   ] = &w->object_class.ohdr;
        w->ohdr[O70X_CLASS_CLASS    ] = &w->class_class.ohdr;
        w->ohdr[O70X_ID_CLASS       ] = &w->id_class.ohdr;
        w->ohdr[O70X_ARRAY_CLASS    ] = &w->array_class.ohdr;
        w->ohdr[O70X_FUNCTION_CLASS ] = &w->function_class.ohdr;
        w->ohdr[O70X_STRING_CLASS   ] = &w->string_class.ohdr;
        w->ohdr[O70X_CTSTRING_CLASS ] = &w->ctstring_class.ohdr;
        w->ohdr[O70X_EXCEPTION_CLASS] = &w->exception_class.ohdr;
        w->ohdr[O70X_MODULE_CLASS   ] = &w->module_class.ohdr;
        w->ohdr[O70X_NULL_ID        ] = &w->null_id.ctstr.ohdr;
        w->ohdr[O70X_FALSE_ID       ] = &w->false_id.ctstr.ohdr;
        w->ohdr[O70X_TRUE_ID        ] = &w->true_id.ctstr.ohdr;
        w->ohdr[O70X_NULL_CLASS_ID  ] = &w->null_class_id.ctstr.ohdr;
        w->ohdr[O70X_BOOL_ID        ] = &w->bool_id.ctstr.ohdr;
        w->ohdr[O70X_INT_ID         ] = &w->int_id.ctstr.ohdr;
        w->ohdr[O70X_OBJECT_ID      ] = &w->object_id.ctstr.ohdr;
        w->ohdr[O70X_CLASS_ID       ] = &w->class_id.ctstr.ohdr;
        w->ohdr[O70X_ID_ID          ] = &w->id_id.ctstr.ohdr;
        w->ohdr[O70X_ARRAY_ID       ] = &w->array_id.ctstr.ohdr;
        w->ohdr[O70X_FUNCTION_ID    ] = &w->function_id.ctstr.ohdr;
        w->ohdr[O70X_STRING_ID      ] = &w->string_id.ctstr.ohdr;
        w->ohdr[O70X_CTSTRING_ID    ] = &w->ctstring_id.ctstr.ohdr;
        w->ohdr[O70X_EXCEPTION_ID   ] = &w->exception_id.ctstr.ohdr;
        w->ohdr[O70X_MODULE_ID      ] = &w->module_id.ctstr.ohdr;

        w->             null_obj.ohdr.ref_count = 1;
        w->            false_obj.ohdr.ref_count = 1;
        w->             true_obj.ohdr.ref_count = 1;
        w->           null_class.ohdr.ref_count = 1;
        w->           bool_class.ohdr.ref_count = 1;
        w->            int_class.ohdr.ref_count = 1;
        w->         object_class.ohdr.ref_count = 1;
        w->          class_class.ohdr.ref_count = 1;
        w->             id_class.ohdr.ref_count = 1;
        w->          array_class.ohdr.ref_count = 1;
        w->       function_class.ohdr.ref_count = 1;
        w->         string_class.ohdr.ref_count = 1;
        w->       ctstring_class.ohdr.ref_count = 1;
        w->      exception_class.ohdr.ref_count = 1;
        w->         module_class.ohdr.ref_count = 1;
        w->        null_id.ctstr.ohdr.ref_count = 1;
        w->       false_id.ctstr.ohdr.ref_count = 1;
        w->        true_id.ctstr.ohdr.ref_count = 1;
        w->  null_class_id.ctstr.ohdr.ref_count = 1;
        w->        bool_id.ctstr.ohdr.ref_count = 1;
        w->         int_id.ctstr.ohdr.ref_count = 1;
        w->      object_id.ctstr.ohdr.ref_count = 1;
        w->       class_id.ctstr.ohdr.ref_count = 1;
        w->          id_id.ctstr.ohdr.ref_count = 1;
        w->       array_id.ctstr.ohdr.ref_count = 1;
        w->    function_id.ctstr.ohdr.ref_count = 1;
        w->      string_id.ctstr.ohdr.ref_count = 1;
        w->    ctstring_id.ctstr.ohdr.ref_count = 1;
        w->   exception_id.ctstr.ohdr.ref_count = 1;
        w->      module_id.ctstr.ohdr.ref_count = 1;

        w->             null_obj.ohdr.class_ox = O70X_NULL_CLASS;
        w->            false_obj.ohdr.class_ox = O70X_BOOL_CLASS;
        w->             true_obj.ohdr.class_ox = O70X_BOOL_CLASS;
        w->           null_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->           bool_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->            int_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->         object_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->          class_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->             id_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->          array_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->       function_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->         string_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->       ctstring_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->      exception_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->         module_class.ohdr.class_ox = O70X_CLASS_CLASS;
        w->        null_id.ctstr.ohdr.class_ox = O70X_ID_CLASS;
        w->       false_id.ctstr.ohdr.class_ox = O70X_ID_CLASS;
        w->        true_id.ctstr.ohdr.class_ox = O70X_ID_CLASS;
        w->  null_class_id.ctstr.ohdr.class_ox = O70X_ID_CLASS;
        w->        bool_id.ctstr.ohdr.class_ox = O70X_ID_CLASS;
        w->         int_id.ctstr.ohdr.class_ox = O70X_ID_CLASS;
        w->      object_id.ctstr.ohdr.class_ox = O70X_ID_CLASS;
        w->       class_id.ctstr.ohdr.class_ox = O70X_ID_CLASS;
        w->          id_id.ctstr.ohdr.class_ox = O70X_ID_CLASS;
        w->       array_id.ctstr.ohdr.class_ox = O70X_ID_CLASS;
        w->    function_id.ctstr.ohdr.class_ox = O70X_ID_CLASS;
        w->      string_id.ctstr.ohdr.class_ox = O70X_ID_CLASS;
        w->    ctstring_id.ctstr.ohdr.class_ox = O70X_ID_CLASS;
        w->   exception_id.ctstr.ohdr.class_ox = O70X_ID_CLASS;
        w->      module_id.ctstr.ohdr.class_ox = O70X_ID_CLASS;

#define A(_id, _str) \
        ban.a = (uint8_t *) (_str); ban.n = sizeof(_str) - 1; \
        rbte = c42_rbtree_find(&path, &w->idt, (uintptr_t) &ban); \
        if (rbte != C42_RBTREE_NOT_FOUND) { r = O70S_BUG; break; } \
        c42_rbtree_insert(&path, &w->_id.rbtn);

        A(      null_id, "null"         );
        A(     false_id, "false"        );
        A(      true_id, "true"         );
        A(null_class_id, "null_class"   );
        A(      bool_id, "bool"         );
        A(       int_id, "int"          );
        A(    object_id, "object"       );
        A(     class_id, "class"        );
        A(        id_id, "id"           );
        A(     array_id, "array"        );
        A(  function_id, "function"     );
        A(    string_id, "string"       );
        A(  ctstring_id, "ctstring"     );
        A( exception_id, "exception"    );
        A(    module_id, "module"       );
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
        mae = C42_MA_ARRAY_FREE(w->ma, w->ohdr, w->om);
        if (mae) return O70S_BUG;
        w->om = 0;
    }

    if (w->mm)
    {
        mae = C42_MA_ARRAY_FREE(w->ma, w->mod, w->mm);
        if (mae) return O70S_BUG;
        w->mm = 0;
    }

    return rs;
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

