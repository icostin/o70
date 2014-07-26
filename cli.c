#include <c42.h>
#include <o70.h>

static uint_fast8_t C42_CALL run_test (c42_svc_t * svc, c42_clia_t * clia);

/* c42_main *****************************************************************/
uint_fast8_t C42_CALL c42_main
(
    c42_svc_t * svc,
    c42_clia_t * clia
)
{
    int e;
    e = run_test(svc, clia);
    if (e) return 1;
    return 0;
}

/* test1 ********************************************************************/
static uint_fast8_t C42_CALL test1 (o70_world_t * w)
{
    o70_status_t os;
    o70_ref_t r;

    os = O70_SCS(w, &r, "jeton");
    if (os)
    {
        c42_io8_fmt(w->err, "test1: failed creating static constant string "
                    "'jeton': $s = $b\n", o70_status_name(os), os);
        return 1;
    }

    os = o70_ref_dec(w, r);
    if (os)
    {
        c42_io8_fmt(w->err, "test1: failed dereferencing 'jeton'\n");
        return 1;
    }

    os = O70_ISCS(w, &r, "beton");
    if (os)
    {
        c42_io8_fmt(w->err, "test1: failed creating internalised static "
                    "constant string 'beton': $s = $b\n", o70_status_name(os),
                    os);
        return 1;
    }

    os = o70_dump_icst(w, w->out);
    if (os) return 1;

    return 0;
}

/* run_test *****************************************************************/
static uint_fast8_t C42_CALL run_test (c42_svc_t * svc, c42_clia_t * clia)
{
    int e, r;
    o70_world_t w;
    o70_init_t ini;
    c42_io8_t * out = &clia->stdio.out;
    c42_io8_t * err = &clia->stdio.err;

    e = c42_io8_fmt(out, "o70: ostropel scripting tool\n"
                    "* libs: $s, $s\n* svc provider: $s\n",
                    o70_lib_name(), c42_lib_name(), svc->provider);
    if (e) return 0x40;

    C42_VAR_CLEAR(ini);
    ini.ma = &svc->ma;
    ini.out = out;
    ini.err = err;
    e = o70_world_init(&w, &ini);
    if (e)
    {
        c42_io8_fmt(err, "o70 test: failed initialising world: $s = $b\n", 
                    o70_status_name(e), e);
        return 1;
    }

    do
    {
        r = test1(&w);
        if (r) c42_io8_fmt(err, "o70 test: test #1 failed\n");
    }
    while (0);

    e = o70_world_finish(&w);
    if (e)
    {
        c42_io8_fmt(err, "o70 test: failed finishing world: $b\n", e);
        r |= 1;
    }

    return r;
}

