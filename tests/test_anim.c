/* Animation, skinning, IK and glTF export tests:
   cc -O1 -Isrc tests/test_anim.c src/anim.c src/gltf.c src/mesh.c -lm && ./a.out [out.glb]
   With a path it also writes a rigged, animated cube there (check it with the Khronos glTF validator). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "anim.h"
#include "gltf.h"

static int fails;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); fails++; } } while (0)
#define NEAR(a, b, e) (fabsf((a) - (b)) < (e))

static int m4_near(M4 a, M4 b, float e)
{
    for (int i = 0; i < 16; i++) if (!NEAR(a.m[i], b.m[i], e)) return 0;
    return 1;
}

static int v3_near(V3 a, V3 b, float e) { return NEAR(a.x, b.x, e) && NEAR(a.y, b.y, e) && NEAR(a.z, b.z, e); }

static void test_math(void)
{
    V3 rs[3] = { { 10, 20, 30 }, { -45, 80, 5 }, { 170, -30, 95 } };
    for (int i = 0; i < 3; i++) {
        M4 a = m4_euler(rs[i]), b = m4_from_quat(quat_from_m4(a));
        CHECK(m4_near(a, b, 1e-4f));
        V3 t, r, s;
        M4 m = m4_trs(v3(1, 2, 3), rs[i], v3(2, 0.5f, 3));
        m4_decompose(m, &t, &r, &s);
        CHECK(v3_near(t, v3(1, 2, 3), 1e-4f) && v3_near(s, v3(2, 0.5f, 3), 1e-4f));
        CHECK(m4_near(m4_trs(t, r, s), m, 1e-3f));
    }
    M4 R = m4_rot_between(v3(1, 0, 0), v3(0, 1, 0));
    CHECK(v3_near(m4_dir(R, v3(1, 0, 0)), v3(0, 1, 0), 1e-5f));
    R = m4_align(v3(1, 0, 0), v3(0, 1, 0), v3(0, 0, 1), v3(1, 0, 0));
    CHECK(v3_near(m4_dir(R, v3(1, 0, 0)), v3(0, 0, 1), 1e-5f) && v3_near(m4_dir(R, v3(0, 1, 0)), v3(1, 0, 0), 1e-5f));

    char b[64];
    CHECK(name_mirror("LeftUpperArm", b, sizeof b) && !strcmp(b, "RightUpperArm"));
    CHECK(name_mirror("RightFoot", b, sizeof b) && !strcmp(b, "LeftFoot"));
    CHECK(name_mirror("arm_L", b, sizeof b) && !strcmp(b, "arm_R"));
    CHECK(name_mirror("leg.r", b, sizeof b) && !strcmp(b, "leg.l"));
    CHECK(name_mirror("L_hand", b, sizeof b) && !strcmp(b, "R_hand"));
    CHECK(!name_mirror("Spine", b, sizeof b) && !name_mirror("Hips", b, sizeof b));
}

static void test_keys(void)
{
    Anim a;
    anim_init(&a);
    Clip *c = anim_clip(&a);
    CHECK(a.n == 1 && !strcmp(c->name, "Take1"));
    Track *t = clip_track(c, 7, 1);
    V3 one = v3(1, 1, 1), z = v3(0, 0, 0), tt, r, s;
    track_set(t, 10, v3(10, 0, 0), z, one, EASE_SMOOTH);
    track_set(t, 0, z, z, one, EASE_SMOOTH);
    CHECK(t->n == 2 && t->k[0].f == 0 && t->k[1].f == 10);
    track_eval(t, 5, &tt, &r, &s);
    CHECK(NEAR(tt.x, 5, 1e-4f));
    track_eval(t, 2.5f, &tt, &r, &s);
    CHECK(tt.x > 0.5f && tt.x < 2.4f);           /* eases in */
    t->k[0].ease = EASE_LINEAR;
    track_eval(t, 2.5f, &tt, &r, &s);
    CHECK(NEAR(tt.x, 2.5f, 1e-4f));
    t->k[0].ease = EASE_STEP;
    track_eval(t, 9, &tt, &r, &s);
    CHECK(NEAR(tt.x, 0, 1e-6f));
    track_eval(t, 30, &tt, &r, &s);
    CHECK(NEAR(tt.x, 10, 1e-6f));                /* holds the last key */
    /* no overshoot past a plateau */
    t->k[0].ease = EASE_SMOOTH;
    track_set(t, 20, v3(10, 0, 0), z, one, EASE_SMOOTH);
    for (float f = 0; f <= 20; f += 0.25f) { track_eval(t, f, &tt, &r, &s); CHECK(tt.x <= 10.0001f && tt.x >= -0.0001f); }
    float nf;
    CHECK(track_neighbor(t, 10, 1, &nf) && nf == 20 && track_neighbor(t, 10, -1, &nf) && nf == 0 && !track_neighbor(t, 20, 1, &nf));
    CHECK(track_move(t, 20, 15) && track_find(t, 15) == 2 && track_find(t, 20) < 0);
    CHECK(track_remove(t, 15) && t->n == 2);
    /* copies are deep */
    Anim b = anim_copy(&a);
    clip_track(anim_clip(&b), 7, 0)->k[0].t.x = 99;
    CHECK(clip_track(anim_clip(&a), 7, 0)->k[0].t.x == 0);
    int k = anim_add_clip(&b, "Walk", 0, 24);
    CHECK(k == 1 && b.n == 2);
    anim_remove_clip(&b, 0);
    CHECK(b.n == 1 && !strcmp(b.c[0].name, "Walk"));
    anim_free(&b);
    clip_drop_obj(c, 7);
    CHECK(c->nt == 0);
    anim_free(&a);
}

static void test_ik(void)
{
    V3 b, c;
    ik_two_bone(v3(0, 0, 0), 1, 1, v3(0, -1.5f, 0), v3(0, 0, 1), &b, &c);
    CHECK(NEAR(v3len(b), 1, 1e-4f) && NEAR(v3len(v3sub(c, b)), 1, 1e-4f));
    CHECK(v3_near(c, v3(0, -1.5f, 0), 1e-4f) && b.z > 0.5f);   /* the knee bends toward the pole */
    ik_two_bone(v3(0, 0, 0), 1, 1, v3(5, 0, 0), v3(0, 1, 0), &b, &c);   /* out of reach: straight */
    CHECK(NEAR(c.x, 2, 1e-3f) && NEAR(b.x, 1, 1e-3f));
}

/* a cube 4 rows tall with two bones: hip (y -0.5 to 0) and knee (0 to 0.5) */
static Mesh rig_cube(M4 *J)
{
    Prim p;
    prim_defaults(&p, PRIM_CUBE);
    p.sh = 4;
    Mesh m = mesh_prim(&p);
    mesh_skin_alloc(&m, 2);
    m.sj[0] = 1; m.sj[1] = 2;
    J[0] = m4_trs(v3(0, -0.5f, 0), v3(0, 0, 0), v3(1, 1, 1));
    J[1] = m4_trs(v3(0, 0, 0), v3(0, 0, 0), v3(1, 1, 1));
    for (int j = 0; j < 2; j++) { M4 ib = m4_inv(J[j]); memcpy(m.ibm + 16 * j, ib.m, sizeof ib.m); }
    Bone bones[2] = { { { 0, -0.5f, 0 }, { 0, 0, 0 }, 0 }, { { 0, 0, 0 }, { 0, 0.5f, 0 }, 1 } };
    M4 id = m4_id();
    skin_auto(&m, &id, bones, 2, 2);
    return m;
}

static int weights_ok(const Mesh *m)
{
    for (int v = 0; v < m->nv; v++) {
        float s = 0;
        for (int k = 0; k < 4; k++) { s += m->ww[4 * v + k]; if (m->ww[4 * v + k] > 0 && m->wj[4 * v + k] >= m->nsj) return 0; }
        if (!NEAR(s, 1, 1e-4f)) return 0;
    }
    return 1;
}

static void test_skin(void)
{
    M4 J[2];
    Mesh m = rig_cube(J);
    CHECK(weights_ok(&m));
    for (int v = 0; v < m.nv; v++) {
        float w1 = 0;
        for (int k = 0; k < 4; k++) if (m.wj[4 * v + k] == 1) w1 = m.ww[4 * v + k];
        if (m.p[v].y < -0.4f) CHECK(w1 < 0.2f);
        if (m.p[v].y > 0.4f) CHECK(w1 > 0.8f);
    }
    V3 *out = malloc(m.nv * sizeof(V3));
    skin_deform(&m, J, out);   /* bind pose: nothing moves */
    for (int v = 0; v < m.nv; v++) CHECK(v3_near(out[v], m.p[v], 1e-4f));
    M4 J2[2] = { J[0], m4_trs(v3(0, 0, 0), v3(0, 0, 90), v3(1, 1, 1)) };   /* bend the knee */
    skin_deform(&m, J2, out);
    for (int v = 0; v < m.nv; v++) {
        if (m.p[v].y < -0.4f) CHECK(v3_near(out[v], m.p[v], 0.15f));
        if (m.p[v].y > 0.45f) CHECK(out[v].x < -0.3f);   /* the top swings over to -X */
    }
    free(out);
    /* weights follow every kind of edit */
    Mesh e = mesh_copy(&m);
    for (int f = 0; f < e.nf; f++) e.fs[f] = mesh_fnormal(&e, f).y > 0.9f;
    mesh_extrude_faces(&e);
    CHECK(e.ww && e.nsj == 2 && weights_ok(&e));
    mesh_offset_faces(&e, 0.5f, 0.1f);
    unsigned char *mask = calloc(e.nv, 1);
    mesh_vmask(&e, COMP_FACE, mask);
    for (int v = 0; v < e.nv; v++)
        if (mask[v]) {   /* the new top copies its source: mostly the knee */
            float w1 = 0;
            for (int k = 0; k < 4; k++) if (e.wj[4 * v + k] == 1) w1 = e.ww[4 * v + k];
            CHECK(w1 > 0.8f);
        }
    free(mask);
    Mesh sm = mesh_smooth(&e, 1);
    CHECK(sm.ww && weights_ok(&sm) && sm.nsj == 2 && sm.sj[1] == 2);
    mesh_triangulate(&sm, NULL);
    CHECK(weights_ok(&sm));
    mesh_mirror(&sm, 0, 1e-3f, 1);
    CHECK(weights_ok(&sm));
    unsigned char *em = malloc(sm.ne);
    memset(em, 1, sm.ne);
    CHECK(mesh_bevel(&sm, em, 0.01f) > 0);
    free(em);
    CHECK(weights_ok(&sm));
    Mesh cp = mesh_copy(&sm);
    CHECK(cp.ww != sm.ww && cp.nsj == 2 && weights_ok(&cp));
    mesh_unskin(&cp);
    CHECK(!cp.ww && !cp.nsj);
    mesh_free(&cp); mesh_free(&sm); mesh_free(&e); mesh_free(&m);
}

/* a rigged cube that bends over a second and comes back, plus a spinning plain box */
static void write_glb(const char *path)
{
    M4 J[2];
    Mesh m = rig_cube(J);
    for (int v = 0; v < m.nv; v++) m.c[v] = v3(1, m.p[v].y + 0.5f, 0.3f);   /* vertex colors */
    Prim p;
    prim_defaults(&p, PRIM_CUBE);
    Mesh box = mesh_prim(&p);
    int sj[2] = { 1, 2 };
    GNode n[5];
    memset(n, 0, sizeof n);
    Quat qi = { 0, 0, 0, 1 };
    n[0].name = "Root"; n[0].parent = -1; n[0].t = v3(0, -0.5f, 0); n[0].s = v3(1, 1, 1); n[0].r = qi;
    n[1].name = "Hip"; n[1].parent = 0; n[1].t = v3(0, 0, 0); n[1].s = v3(1, 1, 1); n[1].r = qi;
    n[2].name = "Knee"; n[2].parent = 1; n[2].t = v3(0, 0.5f, 0); n[2].s = v3(1, 1, 1); n[2].r = qi;
    n[3].name = "Bendy"; n[3].parent = -1; n[3].s = v3(1, 1, 1); n[3].r = qi; n[3].mesh = &m; n[3].angle = 180; n[3].skinJoints = sj;
    n[4].name = "Spinner"; n[4].parent = -1; n[4].t = v3(3, 0, 0); n[4].s = v3(1, 1, 1); n[4].r = qi; n[4].mesh = &box;
    /* joint 1 of the skin is node 1 (hip, at the root's origin) and joint 2 node 2 (knee) */
    m.sj[0] = 1; m.sj[1] = 2;
    int nf = 25;
    float *kr = malloc(nf * 4 * sizeof(float)), *sr = malloc(nf * 4 * sizeof(float)), *st = malloc(nf * 3 * sizeof(float));
    for (int f = 0; f < nf; f++) {
        Quat q = quat_from_m4(m4_euler(v3(0, 0, 90 * sinf(3.14159f * f / (nf - 1)))));
        memcpy(kr + 4 * f, &q, sizeof q);
        q = quat_from_m4(m4_euler(v3(0, 360.0f * f / (nf - 1), 0)));
        memcpy(sr + 4 * f, &q, sizeof q);
        st[3 * f] = 3; st[3 * f + 1] = 0.5f * sinf(3.14159f * f / (nf - 1)); st[3 * f + 2] = 0;
    }
    GChan ch[2] = { { 2, NULL, kr, NULL }, { 4, st, sr, NULL } };
    GAnim an = { "Bend", nf, 24, 2, ch };
    CHECK(gltf_write(path, n, 5, &an, 1) == 0);
    free(kr); free(sr); free(st);
    mesh_free(&m);
    mesh_free(&box);
    FILE *f = fopen(path, "rb");
    unsigned h[3] = { 0 };
    CHECK(f && fread(h, 4, 3, f) == 3 && h[0] == 0x46546C67u && h[1] == 2);
    if (f) { fseek(f, 0, SEEK_END); CHECK((long)h[2] == ftell(f)); fclose(f); }
}

int main(int argc, char **argv)
{
    test_math();
    test_keys();
    test_ik();
    test_skin();
    if (argc > 1) write_glb(argv[1]);
    if (fails) { printf("%d FAILED\n", fails); return 1; }
    printf("all anim tests passed\n");
    return 0;
}
