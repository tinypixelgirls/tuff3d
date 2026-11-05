/* Core geometry tests: cc -O1 -Isrc tests/test_mesh.c src/mesh.c -lm && ./a.out */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mesh.h"

static int fails;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); fails++; } } while (0)

/* every edge used exactly once in each direction => closed + consistently wound */
static int closed_oriented(const Mesh *m)
{
    for (int e = 0; e < m->ne; e++) {
        int a = m->ev[2 * e], b = m->ev[2 * e + 1], ab = 0, ba = 0;
        for (int f = 0; f < m->nf; f++) {
            int o = m->fo[f], n = m->fo[f + 1] - o;
            for (int k = 0; k < n; k++) {
                int x = m->fi[o + k], y = m->fi[o + (k + 1) % n];
                ab += x == a && y == b;
                ba += x == b && y == a;
            }
        }
        if (ab != 1 || ba != 1) return 0;
    }
    return 1;
}

static float volume(const Mesh *m)
{
    float v = 0;
    for (int f = 0; f < m->nf; f++) {
        int o = m->fo[f], n = m->fo[f + 1] - o;
        V3 p0 = m->p[m->fi[o]];
        for (int k = 1; k + 1 < n; k++)
            v += v3dot(p0, v3cross(m->p[m->fi[o + k]], m->p[m->fi[o + k + 1]])) / 6.0f;
    }
    return v;
}


static Mesh cube(void)
{
    Prim p;
    prim_defaults(&p, PRIM_CUBE);
    return mesh_prim(&p);
}

static Mesh plane(int n)
{
    Prim p;
    prim_defaults(&p, PRIM_PLANE);
    p.psw = p.psd = n;
    return mesh_prim(&p);
}

static int count(const unsigned char *f, int n)
{
    int c = 0;
    for (int i = 0; i < n; i++) c += f[i] != 0;
    return c;
}

/* the modeling ops added with the hard/soft edge work */
static void test_ops(void)
{
    Mesh m = cube();
    CHECK(mesh_reverse(&m, NULL) == 6 && volume(&m) < -0.99f);
    mesh_reverse(&m, NULL);
    CHECK(closed_oriented(&m) && volume(&m) > 0.99f);

    /* triangulate and join back */
    CHECK(mesh_triangulate(&m, NULL) == 6 && m.nf == 12 && closed_oriented(&m));
    CHECK(mesh_quadrangulate(&m, NULL, 30) == 6 && m.nf == 6 && closed_oriented(&m));
    CHECK(fabsf(volume(&m) - 1) < 1e-4f);
    mesh_free(&m);
    {
        /* concave L: ear clipping keeps the area */
        V3 pts[6] = { { 0, 0, 0 }, { 2, 0, 0 }, { 2, 0, -1 }, { 1, 0, -1 }, { 1, 0, -2 }, { 0, 0, -2 } };
        int fo[2] = { 0, 6 }, fi[6] = { 0, 1, 2, 3, 4, 5 };
        m = mesh_from_arrays(6, pts, NULL, 1, fo, fi);
        CHECK(mesh_triangulate(&m, NULL) == 1 && m.nf == 4);
        float area = 0;
        for (int f = 0; f < m.nf; f++) {
            V3 a = m.p[m.fi[m.fo[f]]], b = m.p[m.fi[m.fo[f] + 1]], c = m.p[m.fi[m.fo[f] + 2]];
            V3 n = v3cross(v3sub(b, a), v3sub(c, a));
            area += 0.5f * v3len(n);
            CHECK(n.y > 0);
        }
        CHECK(fabsf(area - 3) < 1e-4f);
        mesh_free(&m);
    }

    /* combine, shells, separate, mirrored append keeps faces outward */
    m = cube();
    Mesh c = cube();
    M4 x = m4_trs(v3(3, 0, 0), v3(0, 0, 0), v3(-1, 1, 1));
    mesh_append(&m, &c, &x);
    CHECK(m.nv == 16 && m.nf == 12 && closed_oriented(&m) && fabsf(volume(&m) - 2) < 1e-4f);
    int sh[32];
    CHECK(mesh_shells(&m, sh) == 2 && sh[0] != sh[11]);
    unsigned char fm[32] = { 0 };
    for (int f = 0; f < m.nf; f++) fm[f] = sh[f] == sh[11];
    Mesh part = mesh_subset(&m, fm);
    CHECK(part.nv == 8 && part.nf == 6 && closed_oriented(&part));
    mesh_free(&part);
    mesh_free(&m);

    /* merge by distance: two cubes sharing a face position */
    m = cube();
    x = m4_trs(v3(1, 0, 0), v3(0, 0, 0), v3(1, 1, 1));
    mesh_append(&m, &c, &x);
    CHECK(mesh_merge_dist(&m, NULL, 1e-3f) == 4 && m.nv == 12);
    CHECK(mesh_merge_dist(&m, NULL, 1e-3f) == 0);
    mesh_free(&m);

    /* target weld: a cube corner onto its neighbor */
    m = cube();
    int e0 = 0, a = m.ev[0], b = m.ev[1];
    CHECK(mesh_weld(&m, a, b) && m.nv == 7 && closed_oriented(&m));
    (void)e0;
    mesh_free(&m);

    /* delete edge with cleanup: the cube becomes a wedge */
    m = cube();
    unsigned char em[64] = { 0 };
    em[0] = 1;
    CHECK(mesh_delete_edges(&m, em, 1) > 0);
    CHECK(m.nf == 5 && m.nv == 6 && closed_oriented(&m));
    mesh_free(&m);
    /* delete the four inner edges of a 2x2 plane: one face, borders untouched */
    m = plane(2);
    memset(em, 0, sizeof em);
    for (int e = 0; e < m.ne; e++) em[e] = m.ef[2 * e + 1] >= 0;
    CHECK(count(em, m.ne) == 4);
    CHECK(mesh_delete_edges(&m, em, 1) > 0 && m.nf == 1 && m.nv == 8);
    mesh_free(&m);
    /* delete the middle vertex instead: same result */
    m = plane(2);
    unsigned char vm[64] = { 0 };
    for (int v = 0; v < m.nv; v++) vm[v] = fabsf(m.p[v].x) < 1e-6f && fabsf(m.p[v].z) < 1e-6f;
    CHECK(count(vm, m.nv) == 1);
    CHECK(mesh_delete_verts(&m, vm) > 0 && m.nf == 1 && m.nv == 8);
    mesh_free(&m);

    /* edge ring, insert edge loop, multi-cut */
    m = cube();
    int ra[16], rb[16], closed;
    CHECK(mesh_edge_ring(&m, 0, ra, rb, &closed) == 4 && closed);
    CHECK(mesh_insert_loop(&m, 0, 0.25f) == 4);
    CHECK(m.nf == 10 && m.nv == 12 && closed_oriented(&m) && fabsf(volume(&m) - 1) < 1e-4f);
    CHECK(count(m.es, m.ne) == 4);
    /* the new vertices all sit a quarter of the way along their edges, on one side */
    float lo = 1e9f, hi = -1e9f;
    for (int v = 8; v < 12; v++) {
        V3 q = m.p[v];
        float k = fabsf(q.x) > 0.49f && fabsf(q.y) > 0.49f ? q.z : fabsf(q.x) > 0.49f && fabsf(q.z) > 0.49f ? q.y : q.x;
        lo = fminf(lo, k); hi = fmaxf(hi, k);
    }
    CHECK(fabsf(hi - lo) < 1e-4f && fabsf(fabsf(lo) - 0.25f) < 1e-4f);
    mesh_free(&m);
    m = plane(2);
    int inner = -1;
    for (int e = 0; e < m.ne; e++) if (m.ef[2 * e + 1] >= 0) inner = e;
    CHECK(mesh_insert_loop(&m, inner, 0.5f) == 2 && m.nf == 6);
    mesh_free(&m);
    m = plane(1);
    {
        CutPt cp[2] = { { m.ev[0], m.ev[1], 0.5f }, { 0, 0, 0 } };
        /* the edge opposite edge 0 */
        for (int e = 1; e < m.ne; e++)
            if (m.ev[2 * e] != m.ev[0] && m.ev[2 * e] != m.ev[1] && m.ev[2 * e + 1] != m.ev[0] && m.ev[2 * e + 1] != m.ev[1]) {
                cp[1].a = m.ev[2 * e]; cp[1].b = m.ev[2 * e + 1]; cp[1].t = 0.5f;
            }
        CHECK(mesh_cut(&m, cp, 2) == 1 && m.nf == 2 && m.nv == 6 && count(m.es, m.ne) == 1);
    }
    mesh_free(&m);

    /* bevel: every edge of a cube, then one edge */
    m = cube();
    memset(em, 1, sizeof em);
    CHECK(mesh_bevel(&m, em, 0.1f) == 12);
    CHECK(m.nf == 26 && m.nv == 24 && closed_oriented(&m));
    CHECK(fabsf(volume(&m) - 0.94533f) < 1e-4f);   /* 1 - 12 edge prisms - 8 corners */
    CHECK(count(m.fs, m.nf) == 20);
    mesh_free(&m);
    m = cube();
    memset(em, 0, sizeof em);
    em[0] = 1;
    CHECK(mesh_bevel(&m, em, 0.1f) == 1);
    CHECK(m.nf == 7 && m.nv == 10 && closed_oriented(&m));
    mesh_free(&m);
    /* a loop of edges round a cube's middle (after insert loop) */
    m = cube();
    mesh_insert_loop(&m, 0, 0.5f);
    CHECK(mesh_bevel(&m, m.es, 0.1f) == 4);
    CHECK(m.nf == 14 && closed_oriented(&m) && fabsf(volume(&m) - 1) < 1e-4f);
    mesh_free(&m);

    /* bridge: a floor facing down and a lid facing up become a closed box */
    m = plane(1);
    mesh_reverse(&m, NULL);
    {
        Mesh lid = plane(1);
        x = m4_trs(v3(0, 1, 0), v3(0, 0, 0), v3(1, 1, 1));
        mesh_append(&m, &lid, &x);
        mesh_free(&lid);
    }
    memset(em, 0, sizeof em);
    for (int e = 0; e < m.ne; e++) em[e] = m.ef[2 * e + 1] < 0;
    CHECK(mesh_bridge(&m, em, 0) == 4 && m.nf == 6 && closed_oriented(&m) && fabsf(volume(&m) - 1) < 1e-4f);
    mesh_free(&m);
    m = plane(1);
    mesh_reverse(&m, NULL);
    {
        Mesh lid = plane(1);
        x = m4_trs(v3(0, 1, 0), v3(0, 0, 0), v3(1, 1, 1));
        mesh_append(&m, &lid, &x);
        mesh_free(&lid);
    }
    memset(em, 0, sizeof em);
    for (int e = 0; e < m.ne; e++) em[e] = m.ef[2 * e + 1] < 0;
    CHECK(mesh_bridge(&m, em, 2) == 12 && closed_oriented(&m) && fabsf(volume(&m) - 1) < 1e-4f);
    mesh_free(&m);
    m = cube();
    for (int f = 0; f < m.nf; f++) m.fs[f] = mesh_fnormal(&m, f).z > 0.9f;
    mesh_delete_faces(&m);
    memset(em, 0, sizeof em);
    for (int e = 0; e < m.ne; e++) em[e] = m.ef[2 * e + 1] < 0;
    CHECK(mesh_bridge(&m, em, 0) == -1);   /* one hole: nothing to bridge to */
    mesh_free(&m);

    /* hard / soft overrides */
    m = cube();
    V3 cn[24];
    memset(m.eh, EH_SOFT, m.ne);
    mesh_cnormals(&m, 0, cn);
    for (int k = 0; k < 24; k++) CHECK(fabsf(fabsf(cn[k].x) - 0.57735f) < 1e-3f);
    /* one hard edge alone can't split a corner (the faces still join round the other way); two can */
    m.eh[0] = EH_HARD;
    for (int e = 1; e < m.ne; e++)
        if ((m.ev[2 * e] == m.ev[0] || m.ev[2 * e + 1] == m.ev[0]) && (m.ef[2 * e] == m.ef[1] || m.ef[2 * e + 1] == m.ef[1])) {
            m.eh[e] = EH_HARD;
            break;
        }
    mesh_cnormals(&m, 180, cn);
    int f0 = m.ef[0], c0 = -1, c1 = -1;
    for (int k = m.fo[f0]; k < m.fo[f0 + 1]; k++) if (m.fi[k] == m.ev[0]) c0 = k;
    for (int k = m.fo[m.ef[1]]; k < m.fo[m.ef[1] + 1]; k++) if (m.fi[k] == m.ev[0]) c1 = k;
    CHECK(c0 >= 0 && c1 >= 0 && v3dot(cn[c0], cn[c1]) < 0.99f);
    /* overrides survive an op that keeps those edges */
    m.fs[2] = 1;
    mesh_extrude_faces(&m);
    int hard = 0;
    for (int e = 0; e < m.ne; e++) hard += m.eh[e] == EH_HARD;
    CHECK(hard >= 1);
    mesh_free(&m);
    mesh_free(&c);
}


/* ---- pixel art textures */

static int texels_covered(const Mesh *m)
{
    int n = 0, *tf = malloc((size_t)m->tex->w * m->tex->h * sizeof(int));
    mesh_texel_faces(m, tf);
    for (int i = 0; i < m->tex->w * m->tex->h; i++) n += tf[i] >= 0;
    free(tf);
    return n;
}

static float uv_area(const Mesh *m, int f)
{
    float a = 0;
    int s = m->fo[f], n = m->fo[f + 1] - s;
    for (int k = 0; k < n; k++) {
        const float *p = m->uv + 2 * (s + k), *q = m->uv + 2 * (s + (k + 1) % n);
        a += p[0] * q[1] - q[0] * p[1];
    }
    return fabsf(a) * 0.5f;
}

static int uv_integral(const Mesh *m)
{
    for (int c = 0; c < 2 * m->fo[m->nf]; c++)
        if (m->uv[c] != floorf(m->uv[c])) return 0;
    return 1;
}

static void test_tex(void)
{
    M4 id = m4_id();
    /* a cube at 16 pixels per unit: six 16x16 islands with every corner on a pixel corner */
    Mesh m = cube();
    m.c[0] = v3(1, 0, 0);
    CHECK(mesh_pixel_unwrap(&m, id, 16, 0, NULL, 4096, 0) == 6);
    CHECK(m.uv && m.tex && m.tex->w == 64 && m.tex->h == 64);
    CHECK(uv_integral(&m));
    for (int f = 0; f < m.nf; f++) CHECK(fabsf(uv_area(&m, f) - 256) < 1e-3f);
    CHECK(texels_covered(&m) == 6 * 256);
    {
        int white = 1, red = 0;
        for (int v = 0; v < m.nv; v++) white &= m.c[v].x == 1 && m.c[v].y == 1 && m.c[v].z == 1;
        for (int i = 0; i < m.tex->w * m.tex->h; i++) red += m.tex->px[4 * i + 3] == 255 && m.tex->px[4 * i + 1] < 128;
        CHECK(white && red > 0);   /* vertex colors went into the texture */
    }
    int fisl[64];
    CHECK(mesh_uv_islands(&m, fisl) == 6);
    float rd = 0;
    {
        unsigned char nm[64];
        CHECK(mesh_uv_needy(&m, id, nm) == 0);
        rd = mesh_uv_density(&m, id);
        CHECK(fabsf(rd - 16) < 1e-3f);
    }

    /* the uvs ride along through ops */
    Mesh t = mesh_copy(&m);
    CHECK(t.tex == m.tex && m.tex->refs == 2);
    CHECK(mesh_triangulate(&t, NULL) == 6 && t.nf == 12 && uv_integral(&t) && texels_covered(&t) == 6 * 256);
    CHECK(mesh_quadrangulate(&t, NULL, 30) == 6 && t.nf == 6 && uv_integral(&t));
    for (int f = 0; f < t.nf; f++) CHECK(fabsf(uv_area(&t, f) - 256) < 1e-3f);
    CHECK(tex_own(&t) != m.tex && m.tex->refs == 1);
    mesh_free(&t);

    /* reverse keeps each corner's uv */
    t = mesh_copy(&m);
    CHECK(mesh_reverse(&t, NULL) == 6 && texels_covered(&t) == 6 * 256);
    for (int f = 0; f < t.nf; f++) {
        int s = t.fo[f], n = t.fo[f + 1] - s;
        for (int k = 0; k < n; k++) CHECK(!memcmp(t.uv + 2 * (s + k), m.uv + 2 * (s + n - 1 - k), 2 * sizeof(float)));
    }
    mesh_free(&t);

    /* an edge loop splits a face's pixels between its halves */
    t = mesh_copy(&m);
    CHECK(mesh_insert_loop(&t, 0, 0.5f) == 4 && t.nf == 10);
    CHECK(texels_covered(&t) == 6 * 256);
    mesh_free(&t);

    /* extrude: the cap keeps its pixels, the walls are lines until they get their own */
    t = mesh_copy(&m);
    int top = -1;
    for (int f = 0; f < t.nf; f++) if (mesh_fnormal(&t, f).y > 0.9f) top = f;
    float capuv[8];
    memcpy(capuv, t.uv + 2 * t.fo[top], sizeof capuv);
    t.fs[top] = 1;
    CHECK(mesh_extrude_faces(&t) && t.nf == 10);
    int cap = -1;
    for (int f = 0; f < t.nf; f++) if (t.fs[f]) cap = f;
    CHECK(cap >= 0 && !memcmp(t.uv + 2 * t.fo[cap], capuv, sizeof capuv));
    unsigned char vm[64], nm[64];
    mesh_vmask(&t, COMP_FACE, vm);
    for (int v = 0; v < t.nv; v++) if (vm[v]) t.p[v].y += 1;
    CHECK(mesh_uv_needy(&t, id, nm) == 4);
    unsigned char walls[64];
    memcpy(walls, nm, sizeof walls);
    float keep[200];
    memcpy(keep, t.uv, 2 * t.fo[t.nf] * sizeof(float));
    CHECK(mesh_pixel_unwrap(&t, id, 16, 0, walls, 4096, 0) == 4);
    CHECK(mesh_uv_needy(&t, id, nm) == 0 && uv_integral(&t));
    memcpy(nm, walls, sizeof nm);
    {
        int same = 1;   /* the faces that had pixels keep them where they were */
        for (int f = 0; f < t.nf; f++)
            if (!nm[f] && fabsf(uv_area(&t, f) - 256) < 1e-3f)
                for (int c = t.fo[f]; c < t.fo[f + 1]; c++) same &= keep[2 * c] == t.uv[2 * c] && keep[2 * c + 1] == t.uv[2 * c + 1];
        CHECK(same);
        for (int f = 0; f < t.nf; f++) CHECK(fabsf(uv_area(&t, f) - 256) < 1e-3f);
        CHECK(texels_covered(&t) == 10 * 256);   /* nothing overlaps: the cap took over the old top's pixels */
    }
    mesh_free(&t);

    /* separate and combine */
    unsigned char half[6] = { 1, 1, 1, 0, 0, 0 };
    Mesh a = mesh_subset(&m, half);
    CHECK(a.nf == 3 && a.tex == m.tex && texels_covered(&a) == 3 * 256);
    static const int fo0 = 0;
    Mesh acc = mesh_from_arrays(0, NULL, NULL, 0, &fo0, NULL);
    M4 mv = m4_trs(v3(3, 0, 0), v3(0, 0, 0), v3(1, 1, 1));
    mesh_append(&acc, &a, &id);
    CHECK(acc.uv && acc.tex == m.tex && texels_covered(&acc) == 3 * 256);
    Mesh plain = cube();
    mesh_append(&acc, &plain, &mv);
    CHECK(acc.nf == 9 && acc.tex && acc.tex->w == 64 && acc.tex->h == 64 + 2 + 1);
    CHECK(acc.uv[2 * acc.fo[8]] == 0.5f && acc.uv[2 * acc.fo[8] + 1] == 66.5f);
    CHECK(acc.tex->px[4 * 66 * 64] == 255);
    mesh_free(&acc);
    mesh_free(&a);
    mesh_free(&plain);

    /* flood fill stays inside an island */
    Mesh w = cube();
    mesh_pixel_unwrap(&w, id, 16, 0, NULL, 4096, 0);
    {
        int *tf = malloc(64 * 64 * sizeof(int));
        mesh_texel_faces(&w, tf);
        unsigned char blue[4] = { 0, 0, 255, 255 };
        int x = (int)w.uv[0] + 3, y = (int)w.uv[1] + 3;   /* inside face 0 */
        x = 0; y = 0;
        for (int i = 0; i < 64 * 64; i++) if (tf[i] == 0) { x = i % 64; y = i / 64; break; }
        CHECK(tex_fill(w.tex, tf, x, y, blue) == 256);
        CHECK(tex_fill(w.tex, tf, x, y, blue) == 0);
        free(tf);
    }
    /* ray hit with weights: the middle of the front face is the middle of its island */
    {
        int tri[3];
        float b[3], tt;
        int f = mesh_raycast_tri(&w, v3(0.001f, 0.002f, 5), v3(0, 0, -1), &tt, tri, b);
        CHECK(f >= 0 && fabsf(b[0] + b[1] + b[2] - 1) < 1e-5f);
        float u = 0, v = 0, lo[2] = { 1e9f, 1e9f };
        for (int j = 0; j < 3; j++) { u += b[j] * w.uv[2 * tri[j]]; v += b[j] * w.uv[2 * tri[j] + 1]; }
        for (int c = w.fo[f]; c < w.fo[f + 1]; c++) { lo[0] = fminf(lo[0], w.uv[2 * c]); lo[1] = fminf(lo[1], w.uv[2 * c + 1]); }
        CHECK(fabsf(u - lo[0] - 8) < 0.1f && fabsf(v - lo[1] - 8) < 0.1f);
    }

    /* OBJ with vt + mtl */
    {
        const Mesh *ms[1] = { &w };
        const char *nms[1] = { "box" }, *tfs[1] = { "box.png" };
        CHECK(obj_save("test_tex.obj", 1, ms, &id, nms, NULL, tfs));
        FILE *f = fopen("test_tex.mtl", "r");
        char line[256];
        int mk = 0;
        while (f && fgets(line, sizeof line, f)) mk += !strncmp(line, "map_Kd box.png", 14);
        if (f) fclose(f);
        CHECK(mk == 1);
        Mesh *lm;
        char (*ln)[32], (*lt)[260];
        int n = obj_load("test_tex.obj", &lm, &ln, NULL, &lt);
        CHECK(n == 1);
        if (n == 1) {
            CHECK(lm[0].uv && strstr(lt[0], "box.png") && lm[0].nf == w.nf);
            int same = 1;
            for (int c = 0; lm[0].uv && c < 2 * w.fo[w.nf]; c++) same &= fabsf(lm[0].uv[c] * 64 - w.uv[c]) < 1e-3f;
            CHECK(same);
            mesh_free(&lm[0]);
            free(lm); free(ln); free(lt);
        }
        remove("test_tex.obj");
        remove("test_tex.mtl");
    }
    mesh_free(&w);

    /* islands break at hard edges and unfold across soft ones */
    Prim p;
    prim_defaults(&p, PRIM_CYL);
    p.csides = 8;
    Mesh c = mesh_prim(&p);
    int ci[64];
    CHECK(mesh_pixel_unwrap(&c, id, 16, 0, NULL, 4096, 0) == 10 && mesh_uv_islands(&c, ci) == 10);
    CHECK(mesh_pixel_unwrap(&c, id, 16, 50, NULL, 4096, 0) == 10 && mesh_uv_islands(&c, ci) == 3 && uv_integral(&c));
    CHECK(mesh_pixel_unwrap(&c, id, 16, 180, NULL, 4096, 0) == 10 && mesh_uv_islands(&c, ci) == 3);   /* caps don't fork the strip */
    {
        /* the side strip stands upright: 8 faces side by side, 32 pixels tall */
        int tall = 0;
        for (int f = 0; f < c.nf; f++) {
            float lo = 1e9f, hi = -1e9f;
            for (int k = c.fo[f]; k < c.fo[f + 1]; k++) { lo = fminf(lo, c.uv[2 * k + 1]); hi = fmaxf(hi, c.uv[2 * k + 1]); }
            tall += c.fo[f + 1] - c.fo[f] == 4 && hi - lo == 32;
        }
        CHECK(tall == 8);
    }
    /* a smooth cube unwraps into a band of four walls plus a top and a bottom */
    Mesh sc = cube();
    CHECK(mesh_pixel_unwrap(&sc, id, 16, 180, NULL, 4096, 0) == 6 && mesh_uv_islands(&sc, ci) == 3 && uv_integral(&sc));
    mesh_free(&sc);
    mesh_free(&c);

    /* a turned cube still gets square pixels on top */
    Mesh r = cube();
    M4 rot = m4_trs(v3(0, 0, 0), v3(0, 30, 0), v3(1, 1, 1));
    CHECK(mesh_pixel_unwrap(&r, rot, 16, 0, NULL, 4096, 0) == 6);
    for (int f = 0; f < r.nf; f++) CHECK(fabsf(uv_area(&r, f) - 256) < 1e-3f);
    CHECK(uv_integral(&r));
    /* too many pixels: nothing changes */
    Tex *was = r.tex;
    CHECK(mesh_pixel_unwrap(&r, id, 5000, 0, NULL, 4096, 0) == -1 && r.tex == was);
    mesh_free(&r);
    mesh_free(&m);
}

/* face f's uvs are an upright rectangle (width / height out) */
static int uv_rect(const Mesh *m, int f, float *w, float *h)
{
    int s = m->fo[f];
    if (m->fo[f + 1] - s != 4) return 0;
    float lo[2] = { 1e9f, 1e9f }, hi[2] = { -1e9f, -1e9f };
    for (int k = 0; k < 4; k++) {
        const float *p = m->uv + 2 * (s + k), *q = m->uv + 2 * (s + (k + 1) % 4);
        if ((p[0] != q[0]) == (p[1] != q[1])) return 0;   /* each side runs along x or along y */
        for (int j = 0; j < 2; j++) { lo[j] = fminf(lo[j], p[j]); hi[j] = fmaxf(hi[j], p[j]); }
    }
    if (w) *w = hi[0] - lo[0];
    if (h) *h = hi[1] - lo[1];
    return hi[0] > lo[0] && hi[1] > lo[1];
}

/* the quads' rectangles cover exactly their area in pixels: none of them overlap */
static int quads_apart(const Mesh *m)
{
    int *tf = malloc((size_t)m->tex->w * m->tex->h * sizeof(int)), cov = 0;
    float want = 0, w, h;
    mesh_texel_faces(m, tf);
    for (int f = 0; f < m->nf; f++) if (uv_rect(m, f, &w, &h)) want += w * h;
    for (int i = 0; i < m->tex->w * m->tex->h; i++) cov += tf[i] >= 0 && uv_rect(m, tf[i], NULL, NULL);
    free(tf);
    return fabsf(want - cov) < 0.5f;
}

static int all_quads_rect(const Mesh *m)
{
    for (int f = 0; f < m->nf; f++) if (m->fo[f + 1] - m->fo[f] == 4 && !uv_rect(m, f, NULL, NULL)) return 0;
    return 1;
}

static void test_quads(void)
{
    M4 id = m4_id();
    int isl[2048];
    /* a cube: six 16x16 squares, as before */
    Mesh m = cube();
    CHECK(mesh_pixel_unwrap(&m, id, 16, 0, NULL, 4096, 1) == 6 && uv_integral(&m) && all_quads_rect(&m));
    CHECK(mesh_uv_islands(&m, isl) == 6 && texels_covered(&m) == 6 * 256 && fabsf(mesh_uv_density(&m, id) - 16) < 1e-3f);
    mesh_free(&m);

    /* a frustum: the tapered walls are rectangles as wide as their middle, as tall as their slanted
       sides (1.06 units: 17 pixels) */
    m = cube();
    for (int v = 0; v < m.nv; v++) if (m.p[v].y > 0) { m.p[v].x *= 0.5f; m.p[v].z *= 0.5f; }
    CHECK(mesh_pixel_unwrap(&m, id, 16, 0, NULL, 4096, 1) == 6 && uv_integral(&m) && all_quads_rect(&m) && quads_apart(&m));
    {
        int walls = 0;
        float w, h;
        for (int f = 0; f < m.nf; f++)
            if (fabsf(mesh_fnormal(&m, f).y) < 0.9f && uv_rect(&m, f, &w, &h)) walls += w == 12 && h == 17;
        CHECK(walls == 4);
    }
    /* the old way keeps their true shape: not rectangles */
    CHECK(mesh_pixel_unwrap(&m, id, 16, 0, NULL, 4096, 0) == 6 && !all_quads_rect(&m));
    mesh_free(&m);

    /* a sheared plane (parallelograms) is one grid of rectangles */
    m = plane(4);
    for (int v = 0; v < m.nv; v++) m.p[v].x += 0.8f * m.p[v].z;
    CHECK(mesh_pixel_unwrap(&m, id, 16, 30, NULL, 4096, 1) == 16 && all_quads_rect(&m) && quads_apart(&m));
    CHECK(mesh_uv_islands(&m, isl) == 1);
    mesh_free(&m);

    /* a subdivided cube: each side stays one piece, and rounding doesn't lose pixels along it */
    Prim p;
    prim_defaults(&p, PRIM_CUBE);
    p.sw = p.sh = p.sd = 3;
    m = mesh_prim(&p);
    CHECK(mesh_pixel_unwrap(&m, id, 16, 30, NULL, 4096, 1) == 54 && uv_integral(&m) && all_quads_rect(&m));
    CHECK(mesh_uv_islands(&m, isl) == 6 && texels_covered(&m) == 6 * 256 && quads_apart(&m));
    mesh_free(&m);

    /* faces smaller than a pixel squash like before instead of each taking a pixel */
    p.sw = p.sh = p.sd = 60;
    m = mesh_prim(&p);
    CHECK(mesh_pixel_unwrap(&m, id, 16, 30, NULL, 4096, 1) == m.nf && m.tex->w <= 64 && m.tex->h <= 64);
    {
        int *big = malloc(m.nf * sizeof(int));
        CHECK(mesh_uv_islands(&m, big) == 6);
        free(big);
    }
    mesh_free(&m);

    /* a smooth cube unfolds into one cross of six squares */
    m = cube();
    CHECK(mesh_pixel_unwrap(&m, id, 16, 180, NULL, 4096, 1) == 6 && mesh_uv_islands(&m, isl) == 1 && quads_apart(&m));
    mesh_free(&m);

    /* a sphere: its middle is one grid (rows follow the edge loops), every quad a rectangle */
    prim_defaults(&p, PRIM_SPHERE);
    p.scols = 12; p.srows = 8;
    m = mesh_prim(&p);
    CHECK(mesh_pixel_unwrap(&m, id, 16, 30, NULL, 4096, 1) == m.nf && uv_integral(&m) && all_quads_rect(&m) && quads_apart(&m));
    {
        int n = mesh_uv_islands(&m, isl), big = 0;
        for (int i = 0; i < n; i++) {
            int k = 0;
            for (int f = 0; f < m.nf; f++) k += isl[f] == i;
            if (k > big) big = k;
        }
        CHECK(big == 48);
    }
    mesh_free(&m);

    /* a cylinder: the side is one upright strip, the caps don't fork it */
    prim_defaults(&p, PRIM_CYL);
    p.csides = 8;
    m = mesh_prim(&p);
    CHECK(mesh_pixel_unwrap(&m, id, 16, 180, NULL, 4096, 1) == 10 && mesh_uv_islands(&m, isl) == 3);
    {
        int tall = 0;
        float w, h;
        for (int f = 0; f < m.nf; f++) tall += uv_rect(&m, f, &w, &h) && h == 32;
        CHECK(tall == 8);
    }
    mesh_free(&m);

    /* extruded walls get rectangles of their own; the rest keep their pixels */
    m = cube();
    CHECK(mesh_pixel_unwrap(&m, id, 16, 0, NULL, 4096, 1) == 6);
    int top = -1;
    for (int f = 0; f < m.nf; f++) if (mesh_fnormal(&m, f).y > 0.9f) top = f;
    m.fs[top] = 1;
    CHECK(mesh_extrude_faces(&m) && m.nf == 10);
    unsigned char vm[64], nm[64];
    mesh_vmask(&m, COMP_FACE, vm);
    for (int v = 0; v < m.nv; v++) if (vm[v]) { m.p[v].y += 1; m.p[v].x *= 0.5f; }
    CHECK(mesh_uv_needy(&m, id, nm) == 4);
    CHECK(mesh_pixel_unwrap(&m, id, 16, 0, nm, 4096, 1) == 4 && all_quads_rect(&m) && quads_apart(&m));
    CHECK(mesh_uv_needy(&m, id, nm) == 0);
    mesh_free(&m);
}

int main(void)
{
    Prim p;
    prim_defaults(&p, PRIM_CUBE);
    Mesh m = mesh_prim(&p);
    CHECK(m.nv == 8 && m.nf == 6 && m.ne == 12);
    CHECK(closed_oriented(&m));
    CHECK(fabsf(volume(&m) - 1) < 1e-4f);

    p.sw = 3; p.sh = 2; p.sd = 4;
    Mesh m2 = mesh_prim(&p);
    CHECK(m2.nf == 2 * (3 * 2 + 2 * 4 + 3 * 4));
    CHECK(closed_oriented(&m2));
    CHECK(fabsf(volume(&m2) - 1) < 1e-4f);
    mesh_free(&m2);

    /* extrude top face of the cube */
    int top = -1;
    for (int f = 0; f < m.nf; f++) if (mesh_fnormal(&m, f).y > 0.9f) top = f;
    CHECK(top >= 0);
    m.fs[top] = 1;
    CHECK(mesh_extrude_faces(&m));
    CHECK(m.nv == 12 && m.nf == 10);
    CHECK(closed_oriented(&m));
    int nsel = 0;
    unsigned char mask[64];
    for (int f = 0; f < m.nf; f++) nsel += m.fs[f];
    CHECK(nsel == 1);
    CHECK(mesh_vmask(&m, COMP_FACE, mask) == 4);
    for (int v = 0; v < m.nv; v++) if (mask[v]) m.p[v].y += 1;
    CHECK(fabsf(volume(&m) - 2) < 1e-4f);

    /* merge the top 4 verts into a pyramid apex */
    CHECK(mesh_merge(&m, mask));
    CHECK(m.nv == 9);
    CHECK(closed_oriented(&m));
    CHECK(m.nf == 9); /* the top quad collapsed */
    mesh_free(&m);

    /* cylinders */
    prim_defaults(&p, PRIM_CYL);
    m = mesh_prim(&p);
    CHECK(m.nv == 40 && m.nf == 22 && closed_oriented(&m) && volume(&m) > 0);
    mesh_free(&m);
    p.ccap = 2; p.chseg = 3;
    m = mesh_prim(&p);
    CHECK(closed_oriented(&m) && volume(&m) > 0);
    mesh_free(&m);
    p.ctop = 0; p.ccap = 0;
    m = mesh_prim(&p);
    CHECK(m.nf == 20 * 3 + 1);
    /* extrude the open border */
    int nb = 0;
    for (int e = 0; e < m.ne; e++) if (m.ef[2 * e + 1] < 0) { m.es[e] = 1; nb++; }
    CHECK(nb == 20);
    CHECK(mesh_extrude_edges(&m));
    CHECK(m.nv == 20 * 4 + 20 && m.nf == 20 * 4 + 1);
    nsel = 0;
    for (int e = 0; e < m.ne; e++) nsel += m.es[e];
    CHECK(nsel == 20);
    for (int e = 0; e < m.ne; e++) if (m.es[e]) CHECK(m.ef[2 * e + 1] < 0);
    mesh_free(&m);

    /* sphere */
    prim_defaults(&p, PRIM_SPHERE);
    m = mesh_prim(&p);
    CHECK(m.nv == 20 * 19 + 2 && m.nf == 400 && closed_oriented(&m));
    CHECK(volume(&m) > 3.9f && volume(&m) < 4.2f);
    /* raycast from +Z hits the front */
    float t;
    int f = mesh_raycast(&m, v3(0.01f, 0.02f, 5), v3(0, 0, -1), &t);
    CHECK(f >= 0 && fabsf(t - 4) < 0.05f);
    CHECK(mesh_fnormal(&m, f).z > 0.9f);
    /* delete faces */
    m.fs[0] = m.fs[1] = 1;
    CHECK(mesh_delete_faces(&m) && m.nf == 398);
    mesh_free(&m);

    /* euler round trip */
    V3 r = v3(30, -50, 120);
    V3 r2 = m4_to_euler(m4_euler(r));
    CHECK(fabsf(r.x - r2.x) < 1e-3f && fabsf(r.y - r2.y) < 1e-3f && fabsf(r.z - r2.z) < 1e-3f);
    M4 a = m4_trs(v3(1, 2, 3), r, v3(2, 3, 4)), ai = m4_inv(a);
    V3 q = m4_pt(ai, m4_pt(a, v3(0.3f, -0.7f, 2)));
    CHECK(fabsf(q.x - 0.3f) < 1e-4f && fabsf(q.y + 0.7f) < 1e-4f && fabsf(q.z - 2) < 1e-4f);

    /* shading normals: hard keeps face normals, smooth averages, auto smooth splits at the angle */
    prim_defaults(&p, PRIM_CUBE);
    m = mesh_prim(&p);
    {
        V3 *cn = malloc(m.fo[m.nf] * sizeof(V3));
        int hard = 1, soft = 1;
        float ang[2] = { 30, 0 };
        for (int a = 0; a < 2; a++) {
            mesh_cnormals(&m, ang[a], cn);
            for (int f = 0; f < m.nf; f++)
                for (int c = m.fo[f]; c < m.fo[f + 1]; c++) hard &= v3dot(cn[c], mesh_fnormal(&m, f)) > 0.9999f;
        }
        mesh_cnormals(&m, 180, cn);
        for (int c = 0; c < m.fo[m.nf]; c++)
            soft &= fabsf(fabsf(cn[c].x) - 0.57735f) < 1e-3f && fabsf(fabsf(cn[c].y) - 0.57735f) < 1e-3f;
        CHECK(hard && soft);
        free(cn);
    }
    mesh_free(&m);
    prim_defaults(&p, PRIM_SPHERE);
    m = mesh_prim(&p);
    {
        V3 *cn = malloc(m.fo[m.nf] * sizeof(V3)), *first = calloc(m.nv, sizeof(V3));
        mesh_cnormals(&m, 30, cn);
        int smooth = 1;
        for (int c = 0; c < m.fo[m.nf]; c++) {
            V3 *q = &first[m.fi[c]];
            if (v3dot(*q, *q) == 0) *q = cn[c];
            else smooth &= v3dot(*q, cn[c]) > 0.9999f;
            smooth &= v3dot(cn[c], v3norm(m.p[m.fi[c]])) > 0.99f;
        }
        CHECK(smooth);
        free(cn); free(first);
    }
    mesh_free(&m);

    /* OBJ round trip with colors and normals */
    prim_defaults(&p, PRIM_CUBE);
    m = mesh_prim(&p);
    m.c[3] = v3(1, 0, 0);
    const Mesh *ms[1] = { &m };
    M4 mats[1] = { m4_trs(v3(5, 0, 0), v3(0, 0, 0), v3(1, 1, -1)) };
    const char *names[1] = { "pCube1" };
    float angles[1] = { 30 };
    CHECK(obj_save("test_out.obj", 1, ms, mats, names, angles, NULL));
    Mesh *lm;
    char (*ln)[32];
    float *la;
    int n = obj_load("test_out.obj", &lm, &ln, &la, NULL);
    CHECK(n == 1);
    if (n == 1) {
        CHECK(lm[0].nv == 8 && lm[0].nf == 6 && !strcmp(ln[0], "pCube1"));
        CHECK(closed_oriented(&lm[0]) && volume(&lm[0]) > 0.99f); /* winding flipped for mirror */
        int red = 0;
        for (int v = 0; v < 8; v++) red += lm[0].c[v].y == 0;
        CHECK(red == 1);
        CHECK(la[0] == 30);   /* shading survives the round trip */
        mesh_free(&lm[0]);
        free(lm); free(ln); free(la);
    }
    {
        FILE *f = fopen("test_out.obj", "r");
        char line[256];
        int vn = 0, fslash = 0;
        while (f && fgets(line, sizeof line, f)) { vn += !strncmp(line, "vn ", 3); fslash += line[0] == 'f' && strstr(line, "//") != NULL; }
        if (f) fclose(f);
        CHECK(vn == 6 && fslash == 6);   /* hard cube: one shared normal per face */
    }
    CHECK(obj_save("test_out.obj", 1, ms, mats, names, NULL, NULL));
    n = obj_load("test_out.obj", &lm, &ln, NULL, NULL);
    CHECK(n == 1);
    if (n == 1) {
        CHECK(lm[0].nv == 8 && lm[0].nf == 6);
        mesh_free(&lm[0]);
        free(lm); free(ln);
    }
    /* smoothing groups written by other apps */
    {
        FILE *f = fopen("test_out.obj", "w");
        fputs("v 0 0 0\nv 1 0 0\nv 0 1 0\no a\ns 1\nf 1 2 3\no b\ns off\nf 1 3 2\no c\nf 1 2 3\n", f);
        fclose(f);
        n = obj_load("test_out.obj", &lm, &ln, &la, NULL);
        CHECK(n == 3);
        if (n == 3) {
            CHECK(la[0] == 180 && la[1] == 0 && la[2] == -1);
            for (int i = 0; i < 3; i++) mesh_free(&lm[i]);
            free(lm); free(ln); free(la);
        }
    }
    remove("test_out.obj");
    mesh_free(&m);

    /* multi mask: union of selected verts, edges and faces */
    prim_defaults(&p, PRIM_CUBE);
    m = mesh_prim(&p);
    {
        unsigned char mask[8];
        m.vs[0] = 1;
        m.es[0] = 1;
        CHECK(mesh_vmask(&m, COMP_MULTI, mask) == 1 + (m.ev[0] != 0) + (m.ev[1] != 0));   /* {0} + edge 0's ends */
        m.fs[0] = 1;
        CHECK(mesh_vmask(&m, COMP_MULTI, mask) >= 4);
        CHECK(mesh_vmask(&m, COMP_FACE, mask) == 4);
    }
    mesh_free(&m);

    /* edge loops: middle ring of a 2-high cylinder, a vertical column, a plane border */
    prim_defaults(&p, PRIM_CYL);
    p.csides = 12; p.chseg = 3;
    m = mesh_prim(&p);
    {
        int ring = -1, vert = -1;
        for (int e = 0; e < m.ne; e++) {
            V3 a = m.p[m.ev[2 * e]], b = m.p[m.ev[2 * e + 1]];
            if (fabsf(a.y - b.y) < 1e-4f && fabsf(a.y - (-1.0f / 3)) < 1e-3f && ring < 0) ring = e;
            if (fabsf(a.x - b.x) < 1e-4f && fabsf(a.z - b.z) < 1e-4f && vert < 0) vert = e;
        }
        CHECK(ring >= 0 && vert >= 0);
        CHECK(mesh_edge_loop(&m, ring) == 12);
        memset(m.es, 0, m.ne);
        CHECK(mesh_edge_loop(&m, vert) == 3);   /* stops at the 3-valent cap rings */
        int n = 0;
        for (int e = 0; e < m.ne; e++) n += m.es[e];
        CHECK(n == 3);
    }
    mesh_free(&m);
    prim_defaults(&p, PRIM_PLANE);
    p.psw = p.psd = 3;
    m = mesh_prim(&p);
    {
        int border = -1;
        for (int e = 0; e < m.ne && border < 0; e++)
            if (m.ef[2 * e + 1] < 0 && fabsf(m.p[m.ev[2 * e]].x - 0.5f) < 1e-4f && fabsf(m.p[m.ev[2 * e + 1]].x - 0.5f) < 1e-4f) border = e;
        CHECK(border >= 0 && mesh_edge_loop(&m, border) == 3);
    }
    mesh_free(&m);

    /* grow / shrink / convert on a cube */
    prim_defaults(&p, PRIM_CUBE);
    m = mesh_prim(&p);
    {
        int n = 0;
        m.vs[0] = 1;
        mesh_grow(&m, COMP_VERT, 0);
        for (int v = 0; v < 8; v++) n += m.vs[v];
        CHECK(n == 4);
        mesh_grow(&m, COMP_VERT, 1);
        n = 0;
        for (int v = 0; v < 8; v++) n += m.vs[v];
        CHECK(n == 1 && m.vs[0]);
        mesh_convert(&m, COMP_VERT, COMP_FACE);
        n = 0;
        for (int f = 0; f < 6; f++) n += m.fs[f];
        CHECK(n == 3 && !m.vs[0]);
        memset(m.fs, 0, 6);
        m.fs[0] = 1;
        mesh_convert(&m, COMP_FACE, COMP_EDGE);
        n = 0;
        for (int e = 0; e < 12; e++) n += m.es[e];
        CHECK(n == 4);
        mesh_convert(&m, COMP_EDGE, COMP_FACE);
        n = 0;
        for (int f = 0; f < 6; f++) n += m.fs[f];
        CHECK(n == 5);   /* the face and its 4 neighbors */
        mesh_flip(&m);
        CHECK(closed_oriented(&m) && volume(&m) < -0.99f);
    }
    mesh_free(&m);

    /* fill hole: delete a cube face, fill it back, closed again */
    prim_defaults(&p, PRIM_CUBE);
    m = mesh_prim(&p);
    m.fs[2] = 1;
    mesh_delete_faces(&m);
    CHECK(m.nf == 5 && !closed_oriented(&m));
    {
        unsigned char touch[8] = { 0 };
        CHECK(mesh_fill_holes(&m, touch) == 0 && m.nf == 5);   /* selection not on the hole */
        int bv = -1;
        for (int e = 0; e < m.ne; e++) if (m.ef[2 * e + 1] < 0) bv = m.ev[2 * e];
        touch[bv] = 1;
        CHECK(mesh_fill_holes(&m, touch) == 1);
    }
    CHECK(m.nf == 6 && closed_oriented(&m) && fabsf(volume(&m) - 1) < 1e-4f);
    {
        int sel = 0;
        for (int f = 0; f < m.nf; f++) sel += m.fs[f];
        CHECK(sel == 1);
    }
    mesh_free(&m);
    /* a cylinder without caps has two holes */
    prim_defaults(&p, PRIM_CYL);
    p.ctop = p.cbot = 0;
    m = mesh_prim(&p);
    CHECK(mesh_fill_holes(&m, NULL) == 2 && closed_oriented(&m) && volume(&m) > 0);
    mesh_free(&m);

    /* plane: grid of up-facing quads, open border, extrudable */
    prim_defaults(&p, PRIM_PLANE);
    p.psw = 3; p.psd = 2;
    m = mesh_prim(&p);
    CHECK(m.nv == 4 * 3 && m.nf == 6 && m.ne == 3 * 3 + 4 * 2);
    for (int f = 0; f < m.nf; f++) CHECK(mesh_fnormal(&m, f).y > 0.99f);
    m.fs[0] = 1;
    CHECK(mesh_extrude_faces(&m) && m.nf == 6 + 4);
    mesh_free(&m);

    test_ops();
    test_tex();
    test_quads();
    printf(fails ? "%d FAILED\n" : "all tests passed\n", fails);
    return fails != 0;
}
