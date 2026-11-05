/* tuff3d - platform independent math, mesh, primitives, mesh ops and OBJ io */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mesh.h"

#define PI_F 3.14159265358979f
#define DEG2RAD (PI_F / 180.0f)

static void *xrealloc(void *p, size_t n)
{
    void *q = realloc(p, n ? n : 1);
    if (!q) abort();
    return q;
}

static void *xcalloc(size_t n, size_t s)
{
    void *q = calloc(n ? n : 1, s ? s : 1);
    if (!q) abort();
    return q;
}

/* ---------------------------------------------------------------- math */

M4 m4_id(void)
{
    M4 r;
    memset(&r, 0, sizeof r);
    r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1;
    return r;
}

M4 m4_mul(M4 a, M4 b)
{
    M4 r;
    for (int c = 0; c < 4; c++)
        for (int i = 0; i < 4; i++) {
            float s = 0;
            for (int k = 0; k < 4; k++) s += a.m[k * 4 + i] * b.m[c * 4 + k];
            r.m[c * 4 + i] = s;
        }
    return r;
}

M4 m4_inv(M4 a)
{
    const float *m = a.m;
    float inv[16];
    inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
    inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
    inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
    inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
    inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
    inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
    inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
    inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
    inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
    inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
    inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
    inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
    inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
    inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
    inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
    inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];
    float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
    M4 r;
    if (fabsf(det) < 1e-30f) return m4_id();
    det = 1.0f / det;
    for (int i = 0; i < 16; i++) r.m[i] = inv[i] * det;
    return r;
}

V3 m4_pt(M4 a, V3 p)
{
    const float *m = a.m;
    return v3(m[0] * p.x + m[4] * p.y + m[8] * p.z + m[12],
              m[1] * p.x + m[5] * p.y + m[9] * p.z + m[13],
              m[2] * p.x + m[6] * p.y + m[10] * p.z + m[14]);
}

V3 m4_dir(M4 a, V3 p)
{
    const float *m = a.m;
    return v3(m[0] * p.x + m[4] * p.y + m[8] * p.z,
              m[1] * p.x + m[5] * p.y + m[9] * p.z,
              m[2] * p.x + m[6] * p.y + m[10] * p.z);
}

V3 m4_col(M4 a, int c) { return v3(a.m[c * 4], a.m[c * 4 + 1], a.m[c * 4 + 2]); }

/* R = Rz * Ry * Rx (X applied first, Maya's default xyz rotate order) */
M4 m4_euler(V3 r)
{
    float cx = cosf(r.x * DEG2RAD), sx = sinf(r.x * DEG2RAD);
    float cy = cosf(r.y * DEG2RAD), sy = sinf(r.y * DEG2RAD);
    float cz = cosf(r.z * DEG2RAD), sz = sinf(r.z * DEG2RAD);
    M4 m = m4_id();
    m.m[0] = cz * cy;  m.m[4] = cz * sy * sx - sz * cx;  m.m[8] = cz * sy * cx + sz * sx;
    m.m[1] = sz * cy;  m.m[5] = sz * sy * sx + cz * cx;  m.m[9] = sz * sy * cx - cz * sx;
    m.m[2] = -sy;      m.m[6] = cy * sx;                 m.m[10] = cy * cx;
    return m;
}

V3 m4_to_euler(M4 r)
{
    V3 c0 = v3norm(m4_col(r, 0)), c1 = v3norm(m4_col(r, 1)), c2 = v3norm(m4_col(r, 2));
    float r00 = c0.x, r10 = c0.y, r20 = c0.z, r01 = c1.x, r11 = c1.y, r21 = c1.z, r22 = c2.z;
    float x, y, z;
    if (fabsf(r20) < 0.99999f) {
        y = asinf(-r20);
        x = atan2f(r21, r22);
        z = atan2f(r10, r00);
    } else {
        y = r20 < 0 ? PI_F / 2 : -PI_F / 2;
        x = 0;
        z = atan2f(-r01, r11);
    }
    return v3(x / DEG2RAD, y / DEG2RAD, z / DEG2RAD);
}

M4 m4_axis_angle(V3 a, float ang)
{
    a = v3norm(a);
    float c = cosf(ang), s = sinf(ang), t = 1 - c;
    M4 m = m4_id();
    m.m[0] = t * a.x * a.x + c;       m.m[4] = t * a.x * a.y - s * a.z; m.m[8] = t * a.x * a.z + s * a.y;
    m.m[1] = t * a.x * a.y + s * a.z; m.m[5] = t * a.y * a.y + c;       m.m[9] = t * a.y * a.z - s * a.x;
    m.m[2] = t * a.x * a.z - s * a.y; m.m[6] = t * a.y * a.z + s * a.x; m.m[10] = t * a.z * a.z + c;
    return m;
}

M4 m4_trs(V3 t, V3 r, V3 s)
{
    M4 m = m4_euler(r);
    for (int i = 0; i < 3; i++) {
        m.m[i] *= s.x;
        m.m[4 + i] *= s.y;
        m.m[8 + i] *= s.z;
    }
    m.m[12] = t.x; m.m[13] = t.y; m.m[14] = t.z;
    return m;
}

M4 m4_persp(float fovy, float aspect, float n, float f)
{
    M4 m;
    memset(&m, 0, sizeof m);
    float k = 1.0f / tanf(fovy * 0.5f);
    m.m[0] = k / aspect;
    m.m[5] = k;
    m.m[10] = (f + n) / (n - f);
    m.m[11] = -1;
    m.m[14] = 2 * f * n / (n - f);
    return m;
}

M4 m4_lookat(V3 eye, V3 at, V3 up)
{
    V3 f = v3norm(v3sub(at, eye));
    V3 s = v3norm(v3cross(f, up));
    V3 u = v3cross(s, f);
    M4 m = m4_id();
    m.m[0] = s.x; m.m[4] = s.y; m.m[8] = s.z;
    m.m[1] = u.x; m.m[5] = u.y; m.m[9] = u.z;
    m.m[2] = -f.x; m.m[6] = -f.y; m.m[10] = -f.z;
    m.m[12] = -v3dot(s, eye);
    m.m[13] = -v3dot(u, eye);
    m.m[14] = v3dot(f, eye);
    return m;
}

M4 m4_ortho(float l, float r, float b, float t, float n, float f)
{
    M4 m = m4_id();
    m.m[0] = 2 / (r - l);
    m.m[5] = 2 / (t - b);
    m.m[10] = -2 / (f - n);
    m.m[12] = -(r + l) / (r - l);
    m.m[13] = -(t + b) / (t - b);
    m.m[14] = -(f + n) / (f - n);
    return m;
}

/* ---------------------------------------------------------------- edge hash */

typedef struct { unsigned long long *k; int *v; unsigned mask; } HMap;

static void hm_init(HMap *h, int n)
{
    unsigned s = 16;
    while (s < (unsigned)n * 2u) s <<= 1;
    h->mask = s - 1;
    h->k = xrealloc(NULL, s * sizeof *h->k);
    h->v = xrealloc(NULL, s * sizeof *h->v);
    memset(h->k, 0xff, s * sizeof *h->k);
}

static void hm_free(HMap *h) { free(h->k); free(h->v); }

/* value slot for the ordered pair (a,b); new slots start at -1 */
static int *hm_getd(HMap *h, int a, int b)
{
    unsigned long long key = ((unsigned long long)(unsigned)a << 32) | (unsigned)b;
    unsigned i = (unsigned)((key * 0x9E3779B97F4A7C15ull) >> 32) & h->mask;
    while (h->k[i] != ~0ull && h->k[i] != key) i = (i + 1) & h->mask;
    if (h->k[i] == ~0ull) { h->k[i] = key; h->v[i] = -1; }
    return &h->v[i];
}

/* value slot for undirected edge (a,b) */
static int *hm_get(HMap *h, int a, int b)
{
    if (a > b) { int t = a; a = b; b = t; }
    return hm_getd(h, a, b);
}

/* edge index by vertex pair, for a mesh's current edges */
static void edge_map(HMap *h, const Mesh *m)
{
    hm_init(h, 2 * m->ne + 8);
    for (int e = 0; e < m->ne; e++) *hm_get(h, m->ev[2 * e], m->ev[2 * e + 1]) = e;
}

static int edge_at(HMap *h, int a, int b)
{
    return *hm_get(h, a, b);
}

/* ---------------------------------------------------------------- mesh basics */

/* mesh builder; hp holds (a, b, eh) triples: shading overrides for the edges a-b once built */
typedef struct { Mesh m; int cv, cf, ci, ni; int *hp, nhp, chp; } MB;

static void mb_hard(MB *b, int a, int c, int val)
{
    if (b->nhp + 3 > b->chp) { b->chp = b->chp ? b->chp * 2 : 48; b->hp = xrealloc(b->hp, b->chp * sizeof(int)); }
    b->hp[b->nhp++] = a;
    b->hp[b->nhp++] = c;
    b->hp[b->nhp++] = val;
}

static void mb_init(MB *b)
{
    memset(b, 0, sizeof *b);
    b->cf = 64;
    b->m.fo = xrealloc(NULL, b->cf * sizeof(int));
    b->m.fs = xrealloc(NULL, b->cf);
    b->m.fo[0] = 0;
}

static int mb_vert(MB *b, V3 p, V3 c)
{
    Mesh *m = &b->m;
    if (m->nv == b->cv) {
        b->cv = b->cv ? b->cv * 2 : 64;
        m->p = xrealloc(m->p, b->cv * sizeof(V3));
        m->c = xrealloc(m->c, b->cv * sizeof(V3));
        m->vs = xrealloc(m->vs, b->cv);
    }
    m->p[m->nv] = p;
    m->c[m->nv] = c;
    m->vs[m->nv] = 0;
    return m->nv++;
}

static void mb_face(MB *b, const int *idx, int n, int sel)
{
    Mesh *m = &b->m;
    if (m->nf + 2 > b->cf) {
        b->cf *= 2;
        m->fo = xrealloc(m->fo, b->cf * sizeof(int));
        m->fs = xrealloc(m->fs, b->cf);
    }
    if (b->ni + n > b->ci) {
        b->ci = b->ci ? b->ci * 2 : 256;
        if (b->ci < b->ni + n) b->ci = b->ni + n;
        m->fi = xrealloc(m->fi, b->ci * sizeof(int));
    }
    memcpy(m->fi + b->ni, idx, n * sizeof(int));
    b->ni += n;
    m->fs[m->nf] = (unsigned char)sel;
    m->nf++;
    m->fo[m->nf] = b->ni;
}

static V3 newell(const V3 *p, const int *idx, int n)
{
    V3 r = v3(0, 0, 0);
    for (int i = 0; i < n; i++) {
        V3 a = p[idx[i]], b = p[idx[(i + 1) % n]];
        r.x += (a.y - b.y) * (a.z + b.z);
        r.y += (a.z - b.z) * (a.x + b.x);
        r.z += (a.x - b.x) * (a.y + b.y);
    }
    return r;
}

/* add a face, reversing its winding if it does not face along 'want' */
static void mb_face_dir(MB *b, int *idx, int n, V3 want)
{
    if (v3dot(newell(b->m.p, idx, n), want) < 0)
        for (int i = 0; i < n / 2; i++) { int t = idx[i]; idx[i] = idx[n - 1 - i]; idx[n - 1 - i] = t; }
    mb_face(b, idx, n, 0);
}

/* drop vertices no face uses (selection flags move with them); *mapout (optional) gets old -> new */
static void mesh_compact(Mesh *m, int **mapout)
{
    int ni = m->fo[m->nf];
    int *map = xrealloc(NULL, (m->nv ? m->nv : 1) * sizeof(int));
    for (int i = 0; i < m->nv; i++) map[i] = -1;
    for (int i = 0; i < ni; i++) map[m->fi[i]] = 1;
    int n = 0;
    for (int i = 0; i < m->nv; i++) {
        if (map[i] < 0) continue;
        map[i] = n;
        m->p[n] = m->p[i];
        m->c[n] = m->c[i];
        m->vs[n] = m->vs[i];
        n++;
    }
    for (int i = 0; i < ni; i++) m->fi[i] = map[m->fi[i]];
    m->nv = n;
    if (mapout) *mapout = map;
    else free(map);
}

static Mesh mb_finish(MB *b)
{
    Mesh m = b->m;
    if (!m.p) {
        m.p = xrealloc(NULL, sizeof(V3));
        m.c = xrealloc(NULL, sizeof(V3));
        m.vs = xrealloc(NULL, 1);
    }
    if (!m.fi) m.fi = xrealloc(NULL, sizeof(int));
    int *map, nv0 = m.nv;
    mesh_compact(&m, &map);
    mesh_edges(&m);
    if (b->nhp) {
        HMap h;
        edge_map(&h, &m);
        for (int i = b->nhp - 3; i >= 0; i -= 3) {   /* backwards: the first pair given for an edge wins */
            int a = b->hp[i], c = b->hp[i + 1];
            if (a < 0 || c < 0 || a >= nv0 || c >= nv0 || map[a] < 0 || map[c] < 0) continue;
            int e = edge_at(&h, map[a], map[c]);
            if (e >= 0) m.eh[e] = (unsigned char)b->hp[i + 2];
        }
        hm_free(&h);
    }
    free(map);
    free(b->hp);
    b->hp = NULL;
    b->nhp = b->chp = 0;
    return m;
}

void mesh_free(Mesh *m)
{
    free(m->p); free(m->c); free(m->vs);
    free(m->fo); free(m->fi); free(m->fs);
    free(m->ev); free(m->ef); free(m->es); free(m->eh);
    free(m->uv);
    tex_release(m->tex);
    memset(m, 0, sizeof *m);
}

static void *dup_mem(const void *src, size_t n)
{
    void *d = xrealloc(NULL, n);
    if (n) memcpy(d, src, n);
    return d;
}

Mesh mesh_copy(const Mesh *s)
{
    Mesh m = *s;
    int ni = s->fo[s->nf];
    m.p = dup_mem(s->p, s->nv * sizeof(V3));
    m.c = dup_mem(s->c, s->nv * sizeof(V3));
    m.vs = dup_mem(s->vs, s->nv);
    m.fo = dup_mem(s->fo, (s->nf + 1) * sizeof(int));
    m.fi = dup_mem(s->fi, ni * sizeof(int));
    m.fs = dup_mem(s->fs, s->nf);
    m.ev = dup_mem(s->ev, 2 * s->ne * sizeof(int));
    m.ef = dup_mem(s->ef, 2 * s->ne * sizeof(int));
    m.es = dup_mem(s->es, s->ne);
    m.eh = s->eh ? dup_mem(s->eh, s->ne) : xcalloc(s->ne, 1);
    m.uv = s->uv ? dup_mem(s->uv, 2 * ni * sizeof(float)) : NULL;
    m.tex = tex_retain(s->tex);
    return m;
}

void mesh_edges(Mesh *m)
{
    int ni = m->fo[m->nf];
    free(m->ev); free(m->ef); free(m->es); free(m->eh);
    m->ev = xrealloc(NULL, (2 * ni + 2) * sizeof(int));
    m->ef = xrealloc(NULL, (2 * ni + 2) * sizeof(int));
    m->ne = 0;
    HMap h;
    hm_init(&h, ni);
    for (int f = 0; f < m->nf; f++) {
        int o = m->fo[f], n = m->fo[f + 1] - o;
        for (int k = 0; k < n; k++) {
            int a = m->fi[o + k], b = m->fi[o + (k + 1) % n];
            if (a == b) continue;
            int *e = hm_get(&h, a, b);
            if (*e < 0) {
                *e = m->ne++;
                m->ev[2 * *e] = a < b ? a : b;
                m->ev[2 * *e + 1] = a < b ? b : a;
                m->ef[2 * *e] = f;
                m->ef[2 * *e + 1] = -1;
            } else if (m->ef[2 * *e + 1] < 0 && m->ef[2 * *e] != f) {
                m->ef[2 * *e + 1] = f;
            }
        }
    }
    hm_free(&h);
    m->es = xcalloc(m->ne, 1);
    m->eh = xcalloc(m->ne, 1);
}

V3 mesh_fnormal(const Mesh *m, int f)
{
    return v3norm(newell(m->p, m->fi + m->fo[f], m->fo[f + 1] - m->fo[f]));
}

V3 mesh_fcenter(const Mesh *m, int f)
{
    V3 c = v3(0, 0, 0);
    int o = m->fo[f], n = m->fo[f + 1] - o;
    for (int k = 0; k < n; k++) c = v3add(c, m->p[m->fi[o + k]]);
    return n ? v3mul(c, 1.0f / n) : c;
}

/* Per face-corner normals (out[fo[f] + k]). The corners around a vertex share one normal (the area
   weighted average of their faces) across every soft edge: an edge is soft when its faces are within
   'angle' degrees of each other (0 = hard/flat, 180 = fully smooth, in between = auto smooth), unless
   its eh override says hard or soft. */
static int find_root(int *par, int x)
{
    while (par[x] != x) x = par[x] = par[par[x]];
    return x;
}

static int corner_at(const Mesh *m, int f, int v)
{
    for (int c = m->fo[f]; c < m->fo[f + 1]; c++) if (m->fi[c] == v) return c;
    return -1;
}

void mesh_cnormals(const Mesh *m, float angle, V3 *out)
{
    int nc = m->fo[m->nf];
    V3 *fn = xrealloc(NULL, (m->nf + 1) * sizeof(V3)), *sum = xcalloc(nc + 1, sizeof(V3));
    int *cf = xrealloc(NULL, (nc + 1) * sizeof(int)), *par = xrealloc(NULL, (nc + 1) * sizeof(int));
    for (int f = 0; f < m->nf; f++) {
        fn[f] = newell(m->p, m->fi + m->fo[f], m->fo[f + 1] - m->fo[f]);
        for (int c = m->fo[f]; c < m->fo[f + 1]; c++) { cf[c] = f; par[c] = c; }
    }
    float ct = angle >= 180 ? -2.0f : angle <= 0 ? 2.0f : cosf(angle * DEG2RAD) - 1e-5f;
    for (int e = 0; e < m->ne; e++) {
        int f0 = m->ef[2 * e], f1 = m->ef[2 * e + 1], h = m->eh ? m->eh[e] : EH_AUTO;
        if (f0 < 0 || f1 < 0 || h == EH_HARD) continue;
        if (h != EH_SOFT && v3dot(v3norm(fn[f0]), v3norm(fn[f1])) < ct) continue;
        for (int k = 0; k < 2; k++) {
            int c0 = corner_at(m, f0, m->ev[2 * e + k]), c1 = corner_at(m, f1, m->ev[2 * e + k]);
            if (c0 >= 0 && c1 >= 0) par[find_root(par, c0)] = find_root(par, c1);
        }
    }
    for (int c = 0; c < nc; c++) { int r = find_root(par, c); sum[r] = v3add(sum[r], fn[cf[c]]); }
    for (int c = 0; c < nc; c++) {
        V3 n = v3norm(sum[find_root(par, c)]);
        out[c] = v3dot(n, n) > 0 ? n : v3norm(fn[cf[c]]);
    }
    free(fn); free(sum); free(cf); free(par);
}


int mesh_vmask(const Mesh *m, int comp, unsigned char *mask)
{
    memset(mask, 0, m->nv);
    if (comp == COMP_VERT || comp == COMP_MULTI) {
        for (int i = 0; i < m->nv; i++) mask[i] = m->vs[i] != 0;
    }
    if (comp == COMP_EDGE || comp == COMP_MULTI) {
        for (int e = 0; e < m->ne; e++)
            if (m->es[e]) mask[m->ev[2 * e]] = mask[m->ev[2 * e + 1]] = 1;
    }
    if (comp == COMP_FACE || comp == COMP_MULTI) {
        for (int f = 0; f < m->nf; f++)
            if (m->fs[f])
                for (int k = m->fo[f]; k < m->fo[f + 1]; k++) mask[m->fi[k]] = 1;
    }
    int n = 0;
    for (int i = 0; i < m->nv; i++) n += mask[i];
    return n;
}

/* ---------------------------------------------------------------- textures and uvs */

static unsigned texSeq, uvSeq;

Tex *tex_new(int w, int h)
{
    Tex *t = xcalloc(1, sizeof *t);
    t->w = w > 0 ? w : 1;
    t->h = h > 0 ? h : 1;
    t->refs = 1;
    t->id = ++texSeq;
    t->px = xcalloc((size_t)t->w * t->h, 4);
    return t;
}

Tex *tex_retain(Tex *t)
{
    if (t) t->refs++;
    return t;
}

void tex_release(Tex *t)
{
    if (t && --t->refs <= 0) {
        free(t->px);
        free(t);
    }
}

Tex *tex_own(Mesh *m)
{
    Tex *t = m->tex;
    if (!t || t->refs <= 1) return t;
    Tex *c = tex_new(t->w, t->h);
    memcpy(c->px, t->px, (size_t)t->w * t->h * 4);
    tex_release(t);
    return m->tex = c;
}

void mesh_uv_changed(Mesh *m) { m->uvid = ++uvSeq; }

void mesh_uv_scale(Mesh *m, float sx, float sy)
{
    if (!m->uv) return;
    for (int c = 0; c < m->fo[m->nf]; c++) {
        m->uv[2 * c] *= sx;
        m->uv[2 * c + 1] *= sy;
    }
    mesh_uv_changed(m);
}

static float sdiv(float a, float b) { return b != 0 ? a / b : 0; }

/* closest point to p on triangle a b c (Ericson); w gets its barycentric weights */
static V3 tri_closest(V3 p, V3 a, V3 b, V3 c, float w[3])
{
    V3 ab = v3sub(b, a), ac = v3sub(c, a), ap = v3sub(p, a);
    float d1 = v3dot(ab, ap), d2 = v3dot(ac, ap);
    if (d1 <= 0 && d2 <= 0) { w[0] = 1; w[1] = w[2] = 0; return a; }
    V3 bp = v3sub(p, b);
    float d3 = v3dot(ab, bp), d4 = v3dot(ac, bp);
    if (d3 >= 0 && d4 <= d3) { w[1] = 1; w[0] = w[2] = 0; return b; }
    float vc = d1 * d4 - d3 * d2;
    if (vc <= 0 && d1 >= 0 && d3 <= 0) {
        float v = sdiv(d1, d1 - d3);
        w[0] = 1 - v; w[1] = v; w[2] = 0;
        return v3add(a, v3mul(ab, v));
    }
    V3 cp = v3sub(p, c);
    float d5 = v3dot(ab, cp), d6 = v3dot(ac, cp);
    if (d6 >= 0 && d5 <= d6) { w[2] = 1; w[0] = w[1] = 0; return c; }
    float vb = d5 * d2 - d1 * d6;
    if (vb <= 0 && d2 >= 0 && d6 <= 0) {
        float t = sdiv(d2, d2 - d6);
        w[0] = 1 - t; w[1] = 0; w[2] = t;
        return v3add(a, v3mul(ac, t));
    }
    float va = d3 * d6 - d5 * d4;
    if (va <= 0 && d4 - d3 >= 0 && d5 - d6 >= 0) {
        float t = sdiv(d4 - d3, (d4 - d3) + (d5 - d6));
        w[0] = 0; w[1] = 1 - t; w[2] = t;
        return v3add(b, v3mul(v3sub(c, b), t));
    }
    float den = sdiv(1, va + vb + vc), v = vb * den, t = vc * den;
    w[0] = 1 - v - t; w[1] = v; w[2] = t;
    return v3add(a, v3add(v3mul(ab, v), v3mul(ac, t)));
}

/* closest point on face f (its fan triangles) to p: returns the squared distance; *tk gets the fan
   triangle (corners fo[f], fo[f] + tk, fo[f] + tk + 1) and w the weights in it */
static float face_closest(const Mesh *m, int f, V3 p, int *tk, float w[3])
{
    int s = m->fo[f], n = m->fo[f + 1] - s;
    float best = 1e30f;
    V3 a = m->p[m->fi[s]];
    *tk = 1;
    w[0] = 1; w[1] = w[2] = 0;
    for (int k = 1; k + 1 < n; k++) {
        float ww[3];
        V3 d = v3sub(tri_closest(p, a, m->p[m->fi[s + k]], m->p[m->fi[s + k + 1]], ww), p);
        float dd = v3dot(d, d);
        if (dd < best) { best = dd; *tk = k; memcpy(w, ww, sizeof ww); }
    }
    return best;
}

static void face_uv_at(const Mesh *m, int f, int tk, const float w[3], float out[2])
{
    int s = m->fo[f], c[3] = { s, s + tk, s + tk + 1 };
    out[0] = out[1] = 0;
    for (int j = 0; j < 3; j++) {
        out[0] += w[j] * m->uv[2 * c[j]];
        out[1] += w[j] * m->uv[2 * c[j] + 1];
    }
}

/* squared size (bounding box diagonal) of face f */
static float face_size2(const Mesh *m, int f)
{
    V3 lo = m->p[m->fi[m->fo[f]]], hi = lo;
    for (int c = m->fo[f] + 1; c < m->fo[f + 1]; c++) {
        V3 p = m->p[m->fi[c]];
        lo = v3(fminf(lo.x, p.x), fminf(lo.y, p.y), fminf(lo.z, p.z));
        hi = v3(fmaxf(hi.x, p.x), fmaxf(hi.y, p.y), fmaxf(hi.z, p.z));
    }
    V3 d = v3sub(hi, lo);
    return v3dot(d, d);
}

static int same_pos(V3 a, V3 b) { return a.x == b.x && a.y == b.y && a.z == b.z; }

static int uv_same(const float *a, const float *b) { return fabsf(a[0] - b[0]) < 1e-3f && fabsf(a[1] - b[1]) < 1e-3f; }

/* do the two faces of edge e meet with the same uvs along it? */
static int uv_cont(const Mesh *m, int e)
{
    int f0 = m->ef[2 * e], f1 = m->ef[2 * e + 1];
    if (f0 < 0 || f1 < 0 || !m->uv) return 0;
    for (int k = 0; k < 2; k++) {
        int v = m->ev[2 * e + k], c0 = corner_at(m, f0, v), c1 = corner_at(m, f1, v);
        if (c0 < 0 || c1 < 0 || !uv_same(m->uv + 2 * c0, m->uv + 2 * c1)) return 0;
    }
    return 1;
}

/* the edge from each corner to the next one in its face (-1 for a repeated vertex) */
static int *corner_edges(const Mesh *m)
{
    int nc = m->fo[m->nf], *ce = xrealloc(NULL, (nc + 1) * sizeof(int));
    HMap h;
    edge_map(&h, m);
    for (int f = 0; f < m->nf; f++)
        for (int c = m->fo[f]; c < m->fo[f + 1]; c++) {
            int nx = c + 1 < m->fo[f + 1] ? c + 1 : m->fo[f];
            ce[c] = m->fi[c] == m->fi[nx] ? -1 : edge_at(&h, m->fi[c], m->fi[nx]);
        }
    hm_free(&h);
    return ce;
}

int mesh_uv_islands(const Mesh *m, int *fisl)
{
    int *par = xrealloc(NULL, (m->nf + 1) * sizeof(int)), *id = xrealloc(NULL, (m->nf + 1) * sizeof(int)), n = 0;
    for (int f = 0; f < m->nf; f++) { par[f] = f; id[f] = -1; }
    for (int e = 0; e < m->ne; e++)
        if (uv_cont(m, e)) par[find_root(par, m->ef[2 * e])] = find_root(par, m->ef[2 * e + 1]);
    for (int f = 0; f < m->nf; f++) {
        int r = find_root(par, f);
        if (id[r] < 0) id[r] = n++;
        fisl[f] = id[r];
    }
    free(par);
    free(id);
    return n;
}

typedef void (*TexelFn)(void *ctx, int x, int y, const float w[3]);

/* calls fn for each pixel whose center is inside triangle a b c (pixel coordinates; a center right on
   an edge counts), with the center's barycentric weights */
static void raster_tri(const float *a, const float *b, const float *c, int W, int H, TexelFn fn, void *ctx)
{
    float area = (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0]);
    if (!(fabsf(area) >= 1e-8f)) return;   /* also NaN */
    float x0f = fminf(a[0], fminf(b[0], c[0])), x1f = fmaxf(a[0], fmaxf(b[0], c[0]));
    float y0f = fminf(a[1], fminf(b[1], c[1])), y1f = fmaxf(a[1], fmaxf(b[1], c[1]));
    if (x1f < 0 || y1f < 0 || x0f > W || y0f > H) return;
    int x0 = x0f < 0 ? 0 : (int)floorf(x0f), x1 = x1f > W ? W : (int)ceilf(x1f);
    int y0 = y0f < 0 ? 0 : (int)floorf(y0f), y1 = y1f > H ? H : (int)ceilf(y1f);
    float ia = 1 / area;
    for (int y = y0; y < y1; y++)
        for (int x = x0; x < x1; x++) {
            float px = x + 0.5f, py = y + 0.5f, w[3];
            w[0] = ((b[0] - px) * (c[1] - py) - (b[1] - py) * (c[0] - px)) * ia;
            w[1] = ((c[0] - px) * (a[1] - py) - (c[1] - py) * (a[0] - px)) * ia;
            w[2] = 1 - w[0] - w[1];
            if (w[0] >= -1e-5f && w[1] >= -1e-5f && w[2] >= -1e-5f) fn(ctx, x, y, w);
        }
}

typedef struct { int *out, W, f; } TFCtx;

static void tf_put(void *ctx, int x, int y, const float w[3])
{
    TFCtx *c = ctx;
    (void)w;
    c->out[y * c->W + x] = c->f;
}

void mesh_texel_faces(const Mesh *m, int *out)
{
    if (!m->tex || !m->uv) return;
    int W = m->tex->w, H = m->tex->h;
    for (int i = 0; i < W * H; i++) out[i] = -1;
    TFCtx c = { out, W, 0 };
    for (c.f = 0; c.f < m->nf; c.f++) {
        int s = m->fo[c.f];
        for (int k = 1; s + k + 1 < m->fo[c.f + 1]; k++)
            raster_tri(m->uv + 2 * s, m->uv + 2 * (s + k), m->uv + 2 * (s + k + 1), W, H, tf_put, &c);
    }
}

/* the pixel of t at (u, v); a point just outside every face (on an island's edge, or a face squashed
   to a line) takes the nearest pixel a face covers */
static const unsigned char *tex_at(const Tex *t, const int *cov, float u, float v)
{
    int W = t->w, H = t->h;
    if (u != u) u = 0;
    if (v != v) v = 0;
    u = u < -2 ? -2 : u > W + 2 ? W + 2 : u;
    v = v < -2 ? -2 : v > H + 2 ? H + 2 : v;
    int x = (int)floorf(u), y = (int)floorf(v);
    x = x < 0 ? 0 : x >= W ? W - 1 : x;
    y = y < 0 ? 0 : y >= H ? H - 1 : y;
    if (!cov || cov[y * W + x] >= 0) return t->px + 4 * ((size_t)y * W + x);
    int best = -1;
    float bd = 1e30f;
    for (int yy = y - 2; yy <= y + 2; yy++)
        for (int xx = x - 2; xx <= x + 2; xx++) {
            if (xx < 0 || yy < 0 || xx >= W || yy >= H || cov[yy * W + xx] < 0) continue;
            float dx = xx + 0.5f - u, dy = yy + 0.5f - v, d = dx * dx + dy * dy;
            if (d < bd) { bd = d; best = yy * W + xx; }
        }
    return t->px + 4 * (size_t)(best >= 0 ? best : y * W + x);
}

static int px_same(const unsigned char *a, const unsigned char *b) { return (a[3] == 0 && b[3] == 0) || !memcmp(a, b, 4); }

int tex_fill(Tex *t, const int *tf, int x, int y, const unsigned char rgba[4])
{
    int W = t->w, H = t->h, n = 0;
    if (x < 0 || y < 0 || x >= W || y >= H || tf[y * W + x] < 0) return 0;
    unsigned char old[4];
    memcpy(old, t->px + 4 * ((size_t)y * W + x), 4);
    if (px_same(old, rgba)) return 0;
    int *st = xrealloc(NULL, (size_t)W * H * sizeof(int)), ns = 0;
    st[ns++] = y * W + x;
    memcpy(t->px + 4 * (size_t)st[0], rgba, 4);
    n++;
    while (ns) {
        int i = st[--ns], cx = i % W, cy = i / W;
        int nb[4] = { cx > 0 ? i - 1 : -1, cx < W - 1 ? i + 1 : -1, cy > 0 ? i - W : -1, cy < H - 1 ? i + W : -1 };
        for (int k = 0; k < 4; k++) {
            int j = nb[k];
            if (j < 0 || tf[j] < 0 || !px_same(t->px + 4 * (size_t)j, old)) continue;
            memcpy(t->px + 4 * (size_t)j, rgba, 4);
            st[ns++] = j;
            n++;
        }
    }
    free(st);
    t->ver++;
    return n;
}

/* ---- carrying uvs through mesh ops

   When an op rebuilds a textured mesh, every new face looks for the old face it lies on (the closest
   one around its corners). Corners where an old corner was keep that corner's uv; new corners (cuts,
   bevels) get the uv of the spot they sit on. A face that lies on no old face (a filled hole, a
   bridge) shows the one pixel nearest its middle until Pixel Unwrap gives it pixels of its own. */

typedef struct { const Mesh *m; int *ord, n; float *key; } CornerIx;   /* old corners sorted along a slant */

static const float *sortKey;
static int cmp_key(const void *a, const void *b)
{
    float d = sortKey[*(const int *)a] - sortKey[*(const int *)b];
    return d > 0 ? 1 : d < 0 ? -1 : 0;
}

static V3 slant(void) { return v3norm(v3(0.5773503f, 0.6157731f, 0.5360563f)); }

static void ix_build(CornerIx *ix, const Mesh *m)
{
    int nc = m->fo[m->nf];
    float *k = xrealloc(NULL, (nc + 1) * sizeof(float));
    V3 s = slant();
    ix->m = m;
    ix->n = nc;
    ix->ord = xrealloc(NULL, (nc + 1) * sizeof(int));
    for (int c = 0; c < nc; c++) { ix->ord[c] = c; k[c] = v3dot(m->p[m->fi[c]], s); }
    sortKey = k;
    qsort(ix->ord, nc, sizeof(int), cmp_key);
    ix->key = xrealloc(NULL, (nc + 1) * sizeof(float));
    for (int i = 0; i < nc; i++) ix->key[i] = k[ix->ord[i]];
    free(k);
}

/* the old corners at the old position nearest to p (up to max of them) */
static int ix_nearest(const CornerIx *ix, V3 p, int *out, int max)
{
    const Mesh *m = ix->m;
    float k = v3dot(p, slant()), best = 1e30f;
    int lo = 0, hi = ix->n, bi = -1, n = 0;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        if (ix->key[mid] < k) lo = mid + 1;
        else hi = mid;
    }
    for (int dir = -1; dir <= 1; dir += 2)
        for (int i = dir < 0 ? lo - 1 : lo; i >= 0 && i < ix->n; i += dir) {
            float dk = ix->key[i] - k;
            if (dk * dk > best) break;
            V3 d = v3sub(m->p[m->fi[ix->ord[i]]], p);
            float dd = v3dot(d, d);
            if (dd < best) { best = dd; bi = i; }
        }
    if (bi < 0) return 0;
    V3 bp = m->p[m->fi[ix->ord[bi]]];
    int i0 = bi;
    while (i0 > 0 && ix->key[i0 - 1] == ix->key[bi]) i0--;
    for (int i = i0; i < ix->n && ix->key[i] == ix->key[bi] && n < max; i++)
        if (same_pos(m->p[m->fi[ix->ord[i]]], bp)) out[n++] = ix->ord[i];
    return n;
}

/* uvs for n, which a mesh op rebuilt from o */
static float *uv_transfer(const Mesh *o, const Mesh *n)
{
    enum { MAXC = 48, MAXH = 64 };
    int nnc = n->fo[n->nf], onc = o->fo[o->nf];
    float *uv = xcalloc(2 * (size_t)nnc + 2, sizeof(float));
    if (!o->nf || !onc || !o->uv) return uv;
    CornerIx ix;
    ix_build(&ix, o);
    int *ce = corner_edges(o), *cface = xrealloc(NULL, (onc + 1) * sizeof(int));
    unsigned char *cont = xcalloc(o->ne + 1, 1);
    for (int e = 0; e < o->ne; e++) cont[e] = (unsigned char)uv_cont(o, e);
    for (int f = 0; f < o->nf; f++)
        for (int c = o->fo[f]; c < o->fo[f + 1]; c++) cface[c] = f;
    int cand[MAXC], inS[MAXC], hits[MAXH];
    for (int F = 0; F < n->nf; F++) {
        int s = n->fo[F], cnt = n->fo[F + 1] - s, nc = 0;
        V3 cen = mesh_fcenter(n, F);
        for (int k = 0; k < cnt; k++) {
            int h = ix_nearest(&ix, n->p[n->fi[s + k]], hits, MAXH);
            for (int j = 0; j < h; j++) {
                int g = cface[hits[j]], dup = 0;
                for (int i = 0; i < nc && !dup; i++) dup = cand[i] == g;
                if (!dup && nc < MAXC) cand[nc++] = g;
            }
        }
        if (!nc) continue;
        /* the old face the new one lies on: nearest to its middle, then the one sharing most corners */
        V3 nn = v3norm(newell(n->p, n->fi + s, cnt));
        float fs2 = face_size2(n, F), bd = 1e30f, bdot = -2, bw[3] = { 1, 0, 0 };
        int bi = 0, bm = -1, btk = 1;
        for (int i = 0; i < nc; i++) {
            int g = cand[i], tk, match = 0;
            float w[3], d = face_closest(o, g, cen, &tk, w), dt = v3dot(nn, mesh_fnormal(o, g));
            for (int k = 0; k < cnt; k++)
                for (int c = o->fo[g]; c < o->fo[g + 1]; c++)
                    if (same_pos(o->p[o->fi[c]], n->p[n->fi[s + k]])) { match++; break; }
            float eps = 1e-10f + 1e-8f * fs2;
            if (d < bd - eps || (fabsf(d - bd) <= eps && (match > bm || (match == bm && dt > bdot)))) {
                bd = d; bi = i; bm = match; bdot = dt; btk = tk;
                memcpy(bw, w, sizeof w);
            }
        }
        int best = cand[bi];
        /* faces joined to it without a seam can lend their corners too */
        memset(inS, 0, sizeof inS);
        inS[bi] = 1;
        for (int changed = 1; changed;) {
            changed = 0;
            for (int i = 0; i < nc; i++) {
                if (inS[i]) continue;
                int g = cand[i];
                for (int c = o->fo[g]; c < o->fo[g + 1] && !inS[i]; c++) {
                    int e = ce[c];
                    if (e < 0 || !cont[e]) continue;
                    int other = o->ef[2 * e] == g ? o->ef[2 * e + 1] : o->ef[2 * e];
                    for (int j = 0; j < nc; j++)
                        if (inS[j] && cand[j] == other) { inS[i] = changed = 1; break; }
                }
            }
        }
        /* it keeps real pixels if it faces the same way and is made of those faces' corners (a
           dissolved edge, a filled hole) or lies on the face (a cut); otherwise one flat pixel */
        int all = 1;
        for (int k = 0; k < cnt && all; k++) {
            V3 p = n->p[n->fi[s + k]];
            int found = 0;
            for (int i = 0; i < nc && !found; i++)
                for (int c = o->fo[cand[i]]; inS[i] && c < o->fo[cand[i] + 1] && !found; c++) found = same_pos(o->p[o->fi[c]], p);
            all = found;
        }
        V3 raw = newell(n->p, n->fi + s, cnt);
        int facing = v3dot(raw, raw) < 1e-24f || v3dot(nn, mesh_fnormal(o, best)) > 0;
        if (!facing || (!all && bd > 0.0025f * face_size2(o, best) + 1e-12f)) {
            float u[2];
            face_uv_at(o, best, btk, bw, u);
            for (int k = 0; k < cnt; k++) memcpy(uv + 2 * (s + k), u, sizeof u);
            continue;
        }
        for (int k = 0; k < cnt; k++) {
            V3 p = n->p[n->fi[s + k]];
            int found = 0;
            for (int pass = 0; pass < 2 && !found; pass++)
                for (int i = 0; i < nc && !found; i++) {
                    if (!inS[i] || (pass == 0) != (i == bi)) continue;
                    int g = cand[i];
                    for (int c = o->fo[g]; c < o->fo[g + 1]; c++)
                        if (same_pos(o->p[o->fi[c]], p)) { memcpy(uv + 2 * (s + k), o->uv + 2 * c, 2 * sizeof(float)); found = 1; break; }
                }
            if (!found) {
                int tk;
                float w[3];
                face_closest(o, best, p, &tk, w);
                face_uv_at(o, best, tk, w, uv + 2 * (s + k));
            }
        }
    }
    free(ix.ord); free(ix.key); free(ce); free(cface); free(cont);
    return uv;
}

/* ---- Pixel Unwrap */

#define UV_GAP 2   /* empty pixels between islands, so nothing bleeds across */

typedef struct { int first, n, w, h, x, y; } Isl;   /* faces list[first .. first + n - 1] */

static float face_uv_area(const Mesh *m, int f)
{
    int s = m->fo[f], n = m->fo[f + 1] - s;
    float a = 0;
    for (int k = 0; k < n; k++) {
        const float *p = m->uv + 2 * (s + k), *q = m->uv + 2 * (s + (k + 1) % n);
        a += p[0] * q[1] - q[0] * p[1];
    }
    return fabsf(a) * 0.5f;
}

static V3 *world_pts(const Mesh *m, M4 xf)
{
    V3 *wp = xrealloc(NULL, (m->nv + 1) * sizeof(V3));
    for (int v = 0; v < m->nv; v++) wp[v] = m4_pt(xf, m->p[v]);
    return wp;
}

static int cmp_float(const void *a, const void *b)
{
    float d = *(const float *)a - *(const float *)b;
    return d > 0 ? 1 : d < 0 ? -1 : 0;
}

float mesh_uv_density(const Mesh *m, M4 xf)
{
    if (!m->uv || !m->nf) return 0;
    V3 *wp = world_pts(m, xf);
    float *d = xrealloc(NULL, (m->nf + 1) * sizeof(float)), r = 0;
    int n = 0;
    for (int f = 0; f < m->nf; f++) {
        float ua = face_uv_area(m, f), wa = 0.5f * v3len(newell(wp, m->fi + m->fo[f], m->fo[f + 1] - m->fo[f]));
        if (ua > 0.5f && wa > 1e-12f) d[n++] = sqrtf(ua / wa);
    }
    if (n) {
        qsort(d, n, sizeof(float), cmp_float);
        r = d[n / 2];
    }
    free(wp);
    free(d);
    return r;
}

int mesh_uv_needy(const Mesh *m, M4 xf, unsigned char *fmask)
{
    float dens = mesh_uv_density(m, xf);
    int n = 0;
    memset(fmask, 0, m->nf);
    if (!m->uv || dens <= 0) return 0;
    V3 *wp = world_pts(m, xf);
    for (int f = 0; f < m->nf; f++) {
        float wa = 0.5f * v3len(newell(wp, m->fi + m->fo[f], m->fo[f + 1] - m->fo[f])), want = wa * dens * dens;
        if (want >= 1 && face_uv_area(m, f) < 0.05f * want) { fmask[f] = 1; n++; }
    }
    free(wp);
    return n;
}

/* up and right on a face facing n: walls stand upright, floors have -Z up and ceilings +Z */
static void face_frame(V3 n, V3 *R, V3 *U)
{
    V3 up = fabsf(n.y) < 0.99f ? v3(0, 1, 0) : v3(0, 0, n.y > 0 ? -1.0f : 1.0f);
    *U = v3norm(v3sub(up, v3mul(n, v3dot(up, n))));
    *R = v3cross(*U, n);
}

static int runs(const Mesh *m, int f, int a, int b)   /* does face f go from a to b? */
{
    int s = m->fo[f], n = m->fo[f + 1] - s;
    for (int k = 0; k < n; k++)
        if (m->fi[s + k] == a && m->fi[s + (k + 1) % n] == b) return 1;
    return 0;
}

static void face_bb(const Mesh *m, const float *c2, int f, float *bb)
{
    float *b = bb + 4 * f;
    b[0] = b[1] = 1e30f;
    b[2] = b[3] = -1e30f;
    for (int c = m->fo[f]; c < m->fo[f + 1]; c++) {
        b[0] = fminf(b[0], c2[2 * c]); b[1] = fminf(b[1], c2[2 * c + 1]);
        b[2] = fmaxf(b[2], c2[2 * c]); b[3] = fmaxf(b[3], c2[2 * c + 1]);
    }
}

/* do two 2D triangles overlap? (separating axes; touching is not overlapping) */
static int tri2_overlap(const float *a, const float *b)
{
    const float *t[2] = { a, b };
    for (int i = 0; i < 2; i++)
        for (int k = 0; k < 3; k++) {
            const float *p = t[i] + 2 * k, *q = t[i] + 2 * ((k + 1) % 3);
            float nx = q[1] - p[1], ny = p[0] - q[0], mn[2] = { 1e30f, 1e30f }, mx[2] = { -1e30f, -1e30f };
            for (int j = 0; j < 2; j++)
                for (int v = 0; v < 3; v++) {
                    float d = t[j][2 * v] * nx + t[j][2 * v + 1] * ny;
                    mn[j] = fminf(mn[j], d);
                    mx[j] = fmaxf(mx[j], d);
                }
            if (mx[0] <= mn[1] || mx[1] <= mn[0]) return 0;
        }
    return 1;
}

/* would face f (corners in cf, shrunk a hair so shared edges don't count) cover part of face g
   (corners in cg)? */
static int face2_overlap(const Mesh *m, const float *cf, int f, const float *cg, int g)
{
    int sf = m->fo[f], nf = m->fo[f + 1] - sf, sg = m->fo[g], ng = m->fo[g + 1] - sg;
    float cx = 0, cy = 0, t[6], u[6];
    for (int k = 0; k < nf; k++) { cx += cf[2 * (sf + k)]; cy += cf[2 * (sf + k) + 1]; }
    cx /= nf;
    cy /= nf;
    for (int i = 1; i + 1 < nf; i++) {
        int ci[3] = { sf, sf + i, sf + i + 1 };
        for (int j = 0; j < 3; j++) {
            t[2 * j] = cx + (cf[2 * ci[j]] - cx) * 0.9999f;
            t[2 * j + 1] = cy + (cf[2 * ci[j] + 1] - cy) * 0.9999f;
        }
        for (int j = 1; j + 1 < ng; j++) {
            int cj[3] = { sg, sg + j, sg + j + 1 };
            for (int q = 0; q < 3; q++) { u[2 * q] = cg[2 * cj[q]]; u[2 * q + 1] = cg[2 * cj[q] + 1]; }
            if (tri2_overlap(t, u)) return 1;
        }
    }
    return 0;
}

/* an island while it grows: its faces are a list through nextf */
typedef struct { int head, tail, n, folded; float fold[2], box[4]; } Grow;

static int cmp_key_idx(const void *a, const void *b)
{
    int r = cmp_key(a, b);
    return r ? r : *(const int *)a - *(const int *)b;
}

/* face f alone, laid flat as its own frame sees it */
static void grow_seed(const Mesh *m, const V3 *wp, const V3 *fn, float *c2, float *bb, int *isl, int *nextf, Grow *gr, int id, int f)
{
    V3 R, U;
    face_frame(fn[f], &R, &U);
    for (int c = m->fo[f]; c < m->fo[f + 1]; c++) {
        c2[2 * c] = v3dot(wp[m->fi[c]], R);
        c2[2 * c + 1] = v3dot(wp[m->fi[c]], U);
    }
    face_bb(m, c2, f, bb);
    isl[f] = id;
    nextf[f] = -1;
    gr[id].head = gr[id].tail = f;
    gr[id].n = 1;
    gr[id].folded = 0;
    gr[id].fold[0] = gr[id].fold[1] = 0;
    memcpy(gr[id].box, bb + 4 * f, sizeof gr[id].box);
}

static int box_hit(const float *a, const float *b) { return a[0] < b[2] && b[0] < a[2] && a[1] < b[3] && b[1] < a[3]; }

/* join the islands on the two sides of edge e: the smaller one turns and moves so its copy of the
   edge lands on the bigger one's. 0 if they would overlap, or a bend would make the island fork. */
static int grow_join(const Mesh *m, float *c2, float *t2, float *bb, int *isl, int *nextf, Grow *gr, int e, int bent)
{
    int a = m->ev[2 * e], b = m->ev[2 * e + 1], fx = m->ef[2 * e], fy = m->ef[2 * e + 1];
    if (gr[isl[fy]].n > gr[isl[fx]].n) { int t = fx; fx = fy; fy = t; }
    Grow *X = &gr[isl[fx]], *Y = &gr[isl[fy]];
    int ax = corner_at(m, fx, a), bx = corner_at(m, fx, b), ay = corner_at(m, fy, a), by = corner_at(m, fy, b);
    if (ax < 0 || bx < 0 || ay < 0 || by < 0) return 0;
    float px = c2[2 * bx] - c2[2 * ax], py = c2[2 * bx + 1] - c2[2 * ax + 1], qx = c2[2 * by] - c2[2 * ay], qy = c2[2 * by + 1] - c2[2 * ay + 1];
    float lp = sqrtf(px * px + py * py), lq = sqrtf(qx * qx + qy * qy);
    if (lp < 1e-12f || lq < 1e-12f) return 0;
    float th = atan2f(py, px) - atan2f(qy, qx), cs = cosf(th), sn = sinf(th), dx = px / lp, dy = py / lp;
    float tx = c2[2 * ax] - (cs * c2[2 * ay] - sn * c2[2 * ay + 1]), ty = c2[2 * ax + 1] - (sn * c2[2 * ay] + cs * c2[2 * ay + 1]);
    float yf[2] = { cs * Y->fold[0] - sn * Y->fold[1], sn * Y->fold[0] + cs * Y->fold[1] };
    if (bent) {
        if (X->folded && fabsf(X->fold[0] * dx + X->fold[1] * dy) < 0.866f) return 0;
        if (Y->folded && fabsf(yf[0] * dx + yf[1] * dy) < 0.866f) return 0;
    } else if (X->folded && Y->folded && fabsf(X->fold[0] * yf[0] + X->fold[1] * yf[1]) < 0.866f) {
        return 0;
    }
    float ybox[4] = { 1e30f, 1e30f, -1e30f, -1e30f };
    for (int f = Y->head; f >= 0; f = nextf[f])
        for (int c = m->fo[f]; c < m->fo[f + 1]; c++) {
            float x = c2[2 * c], y = c2[2 * c + 1];
            t2[2 * c] = cs * x - sn * y + tx;
            t2[2 * c + 1] = sn * x + cs * y + ty;
            ybox[0] = fminf(ybox[0], t2[2 * c]); ybox[1] = fminf(ybox[1], t2[2 * c + 1]);
            ybox[2] = fmaxf(ybox[2], t2[2 * c]); ybox[3] = fmaxf(ybox[3], t2[2 * c + 1]);
        }
    /* flat joins can't overlap (they are the real shape); bends are checked face by face */
    if (bent && box_hit(ybox, X->box))
        for (int f = Y->head; f >= 0; f = nextf[f]) {
            float fb[4];
            face_bb(m, t2, f, bb);
            memcpy(fb, bb + 4 * f, sizeof fb);
            for (int g = X->head; g >= 0; g = nextf[g])
                if (box_hit(fb, bb + 4 * g) && face2_overlap(m, t2, f, c2, g)) {
                    for (int r = Y->head; r >= 0; r = nextf[r]) face_bb(m, c2, r, bb);
                    return 0;
                }
        }
    int xi = isl[fx];
    for (int f = Y->head; f >= 0; f = nextf[f]) {
        memcpy(c2 + 2 * m->fo[f], t2 + 2 * m->fo[f], 2 * (m->fo[f + 1] - m->fo[f]) * sizeof(float));
        face_bb(m, c2, f, bb);
        isl[f] = xi;
    }
    nextf[X->tail] = Y->head;
    X->tail = Y->tail;
    X->n += Y->n;
    Y->n = 0;
    for (int k = 0; k < 2; k++) { X->box[k] = fminf(X->box[k], ybox[k]); X->box[k + 2] = fmaxf(X->box[k + 2], ybox[k + 2]); }
    if (!X->folded && (Y->folded || bent)) {
        X->fold[0] = Y->folded ? yf[0] : dx;
        X->fold[1] = Y->folded ? yf[1] : dy;
        X->folded = 1;
    }
    return 1;
}

/* turn an island so face f sits the way its own frame sees it (walls upright) */
static void upright(const Mesh *m, const V3 *wp, const V3 *fn, float *c2, int f, const int *faces, int nfaces)
{
    V3 R, U;
    face_frame(fn[f], &R, &U);
    int s = m->fo[f], n = m->fo[f + 1] - s;
    for (int k = 0; k < n; k++) {
        int c0 = s + k, c1 = s + (k + 1) % n;
        V3 d = v3sub(wp[m->fi[c1]], wp[m->fi[c0]]);
        float qx = v3dot(d, R), qy = v3dot(d, U), px = c2[2 * c1] - c2[2 * c0], py = c2[2 * c1 + 1] - c2[2 * c0 + 1];
        if (qx * qx + qy * qy < 1e-18f || px * px + py * py < 1e-18f) continue;
        float th = atan2f(qy, qx) - atan2f(py, px), cs = cosf(th), sn = sinf(th);
        for (int i = 0; i < nfaces; i++)
            for (int c = m->fo[faces[i]]; c < m->fo[faces[i] + 1]; c++) {
                float x = c2[2 * c], y = c2[2 * c + 1];
                c2[2 * c] = x * cs - y * sn;
                c2[2 * c + 1] = x * sn + y * cs;
            }
        return;
    }
}

/* turn an island so the pixel grid runs along its first face: unless one of that face's edges is
   already level or upright, its longest edge becomes so */
static void straighten(const Mesh *m, float *c2, int sd, const int *faces, int nfaces)
{
    int s = m->fo[sd], n = m->fo[sd + 1] - s;
    float best = 0, ang = 0, q = PI_F / 2;
    for (int k = 0; k < n; k++) {
        const float *p = c2 + 2 * (s + k), *r = c2 + 2 * (s + (k + 1) % n);
        float dx = r[0] - p[0], dy = r[1] - p[1], len = sqrtf(dx * dx + dy * dy);
        if (len < 1e-9f) continue;
        float a = atan2f(dy, dx);
        a -= q * roundf(a / q);
        if (fabsf(a) < 0.5f * DEG2RAD) return;
        if (len > best) { best = len; ang = a; }
    }
    if (best <= 0) return;
    float cs = cosf(-ang), sn = sinf(-ang);
    for (int i = 0; i < nfaces; i++)
        for (int c = m->fo[faces[i]]; c < m->fo[faces[i] + 1]; c++) {
            float x = c2[2 * c], y = c2[2 * c + 1];
            c2[2 * c] = x * cs - y * sn;
            c2[2 * c + 1] = x * sn + y * cs;
        }
}

/* ---- Follow Quads: islands grow from their biggest face like a UV map. Each quad is a rectangle of
   whole pixels (the pixel grid runs along its edges, even on slanted and tapered faces), and a
   strip of quads keeps one row height, so pixel rows follow the mesh's edge loops. Other faces keep
   their shape, with their corners on pixel corners. */

/* how far a face may be stretched to keep its neighbors' rows (or by a pixel): more on a flat face,
   so a tapered face that was cut up stays one piece */
#define Q_STRETCH 1.5f
#define Q_STRETCH_FLAT 2.0f

typedef struct { float cost; int e, f; } QEnt;   /* face f is placed; edge e leads to the next one */
typedef struct { QEnt *a; int n, cap; } QHeap;

static void qh_push(QHeap *h, float cost, int e, int f)
{
    if (h->n == h->cap) { h->cap = h->cap * 2 + 64; h->a = xrealloc(h->a, h->cap * sizeof(QEnt)); }
    int i = h->n++;
    while (i > 0 && h->a[(i - 1) / 2].cost > cost) { h->a[i] = h->a[(i - 1) / 2]; i = (i - 1) / 2; }
    h->a[i].cost = cost; h->a[i].e = e; h->a[i].f = f;
}

static int qh_pop(QHeap *h, QEnt *out)
{
    if (!h->n) return 0;
    *out = h->a[0];
    QEnt last = h->a[--h->n];
    int i = 0;
    for (;;) {
        int c = 2 * i + 1;
        if (c >= h->n) break;
        if (c + 1 < h->n && h->a[c + 1].cost < h->a[c].cost) c++;
        if (h->a[c].cost >= last.cost) break;
        h->a[i] = h->a[c];
        i = c;
    }
    if (h->n) h->a[i] = last;
    return 1;
}

/* the faces of the island being built, bucketed by pixel cell, to find overlaps quickly. A slot is
   in use when its gen is the current one, so starting the next island is just gen++. */
typedef struct {
    int cap, used, gen, *kx, *ky, *sg, *head;   /* slots */
    int *ef, *en, ne, me;                        /* entries: face, next entry */
    int *big, nbig, mbig;                        /* faces over too many cells: always checked */
    float cell;
} QGrid;

#define QG_BIG 4096   /* cells */

static int qg_slot(QGrid *g, int x, int y, int make)
{
    unsigned i = ((unsigned)x * 73856093u ^ (unsigned)y * 19349663u) & (unsigned)(g->cap - 1);
    for (;; i = (i + 1) & (unsigned)(g->cap - 1)) {
        if (g->sg[i] != g->gen) {
            if (!make) return -1;
            g->sg[i] = g->gen; g->kx[i] = x; g->ky[i] = y; g->head[i] = -1;
            g->used++;
            return (int)i;
        }
        if (g->kx[i] == x && g->ky[i] == y) return (int)i;
    }
}

static void qg_alloc(QGrid *g, int cap)
{
    g->cap = cap;
    g->kx = xrealloc(NULL, cap * sizeof(int)); g->ky = xrealloc(NULL, cap * sizeof(int));
    g->head = xrealloc(NULL, cap * sizeof(int)); g->sg = xcalloc(cap, sizeof(int));
}

static void qg_add(QGrid *g, int f, const float *b)
{
    int x0 = (int)floorf(b[0] / g->cell), x1 = (int)floorf(b[2] / g->cell), y0 = (int)floorf(b[1] / g->cell), y1 = (int)floorf(b[3] / g->cell);
    if ((double)(x1 - x0 + 1) * (y1 - y0 + 1) > QG_BIG) {
        if (g->nbig == g->mbig) { g->mbig = g->mbig * 2 + 16; g->big = xrealloc(g->big, g->mbig * sizeof(int)); }
        g->big[g->nbig++] = f;
        return;
    }
    for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++) {
            if (2 * (g->used + 1) > g->cap) {   /* grow: move this island's slots over */
                QGrid o = *g;
                qg_alloc(g, o.cap * 2);
                g->gen = 1;
                g->used = 0;
                for (int i = 0; i < o.cap; i++)
                    if (o.sg[i] == o.gen) g->head[qg_slot(g, o.kx[i], o.ky[i], 1)] = o.head[i];
                free(o.kx); free(o.ky); free(o.head); free(o.sg);
            }
            int s = qg_slot(g, x, y, 1);
            if (g->ne == g->me) { g->me = g->me * 2 + 256; g->ef = xrealloc(g->ef, g->me * sizeof(int)); g->en = xrealloc(g->en, g->me * sizeof(int)); }
            g->ef[g->ne] = f;
            g->en[g->ne] = g->head[s];
            g->head[s] = g->ne++;
        }
}

typedef struct {
    const Mesh *m; const V3 *wp; const unsigned char *use, *join; const int *ce; const float *bend;
    float *c2, *ci, *t2, *ti, *fl, *len, *bb, density;   /* c2: corners in whole pixels (y up), ci: unrounded */
    int *isl, *nextf, *stamp, st;
    unsigned char *rect, *isrect;   /* quads that become rectangles / faces placed as rectangles */
    Grow *gr; QGrid grid; QHeap heap;
} QCtx;

static int q_fits(float got, float want, float k) { return fabsf(got - want) <= 1 || (got <= want * k && got * k >= want); }

/* is quad f fit to be a rectangle: no corner close to straight (that is a triangle in disguise) */
static int q_quad_ok(const QCtx *q, int f)
{
    const Mesh *m = q->m;
    int s = m->fo[f];
    float l[4];
    if (m->fo[f + 1] - s != 4) return 0;
    for (int k = 0; k < 4; k++) {
        const float *p = q->fl + 2 * (s + (k + 3) % 4), *c = q->fl + 2 * (s + k), *n = q->fl + 2 * (s + (k + 1) % 4);
        float ax = p[0] - c[0], ay = p[1] - c[1], bx = n[0] - c[0], by = n[1] - c[1];
        float la = sqrtf(ax * ax + ay * ay), lb = sqrtf(bx * bx + by * by);
        if (la < 1e-6f || lb < 1e-6f) return 0;
        if ((ax * bx + ay * by) / (la * lb) < -0.9f) return 0;          /* nearly straight */
        if (bx * ay - by * ax < 0) return 0;                              /* concave */
        l[k] = lb;
    }
    return 0.5f * (l[0] + l[2]) >= 1 && 0.5f * (l[1] + l[3]) >= 1;      /* under a pixel: its own shape */
}

/* face g laid next to placed face f across edge e, into t2 / ti (its own corners); 0 if it can't go
   there. *dist says how much it had to stretch. */
static int q_place(QCtx *q, int f, int e, int g, float *dist)
{
    const Mesh *m = q->m;
    int a = m->ev[2 * e], b = m->ev[2 * e + 1];
    int caf = corner_at(m, f, a), cbf = corner_at(m, f, b), cag = corner_at(m, g, a), cbg = corner_at(m, g, b);
    if (caf < 0 || cbf < 0 || cag < 0 || cbg < 0) return 0;
    int s = m->fo[g], n = m->fo[g + 1] - s;
    if (s + (cbg - s + 1) % n != cag) {   /* name them so g runs from b to a */
        int t = caf; caf = cbf; cbf = t;
        t = cag; cag = cbg; cbg = t;
        t = a; a = b; b = t;
    }
    const float *A = q->c2 + 2 * caf, *B = q->c2 + 2 * cbf, *Ai = q->ci + 2 * caf, *Bi = q->ci + 2 * cbf;
    float dx = A[0] - B[0], dy = A[1] - B[1], L = sqrtf(dx * dx + dy * dy), k = q->bend[e] > 4e-5f ? Q_STRETCH : Q_STRETCH_FLAT;
    if (q->rect[g] && L >= 0.5f && (dx == 0 || dy == 0)) {
        int k0 = cbg - s, k1 = (k0 + 1) % 4, k2 = (k0 + 2) % 4, k3 = (k0 + 3) % 4;
        float along = 0.5f * (q->len[s + k0] + q->len[s + k2]) * q->density, perp = 0.5f * (q->len[s + k1] + q->len[s + k3]) * q->density;
        float nx = -dy / L, ny = dx / L;   /* into g (faces turn counterclockwise) */
        float nearI = 0.5f * ((Ai[0] + Bi[0]) * nx + (Ai[1] + Bi[1]) * ny), nearP = A[0] * nx + A[1] * ny;
        /* the far side lands where the unrounded one would, so rounding doesn't add up along a strip */
        float P = roundf(nearI + perp) - nearP, farI = nearI + perp;
        if (P < 1) P = 1;
        /* a neighbor already placed on either side sets the row height, so the grid stays whole */
        for (int side = 0; side < 2; side++) {
            int ce = q->ce[s + (side ? k3 : k1)], base = side ? cbf : caf, vb = side ? b : a, vs = m->fi[s + (side ? k3 : k2)];
            if (ce < 0) continue;
            int h = m->ef[2 * ce] == g ? m->ef[2 * ce + 1] : m->ef[2 * ce];
            if (h < 0 || h == g || q->isl[h] != q->isl[f]) continue;
            int hb = corner_at(m, h, vb), hs = corner_at(m, h, vs);
            if (hb < 0 || hs < 0) continue;
            if (q->c2[2 * hb] != q->c2[2 * base] || q->c2[2 * hb + 1] != q->c2[2 * base + 1]) {
                /* that neighbor is elsewhere (across a strip's seam): coming in from here would put g
                   out of line with it, so g waits to be joined from the neighbor's side */
                if (q->isrect[h]) return 0;
                continue;
            }
            float ex = q->c2[2 * hs] - q->c2[2 * base], ey = q->c2[2 * hs + 1] - q->c2[2 * base + 1], pn = ex * nx + ey * ny;
            if (fabsf(ex * dx + ey * dy) > 1e-3f || pn < 1 || !q_fits(pn, perp, k)) continue;
            P = pn;
            farI = q->ci[2 * hs] * nx + q->ci[2 * hs + 1] * ny;
            break;
        }
        if (!q_fits(L, along, k) || !q_fits(P, perp, k)) return 0;
        float *t = q->t2 + 2 * s, *ti = q->ti + 2 * s, mv = farI - nearI;
        t[2 * k0] = B[0]; t[2 * k0 + 1] = B[1]; t[2 * k1] = A[0]; t[2 * k1 + 1] = A[1];
        t[2 * k2] = A[0] + nx * P; t[2 * k2 + 1] = A[1] + ny * P;
        t[2 * k3] = B[0] + nx * P; t[2 * k3 + 1] = B[1] + ny * P;
        ti[2 * k0] = Bi[0]; ti[2 * k0 + 1] = Bi[1]; ti[2 * k1] = Ai[0]; ti[2 * k1 + 1] = Ai[1];
        ti[2 * k2] = Ai[0] + nx * mv; ti[2 * k2 + 1] = Ai[1] + ny * mv;
        ti[2 * k3] = Bi[0] + nx * mv; ti[2 * k3 + 1] = Bi[1] + ny * mv;
        *dist = fabsf(logf(L / fmaxf(along, 0.5f)));
        return 1;
    }
    if (q->rect[g]) return 0;   /* a quad that would come out slanted waits to be a rectangle of its own */
    /* its own shape, turned and scaled (a little) so its copy of the edge lands on f's. This works
       on the unrounded spots, so faces smaller than a pixel can shrink to nothing as they should. */
    const float *Ga = q->fl + 2 * cag, *Gb = q->fl + 2 * cbg;
    float gx = Ga[0] - Gb[0], gy = Ga[1] - Gb[1], lg = sqrtf(gx * gx + gy * gy);
    float ix = Ai[0] - Bi[0], iy = Ai[1] - Bi[1], li = sqrtf(ix * ix + iy * iy);
    if (lg < 1e-6f || li < 1e-6f || !q_fits(li, lg, k)) return 0;
    float th = atan2f(iy, ix) - atan2f(gy, gx), sc = li / lg, cs = cosf(th) * sc, sn = sinf(th) * sc;
    for (int j = 0; j < n; j++) {
        float px = q->fl[2 * (s + j)] - Gb[0], py = q->fl[2 * (s + j) + 1] - Gb[1];
        float x = Bi[0] + cs * px - sn * py, y = Bi[1] + sn * px + cs * py;
        q->ti[2 * (s + j)] = x;
        q->ti[2 * (s + j) + 1] = y;
        q->t2[2 * (s + j)] = roundf(x);
        q->t2[2 * (s + j) + 1] = roundf(y);
    }
    memcpy(q->t2 + 2 * cag, A, 2 * sizeof(float));
    memcpy(q->t2 + 2 * cbg, B, 2 * sizeof(float));
    memcpy(q->ti + 2 * cag, Ai, 2 * sizeof(float));
    memcpy(q->ti + 2 * cbg, Bi, 2 * sizeof(float));
    *dist = fabsf(logf(sc));
    return 1;
}

static int q_hit1(QCtx *q, int g, const float *gb, int h)
{
    if (q->stamp[h] == q->st) return 0;
    q->stamp[h] = q->st;
    if (!box_hit(gb, q->bb + 4 * h)) return 0;
    return (q->rect[g] && q->isrect[h]) || face2_overlap(q->m, q->t2, g, q->c2, h);
}

/* does face g at t2 overlap a face of the island? */
static int q_hits(QCtx *q, int g, const float *gb)
{
    QGrid *G = &q->grid;
    int x0 = (int)floorf(gb[0] / G->cell), x1 = (int)floorf(gb[2] / G->cell), y0 = (int)floorf(gb[1] / G->cell), y1 = (int)floorf(gb[3] / G->cell);
    q->st++;
    for (int i = 0; i < G->nbig; i++)
        if (q_hit1(q, g, gb, G->big[i])) return 1;
    if ((double)(x1 - x0 + 1) * (y1 - y0 + 1) > QG_BIG) {   /* a huge face: every face of the island */
        for (int i = 0; i < G->ne; i++)
            if (q_hit1(q, g, gb, G->ef[i])) return 1;
        return 0;
    }
    for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++) {
            int sl = qg_slot(G, x, y, 0);
            for (int i = sl < 0 ? -1 : G->head[sl]; i >= 0; i = G->en[i])
                if (q_hit1(q, g, gb, G->ef[i])) return 1;
        }
    return 0;
}

static float q_area(const Mesh *m, const float *c2, int f)
{
    int s = m->fo[f], n = m->fo[f + 1] - s;
    float a = 0;
    for (int k = 0; k < n; k++) {
        const float *p = c2 + 2 * (s + k), *r = c2 + 2 * (s + (k + 1) % n);
        a += p[0] * r[1] - r[0] * p[1];
    }
    return 0.5f * a;
}

static void q_add(QCtx *q, int id, int f)
{
    face_bb(q->m, q->c2, f, q->bb);
    q->isl[f] = id;
    q->nextf[f] = -1;
    Grow *G = &q->gr[id];
    if (G->n) q->nextf[G->tail] = f;
    else G->head = f;
    G->tail = f;
    G->n++;
    if (q_area(q->m, q->c2, f) > 0) qg_add(&q->grid, f, q->bb + 4 * f);
    /* the faces it leads to, cheapest first: gentle bends, little stretch; upright edges a touch sooner */
    const Mesh *m = q->m;
    for (int c = m->fo[f]; c < m->fo[f + 1]; c++) {
        int e = q->ce[c];
        if (e < 0 || !q->join[e]) continue;
        int g = m->ef[2 * e] == f ? m->ef[2 * e + 1] : m->ef[2 * e];
        float dist;
        if (g < 0 || q->isl[g] >= 0 || !q_place(q, f, e, g, &dist)) continue;
        V3 d = v3norm(v3sub(q->wp[m->ev[2 * e + 1]], q->wp[m->ev[2 * e]]));
        qh_push(&q->heap, q->bend[e] + 4 * dist - 0.002f * fabsf(d.y), e, f);
    }
}

/* face f alone as a new island: a quad as an upright rectangle, other faces in their own shape
   with the pixel grid along an edge */
static void q_seed(QCtx *q, int id, int f)
{
    const Mesh *m = q->m;
    int s = m->fo[f], n = m->fo[f + 1] - s;
    float *c = q->c2 + 2 * s, *ci = q->ci + 2 * s;
    if (q->rect[f]) {
        float h[2] = { 0, 0 }, l[4];
        for (int k = 0; k < 4; k++) {
            const float *p = q->fl + 2 * (s + k), *r = q->fl + 2 * (s + (k + 1) % 4);
            l[k] = sqrtf((r[0] - p[0]) * (r[0] - p[0]) + (r[1] - p[1]) * (r[1] - p[1]));
            h[k & 1] += fabsf(r[0] - p[0]) / l[k];
        }
        int k0 = h[0] >= h[1] ? 0 : 1;
        const float *p0 = q->fl + 2 * (s + k0), *p1 = q->fl + 2 * (s + (k0 + 1) % 4);
        float w = 0.5f * (l[k0] + l[(k0 + 2) % 4]), ht = 0.5f * (l[(k0 + 1) % 4] + l[(k0 + 3) % 4]), sx = p1[0] >= p0[0] ? 1.0f : -1.0f;
        float W = fmaxf(1, roundf(w)), H = fmaxf(1, roundf(ht));
        float cx[4] = { 0, sx, sx, 0 }, cy[4] = { 0, 0, sx, sx };
        for (int j = 0; j < 4; j++) {
            int k = (k0 + j) % 4;
            c[2 * k] = cx[j] * W; c[2 * k + 1] = cy[j] * H;
            ci[2 * k] = cx[j] * w; ci[2 * k + 1] = cy[j] * ht;
        }
        q->isrect[f] = 1;
    } else {
        memcpy(ci, q->fl + 2 * s, 2 * n * sizeof(float));
        straighten(m, q->ci, f, &f, 1);
        for (int k = 0; k < 2 * n; k++) c[k] = roundf(ci[k]);
    }
    q->gr[id].n = 0;
    q->gr[id].folded = 0;
    q->gr[id].fold[0] = q->gr[id].fold[1] = 0;
    q->grid.gen++;
    q->grid.used = 0;
    q->grid.ne = 0;
    q->grid.nbig = 0;
    q->heap.n = 0;
    q_add(q, id, f);
}

static const float *sortSeed;
static const unsigned char *sortRect;
static int cmp_seed(const void *a, const void *b)
{
    int i = *(const int *)a, j = *(const int *)b;
    if (sortRect[i] != sortRect[j]) return sortRect[j] - sortRect[i];
    float d = sortSeed[j] - sortSeed[i];
    return d > 0 ? 1 : d < 0 ? -1 : i - j;
}

/* lays out the faces in use as islands (c2: pixels, y up); returns how many islands it made */
static int q_grow(const Mesh *m, const V3 *wp, const V3 *fn, const float *fa, const unsigned char *use, const unsigned char *join,
                  const int *ce, const float *bend, float density, float *c2, float *t2, float *bb, int *isl, int *nextf, Grow *gr)
{
    int nf = m->nf, nc = m->fo[nf], nb = 0;
    QCtx q;
    memset(&q, 0, sizeof q);
    q.m = m; q.wp = wp; q.use = use; q.join = join; q.ce = ce; q.bend = bend; q.density = density;
    q.c2 = c2; q.t2 = t2; q.bb = bb; q.isl = isl; q.nextf = nextf; q.gr = gr;
    q.ci = xrealloc(NULL, (2 * (size_t)nc + 2) * sizeof(float));
    q.ti = xrealloc(NULL, (2 * (size_t)nc + 2) * sizeof(float));
    q.fl = xrealloc(NULL, (2 * (size_t)nc + 2) * sizeof(float));
    q.len = xrealloc(NULL, ((size_t)nc + 1) * sizeof(float));
    q.stamp = xcalloc(nf + 1, sizeof(int));
    q.rect = xcalloc(nf + 1, 1);
    q.isrect = xcalloc(nf + 1, 1);
    float *key = xrealloc(NULL, (nf + 1) * sizeof(float)), el = 0;
    int *ord = xrealloc(NULL, (nf + 1) * sizeof(int)), no = 0, nel = 0;
    for (int f = 0; f < nf; f++) {
        V3 R, U;
        face_frame(fn[f], &R, &U);
        int s = m->fo[f], n = m->fo[f + 1] - s;
        for (int k = 0; k < n; k++) {
            V3 p = wp[m->fi[s + k]];
            q.fl[2 * (s + k)] = v3dot(p, R) * density;
            q.fl[2 * (s + k) + 1] = v3dot(p, U) * density;
            q.len[s + k] = v3len(v3sub(wp[m->fi[s + (k + 1) % n]], p));
            if (use[f]) { el += q.len[s + k]; nel++; }
        }
        if (!use[f]) continue;
        q.rect[f] = (unsigned char)q_quad_ok(&q, f);
        /* seeds: quads first, biggest first, walls before floors when it's a tie */
        key[f] = fa[f] * (1 + 1e-3f * (1 - fabsf(fn[f].y)));
        ord[no++] = f;
    }
    sortSeed = key;
    sortRect = q.rect;
    qsort(ord, no, sizeof(int), cmp_seed);
    q.grid.cell = fmaxf(4, roundf(nel ? el / nel * density : 4));
    qg_alloc(&q.grid, 1024);
    for (int i = 0; i < no; i++) {
        int f = ord[i];
        if (isl[f] >= 0) continue;
        int id = nb++;
        q_seed(&q, id, f);
        QEnt it;
        while (qh_pop(&q.heap, &it)) {
            int g = m->ef[2 * it.e] == it.f ? m->ef[2 * it.e + 1] : m->ef[2 * it.e];
            float dist, gb[4];
            if (isl[g] >= 0 || !q_place(&q, it.f, it.e, g, &dist)) continue;
            /* faces in their own shape only bend across edges parallel to the island's other bends,
               so they unroll in strips; a grid of quad rectangles may curve both ways (each quad is
               stretched a little at most) */
            int caf = corner_at(m, it.f, m->ev[2 * it.e]), cbf = corner_at(m, it.f, m->ev[2 * it.e + 1]);
            float dx = q.ci[2 * cbf] - q.ci[2 * caf], dy = q.ci[2 * cbf + 1] - q.ci[2 * caf + 1], dl = sqrtf(dx * dx + dy * dy);
            int bent = bend[it.e] > 4e-5f, grid = q.isrect[it.f] && q.rect[g];
            Grow *G = &gr[id];
            if (bent && !grid && G->folded && fabsf(G->fold[0] * dx + G->fold[1] * dy) < 0.866f * dl) continue;
            /* rounding may squash a face smaller than a pixel, but not one that should get pixels.
               A squashed face covers nothing, so it can't overlap anything either. */
            float ar = q_area(m, t2, g);
            if (dl < 1e-9f || (ar <= 0 && q_area(m, q.ti, g) >= 2)) continue;
            gb[0] = gb[1] = 1e30f;
            gb[2] = gb[3] = -1e30f;
            for (int c = m->fo[g]; c < m->fo[g + 1]; c++) {
                gb[0] = fminf(gb[0], t2[2 * c]); gb[1] = fminf(gb[1], t2[2 * c + 1]);
                gb[2] = fmaxf(gb[2], t2[2 * c]); gb[3] = fmaxf(gb[3], t2[2 * c + 1]);
            }
            if (ar > 0 && q_hits(&q, g, gb)) continue;
            int s = m->fo[g], n = m->fo[g + 1] - s;
            memcpy(c2 + 2 * s, t2 + 2 * s, 2 * n * sizeof(float));
            memcpy(q.ci + 2 * s, q.ti + 2 * s, 2 * n * sizeof(float));
            q.isrect[g] = q.rect[g];
            if (bent && !G->folded) { G->fold[0] = dx / dl; G->fold[1] = dy / dl; G->folded = 1; }
            else if (bent && fabsf(G->fold[0] * dx + G->fold[1] * dy) < 0.866f * dl) G->fold[0] = G->fold[1] = 0;   /* curved both ways */
            q_add(&q, id, g);
        }
    }
    free(q.ci); free(q.ti); free(q.fl); free(q.len); free(q.stamp); free(q.rect); free(q.isrect);
    free(q.grid.kx); free(q.grid.ky); free(q.grid.head); free(q.grid.sg); free(q.grid.ef); free(q.grid.en); free(q.grid.big);
    free(q.heap.a); free(key); free(ord);
    return nb;
}

static int pot(int x)
{
    int p = 1;
    while (p < x && p < (1 << 28)) p *= 2;
    return p;
}

static const Isl *sortIsl;
static int cmp_isl(const void *a, const void *b)
{
    const Isl *p = &sortIsl[*(const int *)a], *q = &sortIsl[*(const int *)b];
    if (p->h != q->h) return q->h - p->h;
    if (p->w != q->w) return q->w - p->w;
    return *(const int *)a - *(const int *)b;
}

/* rows of islands, tallest first, across width W; returns the height used */
static int shelf(Isl *is, const int *io, int n, int W, int place)
{
    int x = 0, y = 0, rh = 0;
    for (int i = 0; i < n; i++) {
        Isl *I = &is[io[i]];
        if (I->w > W) return 1 << 29;
        if (x && x + I->w > W) { y += rh + UV_GAP; x = 0; rh = 0; }
        if (place) { I->x = x; I->y = y; }
        x += I->w + UV_GAP;
        if (I->h > rh) rh = I->h;
    }
    return y + rh;
}

static void sat_build(const unsigned char *occ, int W, int H, int *sat)
{
    for (int x = 0; x <= W; x++) sat[x] = 0;
    for (int y = 0; y < H; y++) {
        int row = 0;
        sat[(y + 1) * (W + 1)] = 0;
        for (int x = 0; x < W; x++) {
            row += occ[y * W + x];
            sat[(y + 1) * (W + 1) + x + 1] = sat[y * (W + 1) + x + 1] + row;
        }
    }
}

/* first spot, row by row, where a w x h island and the gap around it are free */
static int find_free(const int *sat, int W, int H, int w, int h, int *ox, int *oy)
{
    for (int y = 0; y + h <= H; y++)
        for (int x = 0; x + w <= W; x++) {
            int x0 = x - UV_GAP < 0 ? 0 : x - UV_GAP, y0 = y - UV_GAP < 0 ? 0 : y - UV_GAP;
            int x1 = x + w + UV_GAP > W ? W : x + w + UV_GAP, y1 = y + h + UV_GAP > H ? H : y + h + UV_GAP;
            int W1 = W + 1;
            if (!(sat[y1 * W1 + x1] - sat[y0 * W1 + x1] - sat[y1 * W1 + x0] + sat[y0 * W1 + x0])) { *ox = x; *oy = y; return 1; }
        }
    return 0;
}

typedef struct { unsigned char *occ; int W; } OccCtx;

static void occ_put(void *ctx, int x, int y, const float w[3])
{
    OccCtx *c = ctx;
    (void)w;
    c->occ[y * c->W + x] = 1;
}

typedef struct {
    const Mesh *m; const Tex *old; const int *ocov; const float *nuv; Tex *nt; int c[3], bake;
} FillCtx;

static void fill_put(void *ctx, int x, int y, const float w[3])
{
    FillCtx *k = ctx;
    const Mesh *m = k->m;
    unsigned char *out = k->nt->px + 4 * ((size_t)y * k->nt->w + x);
    V3 col = v3(0, 0, 0);
    for (int j = 0; j < 3; j++) col = v3add(col, v3mul(m->c[m->fi[k->c[j]]], w[j]));
    float cc[3] = { col.x, col.y, col.z };
    if (k->old && m->uv) {
        float u = 0, v = 0;
        for (int j = 0; j < 3; j++) { u += w[j] * m->uv[2 * k->c[j]]; v += w[j] * m->uv[2 * k->c[j] + 1]; }
        memcpy(out, tex_at(k->old, k->ocov, u, v), 4);
        if (k->bake)
            for (int j = 0; j < 3; j++) {
                float f = out[j] * (cc[j] < 0 ? 0 : cc[j] > 1 ? 1 : cc[j]);
                out[j] = (unsigned char)(f + 0.5f);
            }
    } else {
        for (int j = 0; j < 3; j++) out[j] = (unsigned char)((cc[j] < 0 ? 0 : cc[j] > 1 ? 1 : cc[j]) * 255 + 0.5f);
        out[3] = 255;
    }
}

int mesh_pixel_unwrap(Mesh *m, M4 xf, float density, float angle, const unsigned char *fmask, int maxsize, int quads)
{
    int nf = m->nf, nc = m->fo[nf], partial = fmask && m->uv && m->tex, nuse = 0;
    if (!nf || !(density > 0)) return 0;
    unsigned char *use = xcalloc(nf + 1, 1);
    for (int f = 0; f < nf; f++) nuse += use[f] = (unsigned char)(partial ? fmask[f] != 0 : 1);
    if (!nuse) { free(use); return 0; }

    V3 *wp = world_pts(m, xf), *fn = xrealloc(NULL, (nf + 1) * sizeof(V3));
    float *fa = xrealloc(NULL, (nf + 1) * sizeof(float));
    for (int f = 0; f < nf; f++) {
        V3 nn = newell(wp, m->fi + m->fo[f], m->fo[f + 1] - m->fo[f]);
        fa[f] = 0.5f * v3len(nn);
        fn[f] = v3norm(nn);
    }
    /* edges an island may unfold across */
    int *ce = corner_edges(m);
    unsigned char *join = xcalloc(m->ne + 1, 1);
    float ct = angle >= 180 ? -2.0f : cosf(fmaxf(angle, 0.5f) * DEG2RAD) - 1e-6f;
    for (int e = 0; e < m->ne; e++) {
        int f0 = m->ef[2 * e], f1 = m->ef[2 * e + 1], a = m->ev[2 * e], b = m->ev[2 * e + 1];
        int h = m->eh ? m->eh[e] : EH_AUTO;
        if (f0 < 0 || f1 < 0 || f0 == f1 || !use[f0] || !use[f1] || h == EH_HARD) continue;
        if (h != EH_SOFT && v3dot(fn[f0], fn[f1]) < ct) continue;
        join[e] = (unsigned char)(runs(m, f0, a, b) != runs(m, f1, a, b));
    }

    /* islands. Follow Quads grows each one from its biggest face (q_grow). Otherwise joinable edges
       go gentlest bend first (flat ones before any bend): each joins the islands on its two sides if
       they fit together without overlapping. Either way an island only bends across edges parallel
       to its earlier bends, so curved surfaces unfold into strips. */
    float *c2 = xrealloc(NULL, (2 * (size_t)nc + 2) * sizeof(float)), *t2 = xrealloc(NULL, (2 * (size_t)nc + 2) * sizeof(float));
    float *bb = xrealloc(NULL, (4 * (size_t)nf + 4) * sizeof(float)), *bend = xrealloc(NULL, (m->ne + 1) * sizeof(float));
    int *isl = xrealloc(NULL, (nf + 1) * sizeof(int)), *nextf = xrealloc(NULL, (nf + 1) * sizeof(int));
    int *eo = xrealloc(NULL, (m->ne + 1) * sizeof(int)), *list = xrealloc(NULL, (nf + 1) * sizeof(int));
    int neo = 0, nl = 0, nis = 0, nb = 0;
    Grow *gr = xrealloc(NULL, (nf + 1) * sizeof(Grow));
    for (int f = 0; f < nf; f++) isl[f] = -1;
    for (int e = 0; e < m->ne; e++)
        if (join[e]) {
            bend[e] = 1 - v3dot(fn[m->ef[2 * e]], fn[m->ef[2 * e + 1]]);
            eo[neo++] = e;
        }
    if (quads) {
        nb = q_grow(m, wp, fn, fa, use, join, ce, bend, density, c2, t2, bb, isl, nextf, gr);
    } else {
        sortKey = bend;
        qsort(eo, neo, sizeof(int), cmp_key_idx);
        for (int i = 0; i < neo; i++) {
            int e = eo[i], f[2] = { m->ef[2 * e], m->ef[2 * e + 1] };
            for (int k = 0; k < 2; k++)
                if (isl[f[k]] < 0) grow_seed(m, wp, fn, c2, bb, isl, nextf, gr, nb++, f[k]);
            if (isl[f[0]] != isl[f[1]]) grow_join(m, c2, t2, bb, isl, nextf, gr, e, bend[e] > 4e-5f);
        }
        for (int f = 0; f < nf; f++)
            if (use[f] && isl[f] < 0) grow_seed(m, wp, fn, c2, bb, isl, nextf, gr, nb++, f);
    }
    Isl *is = xrealloc(NULL, (nb + 1) * sizeof(Isl));
    for (int g = 0; g < nb; g++) {
        if (!gr[g].n) continue;
        Isl *I = &is[nis++];
        int big = gr[g].head;
        I->first = nl;
        for (int f = gr[g].head; f >= 0; f = nextf[f]) {
            list[nl++] = f;
            if (fa[f] > fa[big]) big = f;
        }
        I->n = nl - I->first;
        /* the biggest face sets which way is up, then the pixel grid lines up with it (Follow Quads
           islands start from their biggest face, already upright and in pixels) */
        if (!quads) {
            upright(m, wp, fn, c2, big, list + I->first, I->n);
            straighten(m, c2, big, list + I->first, I->n);
        }
        /* to pixels, y down, every corner on a pixel corner */
        float mnx = 1e30f, mny = 1e30f, sc = quads ? 1 : density;
        for (int j = 0; j < I->n; j++)
            for (int c = m->fo[list[I->first + j]]; c < m->fo[list[I->first + j] + 1]; c++) {
                c2[2 * c] *= sc;
                c2[2 * c + 1] *= -sc;
                mnx = fminf(mnx, c2[2 * c]);
                mny = fminf(mny, c2[2 * c + 1]);
            }
        I->w = I->h = 1;
        for (int j = 0; j < I->n; j++)
            for (int c = m->fo[list[I->first + j]]; c < m->fo[list[I->first + j] + 1]; c++) {
                c2[2 * c] = roundf(c2[2 * c] - mnx);
                c2[2 * c + 1] = roundf(c2[2 * c + 1] - mny);
                if (c2[2 * c] > I->w) I->w = (int)c2[2 * c];
                if (c2[2 * c + 1] > I->h) I->h = (int)c2[2 * c + 1];
            }
    }

    /* pack */
    int *io = xrealloc(NULL, (nis + 1) * sizeof(int)), W = 0, H = 0, ok = 1;
    for (int i = 0; i < nis; i++) io[i] = i;
    sortIsl = is;
    qsort(io, nis, sizeof(int), cmp_isl);
    Tex *old = m->tex;
    if (!partial) {
        int maxw = 8;
        for (int i = 0; i < nis; i++) if (is[i].w > maxw) maxw = is[i].w;
        /* the smallest texture, but long strips count extra: they're awkward to paint on */
        long long best = 0;
        for (int w = pot(maxw); w <= maxsize; w *= 2) {
            int h = pot(shelf(is, io, nis, w, 0));
            if (h < 8) h = 8;
            if (h > maxsize) continue;
            int aspect = w > h ? w / h : h / w;
            long long score = (long long)w * h * (aspect > 2 ? aspect / 2 : 1) * 4 + (aspect > 1);
            if (!W || score < best) { best = score; W = w; H = h; }
        }
        if (W) shelf(is, io, nis, W, 1);
        else ok = 0;
    } else {
        /* into the free space of the texture, which grows when nothing fits */
        W = old->w;
        H = old->h;
        unsigned char *occ = xcalloc((size_t)W * H + 1, 1);
        OccCtx oc = { occ, W };
        for (int f = 0; f < nf; f++) {
            if (use[f]) continue;
            int s = m->fo[f];
            for (int k = 1; s + k + 1 < m->fo[f + 1]; k++)
                raster_tri(m->uv + 2 * s, m->uv + 2 * (s + k), m->uv + 2 * (s + k + 1), W, H, occ_put, &oc);
            for (int c = s; c < m->fo[f + 1]; c++) {   /* faces squashed to a point still use theirs */
                int x = (int)floorf(m->uv[2 * c]), y = (int)floorf(m->uv[2 * c + 1]);
                if (x >= 0 && y >= 0 && x < W && y < H) occ[y * W + x] = 1;
            }
        }
        int *sat = xrealloc(NULL, (size_t)(W + 1) * (H + 1) * sizeof(int));
        sat_build(occ, W, H, sat);
        for (int i = 0; i < nis && ok; i++) {
            Isl *I = &is[io[i]];
            while (!find_free(sat, W, H, I->w, I->h, &I->x, &I->y)) {
                int W2 = W, H2 = H;
                if (I->w > W) W2 = pot(I->w);
                else if (H < W) H2 = H * 2;
                else W2 = W * 2;
                if (W2 > maxsize || H2 > maxsize) { ok = 0; break; }
                unsigned char *o2 = xcalloc((size_t)W2 * H2 + 1, 1);
                for (int y = 0; y < H; y++) memcpy(o2 + (size_t)y * W2, occ + (size_t)y * W, W);
                free(occ);
                occ = o2;
                W = W2;
                H = H2;
                sat = xrealloc(sat, (size_t)(W + 1) * (H + 1) * sizeof(int));
                sat_build(occ, W, H, sat);
            }
            if (!ok) break;
            for (int y = I->y; y < I->y + I->h; y++) memset(occ + (size_t)y * W + I->x, 1, I->w);
            sat_build(occ, W, H, sat);
        }
        free(occ);
        free(sat);
    }

    if (ok) {
        Tex *nt = tex_new(W, H);
        int *ocov = NULL;
        if (old && m->uv) {
            ocov = xrealloc(NULL, ((size_t)old->w * old->h + 1) * sizeof(int));
            mesh_texel_faces(m, ocov);
        }
        if (partial)
            for (int y = 0; y < old->h; y++) memcpy(nt->px + (size_t)y * W * 4, old->px + (size_t)y * old->w * 4, (size_t)old->w * 4);
        float *nuv = xrealloc(NULL, (2 * (size_t)nc + 2) * sizeof(float));
        if (partial) memcpy(nuv, m->uv, 2 * (size_t)nc * sizeof(float));
        for (int i = 0; i < nis; i++)
            for (int j = 0; j < is[i].n; j++) {
                int f = list[is[i].first + j];
                for (int c = m->fo[f]; c < m->fo[f + 1]; c++) {
                    nuv[2 * c] = c2[2 * c] + is[i].x;
                    nuv[2 * c + 1] = c2[2 * c + 1] + is[i].y;
                }
            }
        FillCtx k = { m, old, ocov, nuv, nt, { 0, 0, 0 }, !partial };
        for (int f = 0; f < nf; f++) {
            if (!use[f]) continue;
            int s = m->fo[f];
            for (int t = 1; s + t + 1 < m->fo[f + 1]; t++) {
                k.c[0] = s; k.c[1] = s + t; k.c[2] = s + t + 1;
                raster_tri(nuv + 2 * s, nuv + 2 * (s + t), nuv + 2 * (s + t + 1), W, H, fill_put, &k);
            }
        }
        free(ocov);
        free(m->uv);
        tex_release(m->tex);
        m->uv = nuv;
        m->tex = nt;
        mesh_uv_changed(m);
        if (!partial)
            for (int v = 0; v < m->nv; v++) m->c[v] = v3(1, 1, 1);
    }
    free(use); free(wp); free(fn); free(fa); free(ce); free(join); free(c2); free(t2); free(bb); free(bend);
    free(isl); free(nextf); free(eo); free(list); free(gr); free(is); free(io);
    return ok ? nuse : -1;
}

/* ---------------------------------------------------------------- mesh rebuilds */

/* b was started with mb_from_verts(m), so its vertex numbers match m's: edge overrides carry over */
static void mesh_replace(Mesh *m, MB *b)
{
    for (int e = 0; e < m->ne; e++)
        if (m->eh && m->eh[e]) mb_hard(b, m->ev[2 * e], m->ev[2 * e + 1], m->eh[e]);
    Mesh n = mb_finish(b);
    if (m->uv && m->tex) {
        n.uv = uv_transfer(m, &n);
        n.tex = m->tex;
        m->tex = NULL;
        mesh_uv_changed(&n);
    }
    mesh_free(m);
    *m = n;
}

static MB mb_from_verts(const Mesh *m)
{
    MB b;
    mb_init(&b);
    for (int v = 0; v < m->nv; v++) mb_vert(&b, m->p[v], m->c[v]);
    return b;
}

/* ---------------------------------------------------------------- mesh ops */

/* Extrude selected faces, keeping adjacent faces together. The extruded faces
   stay selected and sit on top of the original ones until moved. */
int mesh_extrude_faces(Mesh *m)
{
    int nsel = 0;
    for (int f = 0; f < m->nf; f++) nsel += m->fs[f] != 0;
    if (!nsel) return 0;

    MB b = mb_from_verts(m);
    int *dup = xrealloc(NULL, m->nv * sizeof(int));
    int *tmp = xrealloc(NULL, (m->fo[m->nf] + 4) * sizeof(int));
    for (int v = 0; v < m->nv; v++) dup[v] = -1;
    HMap h;
    hm_init(&h, m->fo[m->nf]);

    for (int f = 0; f < m->nf; f++) {
        int o = m->fo[f], n = m->fo[f + 1] - o;
        if (!m->fs[f]) { mb_face(&b, m->fi + o, n, 0); continue; }
        for (int k = 0; k < n; k++) {
            int v = m->fi[o + k];
            if (dup[v] < 0) dup[v] = mb_vert(&b, m->p[v], m->c[v]);
            tmp[k] = dup[v];
            int a = v, c = m->fi[o + (k + 1) % n];
            if (a != c) { int *cnt = hm_get(&h, a, c); *cnt = *cnt < 0 ? 1 : *cnt + 1; }
        }
        mb_face(&b, tmp, n, 1);
    }
    /* side walls along the border of the selected region */
    for (int f = 0; f < m->nf; f++) {
        if (!m->fs[f]) continue;
        int o = m->fo[f], n = m->fo[f + 1] - o;
        for (int k = 0; k < n; k++) {
            int a = m->fi[o + k], c = m->fi[o + (k + 1) % n];
            if (a == c || *hm_get(&h, a, c) != 1) continue;
            int q[4] = { a, c, dup[c], dup[a] };
            mb_face(&b, q, 4, 0);
        }
    }
    hm_free(&h);
    free(dup);
    free(tmp);
    mesh_replace(m, &b);
    return 1;
}

/* Extrude selected edges into new quads; the new outer edges become selected. */
int mesh_extrude_edges(Mesh *m)
{
    int nsel = 0;
    for (int e = 0; e < m->ne; e++) nsel += m->es[e] != 0;
    if (!nsel) return 0;

    MB b = mb_from_verts(m);
    int *dup = xrealloc(NULL, m->nv * sizeof(int));
    int made = 0;
    for (int v = 0; v < m->nv; v++) dup[v] = -1;
    for (int f = 0; f < m->nf; f++) mb_face(&b, m->fi + m->fo[f], m->fo[f + 1] - m->fo[f], 0);

    for (int e = 0; e < m->ne; e++) {
        if (!m->es[e]) continue;
        int a = m->ev[2 * e], c = m->ev[2 * e + 1];
        int f = m->ef[2 * e];
        if (f < 0) continue;
        /* orient so that the existing face runs a->c */
        int o = m->fo[f], n = m->fo[f + 1] - o;
        for (int k = 0; k < n; k++)
            if (m->fi[o + k] == c && m->fi[o + (k + 1) % n] == a) { int t = a; a = c; c = t; break; }
        if (dup[a] < 0) { dup[a] = mb_vert(&b, m->p[a], m->c[a]); b.m.vs[dup[a]] = 2; }
        if (dup[c] < 0) { dup[c] = mb_vert(&b, m->p[c], m->c[c]); b.m.vs[dup[c]] = 2; }
        int q[4] = { c, a, dup[a], dup[c] };
        mb_face(&b, q, 4, 0);
        made++;
    }
    free(dup);
    mesh_replace(m, &b);
    /* the only edges joining two new vertices are the new outer edges */
    for (int e = 0; e < m->ne; e++) m->es[e] = m->vs[m->ev[2 * e]] == 2 && m->vs[m->ev[2 * e + 1]] == 2;
    memset(m->vs, 0, m->nv);
    return made > 0;
}

/* Merge all masked vertices into one vertex at their center. */
int mesh_merge(Mesh *m, const unsigned char *mask)
{
    int cnt = 0, target = -1;
    V3 cen = v3(0, 0, 0), col = v3(0, 0, 0);
    for (int v = 0; v < m->nv; v++)
        if (mask[v]) {
            if (target < 0) target = v;
            cen = v3add(cen, m->p[v]);
            col = v3add(col, m->c[v]);
            cnt++;
        }
    if (cnt < 2) return 0;

    MB b = mb_from_verts(m);
    b.m.p[target] = v3mul(cen, 1.0f / cnt);
    b.m.c[target] = v3mul(col, 1.0f / cnt);
    b.m.vs[target] = 1;
    int *tmp = xrealloc(NULL, (m->fo[m->nf] + 1) * sizeof(int));
    for (int f = 0; f < m->nf; f++) {
        int o = m->fo[f], n = m->fo[f + 1] - o, k2 = 0;
        for (int k = 0; k < n; k++) {
            int v = m->fi[o + k];
            if (mask[v]) v = target;
            if (k2 == 0 || tmp[k2 - 1] != v) tmp[k2++] = v;
        }
        while (k2 > 1 && tmp[k2 - 1] == tmp[0]) k2--;
        if (k2 >= 3) mb_face(&b, tmp, k2, 0);
    }
    free(tmp);
    mesh_replace(m, &b);
    return 1;
}

int mesh_delete_faces(Mesh *m)
{
    int nsel = 0;
    for (int f = 0; f < m->nf; f++) nsel += m->fs[f] != 0;
    if (!nsel) return 0;
    MB b = mb_from_verts(m);
    for (int f = 0; f < m->nf; f++)
        if (!m->fs[f]) mb_face(&b, m->fi + m->fo[f], m->fo[f + 1] - m->fo[f], 0);
    mesh_replace(m, &b);
    return 1;
}

int mesh_raycast(const Mesh *m, V3 o, V3 d, float *tout)
{
    int tri[3];
    float b[3];
    return mesh_raycast_tri(m, o, d, tout, tri, b);
}

int mesh_raycast_tri(const Mesh *m, V3 o, V3 d, float *tout, int tri[3], float bary[3])
{
    int best = -1;
    float bt = 1e30f;
    for (int f = 0; f < m->nf; f++) {
        int s = m->fo[f], n = m->fo[f + 1] - s;
        V3 p0 = m->p[m->fi[s]];
        for (int k = 1; k + 1 < n; k++) {
            V3 e1 = v3sub(m->p[m->fi[s + k]], p0), e2 = v3sub(m->p[m->fi[s + k + 1]], p0);
            V3 pv = v3cross(d, e2);
            float det = v3dot(e1, pv);
            if (fabsf(det) < 1e-12f) continue;
            float inv = 1.0f / det;
            V3 tv = v3sub(o, p0);
            float u = v3dot(tv, pv) * inv;
            if (u < 0 || u > 1) continue;
            V3 qv = v3cross(tv, e1);
            float w = v3dot(d, qv) * inv;
            if (w < 0 || u + w > 1) continue;
            float t = v3dot(e2, qv) * inv;
            if (t > 0 && t < bt) {
                bt = t;
                best = f;
                tri[0] = s; tri[1] = s + k; tri[2] = s + k + 1;
                bary[0] = 1 - u - w; bary[1] = u; bary[2] = w;
            }
        }
    }
    if (tout) *tout = bt;
    return best;
}

Mesh mesh_from_arrays(int nv, const V3 *p, const V3 *c, int nf, const int *fo, const int *fi)
{
    MB b;
    mb_init(&b);
    for (int v = 0; v < nv; v++) mb_vert(&b, p[v], c ? c[v] : v3(1, 1, 1));
    for (int f = 0; f < nf; f++) mb_face(&b, fi + fo[f], fo[f + 1] - fo[f], 0);
    return mb_finish(&b);
}

/* ---------------------------------------------------------------- selection helpers */

/* vertex -> incident edges, CSR layout: edges of v are ve[off[v] .. off[v+1]-1] */
static void vert_edges(const Mesh *m, int **off, int **ve)
{
    int *o = xrealloc(NULL, (m->nv + 1) * sizeof(int)), *e = xrealloc(NULL, (2 * m->ne + 1) * sizeof(int));
    memset(o, 0, (m->nv + 1) * sizeof(int));
    for (int i = 0; i < 2 * m->ne; i++) o[m->ev[i] + 1]++;
    for (int v = 0; v < m->nv; v++) o[v + 1] += o[v];
    int *fill = xrealloc(NULL, (m->nv + 1) * sizeof(int));
    memcpy(fill, o, (m->nv + 1) * sizeof(int));
    for (int i = 0; i < 2 * m->ne; i++) e[fill[m->ev[i]]++] = i / 2;
    free(fill);
    *off = o;
    *ve = e;
}

static int shares_face(const Mesh *m, int a, int b)
{
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            if (m->ef[2 * a + i] >= 0 && m->ef[2 * a + i] == m->ef[2 * b + j]) return 1;
    return 0;
}

/* the edge continuing a loop from edge ce through vertex v, or -1: straight across a 4-valent
   vertex, or along an open border */
static int loop_next(const Mesh *m, const int *off, const int *ve, int ce, int v)
{
    int border = m->ef[2 * ce + 1] < 0, cand = -1, n = 0;
    if (!border && off[v + 1] - off[v] != 4) return -1;
    for (int k = off[v]; k < off[v + 1]; k++) {
        int x = ve[k];
        if (x == ce || shares_face(m, x, ce)) continue;
        if (border && m->ef[2 * x + 1] >= 0) continue;
        cand = x;
        n++;
    }
    return n == 1 ? cand : -1;
}

int mesh_edge_loop(Mesh *m, int e)
{
    if (e < 0 || e >= m->ne) return 0;
    int *off, *ve, n = 1;
    unsigned char *seen = xrealloc(NULL, m->ne + 1);
    vert_edges(m, &off, &ve);
    memset(seen, 0, m->ne + 1);
    seen[e] = 1;
    for (int side = 0; side < 2; side++) {
        int ce = e, v = m->ev[2 * e + side];
        for (;;) {
            int nx = loop_next(m, off, ve, ce, v);
            if (nx < 0 || seen[nx]) break;
            seen[nx] = 1;
            n++;
            v = m->ev[2 * nx] == v ? m->ev[2 * nx + 1] : m->ev[2 * nx];
            ce = nx;
        }
    }
    for (int i = 0; i < m->ne; i++) if (seen[i]) m->es[i] = 1;
    free(seen);
    free(off);
    free(ve);
    return n;
}

void mesh_grow(Mesh *m, int comp, int shrink)
{
    unsigned char *vm = xrealloc(NULL, m->nv + 1);
    memset(vm, 0, m->nv + 1);
    if (!shrink) {
        mesh_vmask(m, comp, vm);
        if (comp == COMP_VERT) {
            for (int e = 0; e < m->ne; e++) {
                int a = m->ev[2 * e], b = m->ev[2 * e + 1];
                if (vm[a] || vm[b]) m->vs[a] = m->vs[b] = 1;
            }
        } else if (comp == COMP_EDGE) {
            for (int e = 0; e < m->ne; e++) if (vm[m->ev[2 * e]] || vm[m->ev[2 * e + 1]]) m->es[e] = 1;
        } else {
            for (int f = 0; f < m->nf; f++)
                for (int k = m->fo[f]; k < m->fo[f + 1]; k++) if (vm[m->fi[k]]) m->fs[f] = 1;
        }
    } else {
        /* vm = vertices touched by something unselected; selected parts touching them go */
        if (comp == COMP_VERT) {
            for (int e = 0; e < m->ne; e++) {
                int a = m->ev[2 * e], b = m->ev[2 * e + 1];
                if (!m->vs[a]) vm[b] = 1;
                if (!m->vs[b]) vm[a] = 1;
            }
            for (int v = 0; v < m->nv; v++) if (vm[v]) m->vs[v] = 0;
        } else if (comp == COMP_EDGE) {
            for (int e = 0; e < m->ne; e++) if (!m->es[e]) vm[m->ev[2 * e]] = vm[m->ev[2 * e + 1]] = 1;
            for (int e = 0; e < m->ne; e++) if (vm[m->ev[2 * e]] || vm[m->ev[2 * e + 1]]) m->es[e] = 0;
        } else {
            for (int f = 0; f < m->nf; f++)
                if (!m->fs[f]) for (int k = m->fo[f]; k < m->fo[f + 1]; k++) vm[m->fi[k]] = 1;
            for (int f = 0; f < m->nf; f++)
                for (int k = m->fo[f]; k < m->fo[f + 1]; k++) if (vm[m->fi[k]]) m->fs[f] = 0;
        }
    }
    free(vm);
}

/* Like Maya: going down (faces to edges/verts, edges to verts) keeps the parts of the selection;
   going up selects everything touching it (edges to their faces, verts to touching edges/faces). */
void mesh_convert(Mesh *m, int from, int to)
{
    if (from == to) return;
    unsigned char *vm = xrealloc(NULL, m->nv + 1);
    mesh_vmask(m, from, vm);
    if (to == COMP_VERT) {
        memcpy(m->vs, vm, m->nv);
    } else if (to == COMP_EDGE) {
        for (int e = 0; e < m->ne; e++) {
            int a = vm[m->ev[2 * e]], b = vm[m->ev[2 * e + 1]];
            if (from == COMP_FACE) {
                int f0 = m->ef[2 * e], f1 = m->ef[2 * e + 1];
                m->es[e] = (f0 >= 0 && m->fs[f0]) || (f1 >= 0 && m->fs[f1]);
            } else {
                m->es[e] = a || b;
            }
        }
    } else {
        memset(m->fs, 0, m->nf);
        if (from == COMP_EDGE) {
            for (int e = 0; e < m->ne; e++)
                if (m->es[e])
                    for (int i = 0; i < 2; i++) if (m->ef[2 * e + i] >= 0) m->fs[m->ef[2 * e + i]] = 1;
        } else {
            for (int f = 0; f < m->nf; f++)
                for (int k = m->fo[f]; k < m->fo[f + 1]; k++) if (vm[m->fi[k]]) m->fs[f] = 1;
        }
    }
    if (from == COMP_VERT) memset(m->vs, 0, m->nv);
    if (from == COMP_EDGE) memset(m->es, 0, m->ne);
    if (from == COMP_FACE) memset(m->fs, 0, m->nf);
    free(vm);
}

int mesh_fill_holes(Mesh *m, const unsigned char *touch)
{
    /* each border edge a->b (as its one face winds it) becomes b->a in the cap */
    int *from = xrealloc(NULL, (m->ne + 1) * sizeof(int)), *to = xrealloc(NULL, (m->ne + 1) * sizeof(int));
    int nh = 0;
    for (int e = 0; e < m->ne; e++) {
        int f = m->ef[2 * e];
        if (f < 0 || m->ef[2 * e + 1] >= 0) continue;
        int a = m->ev[2 * e], b = m->ev[2 * e + 1], o = m->fo[f], n = m->fo[f + 1] - o;
        for (int k = 0; k < n; k++)
            if (m->fi[o + k] == b && m->fi[o + (k + 1) % n] == a) { int t = a; a = b; b = t; break; }
        from[nh] = b;
        to[nh] = a;
        nh++;
    }
    /* trace the loops first; cap[] holds them back to back, cstart[i] where loop i begins */
    unsigned char *used = xrealloc(NULL, nh + 1);
    int *cap = xrealloc(NULL, (nh + 1) * sizeof(int)), *cstart = xrealloc(NULL, (nh + 2) * sizeof(int));
    int made = 0;
    memset(used, 0, nh + 1);
    cstart[0] = 0;
    for (int s = 0; s < nh; s++) {
        if (used[s]) continue;
        int n = 0, h = s, hit = !touch, closed = 0, *loop = cap + cstart[made];
        while (h >= 0 && !used[h]) {
            used[h] = 1;
            loop[n++] = from[h];
            hit |= touch && touch[from[h]];
            if (to[h] == from[s]) { closed = 1; break; }
            int nx = -1;
            for (int k = 0; k < nh && nx < 0; k++)
                if (!used[k] && from[k] == to[h]) nx = k;
            h = nx;
        }
        if (closed && hit && n >= 3) { cstart[made + 1] = cstart[made] + n; made++; }   /* else reuse the space */
    }
    if (made) {
        MB b = mb_from_verts(m);
        for (int f = 0; f < m->nf; f++) mb_face(&b, m->fi + m->fo[f], m->fo[f + 1] - m->fo[f], 0);
        for (int i = 0; i < made; i++) mb_face(&b, cap + cstart[i], cstart[i + 1] - cstart[i], 1);
        mesh_replace(m, &b);
    }
    free(from); free(to); free(used); free(cap); free(cstart);
    return made;
}

/* reverse the corners of face f (and their uvs) */
static void face_reverse(Mesh *m, int f)
{
    for (int a = m->fo[f], b = m->fo[f + 1] - 1; a < b; a++, b--) {
        int t = m->fi[a];
        m->fi[a] = m->fi[b];
        m->fi[b] = t;
        if (m->uv) {
            float u[2];
            memcpy(u, m->uv + 2 * a, sizeof u);
            memcpy(m->uv + 2 * a, m->uv + 2 * b, sizeof u);
            memcpy(m->uv + 2 * b, u, sizeof u);
        }
    }
}

void mesh_flip(Mesh *m)
{
    for (int f = 0; f < m->nf; f++) face_reverse(m, f);
    if (m->uv) mesh_uv_changed(m);
}

/* ---------------------------------------------------------------- editable face lists

   Ops that cut faces apart or join them work on plain per face vertex lists, then rebuild. */

typedef struct { int n, cap; int *v; } IV;

static void iv_push(IV *a, int x)
{
    if (a->n == a->cap) { a->cap = a->cap ? a->cap * 2 : 8; a->v = xrealloc(a->v, a->cap * sizeof(int)); }
    a->v[a->n++] = x;
}

static void iv_insert(IV *a, int at, int x)
{
    iv_push(a, 0);
    memmove(a->v + at + 1, a->v + at, (a->n - 1 - at) * sizeof(int));
    a->v[at] = x;
}

typedef struct { int nf, cf; IV *f; unsigned char *sel; } PL;

static void pl_add(PL *p, const int *v, int n, int sel)
{
    if (p->nf == p->cf) {
        p->cf = p->cf ? p->cf * 2 : 64;
        p->f = xrealloc(p->f, p->cf * sizeof(IV));
        p->sel = xrealloc(p->sel, p->cf);
    }
    IV *f = &p->f[p->nf];
    memset(f, 0, sizeof *f);
    for (int k = 0; k < n; k++) iv_push(f, v[k]);
    p->sel[p->nf++] = (unsigned char)sel;
}

static void pl_from(PL *p, const Mesh *m)
{
    memset(p, 0, sizeof *p);
    for (int f = 0; f < m->nf; f++) pl_add(p, m->fi + m->fo[f], m->fo[f + 1] - m->fo[f], m->fs[f]);
}

static void pl_free(PL *p)
{
    for (int f = 0; f < p->nf; f++) free(p->f[f].v);
    free(p->f);
    free(p->sel);
    memset(p, 0, sizeof *p);
}

static void pl_to_mb(const PL *p, MB *b)
{
    for (int f = 0; f < p->nf; f++)
        if (p->f[f].n >= 3) mb_face(b, p->f[f].v, p->f[f].n, p->sel[f]);
}

/* put vertex x between a and b in every face where they are neighbors */
static void pl_split_edge(PL *p, int a, int b, int x)
{
    for (int f = 0; f < p->nf; f++) {
        IV *v = &p->f[f];
        for (int k = 0; k < v->n; k++) {
            int u = v->v[k], w = v->v[(k + 1) % v->n];
            if ((u == a && w == b) || (u == b && w == a)) { iv_insert(v, k + 1, x); break; }
        }
    }
}

/* split a face holding u and w (not neighbors) along u-w; returns 0 if no face has both */
static int pl_cut(PL *p, int u, int w)
{
    for (int f = 0; f < p->nf; f++) {
        IV *v = &p->f[f];
        int n = v->n, iu = -1, iw = -1;
        for (int k = 0; k < n; k++) { if (v->v[k] == u) iu = k; if (v->v[k] == w) iw = k; }
        if (iu < 0 || iw < 0 || (iu + 1) % n == iw || (iw + 1) % n == iu) continue;
        IV a = { 0, 0, NULL }, b = { 0, 0, NULL };
        for (int k = iu;; k = (k + 1) % n) { iv_push(&a, v->v[k]); if (k == iw) break; }
        for (int k = iw;; k = (k + 1) % n) { iv_push(&b, v->v[k]); if (k == iu) break; }
        int sel = p->sel[f];
        free(v->v);
        p->f[f] = a;
        pl_add(p, b.v, b.n, sel);
        free(b.v);
        return 1;
    }
    return 0;
}

/* ---------------------------------------------------------------- modeling ops */

int mesh_edge_find(const Mesh *m, int a, int b)
{
    if (a > b) { int t = a; a = b; b = t; }
    for (int e = 0; e < m->ne; e++)
        if (m->ev[2 * e] == a && m->ev[2 * e + 1] == b) return e;
    return -1;
}

void mesh_set_eh(Mesh *m, const int *trip, int n)
{
    HMap h;
    edge_map(&h, m);
    for (int i = 0; i < n; i++) {
        int a = trip[3 * i], b = trip[3 * i + 1];
        if (a < 0 || b < 0 || a >= m->nv || b >= m->nv) continue;
        int e = edge_at(&h, a, b);
        if (e >= 0) m->eh[e] = (unsigned char)trip[3 * i + 2];
    }
    hm_free(&h);
}

int mesh_reverse(Mesh *m, const unsigned char *fmask)
{
    int n = 0;
    for (int f = 0; f < m->nf; f++) {
        if (fmask && !fmask[f]) continue;
        face_reverse(m, f);
        n++;
    }
    return n;
}

/* is q inside triangle a b c (all on the plane with normal nn)? */
static int in_tri(V3 q, V3 a, V3 b, V3 c, V3 nn)
{
    return v3dot(v3cross(v3sub(b, a), v3sub(q, a)), nn) >= 0 && v3dot(v3cross(v3sub(c, b), v3sub(q, b)), nn) >= 0 &&
           v3dot(v3cross(v3sub(a, c), v3sub(q, c)), nn) >= 0;
}

/* ear clipping, so concave polygons come out right; writes n-2 triangles */
static void ear_clip(const V3 *p, const int *idx, int n, int *out)
{
    int *r = xrealloc(NULL, n * sizeof(int)), rn = n, o = 0;
    memcpy(r, idx, n * sizeof(int));
    V3 nn = newell(p, idx, n);
    while (rn > 3) {
        int best = -1;
        for (int i = 0; i < rn && best < 0; i++) {
            int a = r[(i + rn - 1) % rn], b = r[i], c = r[(i + 1) % rn];
            if (v3dot(v3cross(v3sub(p[b], p[a]), v3sub(p[c], p[b])), nn) <= 0) continue;   /* reflex */
            int ok = 1;
            for (int j = 0; j < rn && ok; j++) {
                int q = r[j];
                if (q == a || q == b || q == c) continue;
                if (in_tri(p[q], p[a], p[b], p[c], nn)) ok = 0;
            }
            if (ok) best = i;
        }
        if (best < 0) best = 0;   /* degenerate: fall back to a fan step */
        out[o++] = r[(best + rn - 1) % rn];
        out[o++] = r[best];
        out[o++] = r[(best + 1) % rn];
        memmove(r + best, r + best + 1, (rn - best - 1) * sizeof(int));
        rn--;
    }
    out[o++] = r[0]; out[o++] = r[1]; out[o++] = r[2];
    free(r);
}

int mesh_triangulate(Mesh *m, const unsigned char *fmask)
{
    int n = 0;
    for (int f = 0; f < m->nf; f++) n += (!fmask || fmask[f]) && m->fo[f + 1] - m->fo[f] > 3;
    if (!n) return 0;
    MB b = mb_from_verts(m);
    int *tri = xrealloc(NULL, 3 * (m->fo[m->nf] + 1) * sizeof(int));
    for (int f = 0; f < m->nf; f++) {
        int s = m->fo[f], k = m->fo[f + 1] - s;
        if ((fmask && !fmask[f]) || k <= 3) { mb_face(&b, m->fi + s, k, m->fs[f]); continue; }
        ear_clip(m->p, m->fi + s, k, tri);
        for (int t = 0; t < k - 2; t++) mb_face(&b, tri + 3 * t, 3, m->fs[f]);
    }
    free(tri);
    mesh_replace(m, &b);
    return n;
}

typedef struct { float score; int e; } QCand;

static int qcand_cmp(const void *a, const void *b)
{
    float d = ((const QCand *)b)->score - ((const QCand *)a)->score;
    return d > 0 ? 1 : d < 0 ? -1 : 0;
}

/* the quad left when the edge a-b between triangles f0 (a->b) and f1 is removed */
static int tri_pair_quad(const Mesh *m, int e, int q[4])
{
    int f0 = m->ef[2 * e], f1 = m->ef[2 * e + 1], a = m->ev[2 * e], b = m->ev[2 * e + 1];
    if (f0 < 0 || f1 < 0) return 0;
    const int *t0 = m->fi + m->fo[f0], *t1 = m->fi + m->fo[f1];
    int i = 0;
    while (i < 3 && t0[i] != a) i++;
    if (i == 3) return 0;
    if (t0[(i + 1) % 3] != b) { int t = a; a = b; b = t; }   /* now f0 runs a->b */
    int x0 = -1, x1 = -1;
    for (int k = 0; k < 3; k++) { if (t0[k] != a && t0[k] != b) x0 = t0[k]; if (t1[k] != a && t1[k] != b) x1 = t1[k]; }
    if (x0 < 0 || x1 < 0 || x0 == x1) return 0;
    q[0] = a; q[1] = x1; q[2] = b; q[3] = x0;
    return 1;
}

int mesh_quadrangulate(Mesh *m, const unsigned char *fmask, float angle)
{
    QCand *c = xrealloc(NULL, (m->ne + 1) * sizeof(QCand));
    int nc = 0;
    float ct = cosf(angle * DEG2RAD);
    for (int e = 0; e < m->ne; e++) {
        int f0 = m->ef[2 * e], f1 = m->ef[2 * e + 1], q[4];
        if (f0 < 0 || f1 < 0 || m->fo[f0 + 1] - m->fo[f0] != 3 || m->fo[f1 + 1] - m->fo[f1] != 3) continue;
        if (fmask && (!fmask[f0] || !fmask[f1])) continue;
        float d = v3dot(mesh_fnormal(m, f0), mesh_fnormal(m, f1));
        if (d < ct || !tri_pair_quad(m, e, q)) continue;
        V3 nn = newell(m->p, q, 4);
        int convex = 1;
        for (int k = 0; k < 4 && convex; k++) {
            V3 p0 = m->p[q[(k + 3) % 4]], p1 = m->p[q[k]], p2 = m->p[q[(k + 1) % 4]];
            convex = v3dot(v3cross(v3sub(p1, p0), v3sub(p2, p1)), nn) > 0;
        }
        if (!convex) continue;
        /* prefer flat pairs whose quad is closest to a square (diagonals of equal length) */
        float d1 = v3len(v3sub(m->p[q[0]], m->p[q[2]])), d2 = v3len(v3sub(m->p[q[1]], m->p[q[3]]));
        c[nc].score = d + 0.5f * fminf(d1, d2) / fmaxf(fmaxf(d1, d2), 1e-12f);
        c[nc++].e = e;
    }
    qsort(c, nc, sizeof *c, qcand_cmp);
    unsigned char *used = xcalloc(m->nf + 1, 1);
    int *quad = xrealloc(NULL, (m->nf + 1) * 4 * sizeof(int)), *qof = xrealloc(NULL, (m->nf + 1) * sizeof(int)), made = 0;
    for (int f = 0; f < m->nf; f++) qof[f] = -1;
    for (int i = 0; i < nc; i++) {
        int e = c[i].e, f0 = m->ef[2 * e], f1 = m->ef[2 * e + 1];
        if (used[f0] || used[f1]) continue;
        used[f0] = used[f1] = 1;
        tri_pair_quad(m, e, quad + 4 * made);
        qof[f0 < f1 ? f0 : f1] = made;
        qof[f0 < f1 ? f1 : f0] = -2;   /* emitted with its partner */
        made++;
    }
    if (made) {
        MB b = mb_from_verts(m);
        for (int f = 0; f < m->nf; f++) {
            if (qof[f] == -2) continue;
            if (qof[f] >= 0) mb_face(&b, quad + 4 * qof[f], 4, 1);
            else mb_face(&b, m->fi + m->fo[f], m->fo[f + 1] - m->fo[f], m->fs[f]);
        }
        mesh_replace(m, &b);
    }
    free(c); free(used); free(quad); free(qof);
    return made;
}

int mesh_shells(const Mesh *m, int *fshell)
{
    int *par = xrealloc(NULL, (m->nv + 1) * sizeof(int)), *id = xrealloc(NULL, (m->nv + 1) * sizeof(int)), n = 0;
    for (int v = 0; v < m->nv; v++) { par[v] = v; id[v] = -1; }
    for (int f = 0; f < m->nf; f++)
        for (int c = m->fo[f] + 1; c < m->fo[f + 1]; c++)
            par[find_root(par, m->fi[c])] = find_root(par, m->fi[m->fo[f]]);
    for (int f = 0; f < m->nf; f++) {
        int r = find_root(par, m->fi[m->fo[f]]);
        if (id[r] < 0) id[r] = n++;
        fshell[f] = id[r];
    }
    free(par); free(id);
    return n;
}

Mesh mesh_subset(const Mesh *m, const unsigned char *fmask)
{
    MB b = mb_from_verts(m);
    for (int f = 0; f < m->nf; f++)
        if (fmask[f]) mb_face(&b, m->fi + m->fo[f], m->fo[f + 1] - m->fo[f], m->fs[f]);
    for (int e = 0; e < m->ne; e++)
        if (m->eh[e]) mb_hard(&b, m->ev[2 * e], m->ev[2 * e + 1], m->eh[e]);
    Mesh r = mb_finish(&b);
    if (m->uv && m->tex) {   /* the faces keep their corners in order, so their uvs copy straight over */
        int k = 0;
        r.uv = xrealloc(NULL, (2 * (size_t)r.fo[r.nf] + 2) * sizeof(float));
        for (int f = 0; f < m->nf; f++) {
            if (!fmask[f]) continue;
            int n = m->fo[f + 1] - m->fo[f];
            memcpy(r.uv + 2 * k, m->uv + 2 * m->fo[f], 2 * n * sizeof(float));
            k += n;
        }
        r.tex = tex_retain(m->tex);
        mesh_uv_changed(&r);
    }
    return r;
}

/* one texture holding a's picture on top and b's below it (a 1x1 white one for a side without a
   picture: vertex colors keep showing there); *yb gets where b starts */
static Tex *tex_stack(const Tex *a, const Tex *b, int *yb)
{
    int wa = a ? a->w : 1, ha = a ? a->h : 1, wb = b ? b->w : 1, hb = b ? b->h : 1;
    Tex *t = tex_new(wa > wb ? wa : wb, ha + UV_GAP + hb);
    *yb = ha + UV_GAP;
    for (int k = 0; k < 2; k++) {
        const Tex *s = k ? b : a;
        int y0 = k ? *yb : 0;
        if (!s) { memset(t->px + 4 * (size_t)y0 * t->w, 255, 4); continue; }
        for (int y = 0; y < s->h; y++) memcpy(t->px + 4 * ((size_t)(y0 + y) * t->w), s->px + 4 * (size_t)y * s->w, 4 * (size_t)s->w);
    }
    return t;
}

void mesh_append(Mesh *dst, const Mesh *src, const M4 *xf)
{
    const float *x = xf->m;
    int flip = x[0] * (x[5] * x[10] - x[6] * x[9]) - x[4] * (x[1] * x[10] - x[2] * x[9]) + x[8] * (x[1] * x[6] - x[2] * x[5]) < 0;
    MB b = mb_from_verts(dst);
    for (int f = 0; f < dst->nf; f++) mb_face(&b, dst->fi + dst->fo[f], dst->fo[f + 1] - dst->fo[f], dst->fs[f]);
    int base = b.m.nv, *tmp = xrealloc(NULL, (src->fo[src->nf] + 1) * sizeof(int));
    for (int v = 0; v < src->nv; v++) mb_vert(&b, m4_pt(*xf, src->p[v]), src->c[v]);
    for (int f = 0; f < src->nf; f++) {
        int s = src->fo[f], n = src->fo[f + 1] - s;
        for (int k = 0; k < n; k++) tmp[k] = base + src->fi[s + (flip ? n - 1 - k : k)];
        mb_face(&b, tmp, n, src->fs[f]);
    }
    for (int e = 0; e < src->ne; e++)
        if (src->eh[e]) mb_hard(&b, base + src->ev[2 * e], base + src->ev[2 * e + 1], src->eh[e]);
    free(tmp);
    /* textures: the faces keep their corners in order (src's maybe reversed), so the uvs line up */
    float *uv = NULL;
    Tex *tex = NULL;
    int dc = dst->fo[dst->nf], sc = src->fo[src->nf], dt = dst->uv && dst->tex && dst->nf, st = src->uv && src->tex && src->nf;
    if (dt || st) {
        int yb = 0;
        if (!dst->nf) tex = tex_retain(src->tex);
        else tex = tex_stack(dt ? dst->tex : NULL, st ? src->tex : NULL, &yb);
        uv = xrealloc(NULL, (2 * (size_t)(dc + sc) + 2) * sizeof(float));
        for (int c = 0; c < dc; c++) {
            uv[2 * c] = dt ? dst->uv[2 * c] : 0.5f;
            uv[2 * c + 1] = dt ? dst->uv[2 * c + 1] : 0.5f;
        }
        for (int f = 0; f < src->nf; f++) {
            int s = src->fo[f], n = src->fo[f + 1] - s;
            for (int k = 0; k < n; k++) {
                int c = s + (flip ? n - 1 - k : k), o = dc + s + k;
                uv[2 * o] = st ? src->uv[2 * c] : 0.5f;
                uv[2 * o + 1] = (st ? src->uv[2 * c + 1] : 0.5f) + yb;
            }
        }
    }
    free(dst->uv);
    tex_release(dst->tex);
    dst->uv = NULL;
    dst->tex = NULL;
    mesh_replace(dst, &b);
    dst->uv = uv;
    dst->tex = tex;
    if (uv) mesh_uv_changed(dst);
}

/* faces renumbered through to[] (merged vertices point at the one they join); faces lose repeated
   corners, and faces left with fewer than 3 go */
static void mesh_remap(Mesh *m, const int *to)
{
    MB b = mb_from_verts(m);
    int *tmp = xrealloc(NULL, (m->fo[m->nf] + 1) * sizeof(int));
    for (int f = 0; f < m->nf; f++) {
        int s = m->fo[f], n = m->fo[f + 1] - s, k2 = 0;
        for (int k = 0; k < n; k++) {
            int v = to[m->fi[s + k]];
            if (!k2 || tmp[k2 - 1] != v) tmp[k2++] = v;
        }
        while (k2 > 1 && tmp[k2 - 1] == tmp[0]) k2--;
        if (k2 >= 3) mb_face(&b, tmp, k2, m->fs[f]);
    }
    for (int e = 0; e < m->ne; e++)
        if (m->eh[e]) mb_hard(&b, to[m->ev[2 * e]], to[m->ev[2 * e + 1]], m->eh[e]);
    free(tmp);
    mesh_replace(m, &b);
}

static int cmp_x(const void *a, const void *b, const V3 *p)
{
    float d = p[*(const int *)a].x - p[*(const int *)b].x;
    return d > 0 ? 1 : d < 0 ? -1 : 0;
}

static const V3 *sortP;
static int cmp_xp(const void *a, const void *b) { return cmp_x(a, b, sortP); }

int mesh_merge_dist(Mesh *m, const unsigned char *vmask, float dist)
{
    int *idx = xrealloc(NULL, (m->nv + 1) * sizeof(int)), n = 0;
    int *par = xrealloc(NULL, (m->nv + 1) * sizeof(int));
    for (int v = 0; v < m->nv; v++) { par[v] = v; if (!vmask || vmask[v]) idx[n++] = v; }
    sortP = m->p;
    qsort(idx, n, sizeof(int), cmp_xp);
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n && m->p[idx[j]].x - m->p[idx[i]].x <= dist; j++)
            if (v3len(v3sub(m->p[idx[i]], m->p[idx[j]])) <= dist) {
                int a = find_root(par, idx[i]), b = find_root(par, idx[j]);
                if (a != b) par[a > b ? a : b] = a < b ? a : b;   /* the lowest number survives */
            }
    int removed = 0;
    V3 *ps = xcalloc(m->nv + 1, sizeof(V3)), *cs = xcalloc(m->nv + 1, sizeof(V3));
    int *cnt = xcalloc(m->nv + 1, sizeof(int));
    for (int v = 0; v < m->nv; v++) {
        int r = find_root(par, v);
        par[v] = r;
        removed += r != v;
        ps[r] = v3add(ps[r], m->p[v]);
        cs[r] = v3add(cs[r], m->c[v]);
        cnt[r]++;
    }
    if (removed) {
        for (int v = 0; v < m->nv; v++)
            if (cnt[v] > 1) { m->p[v] = v3mul(ps[v], 1.0f / cnt[v]); m->c[v] = v3mul(cs[v], 1.0f / cnt[v]); }
        mesh_remap(m, par);
    }
    free(idx); free(par); free(ps); free(cs); free(cnt);
    return removed;
}

int mesh_weld(Mesh *m, int src, int dst)
{
    if (src == dst || src < 0 || dst < 0 || src >= m->nv || dst >= m->nv) return 0;
    int *to = xrealloc(NULL, (m->nv + 1) * sizeof(int));
    for (int v = 0; v < m->nv; v++) to[v] = v;
    to[src] = dst;
    mesh_remap(m, to);
    free(to);
    return 1;
}

/* Join the faces on each side of the masked edges into one face (where they make a single loop),
   then drop candidate vertices (rm) left between just two edges: interior ones always, border
   corners too when rmBorder. Returns how many faces were joined away. */
static int dissolve(Mesh *m, const unsigned char *emask, const unsigned char *rm, int rmBorder)
{
    int *par = xrealloc(NULL, (m->nf + 1) * sizeof(int)), joined = 0;
    for (int f = 0; f < m->nf; f++) par[f] = f;
    for (int e = 0; e < m->ne; e++) {
        int f0 = m->ef[2 * e], f1 = m->ef[2 * e + 1];
        if (emask[e] && f0 >= 0 && f1 >= 0 && f0 != f1) par[find_root(par, f0)] = find_root(par, f1);
    }
    /* faces of each group as a list: first[root] -> nextg[] */
    int *first = xrealloc(NULL, (m->nf + 1) * sizeof(int)), *nextg = xrealloc(NULL, (m->nf + 1) * sizeof(int));
    int *gsize = xcalloc(m->nf + 1, sizeof(int));
    for (int f = 0; f < m->nf; f++) first[f] = -1;
    for (int f = m->nf - 1; f >= 0; f--) {
        par[f] = find_root(par, f);
        nextg[f] = first[par[f]];
        first[par[f]] = f;
        gsize[par[f]]++;
    }
    HMap eh;
    edge_map(&eh, m);
    PL out;
    memset(&out, 0, sizeof out);
    unsigned char *done = xcalloc(m->nf + 1, 1);
    int *hu = xrealloc(NULL, (m->fo[m->nf] + 1) * sizeof(int)), *hw = xrealloc(NULL, (m->fo[m->nf] + 1) * sizeof(int));
    int *nxt = xrealloc(NULL, (m->nv + 1) * sizeof(int)), *outdeg = xcalloc(m->nv + 1, sizeof(int));
    for (int v = 0; v < m->nv; v++) nxt[v] = -1;
    for (int f = 0; f < m->nf; f++) {
        if (done[f]) continue;
        int r = par[f], size = gsize[r], nh = 0, sel = 0;
        if (size == 1) { done[f] = 1; pl_add(&out, m->fi + m->fo[f], m->fo[f + 1] - m->fo[f], m->fs[f]); continue; }
        /* the group's outline: half edges whose other face is outside the group */
        for (int g = first[r]; g >= 0; g = nextg[g]) {
            done[g] = 1;
            sel |= m->fs[g];
            int s = m->fo[g], n = m->fo[g + 1] - s;
            for (int k = 0; k < n; k++) {
                int u = m->fi[s + k], w = m->fi[s + (k + 1) % n], e = edge_at(&eh, u, w);
                int o = e < 0 ? -1 : m->ef[2 * e] == g ? m->ef[2 * e + 1] : m->ef[2 * e];
                if (o >= 0 && par[o] == r) continue;
                hu[nh] = u; hw[nh++] = w;
            }
        }
        int ok = nh >= 3;
        for (int i = 0; i < nh && ok; i++) { if (outdeg[hu[i]]++) ok = 0; nxt[hu[i]] = hw[i]; }
        IV loop = { 0, 0, NULL };
        if (ok) {
            int v = hu[0];
            do { iv_push(&loop, v); v = nxt[v]; } while (v >= 0 && v != hu[0] && loop.n <= nh);
            ok = v == hu[0] && loop.n == nh;
        }
        for (int i = 0; i < nh; i++) { outdeg[hu[i]] = 0; nxt[hu[i]] = -1; }
        if (ok) {
            pl_add(&out, loop.v, loop.n, sel);
            joined += size - 1;
        } else {
            /* pinched or holed: leave these faces as they were */
            for (int g = first[r]; g >= 0; g = nextg[g])
                pl_add(&out, m->fi + m->fo[g], m->fo[g + 1] - m->fo[g], m->fs[g]);
        }
        free(loop.v);
    }
    hm_free(&eh);
    /* leftover vertices between two edges */
    int removed = 0;
    if (rm) {
        HMap ec;
        int nc = 0;
        for (int f = 0; f < out.nf; f++) nc += out.f[f].n;
        hm_init(&ec, nc + 8);
        int *deg = xcalloc(m->nv + 1, sizeof(int)), *border = xcalloc(m->nv + 1, sizeof(int));
        for (int f = 0; f < out.nf; f++)
            for (int k = 0; k < out.f[f].n; k++) {
                int u = out.f[f].v[k], w = out.f[f].v[(k + 1) % out.f[f].n], *c = hm_get(&ec, u, w);
                if (*c < 0) { *c = 1; deg[u]++; deg[w]++; } else (*c)++;
            }
        for (int f = 0; f < out.nf; f++)
            for (int k = 0; k < out.f[f].n; k++) {
                int u = out.f[f].v[k], w = out.f[f].v[(k + 1) % out.f[f].n];
                if (*hm_get(&ec, u, w) == 1) border[u] = border[w] = 1;
            }
        hm_free(&ec);
        for (int f = 0; f < out.nf; f++) {
            IV *v = &out.f[f];
            for (int k = 0; k < v->n && v->n > 3; k++) {
                int x = v->v[k];
                if (!rm[x] || deg[x] != 2 || (border[x] && !rmBorder)) continue;
                memmove(v->v + k, v->v + k + 1, (v->n - k - 1) * sizeof(int));
                v->n--;
                k--;
                removed++;
            }
        }
        free(deg); free(border);
    }
    if (joined || removed) {
        MB b = mb_from_verts(m);
        pl_to_mb(&out, &b);
        mesh_replace(m, &b);
    }
    pl_free(&out);
    free(par); free(done); free(hu); free(hw); free(nxt); free(outdeg); free(first); free(nextg); free(gsize);
    return joined + removed;
}

int mesh_delete_edges(Mesh *m, const unsigned char *emask, int clean)
{
    unsigned char *rm = NULL;
    if (clean) {
        rm = xcalloc(m->nv + 1, 1);
        for (int e = 0; e < m->ne; e++) if (emask[e]) rm[m->ev[2 * e]] = rm[m->ev[2 * e + 1]] = 1;
    }
    int n = dissolve(m, emask, rm, 0);
    free(rm);
    return n;
}

int mesh_delete_verts(Mesh *m, const unsigned char *vmask)
{
    unsigned char *em = xcalloc(m->ne + 1, 1);
    for (int e = 0; e < m->ne; e++)
        em[e] = (vmask[m->ev[2 * e]] || vmask[m->ev[2 * e + 1]]) && m->ef[2 * e + 1] >= 0;
    int n = dissolve(m, em, vmask, 1);
    free(em);
    return n;
}

int mesh_edge_ring(const Mesh *m, int e, int *ra, int *rb, int *closed)
{
    int n = 1;
    unsigned char *seen = xcalloc(m->nf + 1, 1);
    HMap h;
    edge_map(&h, m);
    ra[0] = m->ev[2 * e];
    rb[0] = m->ev[2 * e + 1];
    *closed = 0;
    for (int side = 0; side < 2 && !*closed; side++) {
        int a = ra[0], b = rb[0], f = m->ef[2 * e + side], ce = e;
        if (side == 1) { a = ra[0]; b = rb[0]; }
        while (f >= 0 && !seen[f] && m->fo[f + 1] - m->fo[f] == 4) {
            seen[f] = 1;
            const int *q = m->fi + m->fo[f];
            int i = 0;
            while (i < 4 && q[i] != a) i++;
            if (i == 4) break;
            int d, c;
            if (q[(i + 1) % 4] == b) { d = q[(i + 3) % 4]; c = q[(i + 2) % 4]; }
            else { d = q[(i + 1) % 4]; c = q[(i + 2) % 4]; }
            int ne_ = edge_at(&h, d, c);
            if (ne_ < 0) break;
            if (ne_ == e) { *closed = 1; break; }
            if (side == 0) { ra[n] = d; rb[n] = c; }
            else {
                memmove(ra + 1, ra, n * sizeof(int));
                memmove(rb + 1, rb, n * sizeof(int));
                ra[0] = d; rb[0] = c;
            }
            n++;
            a = d; b = c;
            f = m->ef[2 * ne_] == f ? m->ef[2 * ne_ + 1] : m->ef[2 * ne_];
            ce = ne_;
        }
        (void)ce;
    }
    hm_free(&h);
    free(seen);
    return n;
}

int mesh_cut(Mesh *m, const CutPt *pts, int n)
{
    if (n < 2) return 0;
    PL pl;
    pl_from(&pl, m);
    MB b = mb_from_verts(m);
    HMap eh;
    edge_map(&eh, m);
    /* points already put on each edge: (edge, t from its lower vertex, vertex) */
    int *se = xrealloc(NULL, n * sizeof(int)), *sv = xrealloc(NULL, n * sizeof(int)), ns = 0;
    float *st = xrealloc(NULL, n * sizeof(float));
    int *id = xrealloc(NULL, n * sizeof(int)), cuts = 0;
    for (int k = 0; k < n; k++) {
        int a = pts[k].a, c = pts[k].b;
        float t = pts[k].t;
        id[k] = -1;
        if (a < 0 || a >= m->nv) continue;
        if (c < 0) { id[k] = a; continue; }
        if (c >= m->nv) continue;
        if (a > c) { int x = a; a = c; c = x; t = 1 - t; }
        int e = edge_at(&eh, a, c);
        if (e < 0) continue;
        if (t <= 1e-4f) { id[k] = a; continue; }
        if (t >= 1 - 1e-4f) { id[k] = c; continue; }
        int lo = a, hi = c;
        float tlo = 0, thi = 1;
        for (int i = 0; i < ns && id[k] < 0; i++) {
            if (se[i] != e) continue;
            if (fabsf(st[i] - t) < 1e-4f) id[k] = sv[i];
            else if (st[i] < t && st[i] > tlo) { tlo = st[i]; lo = sv[i]; }
            else if (st[i] > t && st[i] < thi) { thi = st[i]; hi = sv[i]; }
        }
        if (id[k] >= 0) continue;
        int x = mb_vert(&b, v3lerp(m->p[a], m->p[c], t), v3lerp(m->c[a], m->c[c], t));
        pl_split_edge(&pl, lo, hi, x);
        if (m->eh[e]) { mb_hard(&b, lo, x, m->eh[e]); mb_hard(&b, x, hi, m->eh[e]); }
        se[ns] = e; st[ns] = t; sv[ns++] = x;
        id[k] = x;
    }
    int *cu = xrealloc(NULL, n * sizeof(int)), *cw = xrealloc(NULL, n * sizeof(int));
    for (int k = 0; k + 1 < n; k++) {
        int u = id[k], w = id[k + 1];
        if (u < 0 || w < 0 || u == w) continue;
        if (pl_cut(&pl, u, w)) { cu[cuts] = u; cw[cuts++] = w; }
    }
    if (cuts || ns) {
        pl_to_mb(&pl, &b);
        mesh_replace(m, &b);   /* every vertex is still used, so numbers stay put */
        HMap h;
        edge_map(&h, m);
        for (int i = 0; i < cuts; i++) {
            int e = edge_at(&h, cu[i], cw[i]);
            if (e >= 0) m->es[e] = 1;
        }
        hm_free(&h);
    } else {
        free(b.m.p); free(b.m.c); free(b.m.vs); free(b.m.fo); free(b.m.fs); free(b.m.fi); free(b.hp);
    }
    hm_free(&eh);
    pl_free(&pl);
    free(se); free(sv); free(st); free(id); free(cu); free(cw);
    return cuts;
}

int mesh_insert_loop(Mesh *m, int e, float t)
{
    if (e < 0 || e >= m->ne) return 0;
    int *ra = xrealloc(NULL, (m->ne + 1) * sizeof(int)), *rb = xrealloc(NULL, (m->ne + 1) * sizeof(int)), closed;
    int n = mesh_edge_ring(m, e, ra, rb, &closed);
    CutPt *p = xrealloc(NULL, (n + 1) * sizeof(CutPt));
    for (int i = 0; i < n; i++) { p[i].a = ra[i]; p[i].b = rb[i]; p[i].t = t; }
    if (closed) p[n++] = p[0];
    int made = mesh_cut(m, p, n);
    free(ra); free(rb); free(p);
    return made;
}

/* One segment bevel (chamfer). Each selected edge becomes a strip of width about 'width': at its
   ends the corners slide along the unselected edges (or inset between two selected ones), and a
   cap face closes the gap where three or more strips meet. */
int mesh_bevel(Mesh *m, const unsigned char *emask, float width)
{
    unsigned char *sel = xcalloc(m->ne + 1, 1);
    int *k = xcalloc(m->nv + 1, sizeof(int)), any = 0;
    for (int e = 0; e < m->ne; e++)
        if (emask[e] && m->ef[2 * e] >= 0 && m->ef[2 * e + 1] >= 0 && m->ef[2 * e] != m->ef[2 * e + 1]) {
            sel[e] = 1;
            k[m->ev[2 * e]]++;
            k[m->ev[2 * e + 1]]++;
            any = 1;
        }
    if (!any || width <= 0) { free(sel); free(k); return 0; }
    HMap eh, sl;
    edge_map(&eh, m);
    hm_init(&sl, 2 * m->ne + 8);
    MB b = mb_from_verts(m);
    IV owner = { 0, 0, NULL };   /* original vertex of each new vertex (index - m->nv) */
    int nc = m->fo[m->nf], *cv = xrealloc(NULL, (nc + 1) * sizeof(int));
    PL pl;
    memset(&pl, 0, sizeof pl);
#define NEWV(pos, col, own) (iv_push(&owner, own), mb_vert(&b, pos, col))
    for (int f = 0; f < m->nf; f++) {
        int s = m->fo[f], n = m->fo[f + 1] - s;
        IV nf = { 0, 0, NULL };
        for (int j = 0; j < n; j++) {
            int v = m->fi[s + j], pv = m->fi[s + (j + n - 1) % n], nx = m->fi[s + (j + 1) % n];
            cv[s + j] = -1;
            if (!k[v]) { iv_push(&nf, v); continue; }
            int ein = edge_at(&eh, pv, v), eout = edge_at(&eh, v, nx), ends[2] = { ein, eout }, slid[2];
            for (int z = 0; z < 2; z++) {
                int ee = ends[z];
                slid[z] = -1;
                if (ee < 0 || sel[ee]) continue;
                int *slot = hm_getd(&sl, v, ee);
                if (*slot < 0) {
                    int o = m->ev[2 * ee] == v ? m->ev[2 * ee + 1] : m->ev[2 * ee];
                    float L = v3len(v3sub(m->p[o], m->p[v])), d = fminf(width, 0.45f * L), u = L > 0 ? d / L : 0;
                    *slot = NEWV(v3lerp(m->p[v], m->p[o], u), v3lerp(m->c[v], m->c[o], u), v);
                    if (m->eh[ee]) mb_hard(&b, *slot, o, m->eh[ee]);
                }
                slid[z] = *slot;
            }
            int si = ein >= 0 && sel[ein], so = eout >= 0 && sel[eout];
            if (si && so) {
                V3 a = v3sub(m->p[pv], m->p[v]), c = v3sub(m->p[nx], m->p[v]);
                float la = v3len(a), lc = v3len(c);
                V3 p = m->p[v];
                if (la > 0) p = v3add(p, v3mul(a, fminf(width, 0.45f * la) / la));
                if (lc > 0) p = v3add(p, v3mul(c, fminf(width, 0.45f * lc) / lc));
                cv[s + j] = NEWV(p, m->c[v], v);
                iv_push(&nf, cv[s + j]);
            } else if (si) {
                cv[s + j] = slid[1];
                iv_push(&nf, slid[1]);
            } else if (so) {
                cv[s + j] = slid[0];
                iv_push(&nf, slid[0]);
            } else {
                iv_push(&nf, slid[0]);
                iv_push(&nf, slid[1]);
            }
        }
        pl_add(&pl, nf.v, nf.n, 0);
        free(nf.v);
    }
#undef NEWV
    /* strips: f0 runs u->w along the edge, f1 runs w->u */
    for (int e = 0; e < m->ne; e++) {
        if (!sel[e]) continue;
        int f0 = m->ef[2 * e], f1 = m->ef[2 * e + 1], c0u = -1, c0w = -1, c1u = -1, c1w = -1, u = -1, w = -1;
        int s = m->fo[f0], n = m->fo[f0 + 1] - s;
        for (int j = 0; j < n; j++) {
            int a = m->fi[s + j], c = m->fi[s + (j + 1) % n];
            if ((a == m->ev[2 * e] && c == m->ev[2 * e + 1]) || (c == m->ev[2 * e] && a == m->ev[2 * e + 1])) {
                u = a; w = c; c0u = cv[s + j]; c0w = cv[s + (j + 1) % n];
            }
        }
        s = m->fo[f1]; n = m->fo[f1 + 1] - s;
        for (int j = 0; j < n; j++) {
            if (m->fi[s + j] == u) c1u = cv[s + j];
            if (m->fi[s + j] == w) c1w = cv[s + j];
        }
        int q[4] = { c0w, c0u, c1u, c1w }, qn = 0, out[4];
        for (int i = 0; i < 4; i++) if (q[i] >= 0 && (!qn || out[qn - 1] != q[i])) out[qn++] = q[i];
        if (qn > 1 && out[qn - 1] == out[0]) qn--;
        if (qn >= 3) pl_add(&pl, out, qn, 1);
        if (m->eh[e]) for (int i = 0; i < qn; i++) mb_hard(&b, out[i], out[(i + 1) % qn], m->eh[e]);
    }
    /* caps: the open outline among each old vertex's new vertices */
    {
        HMap dh;
        int tot = 0;
        for (int f = 0; f < pl.nf; f++) tot += pl.f[f].n;
        hm_init(&dh, tot + 8);
        for (int f = 0; f < pl.nf; f++)
            for (int j = 0; j < pl.f[f].n; j++) *hm_getd(&dh, pl.f[f].v[j], pl.f[f].v[(j + 1) % pl.f[f].n]) = 1;
        int *nx = xrealloc(NULL, (b.m.nv + 1) * sizeof(int));
        for (int v = 0; v < b.m.nv; v++) nx[v] = -1;
        int pf = pl.nf;
        for (int f = 0; f < pf; f++)
            for (int j = 0; j < pl.f[f].n; j++) {
                int a = pl.f[f].v[j], c = pl.f[f].v[(j + 1) % pl.f[f].n];
                if (a < m->nv || c < m->nv || owner.v[a - m->nv] != owner.v[c - m->nv]) continue;
                if (*hm_getd(&dh, c, a) == 1) continue;
                nx[c] = a;   /* the cap runs against the open edge */
            }
        for (int v = m->nv; v < b.m.nv; v++) {
            if (nx[v] < 0) continue;
            IV loop = { 0, 0, NULL };
            int x = v, ok = 0;
            while (x >= 0 && loop.n <= 64) {
                iv_push(&loop, x);
                int y = nx[x];
                nx[x] = -1;
                if (y == v) { ok = 1; break; }
                x = y;
            }
            if (ok && loop.n >= 3) pl_add(&pl, loop.v, loop.n, 1);
            free(loop.v);
        }
        free(nx);
        hm_free(&dh);
    }
    pl_to_mb(&pl, &b);
    pl_free(&pl);
    /* overrides on edges that kept both old ends carry over; mesh_replace adds them */
    int made = 0;
    for (int e = 0; e < m->ne; e++) made += sel[e];
    mesh_replace(m, &b);
    hm_free(&eh); hm_free(&sl);
    free(sel); free(k); free(cv); free(owner.v);
    return made;
}

/* selected border edges as runs of vertices; returns the number of runs (at most 2 kept) */
static int border_runs(const Mesh *m, const unsigned char *emask, IV run[2], int closed[2])
{
    int *deg = xcalloc(m->nv + 1, sizeof(int)), nr = 0;
    unsigned char *use = xcalloc(m->ne + 1, 1);
    for (int e = 0; e < m->ne; e++)
        if (emask[e] && m->ef[2 * e] >= 0 && m->ef[2 * e + 1] < 0) { use[e] = 1; deg[m->ev[2 * e]]++; deg[m->ev[2 * e + 1]]++; }
    for (int v = 0; v < m->nv; v++) if (deg[v] > 2) { free(deg); free(use); return -1; }
    for (int pass = 0; pass < 2; pass++)   /* open runs start at an end; then closed loops */
        for (int s = 0; s < m->ne; s++) {
            if (!use[s]) continue;
            int a = m->ev[2 * s], c = m->ev[2 * s + 1];
            if (pass == 0 && deg[a] != 1 && deg[c] != 1) continue;
            if (pass == 0 && deg[a] != 1) { int t = a; a = c; c = t; }
            if (nr == 2) { nr = 3; goto out; }   /* more than two runs */
            IV *r = &run[nr];
            memset(r, 0, sizeof *r);
            closed[nr] = pass == 1;
            iv_push(r, a);
            int cur = s, v = c;
            use[s] = 0;
            for (;;) {
                if (v == r->v[0]) break;
                iv_push(r, v);
                int nx = -1;
                for (int e = 0; e < m->ne && nx < 0; e++)
                    if (use[e] && (m->ev[2 * e] == v || m->ev[2 * e + 1] == v)) nx = e;
                if (nx < 0) break;
                use[nx] = 0;
                v = m->ev[2 * nx] == v ? m->ev[2 * nx + 1] : m->ev[2 * nx];
                cur = nx;
            }
            (void)cur;
            nr++;
        }
out:
    free(deg); free(use);
    return nr;
}

int mesh_bridge(Mesh *m, const unsigned char *emask, int divs)
{
    IV r[3] = { { 0, 0, NULL }, { 0, 0, NULL }, { 0, 0, NULL } };
    int cl[3] = { 0, 0, 0 }, nr = border_runs(m, emask, r, cl), res = 0;
    if (nr != 2 || cl[0] != cl[1]) { res = -1; goto done; }
    int n = r[0].n;
    if (r[1].n != n || n < 2) { res = -2; goto done; }
    int closed = cl[0], *A = r[0].v, *B = xrealloc(NULL, n * sizeof(int));
    /* run A along its face's winding, so new faces hold A's edges backwards */
    {
        int e = mesh_edge_find(m, A[0], A[1]), f = m->ef[2 * e], s = m->fo[f], fn = m->fo[f + 1] - s, fwd = 0;
        for (int j = 0; j < fn; j++) if (m->fi[s + j] == A[0] && m->fi[s + (j + 1) % fn] == A[1]) fwd = 1;
        if (!fwd) {
            for (int i = 0; i < n / 2; i++) { int t = A[i]; A[i] = A[n - 1 - i]; A[n - 1 - i] = t; }
            if (closed) {   /* keep A[0] first */
                int t = A[n - 1];
                memmove(A + 1, A, (n - 1) * sizeof(int));
                A[0] = t;
            }
        }
    }
    /* line B up with A: the start and direction that keep matching vertices closest */
    float best = 1e30f;
    for (int s = 0; s < (closed ? n : 1); s++)
        for (int dir = 0; dir < 2; dir++) {
            float d = 0;
            for (int i = 0; i < n; i++) {
                int j = closed ? (dir ? (s - i + n) % n : (s + i) % n) : (dir ? n - 1 - i : i);
                d += v3len(v3sub(m->p[A[i]], m->p[r[1].v[j]]));
            }
            if (d < best) {
                best = d;
                for (int i = 0; i < n; i++) B[i] = r[1].v[closed ? (dir ? (s - i + n) % n : (s + i) % n) : (dir ? n - 1 - i : i)];
            }
        }
    MB b = mb_from_verts(m);
    for (int f = 0; f < m->nf; f++) mb_face(&b, m->fi + m->fo[f], m->fo[f + 1] - m->fo[f], 0);
    int rows = divs + 2, *R = xrealloc(NULL, rows * n * sizeof(int));
    for (int i = 0; i < n; i++) { R[i] = A[i]; R[(rows - 1) * n + i] = B[i]; }
    for (int rr = 1; rr < rows - 1; rr++) {
        float t = (float)rr / (rows - 1);
        for (int i = 0; i < n; i++)
            R[rr * n + i] = mb_vert(&b, v3lerp(m->p[A[i]], m->p[B[i]], t), v3lerp(m->c[A[i]], m->c[B[i]], t));
    }
    int segs = closed ? n : n - 1;
    for (int rr = 0; rr < rows - 1; rr++)
        for (int i = 0; i < segs; i++) {
            int i2 = (i + 1) % n, q[4] = { R[rr * n + i2], R[rr * n + i], R[(rr + 1) * n + i], R[(rr + 1) * n + i2] };
            mb_face(&b, q, 4, 1);
            res++;
        }
    mesh_replace(m, &b);
    free(R);
    free(B);
done:
    for (int i = 0; i < 3; i++) free(r[i].v);
    return res;
}

/* ---------------------------------------------------------------- primitives */

void prim_defaults(Prim *p, int type)
{
    memset(p, 0, sizeof *p);
    p->type = type;
    p->w = p->h = p->d = 1; p->sw = p->sh = p->sd = 1;
    p->cr = 1; p->ch = 2; p->csides = 20; p->chseg = 1; p->ccap = 0; p->ctop = p->cbot = 1;
    p->sr = 1; p->scols = 20; p->srows = 20;
    p->pw = p->pd = 1; p->psw = p->psd = 10;
}

static float clampf(float v, float lo, float hi) { return v < lo ? lo : v > hi ? hi : v; }
static int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

void prim_clamp(Prim *p)
{
    p->w = clampf(p->w, 1e-4f, 1e6f); p->h = clampf(p->h, 1e-4f, 1e6f); p->d = clampf(p->d, 1e-4f, 1e6f);
    p->sw = clampi(p->sw, 1, 200); p->sh = clampi(p->sh, 1, 200); p->sd = clampi(p->sd, 1, 200);
    p->cr = clampf(p->cr, 1e-4f, 1e6f); p->ch = clampf(p->ch, 1e-4f, 1e6f);
    p->csides = clampi(p->csides, 3, 512); p->chseg = clampi(p->chseg, 1, 200); p->ccap = clampi(p->ccap, 0, 100);
    p->ctop = p->ctop != 0; p->cbot = p->cbot != 0;
    p->sr = clampf(p->sr, 1e-4f, 1e6f);
    p->scols = clampi(p->scols, 3, 512); p->srows = clampi(p->srows, 2, 512);
    p->pw = clampf(p->pw, 1e-4f, 1e6f); p->pd = clampf(p->pd, 1e-4f, 1e6f);
    p->psw = clampi(p->psw, 1, 500); p->psd = clampi(p->psd, 1, 500);
}

static const V3 WHITE = { 1, 1, 1 };

static Mesh prim_cube(const Prim *p)
{
    MB b;
    mb_init(&b);
    int n[3] = { p->sw, p->sh, p->sd };
    float sz[3] = { p->w, p->h, p->d };
    int nl = (n[0] + 1) * (n[1] + 1) * (n[2] + 1);
    int *lat = xrealloc(NULL, nl * sizeof(int));
    for (int i = 0; i < nl; i++) lat[i] = -1;
    for (int ax = 0; ax < 3; ax++) {
        int u = (ax + 1) % 3, v = (ax + 2) % 3;
        for (int side = 0; side < 2; side++) {
            V3 want = v3(0, 0, 0);
            (&want.x)[ax] = side ? 1.0f : -1.0f;
            for (int iu = 0; iu < n[u]; iu++)
                for (int iv = 0; iv < n[v]; iv++) {
                    static const int du[4] = { 0, 1, 1, 0 }, dv[4] = { 0, 0, 1, 1 };
                    int q[4];
                    for (int k = 0; k < 4; k++) {
                        int c[3];
                        c[ax] = side ? n[ax] : 0;
                        c[u] = iu + du[k];
                        c[v] = iv + dv[k];
                        int li = (c[2] * (n[1] + 1) + c[1]) * (n[0] + 1) + c[0];
                        if (lat[li] < 0) {
                            V3 pos;
                            for (int a = 0; a < 3; a++) (&pos.x)[a] = sz[a] * ((float)c[a] / n[a] - 0.5f);
                            lat[li] = mb_vert(&b, pos, WHITE);
                        }
                        q[k] = lat[li];
                    }
                    mb_face_dir(&b, q, 4, want);
                }
        }
    }
    free(lat);
    return mb_finish(&b);
}

/* cap for a cylinder ring: n-gon when rings == 0, else concentric quads + fan */
static void cyl_cap(MB *b, const int *ring, int sides, int rings, float r, float y, V3 want)
{
    int *tmp = xrealloc(NULL, (sides + 4) * sizeof(int));
    if (rings == 0) {
        memcpy(tmp, ring, sides * sizeof(int));
        mb_face_dir(b, tmp, sides, want);
        free(tmp);
        return;
    }
    int *outer = xrealloc(NULL, sides * sizeof(int));
    int *inner = xrealloc(NULL, sides * sizeof(int));
    memcpy(outer, ring, sides * sizeof(int));
    for (int k = 1; k < rings; k++) {
        float rr = r * (1.0f - (float)k / rings);
        for (int i = 0; i < sides; i++) {
            float a = 2 * PI_F * i / sides;
            inner[i] = mb_vert(b, v3(rr * cosf(a), y, -rr * sinf(a)), WHITE);
        }
        for (int i = 0; i < sides; i++) {
            int j = (i + 1) % sides;
            int q[4] = { outer[i], outer[j], inner[j], inner[i] };
            mb_face_dir(b, q, 4, want);
        }
        memcpy(outer, inner, sides * sizeof(int));
    }
    int c = mb_vert(b, v3(0, y, 0), WHITE);
    for (int i = 0; i < sides; i++) {
        int t[3] = { outer[i], outer[(i + 1) % sides], c };
        mb_face_dir(b, t, 3, want);
    }
    free(outer);
    free(inner);
    free(tmp);
}

static Mesh prim_cylinder(const Prim *p)
{
    MB b;
    mb_init(&b);
    int n = p->csides, hs = p->chseg;
    int *ring = xrealloc(NULL, (hs + 1) * n * sizeof(int));
    for (int j = 0; j <= hs; j++) {
        float y = p->ch * ((float)j / hs - 0.5f);
        for (int i = 0; i < n; i++) {
            float a = 2 * PI_F * i / n;
            ring[j * n + i] = mb_vert(&b, v3(p->cr * cosf(a), y, -p->cr * sinf(a)), WHITE);
        }
    }
    for (int j = 0; j < hs; j++)
        for (int i = 0; i < n; i++) {
            int i2 = (i + 1) % n;
            float a = 2 * PI_F * (i + 0.5f) / n;
            int q[4] = { ring[j * n + i], ring[j * n + i2], ring[(j + 1) * n + i2], ring[(j + 1) * n + i] };
            mb_face_dir(&b, q, 4, v3(cosf(a), 0, -sinf(a)));
        }
    if (p->cbot) cyl_cap(&b, ring, n, p->ccap, p->cr, -p->ch * 0.5f, v3(0, -1, 0));
    if (p->ctop) cyl_cap(&b, ring + hs * n, n, p->ccap, p->cr, p->ch * 0.5f, v3(0, 1, 0));
    free(ring);
    return mb_finish(&b);
}

static Mesh prim_sphere(const Prim *p)
{
    MB b;
    mb_init(&b);
    int R = p->srows, C = p->scols;
    float r = p->sr;
    int top = mb_vert(&b, v3(0, r, 0), WHITE);
    int *ring = xrealloc(NULL, (R + 1) * C * sizeof(int));
    for (int i = 1; i < R; i++) {
        float phi = PI_F * i / R, y = r * cosf(phi), rr = r * sinf(phi);
        for (int j = 0; j < C; j++) {
            float th = 2 * PI_F * j / C;
            ring[i * C + j] = mb_vert(&b, v3(rr * cosf(th), y, -rr * sinf(th)), WHITE);
        }
    }
    int bot = mb_vert(&b, v3(0, -r, 0), WHITE);
    for (int j = 0; j < C; j++) {
        int j2 = (j + 1) % C;
        int t[3] = { top, ring[C + j], ring[C + j2] };
        mb_face_dir(&b, t, 3, v3(0, 1, 0));
        for (int i = 1; i + 1 < R; i++) {
            int q[4] = { ring[i * C + j], ring[(i + 1) * C + j], ring[(i + 1) * C + j2], ring[i * C + j2] };
            V3 c = v3add(v3add(b.m.p[q[0]], b.m.p[q[1]]), v3add(b.m.p[q[2]], b.m.p[q[3]]));
            mb_face_dir(&b, q, 4, c);
        }
        int t2[3] = { ring[(R - 1) * C + j], bot, ring[(R - 1) * C + j2] };
        mb_face_dir(&b, t2, 3, v3(0, -1, 0));
    }
    free(ring);
    return mb_finish(&b);
}

static Mesh prim_plane(const Prim *p)
{
    MB b;
    mb_init(&b);
    int W = p->psw, D = p->psd;
    int *g = xrealloc(NULL, (W + 1) * (D + 1) * sizeof(int));
    for (int j = 0; j <= D; j++)
        for (int i = 0; i <= W; i++)
            g[j * (W + 1) + i] = mb_vert(&b, v3(p->pw * ((float)i / W - 0.5f), 0, p->pd * ((float)j / D - 0.5f)), WHITE);
    for (int j = 0; j < D; j++)
        for (int i = 0; i < W; i++) {
            int q[4] = { g[j * (W + 1) + i], g[j * (W + 1) + i + 1], g[(j + 1) * (W + 1) + i + 1], g[(j + 1) * (W + 1) + i] };
            mb_face_dir(&b, q, 4, v3(0, 1, 0));
        }
    free(g);
    return mb_finish(&b);
}

Mesh mesh_prim(const Prim *pp)
{
    Prim p = *pp;
    prim_clamp(&p);
    switch (p.type) {
    case PRIM_CYL: return prim_cylinder(&p);
    case PRIM_SPHERE: return prim_sphere(&p);
    case PRIM_PLANE: return prim_plane(&p);
    default: return prim_cube(&p);
    }
}

/* ---------------------------------------------------------------- OBJ io */

/* path with its extension swapped for ext (".mtl"); *base points at the file name part */
static void swap_ext(const char *path, const char *ext, char *out, size_t n, const char **base)
{
    snprintf(out, n, "%s", path);
    char *slash = strrchr(out, '/'), *bs = strrchr(out, '\\'), *dot = strrchr(out, '.');
    if (bs && (!slash || bs > slash)) slash = bs;
    if (dot && (!slash || dot > slash)) *dot = 0;
    size_t l = strlen(out);
    snprintf(out + l, n - l, "%s", ext);
    *base = slash ? out + (slash - out) + 1 : out;
}

int obj_save(const char *path, int n, const Mesh *const *ms, const M4 *mats, const char *const *names, const float *angles,
             const char *const *texfiles)
{
    FILE *f = fopen(path, "w");
    if (!f) return 0;
    int colors = 0, anytex = 0;
    for (int i = 0; i < n && !colors; i++)
        for (int v = 0; v < ms[i]->nv; v++) {
            V3 c = ms[i]->c[v];
            if (c.x != 1 || c.y != 1 || c.z != 1) { colors = 1; break; }
        }
    for (int i = 0; i < n; i++) anytex |= texfiles && texfiles[i] && texfiles[i][0] && ms[i]->uv && ms[i]->tex;
    fprintf(f, "# tuff3d\n");
    if (anytex) {
        /* one material per textured object, and a plain one so untextured objects don't pick up the
           previous object's texture */
        char mp[1024];
        const char *mb;
        swap_ext(path, ".mtl", mp, sizeof mp, &mb);
        FILE *mf = fopen(mp, "w");
        if (!mf) { fclose(f); return 0; }
        fprintf(mf, "# tuff3d\nnewmtl tuff3d_plain\nKd 1 1 1\n");
        for (int i = 0; i < n; i++)
            if (texfiles[i] && texfiles[i][0] && ms[i]->uv && ms[i]->tex)
                fprintf(mf, "\nnewmtl %s\nKd 1 1 1\nmap_Kd %s\n", names[i], texfiles[i]);
        int ok = !ferror(mf);
        if (fclose(mf) || !ok) { fclose(f); return 0; }
        fprintf(f, "mtllib %s\n", mb);
    }
    int base = 1, nbase = 1, tbase = 1;
    for (int i = 0; i < n; i++) {
        const Mesh *m = ms[i];
        M4 M = mats[i];
        V3 c0 = m4_col(M, 0), c1 = m4_col(M, 1), c2 = m4_col(M, 2);
        int flip = v3dot(v3cross(c0, c1), c2) < 0, tex = anytex && texfiles[i] && texfiles[i][0] && m->uv && m->tex;
        fprintf(f, "o %s\n", names[i]);
        for (int v = 0; v < m->nv; v++) {
            V3 p = m4_pt(M, m->p[v]);
            if (colors)
                fprintf(f, "v %.6g %.6g %.6g %.4g %.4g %.4g\n", p.x, p.y, p.z, m->c[v].x, m->c[v].y, m->c[v].z);
            else
                fprintf(f, "v %.6g %.6g %.6g\n", p.x, p.y, p.z);
        }
        /* corner normals in world space (inverse transpose), shared where equal */
        int nc = m->fo[m->nf], *ni = NULL, *ti = NULL;
        if (angles) {
            V3 *cn = xrealloc(NULL, (nc + 1) * sizeof(V3));
            int *vfirst = xrealloc(NULL, (m->nv + 1) * sizeof(int));
            ni = xrealloc(NULL, (nc + 1) * sizeof(int));
            mesh_cnormals(m, angles[i], cn);
            M4 N = m4_inv(M);
            for (int v = 0; v < m->nv; v++) vfirst[v] = -1;
            for (int fc = 0; fc < m->nf; fc++)
                for (int c = m->fo[fc]; c < m->fo[fc + 1]; c++) {
                    int v = m->fi[c];
                    cn[c] = v3norm(v3(N.m[0] * cn[c].x + N.m[1] * cn[c].y + N.m[2] * cn[c].z,
                                      N.m[4] * cn[c].x + N.m[5] * cn[c].y + N.m[6] * cn[c].z,
                                      N.m[8] * cn[c].x + N.m[9] * cn[c].y + N.m[10] * cn[c].z));
                    if (vfirst[v] >= 0 && !memcmp(&cn[vfirst[v]], &cn[c], sizeof(V3))) ni[c] = ni[vfirst[v]];
                    else if (c > m->fo[fc] && !memcmp(&cn[c - 1], &cn[c], sizeof(V3))) ni[c] = ni[c - 1];
                    else {
                        ni[c] = nbase++;
                        fprintf(f, "vn %.5g %.5g %.5g\n", cn[c].x, cn[c].y, cn[c].z);
                    }
                    if (vfirst[v] < 0) vfirst[v] = c;
                }
            free(cn); free(vfirst);
            fprintf(f, "# tuff3d normal angle %g\ns %s\n", angles[i], angles[i] > 0 ? "1" : "off");
        }
        if (tex) {
            /* texture coordinates, 0..1 with v up, shared by a vertex's corners where they agree */
            int *vfirst = xrealloc(NULL, (m->nv + 1) * sizeof(int));
            ti = xrealloc(NULL, (nc + 1) * sizeof(int));
            for (int v = 0; v < m->nv; v++) vfirst[v] = -1;
            for (int c = 0; c < nc; c++) {
                int v = m->fi[c];
                if (vfirst[v] >= 0 && !memcmp(m->uv + 2 * vfirst[v], m->uv + 2 * c, 2 * sizeof(float))) ti[c] = ti[vfirst[v]];
                else {
                    ti[c] = tbase++;
                    fprintf(f, "vt %.7g %.7g\n", m->uv[2 * c] / m->tex->w, 1 - m->uv[2 * c + 1] / m->tex->h);
                }
                if (vfirst[v] < 0) vfirst[v] = c;
            }
            free(vfirst);
        }
        if (anytex) fprintf(f, "usemtl %s\n", tex ? names[i] : "tuff3d_plain");
        for (int fc = 0; fc < m->nf; fc++) {
            int o = m->fo[fc], cnt = m->fo[fc + 1] - o;
            fputc('f', f);
            for (int k = 0; k < cnt; k++) {
                int c = o + (flip ? cnt - 1 - k : k);
                if (ni && ti) fprintf(f, " %d/%d/%d", base + m->fi[c], ti[c], ni[c]);
                else if (ni) fprintf(f, " %d//%d", base + m->fi[c], ni[c]);
                else if (ti) fprintf(f, " %d/%d", base + m->fi[c], ti[c]);
                else fprintf(f, " %d", base + m->fi[c]);
            }
            fputc('\n', f);
        }
        free(ni);
        free(ti);
        base += m->nv;
    }
    int ok = !ferror(f);
    if (fclose(f)) ok = 0;
    return ok;
}

static char *read_line(FILE *f, char **buf, size_t *cap)
{
    size_t len = 0;
    int ch;
    while ((ch = fgetc(f)) != EOF) {
        if (len + 2 > *cap) { *cap = *cap ? *cap * 2 : 256; *buf = xrealloc(*buf, *cap); }
        if (ch == '\n') break;
        (*buf)[len++] = (char)ch;
    }
    if (ch == EOF && len == 0) return NULL;
    if (!*buf) { *cap = 16; *buf = xrealloc(NULL, *cap); }
    (*buf)[len] = 0;
    return *buf;
}

typedef struct { char name[64], map[260]; } ObjMtl;

typedef struct {
    V3 *p, *c; int np, cp;
    float *vt; int nvt, cvt;
    int *fo, *fi, *ft; int nf, cf, ni, ci;   /* ft: texture coordinate of each corner, -1 for none */
    Mesh *ms; char (*names)[32]; float *angles; char (*texs)[260]; int nm, cm;
    char name[32];
    float angle; int exact;   /* shading of the current object: -1 unknown; exact = from our own comment */
    ObjMtl *mtl; int nmtl, cur, omat;   /* materials, the one in use, the current object's */
    int *map;
} ObjRead;

static void objr_flush(ObjRead *r)
{
    if (!r->nf) return;
    MB b;
    mb_init(&b);
    r->map = xrealloc(r->map, (r->np + 1) * sizeof(int));
    for (int i = 0; i < r->np; i++) r->map[i] = -1;
    int *tmp = xrealloc(NULL, (r->ni + 1) * sizeof(int)), *kept = xrealloc(NULL, (r->ni + 1) * sizeof(int)), nk = 0;
    int tex = r->omat >= 0 && r->mtl[r->omat].map[0];
    for (int f = 0; f < r->nf; f++) {
        int o = r->fo[f], n = r->fo[f + 1] - o, ok = 1;
        for (int k = 0; k < n; k++) {
            int g = r->fi[o + k];
            if (g < 0 || g >= r->np) { ok = 0; break; }
            if (r->map[g] < 0) r->map[g] = mb_vert(&b, r->p[g], r->c[g]);
            tmp[k] = r->map[g];
        }
        if (!ok) continue;
        mb_face(&b, tmp, n, 0);
        for (int k = 0; k < n; k++) {
            kept[nk++] = r->ft[o + k];
            if (r->ft[o + k] < 0 || r->ft[o + k] >= r->nvt) tex = 0;
        }
    }
    free(tmp);
    if (r->nm == r->cm) {
        r->cm = r->cm ? r->cm * 2 : 8;
        r->ms = xrealloc(r->ms, r->cm * sizeof(Mesh));
        r->names = xrealloc(r->names, r->cm * sizeof(*r->names));
        r->angles = xrealloc(r->angles, r->cm * sizeof(float));
        r->texs = xrealloc(r->texs, r->cm * sizeof(*r->texs));
    }
    r->angles[r->nm] = r->angle;
    r->ms[r->nm] = mb_finish(&b);
    memcpy(r->names[r->nm], r->name, 32);
    r->texs[r->nm][0] = 0;
    if (tex && nk) {
        /* 0..1 with v down for now; the caller scales them once it knows the picture's size */
        Mesh *m = &r->ms[r->nm];
        m->uv = xrealloc(NULL, (2 * (size_t)nk + 2) * sizeof(float));
        for (int c = 0; c < nk; c++) {
            m->uv[2 * c] = r->vt[2 * kept[c]];
            m->uv[2 * c + 1] = 1 - r->vt[2 * kept[c] + 1];
        }
        mesh_uv_changed(m);
        snprintf(r->texs[r->nm], 260, "%s", r->mtl[r->omat].map);
    }
    free(kept);
    r->nm++;
    r->nf = r->ni = 0;
    r->omat = -1;
}

/* the folder part of path (with its slash), or "" */
static void dir_of(const char *path, char *out, size_t n)
{
    const char *a = strrchr(path, '/'), *b = strrchr(path, '\\');
    if (b && (!a || b > a)) a = b;
    snprintf(out, n, "%.*s", a ? (int)(a - path + 1) : 0, path);
}

static int is_abs(const char *p) { return p[0] == '/' || p[0] == '\\' || (p[0] && p[1] == ':'); }

/* materials from an .mtl file: name and picture (map_Kd, relative to the .mtl's folder) */
static void mtl_read(ObjRead *r, const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return;
    char dir[260], *buf = NULL, *line;
    size_t cap = 0;
    dir_of(path, dir, sizeof dir);
    while ((line = read_line(f, &buf, &cap))) {
        char *s = line;
        while (*s == ' ' || *s == '\t') s++;
        s[strcspn(s, "\r")] = 0;
        if (!strncmp(s, "newmtl", 6) && (s[6] == ' ' || s[6] == '\t')) {
            r->mtl = xrealloc(r->mtl, (r->nmtl + 1) * sizeof(ObjMtl));
            ObjMtl *m = &r->mtl[r->nmtl++];
            memset(m, 0, sizeof *m);
            sscanf(s + 7, "%63s", m->name);
        } else if (!strncmp(s, "map_Kd", 6) && (s[6] == ' ' || s[6] == '\t') && r->nmtl) {
            char *q = s + 7, *e;
            while (*q == ' ' || *q == '\t') q++;
            for (e = q + strlen(q); e > q && (e[-1] == ' ' || e[-1] == '\t'); ) *--e = 0;
            if (*q == '-') {   /* options (-s 1 1 1 ...): the file is the last word */
                char *w = strrchr(q, ' '), *t = strrchr(q, '\t');
                if (t && (!w || t > w)) w = t;
                if (w) q = w + 1;
            }
            ObjMtl *m = &r->mtl[r->nmtl - 1];
            if (is_abs(q)) snprintf(m->map, sizeof m->map, "%s", q);
            else snprintf(m->map, sizeof m->map, "%s%s", dir, q);
        }
    }
    free(buf);
    fclose(f);
}

int obj_load(const char *path, Mesh **ms, char (**names)[32], float **angles, char (**texfiles)[260])
{
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    ObjRead r;
    memset(&r, 0, sizeof r);
    r.cf = 64;
    r.fo = xrealloc(NULL, r.cf * sizeof(int));
    r.fo[0] = 0;
    strcpy(r.name, "imported");
    r.angle = -1;
    r.cur = r.omat = -1;
    char *buf = NULL, dir[260];
    size_t cap = 0;
    char *line;
    dir_of(path, dir, sizeof dir);
    while ((line = read_line(f, &buf, &cap))) {
        char *s = line;
        while (*s == ' ' || *s == '\t') s++;
        if (s[0] == 'v' && (s[1] == ' ' || s[1] == '\t')) {
            float a[6] = { 0, 0, 0, 1, 1, 1 };
            int k = sscanf(s + 2, "%f %f %f %f %f %f", &a[0], &a[1], &a[2], &a[3], &a[4], &a[5]);
            if (k < 3) continue;
            if (k < 6) a[3] = a[4] = a[5] = 1;
            if (a[3] > 1 || a[4] > 1 || a[5] > 1) { a[3] /= 255; a[4] /= 255; a[5] /= 255; }
            if (r.np == r.cp) {
                r.cp = r.cp ? r.cp * 2 : 1024;
                r.p = xrealloc(r.p, r.cp * sizeof(V3));
                r.c = xrealloc(r.c, r.cp * sizeof(V3));
            }
            r.p[r.np] = v3(a[0], a[1], a[2]);
            r.c[r.np] = v3(a[3], a[4], a[5]);
            r.np++;
        } else if (s[0] == 'v' && s[1] == 't' && (s[2] == ' ' || s[2] == '\t')) {
            float u = 0, v = 0;
            if (sscanf(s + 3, "%f %f", &u, &v) < 1) continue;
            if (r.nvt == r.cvt) { r.cvt = r.cvt ? r.cvt * 2 : 1024; r.vt = xrealloc(r.vt, 2 * r.cvt * sizeof(float)); }
            r.vt[2 * r.nvt] = u;
            r.vt[2 * r.nvt + 1] = v;
            r.nvt++;
        } else if (s[0] == 'f' && (s[1] == ' ' || s[1] == '\t')) {
            char *q = s + 2;
            int n = 0;
            for (;;) {
                while (*q == ' ' || *q == '\t' || *q == '\r') q++;
                if (!*q) break;
                char *end;
                long idx = strtol(q, &end, 10), ti = 0;
                if (end == q) break;
                if (*end == '/' && end[1] != '/') {
                    char *e2;
                    ti = strtol(end + 1, &e2, 10);
                    if (e2 == end + 1) ti = 0;
                }
                while (*end && *end != ' ' && *end != '\t' && *end != '\r') end++;
                q = end;
                int g = idx < 0 ? r.np + (int)idx : (int)idx - 1;
                if (r.ni + 1 > r.ci) {
                    r.ci = r.ci ? r.ci * 2 : 1024;
                    r.fi = xrealloc(r.fi, r.ci * sizeof(int));
                    r.ft = xrealloc(r.ft, r.ci * sizeof(int));
                }
                r.ft[r.ni] = ti < 0 ? r.nvt + (int)ti : ti ? (int)ti - 1 : -1;
                r.fi[r.ni++] = g;
                n++;
            }
            if (n < 3) { r.ni -= n; continue; }
            if (r.nf + 2 > r.cf) { r.cf *= 2; r.fo = xrealloc(r.fo, r.cf * sizeof(int)); }
            if (!r.nf) r.omat = r.cur;
            r.nf++;
            r.fo[r.nf] = r.ni;
        } else if ((s[0] == 'o' || s[0] == 'g') && (s[1] == ' ' || s[1] == '\t')) {
            objr_flush(&r);
            char nm[32] = { 0 };
            if (sscanf(s + 2, "%31s", nm) == 1) strcpy(r.name, nm);
            if (s[0] == 'o') r.angle = -1, r.exact = 0;
        } else if (s[0] == 's' && (s[1] == ' ' || s[1] == '\t') && !r.exact) {
            /* smoothing groups from other apps: off/0 = hard, any group = smooth */
            char *q = s + 2;
            while (*q == ' ' || *q == '\t') q++;
            r.angle = !strncmp(q, "off", 3) || atoi(q) == 0 ? 0 : 180;
        } else if (!strncmp(s, "# tuff3d normal angle ", 22)) {
            r.angle = (float)atof(s + 22);
            r.exact = 1;
        } else if (!strncmp(s, "mtllib", 6) && (s[6] == ' ' || s[6] == '\t')) {
            char mp[600], *q = s + 7, *e;
            while (*q == ' ' || *q == '\t') q++;
            for (e = q + strlen(q); e > q && (e[-1] == ' ' || e[-1] == '\t' || e[-1] == '\r'); ) *--e = 0;
            if (is_abs(q)) snprintf(mp, sizeof mp, "%s", q);
            else snprintf(mp, sizeof mp, "%s%s", dir, q);
            mtl_read(&r, mp);
        } else if (!strncmp(s, "usemtl", 6) && (s[6] == ' ' || s[6] == '\t')) {
            char nm[64] = "";
            sscanf(s + 7, "%63s", nm);
            r.cur = -1;
            for (int i = 0; i < r.nmtl; i++) if (!strcmp(r.mtl[i].name, nm)) r.cur = i;
            if (!r.nf) r.omat = r.cur;
        }
    }
    objr_flush(&r);
    fclose(f);
    free(buf); free(r.p); free(r.c); free(r.vt); free(r.fo); free(r.fi); free(r.ft); free(r.map); free(r.mtl);
    *ms = r.ms;
    *names = r.names;
    if (angles) *angles = r.angles;
    else free(r.angles);
    if (texfiles) *texfiles = r.texs;
    else {
        for (int i = 0; i < r.nm; i++) { free(r.ms[i].uv); r.ms[i].uv = NULL; }
        free(r.texs);
    }
    return r.nm;
}
