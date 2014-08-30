#include <c42.h>
#include <o70.h>

#define RCINI 1
#define RCRUN 2
#define RCFIN 4
#define RCOUT 8
#define RCBUG 127

typedef struct test_s test_t;
struct test_s
{
    uint_fast8_t C42_CALL (* fn) (c42_svc_t * , c42_clia_t * );
    char const * name;
};

static uint_fast8_t C42_CALL run_test (c42_svc_t * svc, c42_clia_t * clia);
static uint_fast8_t C42_CALL test1 (c42_svc_t * svc, c42_clia_t * clia);
static uint_fast8_t C42_CALL test2 (c42_svc_t * svc, c42_clia_t * clia);

static test_t tests[] =
{
    { test1, "ctstr" },
    { test2, "prop" },
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

/* test1 ********************************************************************/
static uint_fast8_t C42_CALL test1 (c42_svc_t * svc, c42_clia_t * clia)
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
        c42_io8_fmt(err, "o70 test2: failed initialising world: $s = $b\n", 
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
            c42_io8_fmt(err, "test1: failed creating static constant string "
                        "'jeton': $s = $b\n", o70_status_name(os), os);
            break;
        }
        c42_io8_fmt(out, "rj1: $xd\n", rj1);

        os = O70_ICTSTR(&w, &rb, "beton");
        if (os)
        {
            c42_io8_fmt(err, "test1: failed creating internalised static "
                        "constant string 'beton': $s = $b\n", o70_status_name(os),
                        os);
            break;
        }
        c42_io8_fmt(out, "rb: $xd\n", rb);

        os = O70_ICTSTR(&w, &rj2, "jeton");
        if (os)
        {
            c42_io8_fmt(err, "test1: failed creating internalised static "
                        "constant string 'beton': $s = $b\n", o70_status_name(os),
                        os);
            break;
        }
        c42_io8_fmt(out, "rj2: $xd\n", rj2);

        os = o70_ctstr_intern(&w, rj1, &rj3);
        if (os)
        {
            c42_io8_fmt(err, "test1: failed interning rj1: $i\n", os);
            break;
        }
        c42_io8_fmt(err, "rj3: $xd\n", rj3);

        os = o70_ref_dec(&w, rb);
        if (os)
        {
            c42_io8_fmt(err, "test1: failed dereferencing 'beton'\n");
            break;
        }
        os = o70_ref_dec(&w, rj1);
        if (os)
        {
            c42_io8_fmt(err, "test1: failed dereferencing 'jeton'\n");
            break;
        }
        os = o70_ref_dec(&w, rj2);
        if (os)
        {
            c42_io8_fmt(err, "test1: failed dereferencing intern 'jeton'\n");
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
        c42_io8_fmt(err, "o70 test1: failed finishing world: $s = $b\n",
                    o70_status_name(osf), osf);
    }

    // rc = (os ? RCRUN : 0) | (osf ? RCFIN : 0);
    // if (c42_io8_fmt(out, "$s test $i\n", rc ? "fail" : "pass", 1)) rc |= RCOUT;
    return rc;
}

/* test2 ********************************************************************/
static uint_fast8_t C42_CALL test2 (c42_svc_t * svc, c42_clia_t * clia)
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
        c42_io8_fmt(err, "o70 test2: failed initialising world: $s = $b\n", 
                    o70_status_name(os), os);
        return RCINI;
    }
    do
    {
        os = 0;
    }
    while (0);
    if (os == O70S_BUG) return RCBUG;
    osf = o70_world_finish(&w);
    if (osf)
    {
        c42_io8_fmt(err, "o70 test2: failed finishing world: $s = $b\n",
                    o70_status_name(osf), osf);
    }

    // rc = (os ? RCRUN : 0) | (osf ? RCFIN : 0);
    // if (c42_io8_fmt(out, "$s test $i\n", rc ? "fail" : "pass", 2)) rc |= RCOUT;
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


    // C42_VAR_CLEAR(ini);
    // ini.ma = &svc->ma;
    // ini.out = out;
    // ini.err = err;
    // e = o70_world_init(&w, &ini);
    // if (e)
    // {
    //     c42_io8_fmt(err, "o70 test: failed initialising world: $s = $b\n", 
    //                 o70_status_name(e), e);
    //     return 1;
    // }

    // do
    // {
    //     r = test1(svc, clia);
    //     if (r) c42_io8_fmt(err, "o70 test: test #1 failed\n");
    //     r = test2(svc, clia);
    //     if (r) c42_io8_fmt(err, "o70 test: test #2 failed\n");
    // }
    // while (0);

    // e = o70_world_finish(&w);
    // if (e)
    // {
    //     c42_io8_fmt(err, "o70 test: failed finishing world: $b\n", e);
    //     r |= 1;
    // }

    return rc;
}

