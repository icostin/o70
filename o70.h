#ifndef _O70_H
#define _O70_H

#include <c42.h>

#if O70_STATIC
#define O70_API
#elif O70_LIB_BUILD
#define O70_API C42_LIB_EXPORT
#else
#define O70_API C42_LIB_IMPORT
#endif

/* defines {{{1 */
/* O70CFG_INIT_MOD_NUM ******************************************************/
/**
 *  Initial number of modules to allocate.
 */
#define O70CFG_INIT_MOD_NUM 4

/* O70_RTOX *****************************************************************/
/**
 *  converts an object reference to object index.
 */
#define O70_RTOX(_ref) ((_ref) >> 1)
/* O70_XTOR *****************************************************************/
/**
 *  Converts an object index to an object reference.
 */
#define O70_XTOR(_idx) (((_idx) << 1) | 0)

/* O70_IS_OREF **************************************************************/
/**
 *  Returns true only if the given reference points to a regular object.
 */
#define O70_IS_OREF(_ref) (!((_ref) & 1))

/* Object model constants */
#define O70M_OBJECT     (1 << 0) /**< regular object (with a property bag) */
#define O70M_CLASS      (1 << 1) /**< class model */
#define O70M_SCTSTR     (1 << 2) 
#define O70M_ACTSTR     (1 << 3)
#define O70M_ICTSTR     (1 << 4) 
#define O70M_STR        (1 << 5) 
#define O70M_ARRAY      (1 << 6) 
#define O70M_FUNCTION   (1 << 7) 
#define O70M_EXCEPTION  (1 << 8) 
#define O70M_MODULE     (1 << 9)

/* enums {{{1 */
/* o70_opcodes **************************************************************/
/**
 *  Opcodes for the scripting VM.
 *
 * _lsx: local slot index
 * _idcx: identifier const index
 * _cbx: code block index
 *
 * resolve out_lsx, name_idcx
 * fget val_lsx, obj_lsx, name_idcx
 * fset val_lsx, obj_lsx, name_idcx
 * aget val_lsx, arr_lsx, index_lsx
 * aset val_lsx, arr_lsx, index_lsx
 * call val_lsx, func_lsx, arg0_lsx, ...
 * goto tgt_cbx
 * branch cond_lsx, true_lsx, false_lsx
 * ret val_lsx
 * throw exc_lsx
 */
enum o70_opcodes
{
    O70O_COPY, // copy local var dest_lsx, src_lsx
    O70O_PGET, // get from parent var context: depth, psx, lsx
    O70O_PSET, // set var into parent var context: depth, psx, lsx
    O70O_FGET, // field get
    O70O_FSET,
    O70O_AGET,
    O70O_ASET,
    O70O_CALL,
    O70O_GOTO,
    O70O_BRANCH,
    O70O_RET,
    O70O_THROW,
};

/* o70_statuses *************************************************************/
/**
 *  Status codes returned by functions in this library
 */
enum o70_statuses
{
    O70S_OK = 0, /**< all ok */
    O70S_PENDING, /**< operation is pending; more scripting code needs to be
                    executed before completing the operation */
    O70S_BAD_ARG, /**< some argument or input field has an incorrect value */
    O70S_NO_MEM, /**< allocation failed */
    O70S_IO_ERROR, /**< I/O error */

    O70S_BUG = 0x70,
    O70S_TODO,
};

enum o70_builtin_object_indexes
{
    O70X_NULL,
    O70X_FALSE,
    O70X_TRUE,
    O70X_OBJECT_CLASS,
    O70X_CLASS_CLASS,
    O70X_NULL_CLASS,
    O70X_BOOL_CLASS,
    O70X_INT_CLASS,
    O70X_ARRAY_CLASS,
    O70X_FUNCTION_CLASS,
    O70X_STR_CLASS,
    O70X_CTSTR_CLASS,
    O70X_ACTSTR_CLASS,
    O70X_EXCEPTION_CLASS,
    O70X_MODULE_CLASS,
    O70X_NULL_CTSTR,
    O70X_FALSE_CTSTR,
    O70X_TRUE_CTSTR,
    O70X_NULL_CLASS_CTSTR,
    O70X_BOOL_CTSTR,
    O70X_INT_CTSTR,
    O70X_OBJECT_CTSTR,
    O70X_CLASS_CTSTR,
    O70X_ARRAY_CTSTR,
    O70X_FUNCTION_CTSTR,
    O70X_STR_CTSTR,
    O70X_CTSTR_CTSTR,
    O70X_ACTSTR_CTSTR,
    O70X_EXCEPTION_CTSTR,
    O70X_MODULE_CTSTR,

    O70X__COUNT /* Dracula ^..^ */
};

/* forward type definitions {{{1 */
/* o70_status_t *************************************************************/
/**
 *  Fast integer type to hold a status code.
 */
typedef uint_fast8_t o70_status_t;
/* o70_pkstat_t *************************************************************/
/**
 *  Smallest integer guaranteed to hold a status code.
 */
typedef uint8_t o70_pkstat_t;

/* o70_ref_t ****************************************************************/
/**
 *  Reference to objects.
 */
typedef uint32_t o70_ref_t;

/* o70_oidx_t ***************************************************************/
/**
 *  Integer type to hold an object index.
 */
typedef uint32_t o70_oidx_t;

/* o70_world_t **************************************************************/
/**
 *  The root structure describing a scripting instance.
 */
typedef struct o70_world_s o70_world_t;

/* o70_flow_t ***************************************************************/
/**
 *  Structure holding the context for a flow of execution.
 *  This is equivalent of a thread context with is call stack.
 */
typedef struct o70_flow_s o70_flow_t;

/* o70_ohdr_t ***************************************************************/
/**
 *  Object header.
 */
typedef struct o70_ohdr_s o70_ohdr_t;


/* o70_init_t ***************************************************************/
/**
 *  Parameters for initing a scripting instance.
 */
typedef struct o70_init_s o70_init_t;

/* o70_prop_node_t **********************************************************/
/**
 *  Property node.
 *  This is a pair of (key, value) with indexing information for quick
 *  lookup. The key is a ref to a ctstr object
 */
typedef struct o70_prop_node_s o70_prop_node_t;

/* o70_prop_bag_t ***********************************************************/
/**
 *  Property bag.
 *  A set of properties.
 */
typedef struct o70_prop_bag_s o70_prop_bag_t;

/* o70_kv_t *****************************************************************/
/**
 *  Key-value pair.
 *  Key and value are object references.
 */
typedef struct o70_kv_s o70_kv_t;

/* o70_object_t *************************************************************/
/**
 *  Standard object structure.
 *  This is for objects that act as dynamic property bags.
 */
typedef struct o70_object_s o70_object_t;

/* o70_class_t **************************************************************/
/**
 *  Class structure.
 */
typedef struct o70_class_s o70_class_t;

/* o70_ctstr_t **************************************************************/
/**
 *  Constant str.
 */
typedef struct o70_ctstr_s o70_ctstr_t;

/* o70_str_t ****************************************************************/
/**
 *  Byte String.
 */
typedef struct o70_str_s o70_str_t;

/* o70_array_t **************************************************************/
/**
 *  Array of object references.
 */
typedef struct o70_array_s o70_array_t;

/* o70_function_t ***********************************************************/
/**
 *  Function object.
 */
typedef struct o70_function_s o70_function_t;

/* o70_struct_t *************************************************************/
/**
 *  Struct object.
 */
typedef struct o70_struct_s o70_struct_t;

/* o70_struct_class_t *******************************************************/
/**
 *  Class object for Struct-derived objects.
 */
typedef struct o70_struct_class_s o70_struct_class_t;

/* o70_varctx_t *************************************************************/
/**
 *  Variable context object.
 */
typedef struct o70_varctx_s o70_varctx_t;

/* o70_varctx_class_t *******************************************************/
/**
 *  Class object for Variable Contexts.
 */
typedef struct o70_varctx_class_s o70_varctx_class_t;

/* o70_module_t *************************************************************/
/**
 *  Module.
 */
typedef struct o70_module_s o70_module_t;

/* o70_esf_t ****************************************************************/
/**
 *  Execution stack frame.
 */
typedef struct o70_esf_s o70_esf_t;

/* o70_ehi_t ****************************************************************/
/**
 *  Exception handler item.
 */
typedef struct o70_ehi_s o70_ehi_t;

/* o70_slim_obj_t ***********************************************************/
/**
 *  Slim object (object without properties).
 */
typedef struct o70_slim_obj_s o70_slim_obj_t;

/* o70_exception_t **********************************************************/
/**
 *  Exception object
 */
typedef struct o70_exception_s o70_exception_t;


#if 0
/* o70_obj_init_f ***********************************************************/
/**
 *  Object initialiser function prototype.
 *  The implementation is allowed to modify the flow for instance
 *  to allow execution to run a script function that inits the object.
 *  @param flow [in]    execution flow
 *  @param obj  [in]    reference to the object being inited
 *  @retval O70S_OK     init ok
 */
typedef o70_status_t (C42_CALL * o70_obj_init_f)
    (
        o70_flow_t * flow,
        o70_ref_t obj
    );
#endif

/* function pointer types {{{1 */
/* o70_obj_finish_f *********************************************************/
/**
 *  Finishes an object.
 *  This is responsible for cleaning up references to other objects and
 *  releasing any native resources held by the object.
 *  @retval O70S_OK     init ok
 */
typedef o70_status_t (C42_CALL * o70_obj_finish_f)
    (
        o70_world_t * w,
        o70_ref_t obj
    );

/* o70_obj_get_prop_f *******************************************************/
/**
 *  Gets an object property.
 *  For primitive objects this usually fills @a value_ptr and returns #O70S_OK.
 *  In more complex scenarios this can push a function call in the execution
 *  stack and return #O70S_PENDING.
 */
typedef o70_status_t (C42_CALL * o70_obj_get_prop_f)
    (
        o70_flow_t * flow,
        o70_ref_t obj,
        o70_ref_t prop_id,
        o70_ref_t * value_ptr,
        void * context
    );

/* o70_obj_set_prop_f *******************************************************/
/**
 *  Sets an object property.
 *  For primitive objects this usually updates the object instance and
 *  returns #O70S_OK.
 *  In more complex scenarios this can push a function call in the execution
 *  stack and return #O70S_PENDING.
 */
typedef o70_status_t (C42_CALL * o70_obj_set_prop_f)
    (
        o70_flow_t * flow,
        o70_ref_t obj,
        o70_ref_t prop_id,
        o70_ref_t value,
        void * context
    );

/* structs and unions {{{1 */
struct o70_esf_s
{
    uint32_t mod_idx; /**< index in world's module table */
    uint32_t insn_idx; /**< index in module's instruction table */
    o70_ref_t func; /**< reference to function currently executing */
    o70_ref_t varctx; /**< object reference to var context with locals */
};

struct o70_flow_s
{
    o70_world_t * world; /**< pointer to the world */
    o70_esf_t * stack; /**< execution stack */
    o70_ref_t exc; /**< exception being thrown (or null) */
    uint32_t n; /**< number of used stack frames */
    uint32_t m; /**< number of allocated stack frames */
};

struct o70_init_s
{
    c42_ma_t * ma; /**< memory allocator */

    uint64_t ma_total_limit; /**< max total memory to allocate; 0 means don't care */
    uint64_t ma_block_limit; /**< max size of any allocated block */
    uint64_t ma_count_limit; /**< max number of blocks to allocate */

    c42_io8_t * in; /**< standard input */
    c42_io8_t * out; /**< standard output */
    c42_io8_t * err; /**< standard error */
};

struct o70_ohdr_s
{
    union
    {
        uint32_t nref; /**< number of references to the object */
        uint32_t ndx; /**< next destroy index */
    };
    uint32_t class_ox; /**< object index for current object's class */
};

struct o70_kv_s
{
    o70_ref_t key; /**< key */
    o70_ref_t val; /**< value */
};

struct o70_prop_node_s
{
    c42_rbtree_node_t rbtn; /**< red/black tree node */
    o70_kv_t kv; /**< payload data (id + value) */
};

struct o70_prop_bag_s
{
    c42_rbtree_t rbt; /**< red/black tree */
};

struct o70_slim_obj_s
{
    o70_ohdr_t ohdr; /**< object header */
};

struct o70_object_s
{
    o70_ohdr_t ohdr; /**< object header */
    o70_prop_bag_t fields; /**< property bag */
};

struct o70_class_s
{
    o70_ohdr_t ohdr; /**< object header */
    o70_prop_bag_t fields; /**< fields */
    o70_prop_bag_t methods; /**< methods */
    o70_obj_finish_f finish; /**< finish callback */
    o70_obj_get_prop_f get_prop; /**< get property */
    o70_obj_set_prop_f set_prop; /**< set property */

    void * finish_context; /**< context for o70_class_t#finish */
    void * prop_context;
    /**< context for o70_class_t#get_prop and o70_class_t#set_prop */

    uint32_t model; /**< bitmask for primitive types compatible with instances
                         of this class */
    size_t isize; /**< size in bytes of instances */
    // size_t class_size; /**< size in bytes of the class object */
};

struct o70_ctstr_s
{
    o70_ohdr_t ohdr; /**< object header */
    c42_u8an_t data; /**< str data (pointer & size) */
};

struct o70_str_s
{
    o70_ohdr_t ohdr; /**< object header */
    c42_u8an_t data; /**< str data (pointer & size) */
    size_t asize; /**< allocated size */
};

struct o70_array_s
{
    o70_ohdr_t ohdr; /**< object header */
    o70_ref_t * t; /**< table of items */
    size_t n; /**< number of used items */
    size_t m; /**< number of items allocated */
};

struct o70_function_s
{
    o70_ohdr_t ohdr; /**< object header */
    o70_module_t * mod; /**< module containing the code */
    uint32_t fx; /**< function index in the module */
    o70_ref_t var_ctx_class; /**< ref to class representing the variable context
                               used during execution of the given function */
    o70_ref_t parent_var_ctx; /** ref to parent variable context - for regular
                                functions this points to the module containing
                                the function */
    uint32_t * asx; /**< arg slot index table; tells into which slot to put
                         the arguments to the function */
    uint32_t * isx; /**< inited arg slot index table; tells into which slot
                         to put pre-inited args */
    o70_ref_t * iv; /**< inited arg values */
    size_t an; /**< number of arguments */
    size_t in; /**< number of inited args */
};

struct o70_exception_s
{
    o70_ohdr_t ohdr; /**< object header */
};

struct o70_struct_class_s
{
    o70_class_t cls; /**< class data */
    o70_prop_bag_t named_slots; /**< key: ids, value: slot indexes */
    size_t num_slots; /**< number of slots; deducible also from
                            cls.instance_size */
};

struct o70_struct_s
{
    o70_ohdr_t ohdr; /**< object header */
    o70_ref_t slots[0]; /**< data slots */
};

struct o70_varctx_s
{
    o70_ohdr_t ohdr; /**< object header */
    o70_varctx_t * parent; /**< parent object context */
    o70_ref_t slots[0]; /**< variable slots */
};

struct o70_varctx_class_s
{
    o70_class_t cls; /**< standard class fields */
    o70_ref_t * idt; /**< table with ids for named variables, sorted by
                          identifier ref value; ni items */
    uint32_t * sxt; /**< slot indexes corresponding to ids in the table above;
                         ni items */
    size_t ni; /**< number of ids */
    size_t ns; /**< number of slots */
};

struct o70_ehi_s
{
    uint32_t eci; /**< exception class index in constants table */
    uint32_t hix; /**< handler instruction index */
};

struct o70_module_s
{
    uint8_t * opc; /**< table of opcodes */
    uint32_t * aofs; /**< arg offsets */
    uint32_t * ia; /**< instruction args */
    uint32_t * ieh; /**< instruction exception handler */
    uint32_t * ffix; /**< function body first instruction index
                            (nfb + 1 items) */
    o70_ref_t * fvcc; /**< function var-context class table; nf items */
    o70_ref_t * ct; /**< table of constants */
    o70_ehi_t * ehi; /**< array of all items from all exception handlers;
                            has @anehi items */
    uint32_t * ehc; /**< exception handler chain;
                          for each exception handler chain this points to the
                          index of the first exc. handler item;
                          has nehc + 1 items */
    size_t ni; /**< number of instructions for all functions/code blocks */
    size_t na; /**< number of instruction arguments for all the code */
    size_t nf; /**< number of functions */
    size_t nc; /**< number of constants */
    size_t nehc; /**< number of exception handler chains */
    size_t nehi; /**< number of exception handler items */
    o70_ref_t varctx; /**< var context for the globals of this module */
};

struct o70_world_s
{
    union
    {
        o70_ohdr_t * * ohdr; /**< table to all object headers */
        void * * ot; /**< object table - easy to cast */
        uintptr_t * nfx; /**< next free index table */
    };
    o70_module_t * * mod; /**< array of pointers to modules */

    o70_prop_bag_t ics; /**< internalised constant strings */

    size_t on; /**< number of inited object entries */
    size_t om; /**< number of allocated object entries */
    size_t ffx; /**< first free index */
    size_t fdx; /**< first destroy index; this is the head of the list of
                  indices to objects to destroy; the indices are chained by
                  using ohdr->nref/nfx */

    size_t mn; /**< number of used modules */
    size_t mm; /**< number of allocated modules */

    size_t fn; /**< number of flows */

    c42_ma_t ma; /**< memory allocator */
    c42_malim_ctx_t mactx; /**< allocator context */

    c42_io8_t * in; /**< standard input */
    c42_io8_t * out; /**< standard output */
    c42_io8_t * err; /**< standard error */

    o70_slim_obj_t null_obj; /**< null object */
    o70_slim_obj_t false_obj; /**< false object */
    o70_slim_obj_t true_obj; /**< true object */
    o70_class_t null_class; /**< null class */
    o70_class_t bool_class; /**< bool class */
    o70_class_t int_class; /**< int class */
    o70_class_t object_class; /**< object class */
    o70_class_t class_class; /**< class class */
    o70_class_t array_class; /**< array class */
    o70_class_t function_class; /**< function class */
    o70_class_t str_class; /**< str class */
    o70_class_t ctstr_class; /**< const str class */
    o70_class_t actstr_class; /**< allocated const str class */
    o70_class_t exception_class; /**< exception class */
    o70_class_t module_class; /**< module class */
    o70_ctstr_t null_ctstr; /**< null ctstr */
    o70_ctstr_t false_ctstr; /**< false ctstr */
    o70_ctstr_t true_ctstr; /**< true ctstr */
    o70_ctstr_t null_class_ctstr; /**< null ctstr */
    o70_ctstr_t bool_ctstr; /**< bool ctstr */
    o70_ctstr_t int_ctstr; /**< int ctstr */
    o70_ctstr_t object_ctstr; /**< object ctstr */
    o70_ctstr_t class_ctstr; /**< class ctstr */
    o70_ctstr_t array_ctstr; /**< array ctstr */
    o70_ctstr_t function_ctstr; /**< function ctstr */
    o70_ctstr_t str_ctstr; /**< str ctstr */
    o70_ctstr_t ctstr_ctstr; /**< ctstr ctstr */
    o70_ctstr_t actstr_ctstr; /**< actstr ctstr */
    o70_ctstr_t exception_ctstr; /**< exception ctstr */
    o70_ctstr_t module_ctstr; /**< module ctstr */
    o70_module_t mcore; /**< core module */

    o70_pkstat_t aux_status; /**< aux status when the function returns the
                               "main" status */
};

/* exported functions and API-style macros {{{1 */
/* o70_lib_name *************************************************************/
/**
 *  Returns a static str identifying the library with its configuration.
 */
O70_API uint8_t const * C42_CALL o70_lib_name ();

/* o70_status_name **********************************************************/
/**
 *  Returns a static string describing the given status code.
 */
O70_API uint8_t const * C42_CALL o70_status_name (o70_status_t sc);

/* o70_world_init ***********************************************************/
/**
 *  Initialises an instance.
 *  @param w [out]          world structure to initialise
 *  @param ini [in]         initialisation parameters
 *  @retval O70S_OK         init ok
 *  @retval O70S_BAD_ARG    bad init data
 *  @retval O70S_NO_MEM     allocation failed
 *  @retval O70S_BUG        allocator returned heap corruption
 */
O70_API o70_status_t C42_CALL o70_world_init
(
    o70_world_t * w,
    o70_init_t * ini
);

/* o70_world_finish *********************************************************/
/**
 *  Finishes a scripting instance.
 *  This should be called after a successful call to o70_world_init() but
 *  it can safely be called multiple times after successful/unsuccessful calls
 *  to o70_world_init().
 *  @retval O70S_OK         everything went fine
 *  @retval O70S_BUG        allocator returned heap corruption
 */
O70_API o70_status_t C42_CALL o70_world_finish
(
    o70_world_t * w
);

/* o70_ref_inc **************************************************************/
/**
 *  Increments the ref counter for the given object
 */
C42_INLINE void o70_ref_inc
(
    o70_world_t * w,
    o70_ref_t oref
)
{
    if (O70_IS_OREF(oref)) w->ohdr[O70_RTOX(oref)]->nref += 1;
}

O70_API o70_status_t C42_CALL _o70_obj_destroy (o70_world_t * w);

/* o70_ref_dec **************************************************************/
/**
 *  Decrements the number of refs to the given object.
 *  If there are no refs left, then the object will be destroyed.
 *  Object destructors cannot 'call' interpreted code therefore there is no
 *  need for a flow.
 *  Destroying the current object may cause the destruction of other objects
 *  however, no cycles can be created because the objects destroyed before 
 *  had 0 refs (nobody still pointing to them).
 *  TODO: this func should just queue the object in a destroy list and have 
 *  another api for killing - this will avoid nesting too many calls on the
 *  real stack
 */
C42_INLINE o70_status_t o70_ref_dec
(
    o70_world_t * w,
    o70_ref_t oref
)
{
    uint32_t ox;
    o70_status_t st;

    if (O70_IS_OREF(oref)) 
    {
        ox = O70_RTOX(oref);
        if (!(w->ohdr[ox]->nref -= 1))
        {
            if (ox < O70X__COUNT) return O70S_BUG;
            w->ohdr[ox]->ndx = w->fdx;
            w->fdx = ox;
            st = _o70_obj_destroy(w);
            return st;
        }
    }
    return 0;
}

/* o70_flow_create **********************************************************/
/**
 *  Creates an execution flow.
 *  @param w [in]                   world
 *  @param flow_ptr [out]           receives the pointer to the allocated flow
 *  @param max_stack_depth [in]     max number of stack frames
 *  @retval O70S_OK                 all ok
 *  @retval O70S_BAD_ARG            invalid @a max_stack_depth
 *  @retval O70S_NO_MEM             allocation failed
 *  @retval O70S_BUG                allocator returned heap corruption
 */
O70_API o70_status_t C42_CALL o70_flow_create
(
    o70_world_t * w,
    o70_flow_t * * flow_ptr,
    uint32_t max_stack_depth
);

/* o70_flow_destroy *********************************************************/
/**
 *  Destroys an execution flow.
 *  @param flow [in]                flow to destroy
 *  @retval O70S_OK                 all ok
 *  @retval O70S_BUG                allocator returned heap corruption
 */
O70_API o70_status_t C42_CALL o70_flow_destroy
(
    o70_flow_t * flow
);

/* o70_ctstr_intern *********************************************************/
/**
 *  Returns the 'internalised' constant string.
 *  Internalised strings have the property that if 2 strings have the same
 *  content then their internalised strings are the same object.
 */
O70_API o70_status_t C42_CALL o70_ctstr_intern
(
    o70_world_t * w,
    o70_ref_t in,
    o70_ref_t * out
);

/* o70_ctstr_static *********************************************************/
/**
 *  Allocates a new static constant string object.
 */
O70_API o70_status_t C42_CALL o70_ctstr_static
(
    o70_world_t * w,
    o70_ref_t * out,
    void const * ptr,
    size_t len
);

/* O70_CTSTR ****************************************************************/
/**
 *  Creates a static constant string from a given string literal.
 */
#define O70_CTSTR(_w, _ref_ptr, _str_lit) \
    (o70_ctstr_static((_w), (_ref_ptr), (_str_lit), sizeof(_str_lit) - 1))

/* o70_ctstr_static_intern **************************************************/
/**
 *  Given a static byte buffer it returns an internalized ctstr object.
 */
O70_API o70_status_t C42_CALL o70_ctstr_static_intern
(
    o70_world_t * w,
    o70_ref_t * out,
    void const * ptr,
    size_t len
);

/* O70_ICTSTR ***************************************************************/
/**
 *  internalised static const string.
 */
#define O70_ICTSTR(_w, _ref_ptr, _str_lit) \
    (o70_ctstr_static_intern((_w), (_ref_ptr), \
                             (_str_lit), sizeof(_str_lit) - 1))

/* o70_dump_icst ************************************************************/
/**
 *  Dumps in escaped form all items from the internalised constant strings tree
 */
O70_API o70_status_t C42_CALL o70_dump_icst
(
    o70_world_t * w,
    c42_io8_t * o
);

/* o70_obj_create ***********************************************************/
/**
 *  Creates a new <<object>> instance with an empty set of dynamic fields.
 */
O70_API o70_status_t C42_CALL o70_obj_create
(
    o70_world_t * w,
    o70_ref_t * out
);

/* o70_obj_raw_get **********************************************************/
/**
 *  Retrieves a raw property of an object instance.
 *  The property must be in the object's property bag as this function does
 *  not run the property getter for the object.
 *  @param w [in] world
 *  @param obj [in] any instance derived from object
 *  @param name [in] an internalised const string representing property name
 *  @param value [out] will get value of property on success
 */
O70_API o70_status_t C42_CALL o70_obj_raw_get
(
    o70_world_t * w,
    o70_ref_t obj,
    o70_ref_t name,
    o70_ref_t * value
);

/* o70_obj_raw_put **********************************************************/
/**
 *  Puts a raw property in an object instance.
 *  The property will be modified/added to the property bag of the object.
 *  @param w [in] world
 *  @param obj [in] any instance derived from object
 *  @param name [in] an internalised const string representing property name
 *  @param value [in] value to be set; 
 *  @note the ref count of @a value will be incremented
 *  @note if the property did not exist in the object then the ref count of
 *      @a name will be incremented
 */
O70_API o70_status_t C42_CALL o70_obj_raw_put
(
    o70_world_t * w,
    o70_ref_t obj,
    o70_ref_t name,
    o70_ref_t value
);

/* }}}1 */

#endif

