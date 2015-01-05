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

/* O70CFG_INIT_STR_SIZE *****************************************************/
/**
 *  Initial string alloc size.
 */
#define O70CFG_INIT_STR_SIZE 10

#define O70CFG_ICODE_INSN_COUNT_INIT 8
#define O70CFG_ICODE_IARG_COUNT_INIT 8
#define O70CFG_ICODE_CTAB_COUNT_INIT 8

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

/* O70_IS_FASTINT ***************************************************************/
/**
 *  Returns true if and only if the given reference points to an int.
 */
#define O70_IS_FASTINT(_ref) ((_ref) & 1)

/* O70_FITOR ****************************************************************/
/**
 *  Converts an int value to a fastint reference.
 */
#define O70_FITOR(_value) (((_value) << 1) | 1)

/* O70_RTOFI ****************************************************************/
/**
 *  ref to fastint
 */
#define O70_RTOFI(_r) ((int32_t) (_r) >> 1)

/* Object model constants */
#define O70M_DYNOBJ     (1 << 0) /**< regular object (with a property bag) */
#define O70M_CLASS      (1 << 1) /**< class model */
#define O70M_SCTSTR     (1 << 2)
#define O70M_ACTSTR     (1 << 3)
#define O70M_ICTSTR     (1 << 4)
#define O70M_STR        (1 << 5)
#define O70M_ARRAY      (1 << 6)
#define O70M_FUNCTION   (1 << 7)
#define O70M_IFUNC      (1 << 8)
#define O70M_EXECTX     (1 << 9)
#define O70M_EXCEPTION  (1 << 10)
#define O70M_MODULE     (1 << 11)
#define O70M_ICODE      (1 << 12)

#define O70M_ANY_CTSTR  (O70M_SCTSTR | O70M_ACTSTR | O70M_ICTSTR)

/* hardcoded refs */
#define O70R_NULL       (O70_XTOR(O70X_NULL))
#define O70R_FALSE      (O70_XTOR(O70X_FALSE))
#define O70R_TRUE       (O70_XTOR(O70X_TRUE))

#if _DEBUG
#define O70_LOG(_w, ...) (c42_io8_fmt((_w)->err, __VA_ARGS__))

#define O70_ASSERT(_w, _expr) \
    if ((_expr)) ; \
    else do { \
        O70_LOG((_w), "*** BUG *** $s:$i:$s: assertion failed ($s)\n", \
                __FILE__, __LINE__, __FUNCTION__, #_expr); \
        *(char volatile *) NULL = 0; \
    } while (0)
#else
#define O70_LOG(_w, ...)
#define O70_ASSERT(_w, _expr)
#endif

/* enums {{{1 */
/* o70_opcodes **************************************************************/
/**
 *  Opcodes for the scripting VM.
 *
 * _lsx: local slot index
 * _idcx: identifier const index
 * _cbx: code block index
 * _ctx: const table index
 *
 * ??resolve out_lsx, name_idcx
 * const dest_lsx, src_ctx
 * fget val_lsx, obj_lsx, name_idcx
 * fset val_lsx, obj_lsx, name_idcx
 * aget val_lsx, arr_lsx, index_lsx
 * aset val_lsx, arr_lsx, index_lsx
 * call val_lsx, func_lsx, arg0_lsx, ...
 * goto tgt_cbx
 * branch cond_lsx, true_lsx, false_lsx
 * ret val_lsx
 * ret_const val_ctx
 * throw exc_lsx
 */
enum o70_opcodes
{
    O70O_CONST = 0, // sets a local var to a const: lsx, ctx
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
    O70O_RET_CONST,
    O70O_THROW,

    O70O__COUNT
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
    O70S_EXC, /**< exception thrown */
    O70S_MISSING, /**< item/property missing */
    O70S_BAD_ARG, /**< some argument or input field has an incorrect value */
    O70S_BAD_TYPE, /**< an object has the wrong type */
    O70S_NO_MEM, /**< allocation failed */
    O70S_IO_ERROR, /**< I/O error */
    O70S_BAD_FMT,
    O70S_BAD_UTF8,
    O70S_CONV_ERROR,
    O70S_STACK_FULL,
    O70S_BAD_STATE,
    O70S_BAD_IFUNC,

    O70S_OTHER,
    O70S_BUG = 0x40,
    O70S_TODO,
};

enum o70_builtin_object_indexes
{
    O70X_NULL,
    O70X_FALSE,
    O70X_TRUE,
    O70X_DYNOBJ_CLASS,
    O70X_CLASS_CLASS,
    O70X_NULL_CLASS,
    O70X_BOOL_CLASS,
    O70X_INT_CLASS,
    O70X_ARRAY_CLASS,
    O70X_FUNCTION_CLASS,
    O70X_IFUNC_CLASS,
    O70X_STR_CLASS,
    O70X_CTSTR_CLASS,
    O70X_ACTSTR_CLASS,
    O70X_ICTSTR_CLASS,
    O70X_IACTSTR_CLASS,
    O70X_EXCEPTION_CLASS,
    O70X_MODULE_CLASS,
    O70X_ICODE_CLASS,
    O70X_NULL_ICTSTR,
    O70X_FALSE_ICTSTR,
    O70X_TRUE_ICTSTR,
    O70X_NULL_CLASS_ICTSTR,
    O70X_BOOL_ICTSTR,
    O70X_INT_ICTSTR,
    O70X_DYNOBJ_ICTSTR,
    O70X_CLASS_ICTSTR,
    O70X_ARRAY_ICTSTR,
    O70X_FUNCTION_ICTSTR,
    O70X_IFUNC_ICTSTR,
    O70X_STR_ICTSTR,
    O70X_CTSTR_ICTSTR,
    O70X_ACTSTR_ICTSTR,
    O70X_ICTSTR_ICTSTR,
    O70X_IACTSTR_ICTSTR,
    O70X_EXCEPTION_ICTSTR,
    O70X_MODULE_ICTSTR,
    O70X_ICODE_ICTSTR,

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

/* o70_dynobj_t *************************************************************/
/**
 *  Standard object structure.
 *  This is for objects that act as dynamic property bags.
 */
typedef struct o70_dynobj_s o70_dynobj_t;

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

/* o70_ifunc_t **************************************************************/
/**
 *  Interpreted function
 */
typedef struct o70_ifunc_s o70_ifunc_t;

/* o70_icode_t **************************************************************/
/**
 *  Inerpreted code.
 */
typedef struct o70_icode_s o70_icode_t;

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

/* o70_exectx_t *************************************************************/
/**
 *  Execution context - instance data of a function containing the state
 *  of execution and local variables.
 */
typedef struct o70_exectx_s o70_exectx_t;

/* o70_ifunc_exectx_t *******************************************************/
/**
 *  Execution context for interpreted functions.
 */
typedef struct o70_ifunc_exectx_s o70_ifunc_exectx_t;

/* o70_ehi_t ****************************************************************/
/**
 *  Exception handler item.
 */
typedef struct o70_ehi_s o70_ehi_t;

/* o70_ehc_t ****************************************************************/
/**
 *  Exception handler chain.
 */
typedef struct o70_ehc_s o70_ehc_t;

/* o70_insn_t ***************************************************************/
/**
 *  Insn details.
 */
typedef struct o70_insn_s o70_insn_t;

/* o70_object_t ***********************************************************/
/**
 *  Slim object (object without properties).
 */
typedef struct o70_object_s o70_object_t;

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

/* o70_func_exec_f **********************************************************/
/**
 *  Function handler that executes a portion of code for a script function.
 * @retval O70S_OK function finished execution normally, the current execution
 *  context should be popped from the stack
 * @retval O70S_PENDING a script sub-function was called so we return here
 *  so that the flow manager calls the handler for that function
 * @retval O70S_THROW exception thrown
 */
typedef o70_status_t (C42_CALL * o70_func_exec_f)
    (
        o70_flow_t * flow,
        o70_exectx_t * e
    );

/* o70_func_init_f **********************************************************/
/**
 *  Function handler to init execution context.
 */
typedef o70_status_t (C42_CALL * o70_func_init_f)
    (
        o70_flow_t * flow,
        o70_exectx_t * e
    );


/* structs and unions {{{1 */

struct o70_flow_s
{
    o70_world_t * world; /**< pointer to the world */
    o70_ref_t * stack;
    /**< execution stack - references point to various exectx structures */
    o70_ref_t rv; /**< return value */
    o70_ref_t exc; /**< exception being thrown (or null) */
    unsigned int n; /**< number of execution contexts in the stack */
    unsigned int m; /**< number of allocated refs in the stack */
    unsigned int steps; /**< number of steps executed by the last function->exec
                            handler */
    c42_np_t wfl; /**< world flow links - list entry for all flows */
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
        int32_t nref;
        /**< number of references to the object; if this is
            negative it means the object is in the destroy list and should not
            have references to it from objects that are not collected as well;
            use ~ndx for the next object index in the destroy list.
            */
        int32_t ndx; /**< next destroy index */
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
    c42_rbtree_t rbt; /**< red/black tree; items are o70_prop_node_t */
};

struct o70_object_s
{
    o70_ohdr_t ohdr; /**< object header */
};

struct o70_dynobj_s
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

    o70_ref_t * sct; /**< superclass table */
    o70_ref_t * scn; /**< number of superclasses */
    size_t isize; /**< size in bytes of instances */
    // size_t class_size; /**< size in bytes of the class object */
    uint32_t model; /**< bitmask for primitive types compatible with instances
                         of this class */
    o70_ref_t name; /**< class name - for information purposes */
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

struct o70_exectx_s
{
    o70_ohdr_t ohdr; /**< object header;
        reference to function that runs the code in this context */

    uintptr_t xp; /**< execution pointer - where to execute from */

    o70_ref_t * lv;
    /**< array of local variables; this would typically point in the same
     * allocated block as this structure, after all extra fields used by
     * the function execution handler */
};

struct o70_function_s
{
    o70_class_t cls; /**< class data for the execution context */
    // o70_module_t * mod; /**< module containing the code */
    // uint32_t fx; /**< function index in the module */
    o70_ref_t parent;
    /**< parent execution context - this is the context of the code/function
     * that created this function, allowing us to access its variables;
     * for instance, if this function is declared in a module the parent
     * contains all module globals; if the function is a nested fuction,
     * the parent is the context of the function in which this one is declared
     **/
    o70_ref_t self;
    /**< self-reference provided for functions that receive the direct
     *   pointer */
    // o70_ref_t * idt; /**< table with ids for named variables, sorted by
    //                       identifier ref value; idn items */
    // uint32_t * sxt; /**< slot indexes corresponding to ids in the table above;
    //                      idn items */
    // uint32_t * asx; /**< arg slot index table; tells into which slot to put
    //                      the arguments to the function */
    o70_ref_t * ant; /**< arg name table - items are references to ictstr
                       instances */
    uint32_t * axt; /**< arg index table - table with indexes into the slots
                      array where to put those arguments */
    uint32_t * isx; /**< inited arg slot index table; tells into which slot
                         to put pre-inited args */
    o70_ref_t * ivt; /**< inited arg values */
    o70_func_init_f init; /**< handler for initing the function exectx */
    o70_func_exec_f exec; /**< handler that executes the function */
    void * context; /**< context for init/exec; interpreted functions can
                      store here the reference to their icode object */
    //size_t idn; /**< number of ids */
    size_t sn; /**< number of slots in the execution context */
    size_t an; /**< number of arguments */
    size_t ivn; /**< number of inited args */
};

struct o70_insn_s
{
    uint8_t opcode; /**< opcode */
    uint8_t ecx; /**< exception chain index */
    uint16_t ax; /**< index in the opcode args array */
};

struct o70_ifunc_exectx_s
{
    o70_exectx_t exectx;
    o70_ref_t lv[0]; /**< local vars; the field .exectx.lv should point here */
};

struct o70_ifunc_s
{
    o70_function_t func;
    o70_insn_t * it; /**< insn table */
    uint16_t * at; /**< insn args table */
    o70_ref_t * ct; /**< const table */
    o70_ehi_t * eht; /**< exception handler table; this contains all the
                       exception chains concatenated */
    o70_ehc_t * ect; /**< exception handler chain table; */
    unsigned int in; /**< number of instructions */
    unsigned int an; /**< number of insn args */
    unsigned int cn; /**< number of constants in the const pool */
    unsigned int ehn; /**< number of exception handlers */
    unsigned int ecn; /**< number of exception chains; up to 255 since
                    o70_insn_t#ecx is byte */
    unsigned int im; /**< number of allocated instructions */
    unsigned int am; /**< number of allocated insn args */
    unsigned int cm; /**< number of allocated constants in the const pool */
    unsigned int ehm; /**< number of allocated exception handlers */
    unsigned int ecm; /**< number of allocated exception chains */
    uint8_t modifiable;
};

struct o70_icode_s
{
    o70_ohdr_t ohdr;
    o70_insn_t * it; /**< insn table */
    uint16_t * at; /**< insn args table */
    o70_ref_t * ct; /**< const table */
    o70_ehi_t * eht; /**< exception handler table; this contains all the
                       exception chains concatenated */
    o70_ehc_t * ect; /**< exception handler chain table; */
    unsigned int in; /**< number of instructions */
    unsigned int an; /**< number of insn args */
    unsigned int cn; /**< number of constants in the const pool */
    unsigned int ehn; /**< number of exception handlers */
    unsigned int ecn; /**< number of exception chains; up to 255 since
                    o70_insn_t#ecx is byte */
    unsigned int im; /**< number of allocated instructions */
    unsigned int am; /**< number of allocated insn args */
    unsigned int cm; /**< number of allocated constants in the const pool */
    unsigned int ehm; /**< number of allocated exception handlers */
    unsigned int ecm; /**< number of allocated exception chains */
    uint8_t modifiable;
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

struct o70_varctx_class_s
{
    o70_class_t cls; /**< standard class fields */
};

struct o70_ehi_s
{
    uint16_t ecx; /**< exception class index in constants table */
    uint16_t hix; /**< handler instruction index */
};

struct o70_ehc_s
{
    uint16_t fehx; /**< first exception handler index */
    uint16_t ehn; /**< number of exception handlers */
};

struct o70_module_s
{
    uint8_t * opc; /**< table of opcodes */
    uint32_t * ao; /**< arg offsets */
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

    c42_np_t wfl; /**< list of flows linked by their field o70_flow_t#wfl */
    o70_object_t null_obj; /**< null object */
    o70_object_t false_obj; /**< false object */
    o70_object_t true_obj; /**< true object */
    o70_class_t null_class; /**< null class */
    o70_class_t bool_class; /**< bool class */
    o70_class_t int_class; /**< int class */
    o70_class_t dynobj_class; /**< dynobj class */
    o70_class_t class_class; /**< class class */
    o70_class_t array_class; /**< array class */
    o70_class_t function_class; /**< function class */
    o70_class_t ifunc_class; /**< ifunc class */
    o70_class_t str_class; /**< str class */
    o70_class_t ctstr_class; /**< const str class */
    o70_class_t actstr_class; /**< allocated const str class */
    o70_class_t ictstr_class; /**< internalized const str class */
    o70_class_t iactstr_class; /**< internalized allocated const str class */
    o70_class_t exception_class; /**< exception class */
    o70_class_t icode_class; /**< icode class */
    o70_class_t module_class; /**< module class */
    o70_ctstr_t null_ictstr; /**< null ctstr */
    o70_ctstr_t false_ictstr; /**< false ctstr */
    o70_ctstr_t true_ictstr; /**< true ctstr */
    o70_ctstr_t null_class_ictstr; /**< null ctstr */
    o70_ctstr_t bool_ictstr; /**< bool ctstr */
    o70_ctstr_t int_ictstr; /**< int ctstr */
    o70_ctstr_t dynobj_ictstr; /**< object ctstr */
    o70_ctstr_t class_ictstr; /**< class ctstr */
    o70_ctstr_t array_ictstr; /**< array ctstr */
    o70_ctstr_t function_ictstr; /**< function ctstr */
    o70_ctstr_t ifunc_ictstr; /**< ifunc ctstr */
    o70_ctstr_t str_ictstr; /**< str ctstr */
    o70_ctstr_t ctstr_ictstr; /**< ctstr ctstr */
    o70_ctstr_t actstr_ictstr; /**< actstr ctstr */
    o70_ctstr_t ictstr_ictstr; /**< ictstr ctstr */
    o70_ctstr_t iactstr_ictstr; /**< iactstr ctstr */
    o70_ctstr_t exception_ictstr; /**< exception ctstr */
    o70_ctstr_t module_ictstr; /**< module ctstr */
    o70_ctstr_t icode_ictstr; /**< icode ctstr */
    o70_module_t mcore; /**< core module */
    o70_ehc_t empty_ehc; /**< the empty exception handler chain to which all
                           functions default to */
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

/* o70_is_valid_oref ********************************************************/
/**
 *  Returns non-zero if the given reference points to a used object, as opposed
 *  to an unused entry in the object table, or a fast int
 */
C42_INLINE int C42_CALL o70_is_valid_oref
(
    o70_world_t * w,
    o70_ref_t oref
)
{
    return O70_IS_OREF(oref)
        && O70_RTOX(oref) < w->on
        && (w->nfx[O70_RTOX(oref)] & 1) == 0;
}

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
    if (!O70_IS_OREF(oref)) return;
    O70_ASSERT(w, o70_is_valid_oref(w, oref));
    w->ohdr[O70_RTOX(oref)]->nref += 1;
}

O70_API o70_status_t C42_CALL _o70_obj_destroy (o70_world_t * w);

/* o70_ox_ref_dec ***********************************************************/
C42_INLINE o70_status_t o70_ox_ref_dec
(
    o70_world_t * w,
    o70_oidx_t ox
)
{
    /* most common branch: decrement ref count which remains positive */
    if (w->ohdr[ox]->nref > 1)
    {
        w->ohdr[ox]->nref -= 1;
        return 0;
    }

    /* if the object is already in the destroy chain then leave it untouched */
    if (ox < O70X__COUNT || w->ohdr[ox]->nref < 0) return 0;

    /* if we got here then the object just got its last reference removed;
     * we must add it to the destroy chain */
#if _DEBUG
    if (w->ohdr[ox]->nref != 1) return O70S_BUG;
    if (ox < O70X__COUNT) return O70S_BUG;
#endif
    w->ohdr[ox]->ndx = ~w->fdx;
    w->fdx = ox;

    return _o70_obj_destroy(w);
}

/* o70_ref_dec **************************************************************/
/**
 *  Decrements the number of refs to the given object.
 *  If there are no refs left, then the object will be destroyed.
 *  Object destructors cannot 'call' interpreted code therefore there is no
 *  need for a flow.
 *  Destroying the current object may cause the destruction of other objects
 *  however, no cycles can be created because the objects destroyed before
 *  had 0 refs (nobody still pointing to them).
 *  @retval 0 all ok
 *  @retval O70S_BUG bug
 */
C42_INLINE o70_status_t o70_ref_dec
(
    o70_world_t * w,
    o70_ref_t oref
)
{
    /* if not an obj ref then do nothing */
    if (!O70_IS_OREF(oref)) return 0;
    O70_ASSERT(w, o70_is_valid_oref(w, oref));
    return o70_ox_ref_dec(w, O70_RTOX(oref));
}

/* o70_class_ptr ************************************************************/
/**
 *  Returns the pointer to the class corresponding to the given object.
 *  @note this does not change the ref count for the returned class.
 */
C42_INLINE o70_class_t * o70_class_ptr
(
    o70_world_t * w,
    o70_ref_t r
)
{
    O70_ASSERT(w, !O70_IS_OREF(r) || o70_is_valid_oref(w, r));
    return O70_IS_OREF(r)
        ? w->ot[w->ohdr[O70_RTOX(r)]->class_ox]
        : &w->int_class;
}

/* o70_model ****************************************************************/
/**
 *  Returns the model of the given reference.
 */
C42_INLINE int o70_model
(
    o70_world_t * w,
    o70_ref_t r
)
{
    return o70_class_ptr(w, r)->model;
}

/* o70_ptr **************************************************************/
/**
 *  Returns the object pointer given a valid reference
 */
C42_INLINE void * o70_ptr
(
    o70_world_t * w,
    o70_ref_t r
)
{
    O70_ASSERT(w, o70_is_valid_oref(w, r));
    return w->ot[O70_RTOX(r)];
}

/* o70_obj_class_name *******************************************************/
/**
 *  Returns the reference to the ctstr specified as the name of this object's
 *  class.
 *  @note this does not increment the ref count of the returned object
 *  @warning the input reference must be valid or else...
 */
C42_INLINE o70_ref_t o70_obj_class_name
(
    o70_world_t * w,
    o70_ref_t obj
)
{
    return o70_class_ptr(w, obj)->name;
}

/* o70_flow_create **********************************************************/
/**
 *  Creates an execution flow.
 *  @param w [in]                   world
 *  @param flow_ptr [out]           receives the pointer to the allocated flow
 *  @retval O70S_OK                 all ok
 *  @retval O70S_BAD_ARG            invalid @a max_stack_depth
 *  @retval O70S_NO_MEM             allocation failed
 *  @retval O70S_BUG                allocator returned heap corruption
 */
O70_API o70_status_t C42_CALL o70_flow_create
(
    o70_world_t * w,
    o70_flow_t * * flow_ptr,
    unsigned int max_depth
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
 *  @note the returned reference will have its ref count incremented.
 */
O70_API o70_status_t C42_CALL o70_ctstr_intern
(
    o70_world_t * w,
    o70_ref_t in,
    o70_ref_t * out
);

/* o70_ctstr *********************************************************/
/**
 *  Allocates a new static constant string object.
 */
O70_API o70_status_t C42_CALL o70_ctstr
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
    (o70_ctstr((_w), (_ref_ptr), (_str_lit), sizeof(_str_lit) - 1))

/* o70_ictstr **************************************************/
/**
 *  Given a static byte buffer it returns an internalized ctstr object.
 */
O70_API o70_status_t C42_CALL o70_ictstr
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
    (o70_ictstr((_w), (_ref_ptr), (_str_lit), sizeof(_str_lit) - 1))

/* o70_dump_icst ************************************************************/
/**
 *  Dumps in escaped form all items from the internalised constant strings tree
 */
O70_API o70_status_t C42_CALL o70_dump_icst
(
    o70_world_t * w,
    c42_io8_t * o
);

/* o70_dynobj_create ********************************************************/
/**
 *  Creates a new <<dynobj>> instance with an empty set of dynamic fields.
 */
O70_API o70_status_t C42_CALL o70_dynobj_create
(
    o70_world_t * w,
    o70_ref_t * out
);

/* o70_dynobj_raw_get *******************************************************/
/**
 *  Retrieves a raw property of an object instance.
 *  The property must be in the object's property bag as this function does
 *  not run the property getter for the object.
 *  @param w [in] world
 *  @param obj [in] any instance derived from object
 *  @param name [in] an internalised const string representing property name
 *  @param value [out] will get value of property on success
 *  @retval 0 property found and value returned
 *  @retval O70S_MISSING property not found
 *  @retval O70S_BUG critical error
 */
O70_API o70_status_t C42_CALL o70_dynobj_raw_get
(
    o70_world_t * w,
    o70_ref_t obj,
    o70_ref_t name,
    o70_ref_t * value
);

/* o70_dynobj_raw_put *******************************************************/
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
 *  @retval 0 ok
 *  @retval O70S_NO_MEM failed allocating memory to store the new property
 *  @retval O70S_BUG bug found; don't touch anything, exit asap
 */
O70_API o70_status_t C42_CALL o70_dynobj_raw_put
(
    o70_world_t * w,
    o70_ref_t obj,
    o70_ref_t name,
    o70_ref_t value
);

/* o70_ctstr_len ************************************************************/
/**
 *  Convenience function that returns string length.
 *
 */
O70_API size_t C42_CALL o70_ctstr_len
(
    o70_world_t * w,
    o70_ref_t r
);

/* o70_ctstr_data ***********************************************************/
/**
 *  Convenience function that returns a pointer to string data (or NULL if
 *  not a string or empty string).
 */
O70_API uint8_t * C42_CALL o70_ctstr_data
(
    o70_world_t * w,
    o70_ref_t r
);

/* o70_str_create ***********************************************************/
/**
 *  Creates an empty string object
 */
O70_API o70_status_t C42_CALL o70_str_create
(
    o70_world_t * w,
    o70_ref_t * obj_p
);

/* o70_str_vafmt ************************************************************/
/**
 *  Appends a formatted string.
 *  See c42_write_vfmt() in c42 library for formatting rules.
 */
O70_API o70_status_t C42_CALL o70_str_vafmt
(
    o70_world_t * w,
    o70_ref_t r,
    char const * fmt,
    va_list va
);

/* o70_str_afmt *************************************************************/
/**
 *  Appends a formatted string.
 *  See c42_write_vfmt() in c42 library for formatting rules.
 */
O70_API o70_status_t C42_CALL o70_str_afmt
(
    o70_world_t * w,
    o70_ref_t r,
    char const * fmt,
    ...
);

/* o70_str_len **************************************************************/
/**
 *  Convenience function that returns string length.
 *
 */
O70_API size_t C42_CALL o70_str_len
(
    o70_world_t * w,
    o70_ref_t r
);

/* o70_str_data *************************************************************/
/**
 *  Convenience function that returns a pointer to string data (or NULL if
 *  not a string or empty string).
 */
O70_API uint8_t * C42_CALL o70_str_data
(
    o70_world_t * w,
    o70_ref_t r
);

/* o70_str_append_obj_short_desc ********************************************/
/**
 *  Append short description.
 *  @param w world
 *  @param str string object where description should be appended to
 *  @param obj object to be described
 */
O70_API o70_status_t C42_CALL o70_str_append_obj_short_desc
(
    o70_world_t * w,
    o70_ref_t str,
    o70_ref_t obj
);

/* o70_obj_short_desc *******************************************************/
/**
 *  Generates a string with a short description for the given object.
 */
O70_API o70_status_t C42_CALL o70_obj_short_desc
(
    o70_world_t * w,
    o70_ref_t obj,
    o70_ref_t * out
);

/* o70_dump_object_map ******************************************************/
/**
 *  Outputs a list of all object in the given world.
 */
O70_API o70_status_t C42_CALL o70_dump_object_map
(
    o70_world_t * w,
    c42_io8_t * io
);

/* o70_ifunc_create *********************************************************/
/**
 *  Creates an empty function.
 */
O70_API o70_status_t C42_CALL o70_ifunc_create
(
    o70_world_t * w,
    o70_ref_t * out,
    size_t sn
);

/* o70_ifunc_ita ************************************************************/
/**
 *  Reallocates the instruction table.
 */
O70_API o70_status_t C42_CALL o70_ifunc_ita
(
    o70_world_t * w,
    o70_ifunc_t * ifunc,
    unsigned int im
);

/* o70_ifunc_ata ************************************************************/
/**
 *  Resizes the insn arg table.
 */
O70_API o70_status_t C42_CALL o70_ifunc_ata
(
    o70_world_t * w,
    o70_ifunc_t * ifunc,
    unsigned int am
);

/* o70_ifunc_cta ************************************************************/
/**
 *  Resizes the const table.
 */
O70_API o70_status_t C42_CALL o70_ifunc_cta
(
    o70_world_t * w,
    o70_ifunc_t * ifunc,
    unsigned int cm
);

/* o70_ifunc_eha ************************************************************/
/**
 *  Resizes the exception handler table.
 */
O70_API o70_status_t C42_CALL o70_ifunc_eha
(
    o70_world_t * w,
    o70_ifunc_t * ifunc,
    unsigned int ehm
);

/* o70_ifunc_eca ************************************************************/
/**
 *  Resizes the exception chain table.
 */
O70_API o70_status_t C42_CALL o70_ifunc_eca
(
    o70_world_t * w,
    o70_ifunc_t * ifunc,
    unsigned int ecm
);

/* o70_ifunc_append_ret *****************************************************/
/**
 *  Appends a ret instruction.
 */
O70_API o70_status_t C42_CALL o70_ifunc_append_ret
(
    o70_world_t * w,
    o70_ifunc_t * ifunc,
    unsigned int sx
);

/* o70_ifunc_add_const ******************************************************/
/**
 *  Adds a reference to the constant table of an interpreted function.
 */
O70_API o70_status_t C42_CALL o70_ifunc_add_const
(
    o70_world_t * w,
    o70_ifunc_t * ifunc,
    o70_ref_t value,
    uint16_t * ax
);

/* o70_ifunc_append_ret_const ***********************************************/
O70_API o70_status_t C42_CALL o70_ifunc_append_ret_const
(
    o70_world_t * w,
    o70_ifunc_t * ifunc,
    o70_ref_t value
);

/* o70_push_call ************************************************************/
/**
 *  Pushes a new execution context for the given function.
 */
O70_API o70_status_t C42_CALL o70_push_call
(
    o70_flow_t * flow,
    o70_ref_t func
);

/* o70_exec *****************************************************************/
/**
 *  Executes a flow.
 *  @retval 0 executed successfully until the depth of the call stack got
 *            under @a stop_depth
 *  @retval O70S_EXC and exception has been thrown which caused the stack
 *          to unwind beyond @a stop_depth
 *  @retval O70S_PENDING reached @a steps_limit
 *  @retval O70S_BAD_ARG @a stop_depth was passed as 0
 *  @note to execute until the flow is finished pass 1 for @a stop_depth
 *  @note to execute a function call until it returns use o70_push_call()
 *      followed by o70_exec(flow, flow->n, ...)
 */
O70_API o70_status_t C42_CALL o70_exec
(
    o70_flow_t * flow,
    unsigned int stop_depth,
    unsigned int steps_limit
);

/* o70_opcode_name **********************************************************/
/**
 *  Returns a static string with the name of the opcode.
 */
O70_API char * C42_CALL o70_opcode_name
(
    uint8_t opcode
);

/* }}}1 */

#endif

