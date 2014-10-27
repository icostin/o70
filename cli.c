#include <c42.h>
#include <o70.h>

#define RCINI 1
#define RCRUN 2
#define RCFIN 4
#define RCOUT 8
#define RCBUG 127

#define E(...) ((void) (c42_io8_fmt(err, "$s:$i: ", __FILE__, __LINE__) \
    || c42_io8_fmt(err, __VA_ARGS__)))

#define ZOB(_expr) \
    if (!(_expr)) ; else { E("expr ($s) is not zero\n", #_expr); break; }
#define TOB(_expr) \
    if ((_expr)) ; else { E("expr ($s) is not zero\n", #_expr); break; }

typedef struct test_s test_t;
struct test_s
{
    uint_fast8_t C42_CALL (* fn) (c42_svc_t * , c42_clia_t * );
    char const * name;
};

static uint_fast8_t C42_CALL run_test (c42_svc_t * svc, c42_clia_t * clia);
static uint_fast8_t C42_CALL test0 (c42_svc_t * svc, c42_clia_t * clia);
static uint_fast8_t C42_CALL test1 (c42_svc_t * svc, c42_clia_t * clia);
static uint_fast8_t C42_CALL test2 (c42_svc_t * svc, c42_clia_t * clia);

static test_t tests[] =
{
    { test0, "ctstr" },
    { test1, "prop" },
    { test2, "flow" },
};

/* c42_main *****************************************************************/
uint_fast8_t C42_CALL c42_main
(
    c42_svc_t * svc,
    c42_clia_t * clia
)
{
    int e;
    e = run_test(svc, clia);
    if (e) return e;

    return 0;
}

/* test0 ********************************************************************/
static uint_fast8_t C42_CALL test0 (c42_svc_t * svc, c42_clia_t * clia)
{
    o70_init_t ini;
    o70_world_t w;
    c42_io8_t * out = &clia->stdio.out;
    c42_io8_t * err = &clia->stdio.err;
    o70_status_t os, osf;
    uint_fast8_t rc = 0;

    C42_VAR_CLEAR(ini);
    ini.ma = &svc->ma;
    ini.out = out;
    ini.err = err;
    os = o70_world_init(&w, &ini);
    if (os)
    {
        c42_io8_fmt(err, "o70 test0: failed initialising world: $s = $b\n",
                    o70_status_name(os), os);
        return RCINI;
    }
    do
    {
        o70_ref_t rj1, rj2, rj3, rb;
        os = 0;

        os = O70_CTSTR(&w, &rj1, "jeton");
        if (os)
        {
            c42_io8_fmt(err, "test0: failed creating static constant string "
                        "'jeton': $s = $b\n", o70_status_name(os), os);
            break;
        }
        c42_io8_fmt(out, "rj1: $xd\n", rj1);

        os = O70_ICTSTR(&w, &rb, "beton");
        if (os)
        {
            c42_io8_fmt(err, "test0: failed creating internalised static "
                        "constant string 'beton': $s = $b\n", o70_status_name(os),
                        os);
            break;
        }
        c42_io8_fmt(out, "rb: $xd\n", rb);

        os = O70_ICTSTR(&w, &rj2, "jeton");
        if (os)
        {
            c42_io8_fmt(err, "test0: failed creating internalised static "
                        "constant string 'beton': $s = $b\n", o70_status_name(os),
                        os);
            break;
        }
        c42_io8_fmt(out, "rj2: $xd\n", rj2);

        os = o70_ctstr_intern(&w, rj1, &rj3);
        if (os)
        {
            c42_io8_fmt(err, "test0: failed interning rj1: $i\n", os);
            break;
        }
        c42_io8_fmt(err, "rj3: $xd\n", rj3);

        os = o70_ref_dec(&w, rb);
        if (os)
        {
            c42_io8_fmt(err, "test0: failed dereferencing 'beton'\n");
            break;
        }
        os = o70_ref_dec(&w, rj1);
        if (os)
        {
            c42_io8_fmt(err, "test0: failed dereferencing 'jeton'\n");
            break;
        }
        os = o70_ref_dec(&w, rj2);
        if (os)
        {
            c42_io8_fmt(err, "test0: failed dereferencing intern 'jeton'\n");
            break;
        }

        os = o70_dump_icst(&w, out);
    }
    while (0);
    if (os == O70S_BUG) return RCBUG;
    if (os) rc |= RCRUN;
    osf = o70_world_finish(&w);
    if (osf)
    {
        c42_io8_fmt(err, "o70 test0: failed finishing world: $s = $b\n",
                    o70_status_name(osf), osf);
        rc |= RCFIN;
    }

    // rc = (os ? RCRUN : 0) | (osf ? RCFIN : 0);
    // if (c42_io8_fmt(out, "$s test $i\n", rc ? "fail" : "pass", 1)) rc |= RCOUT;
    return rc;
}

/* test1 ********************************************************************/
static uint_fast8_t C42_CALL test1 (c42_svc_t * svc, c42_clia_t * clia)
{
    o70_init_t ini;
    o70_world_t w;
    c42_io8_t * out = &clia->stdio.out;
    c42_io8_t * err = &clia->stdio.err;
    o70_status_t os, osf;
    o70_ref_t obj, obk, afi, bfi, cfi, v;
    uint_fast8_t rc = 0, ok = 0;

    C42_VAR_CLEAR(ini);
    ini.ma = &svc->ma;
    ini.out = out;
    ini.err = err;
    os = o70_world_init(&w, &ini);
    if (os)
    {
        c42_io8_fmt(err, "o70 test1: failed initialising world: $s = $b\n",
                    o70_status_name(os), os);
        return RCINI;
    }
    do
    {
        ZOB(os = o70_dynobj_create(&w, &obj));
        ZOB(os = O70_CTSTR(&w, &afi, "a_field"));
        ZOB(os = o70_dynobj_raw_put(&w, obj, afi, O70R_NULL));
        ZOB(os = o70_dynobj_raw_put(&w, obj, afi, O70_FITOR(1)));
        ZOB(os = O70_CTSTR(&w, &bfi, "bfi"));
        ZOB(os = o70_dynobj_raw_put(&w, obj, bfi, O70R_TRUE));
        ZOB(os = O70_CTSTR(&w, &cfi, "cfld"));
        ZOB(os = o70_dynobj_raw_put(&w, obj, cfi, O70R_FALSE));
        ZOB(os = o70_dynobj_raw_get(&w, obj, cfi, &v));
        TOB(v == O70R_FALSE);
        ZOB(os = o70_dynobj_raw_get(&w, obj, bfi, &v));
        TOB(v == O70R_TRUE);
        ZOB(os = o70_dynobj_raw_get(&w, obj, afi, &v));
        TOB(v == O70_FITOR(1));
        TOB(w.ohdr[O70_RTOX(obj)]->nref == 1);
        ZOB(os = o70_obj_short_desc(&w, obj, &obk));
        ZOB(c42_io8_fmt(out, "o70 test1: obj desc: \"$.*s\"\n",
                        o70_str_len(&w, obk), o70_str_data(&w, obk)));
        ZOB(os = o70_dynobj_raw_put(&w, obj, cfi, obk));
        ZOB(os = o70_ref_dec(&w, obk));
        ZOB(os = o70_dump_object_map(&w, w.out));
        ZOB(c42_io8_fmt(out, "==============\n"));
        ZOB(os = o70_ref_dec(&w, obj));
        TOB(w.ffx = O70_RTOX(obj));
        ZOB(os = o70_str_create(&w, &obj));
        ZOB(os = o70_str_afmt(&w, obj, "I'm s$Xd.", obj));
        ZOB(c42_io8_fmt(out, "o70 test1: str fmt: $.*s (len: $z)\n",
                        o70_str_len(&w, obj), o70_str_data(&w, obj),
                        o70_str_len(&w, obj)));
        ZOB(os = o70_dump_object_map(&w, w.out));
        ok = 1;
    }
    while (0);
    if (os == O70S_BUG) return RCBUG;
    if (!ok)
    {
        c42_io8_fmt(err, "o70 test1: last error $s = $b\n",
                    o70_status_name(os), os);
        rc |= RCRUN;
    }
    osf = o70_world_finish(&w);
    if (osf)
    {
        c42_io8_fmt(err, "o70 test1: failed finishing world: $s = $b\n",
                    o70_status_name(osf), osf);
        rc |= RCFIN;
    }

    // rc = (os ? RCRUN : 0) | (osf ? RCFIN : 0);
    // if (c42_io8_fmt(out, "$s test $i\n", rc ? "fail" : "pass", 2)) rc |= RCOUT;
    return rc;
}

/* test2 ********************************************************************/
static uint_fast8_t C42_CALL test2 (c42_svc_t * svc, c42_clia_t * clia)
{
    o70_init_t ini;
    o70_world_t w;
    o70_flow_t * flow;
    c42_io8_t * out = &clia->stdio.out;
    c42_io8_t * err = &clia->stdio.err;
    o70_status_t os, osf;
    uint_fast8_t rc = 0, ok = 0;
    o70_ref_t fr;
    o70_ifunc_t * ifunc;

    C42_VAR_CLEAR(ini);
    ini.ma = &svc->ma;
    ini.out = out;
    ini.err = err;
    os = o70_world_init(&w, &ini);
    if (os)
    {
        c42_io8_fmt(err, "o70 test: failed initialising world: $s = $b\n",
                    o70_status_name(os), os);
        return RCINI;
    }
    do
    {
        ZOB(os = o70_flow_create(&w, &flow, 2));
        ZOB(os = o70_ifunc_create(&w, &fr, 2));
        ifunc = o70_ptr(&w, fr);
        ZOB(os = o70_ifunc_append_ret_const(&w, ifunc, O70R_TRUE));
        ZOB(os = o70_push_call(flow, fr));
        ZOB(os = o70_exec(flow, flow->n, 0x80000000));
        ok = 1;
    }
    while (0);
    if (os == O70S_BUG) return RCBUG;
    if (!ok)
    {
        c42_io8_fmt(err, "o70 test: last error $s = $b\n",
                    o70_status_name(os), os);
        rc |= RCRUN;
    }
    osf = o70_world_finish(&w);
    if (osf)
    {
        c42_io8_fmt(err, "o70 test: failed finishing world: $s = $b\n",
                    o70_status_name(osf), osf);
        rc |= RCFIN;
    }

    return rc;
}

/* run_test *****************************************************************/
static uint_fast8_t C42_CALL run_test (c42_svc_t * svc, c42_clia_t * clia)
{
    unsigned int e, rc, t;
    // o70_world_t w;
    // o70_init_t ini;
    c42_io8_t * out = &clia->stdio.out;
    //c42_io8_t * err = &clia->stdio.err;

    e = c42_io8_fmt(out, "o70: ostropel scripting tool\n"
                    "* libs: $s, $s\n* svc provider: $s\n",
                    o70_lib_name(), c42_lib_name(), svc->provider);
    if (e) return RCOUT;

    for (t = 0; t < C42_ARRAY_LIT_COUNT(tests); ++t)
    {
        rc = tests[t].fn(svc, clia);
        if (c42_io8_fmt(out, "test #$i $<16s => $s\n", t, tests[t].name,
                        rc ? "FAIL" : "pass"))
            rc |= RCOUT;
        if (rc) break;
    }

    return rc;
}

