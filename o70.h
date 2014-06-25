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
 *  This is a pair of (identifier, value) with indexing information for quick
 *  lookup.
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

/* o70_ctstr_s **************************************************************/
/**
 *  Constant string.
 */
typedef struct o70_ctstr_s o70_ctstr_t;

/* o70_id_t *****************************************************************/
/**
 *  Identifier.
 */
typedef struct o70_id_s o70_id_t;

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

/* o70_obj_finish_f *********************************************************/
/**
 *  Finishes an object.
 *  This is responsible for cleaning up references to other objects and
 *  releasing any native resources held by the object.
 *  @retval O70S_OK     init ok
 */
typedef o70_status_t (C42_CALL * o70_obj_finish_f)
    (
        o70_flow_t * flow,
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

    uint64_t ma_total_limit; /**< max total memory to allocate */
    uint64_t ma_block_limit; /**< max size of any allocated block */
    uint64_t ma_count_limit; /**< max number of blocks to allocate */

    c42_io8_t * in; /**< standard input */
    c42_io8_t * out; /**< standard output */
    c42_io8_t * err; /**< standard error */
};

struct o70_ohdr_s
{
    uint32_t ref_count; /**< number of references to the object */
    uint32_t class_ox; /**< object index for current object's class */
};

struct o70_kv_s
{
    o70_ref_t id; /**< id */
    o70_ref_t val; /**< value */
};

struct o70_prop_node_s
{
    o70_kv_t kv; /**< payload data (id + value) */
    c42_rbtree_node_t rbtn; /**< red/black tree node */
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
    /**< set property */

    void * finish_context; /**< context for o70_class_t#finish */
    void * prop_context;
    /**< context for o70_class_t#get_prop and o70_class_t#set_prop */

    // size_t instance_size; /**< size in bytes of instances */
    // size_t class_size; /**< size in bytes of the class object */
};

struct o70_ctstr_s
{
    o70_ohdr_t ohdr; /**< object header */
    uint8_t const * data; /**< string data pointer */
    size_t len; /**< string length in bytes */
};

struct o70_id_s
{
    o70_ohdr_t ohdr; /**< object header */
    o70_ref_t ctstr; /**< const string containing the name of the identifier */
    c42_rbtree_node_t rbtn; /**< rbtree node */
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
        uintptr_t * nfx; /**< next free index table */
    };
    o70_module_t * * mod; /**< array of pointers to modules */

    c42_rbtree_t idt; /**< tree of identifiers */

    size_t on; /**< number of inited object entries */
    size_t om; /**< number of allocated object entries */
    size_t ffx; /**< first free index */

    size_t mn; /**< number of used modules */
    size_t mm; /**< number of allocated modules */

    size_t fn; /**< number of flows */

    c42_ma_t * ma; /**< memory allocator */

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
    o70_class_t id_class; /**< id class */
    o70_class_t array_class; /**< array class */
    o70_class_t function_class; /**< function class */
    o70_class_t string_class; /**< string class */
    o70_class_t ctstring_class; /**< const string class */
    o70_class_t exception_class; /**< exception class */
    o70_class_t module_class; /**< module class */
    o70_slim_obj_t null_id; /**< null id */
    o70_slim_obj_t false_id; /**< false id */
    o70_slim_obj_t true_id; /**< true id */
    o70_class_t null_class_id; /**< null id */
    o70_class_t bool_id; /**< bool id */
    o70_class_t int_id; /**< int id */
    o70_class_t object_id; /**< object id */
    o70_class_t class_id; /**< class id */
    o70_class_t id_id; /**< id id */
    o70_class_t array_id; /**< array id */
    o70_class_t function_id; /**< function id */
    o70_class_t string_id; /**< string id */
    o70_class_t ctstring_id; /**< const string id */
    o70_class_t exception_id; /**< exception id */
    o70_class_t module_id; /**< module id */
};
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
    O70X_IDENTIFIER_CLASS,
    O70X_STRING_CLASS,
    O70X_CTSTRING_CLASS,
    O70X_EXCEPTION_CLASS,
    O70X_MODULE_CLASS,
};

/* o70_lib_name *************************************************************/
/**
 *  Returns a static string identifying the library with its configuration.
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
 *  This should be called after a successful call to o70_world_init() and
 *  it can safely be called multiple times after successful/unsuccessful calls
 *  to o70_world_init().
 *  @retval O70S_OK         everything went fine
 *  @retval O70S_BUG        allocator returned heap corruption
 */
O70_API o70_status_t C42_CALL o70_world_finish
(
    o70_world_t * w
);

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

#endif

