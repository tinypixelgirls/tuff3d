/* tuff3d - keyframe animation, skinning and IK math (platform independent) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "anim.h"

static void *xalloc(size_t n)
{
    void *p = calloc(1, n ? n : 1);
    if (!p) abort();
    return p;
}

static void *xgrow(void *p, size_t n)
{
    void *q = realloc(p, n ? n : 1);
    if (!q) abort();
    return q;
}

/* ---------------------------------------------------------------- rotations */

M4 m4_rot3(M4 m)
{
    M4 r = m4_id();
    for (int c = 0; c < 3; c++) {
        V3 v = v3norm(m4_col(m, c));
        r.m[4 * c] = v.x; r.m[4 * c + 1] = v.y; r.m[4 * c + 2] = v.z;
    }
    return r;
}

Quat quat_from_m4(M4 in)
{
    M4 m = m4_rot3(in);
    float m00 = m.m[0], m11 = m.m[5], m22 = m.m[10], tr = m00 + m11 + m22;
    float m10 = m.m[1], m01 = m.m[4], m20 = m.m[2], m02 = m.m[8], m21 = m.m[6], m12 = m.m[9];
    Quat q;
    if (tr > 0) {
        float s = sqrtf(tr + 1) * 2;
        q.w = s / 4; q.x = (m21 - m12) / s; q.y = (m02 - m20) / s; q.z = (m10 - m01) / s;
    } else if (m00 > m11 && m00 > m22) {
        float s = sqrtf(1 + m00 - m11 - m22) * 2;
        q.w = (m21 - m12) / s; q.x = s / 4; q.y = (m01 + m10) / s; q.z = (m02 + m20) / s;
    } else if (m11 > m22) {
        float s = sqrtf(1 + m11 - m00 - m22) * 2;
        q.w = (m02 - m20) / s; q.x = (m01 + m10) / s; q.y = s / 4; q.z = (m12 + m21) / s;
    } else {
        float s = sqrtf(1 + m22 - m00 - m11) * 2;
        q.w = (m10 - m01) / s; q.x = (m02 + m20) / s; q.y = (m12 + m21) / s; q.z = s / 4;
    }
    float l = sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    q.x /= l; q.y /= l; q.z /= l; q.w /= l;
    return q;
}

M4 m4_from_quat(Quat q)
{
    M4 m = m4_id();
    float x = q.x, y = q.y, z = q.z, w = q.w;
    m.m[0] = 1 - 2 * (y * y + z * z); m.m[4] = 2 * (x * y - z * w);     m.m[8] = 2 * (x * z + y * w);
    m.m[1] = 2 * (x * y + z * w);     m.m[5] = 1 - 2 * (x * x + z * z); m.m[9] = 2 * (y * z - x * w);
    m.m[2] = 2 * (x * z - y * w);     m.m[6] = 2 * (y * z + x * w);     m.m[10] = 1 - 2 * (x * x + y * y);
    return m;
}

void m4_decompose(M4 m, V3 *t, V3 *r, V3 *s)
{
    V3 c0 = m4_col(m, 0), c1 = m4_col(m, 1), c2 = m4_col(m, 2);
    float sx = v3len(c0), sy = v3len(c1), sz = v3len(c2);
    if (v3dot(v3cross(c0, c1), c2) < 0) sx = -sx;   /* mirrored: carry it on X */
    *t = m4_col(m, 3);
    *s = v3(sx, sy, sz);
    M4 R = m4_id();
    V3 cs[3] = { v3mul(c0, sx ? 1 / sx : 0), v3mul(c1, sy ? 1 / sy : 0), v3mul(c2, sz ? 1 / sz : 0) };
    for (int c = 0; c < 3; c++) { R.m[4 * c] = cs[c].x; R.m[4 * c + 1] = cs[c].y; R.m[4 * c + 2] = cs[c].z; }
    *r = m4_to_euler(R);
}

M4 m4_rot_between(V3 from, V3 to)
{
    V3 a = v3norm(from), b = v3norm(to), ax = v3cross(a, b);
    float s = v3len(ax), c = v3dot(a, b);
    if (s < 1e-7f) {
        if (c > 0) return m4_id();
        V3 p = fabsf(a.x) < 0.9f ? v3(1, 0, 0) : v3(0, 1, 0);
        return m4_axis_angle(v3cross(a, p), 3.14159265f);
    }
    return m4_axis_angle(ax, atan2f(s, c));
}

static M4 frame_of(V3 d, V3 n)
{
    V3 x = v3norm(d), y = v3sub(n, v3mul(x, v3dot(n, x)));
    if (v3len(y) < 1e-7f) y = v3cross(x, fabsf(x.x) < 0.9f ? v3(1, 0, 0) : v3(0, 1, 0));
    y = v3norm(y);
    V3 z = v3cross(x, y);
    M4 m = m4_id();
    V3 c[3] = { x, y, z };
    for (int i = 0; i < 3; i++) { m.m[4 * i] = c[i].x; m.m[4 * i + 1] = c[i].y; m.m[4 * i + 2] = c[i].z; }
    return m;
}

M4 m4_align(V3 d0, V3 n0, V3 d1, V3 n1)
{
    M4 f0 = frame_of(d0, n0), f1 = frame_of(d1, n1), t = m4_id();
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++) t.m[4 * j + i] = f0.m[4 * i + j];   /* transpose = inverse */
    return m4_mul(f1, t);
}

/* ---------------------------------------------------------------- keyframes */

static void clip_init(Clip *c, const char *name, int start, int end)
{
    memset(c, 0, sizeof *c);
    snprintf(c->name, sizeof c->name, "%s", name);
    c->start = start;
    c->end = end > start ? end : start + 1;
}

static void clip_free(Clip *c)
{
    for (int i = 0; i < c->nt; i++) free(c->t[i].k);
    free(c->t);
    c->t = NULL;
    c->nt = c->ct = 0;
}

void anim_init(Anim *a)
{
    memset(a, 0, sizeof *a);
    a->fps = 24;
    anim_add_clip(a, "Take1", 0, 48);
}

void anim_free(Anim *a)
{
    for (int i = 0; i < a->n; i++) clip_free(&a->c[i]);
    free(a->c);
    memset(a, 0, sizeof *a);
}

Anim anim_copy(const Anim *a)
{
    Anim r = *a;
    r.c = xalloc((a->cap ? a->cap : 1) * sizeof(Clip));
    for (int i = 0; i < a->n; i++) {
        Clip *d = &r.c[i];
        const Clip *s = &a->c[i];
        *d = *s;
        d->t = xalloc((s->ct ? s->ct : 1) * sizeof(Track));
        for (int k = 0; k < s->nt; k++) {
            d->t[k] = s->t[k];
            d->t[k].k = xalloc((s->t[k].cap ? s->t[k].cap : 1) * sizeof(Key));
            memcpy(d->t[k].k, s->t[k].k, s->t[k].n * sizeof(Key));
        }
    }
    return r;
}

int anim_add_clip(Anim *a, const char *name, int start, int end)
{
    if (a->n == a->cap) {
        a->cap = a->cap ? a->cap * 2 : 4;
        a->c = xgrow(a->c, a->cap * sizeof(Clip));
    }
    clip_init(&a->c[a->n], name, start, end);
    return a->n++;
}

void anim_remove_clip(Anim *a, int i)
{
    if (i < 0 || i >= a->n) return;
    clip_free(&a->c[i]);
    memmove(a->c + i, a->c + i + 1, (a->n - i - 1) * sizeof(Clip));
    a->n--;
    if (a->cur >= a->n) a->cur = a->n - 1;
    if (a->cur < 0) a->cur = 0;
}

Clip *anim_clip(Anim *a)
{
    if (!a->n) anim_add_clip(a, "Take1", 0, 48);
    if (a->cur < 0 || a->cur >= a->n) a->cur = 0;
    return &a->c[a->cur];
}

Track *clip_track(Clip *c, unsigned obj, int create)
{
    for (int i = 0; i < c->nt; i++) if (c->t[i].obj == obj) return &c->t[i];
    if (!create) return NULL;
    if (c->nt == c->ct) {
        c->ct = c->ct ? c->ct * 2 : 8;
        c->t = xgrow(c->t, c->ct * sizeof(Track));
    }
    Track *t = &c->t[c->nt++];
    memset(t, 0, sizeof *t);
    t->obj = obj;
    return t;
}

void clip_drop_obj(Clip *c, unsigned obj)
{
    for (int i = 0; i < c->nt; i++)
        if (c->t[i].obj == obj) {
            free(c->t[i].k);
            memmove(c->t + i, c->t + i + 1, (c->nt - i - 1) * sizeof(Track));
            c->nt--;
            return;
        }
}

int track_find(const Track *t, float f)
{
    for (int i = 0; t && i < t->n; i++) if (fabsf(t->k[i].f - f) < 0.5f) return i;
    return -1;
}

void track_set(Track *t, float f, V3 tt, V3 r, V3 s, int ease)
{
    int i = track_find(t, f);
    if (i < 0) {
        if (t->n == t->cap) {
            t->cap = t->cap ? t->cap * 2 : 8;
            t->k = xgrow(t->k, t->cap * sizeof(Key));
        }
        i = t->n;
        while (i > 0 && t->k[i - 1].f > f) { t->k[i] = t->k[i - 1]; i--; }
        t->n++;
    }
    t->k[i].f = f;
    t->k[i].t = tt;
    t->k[i].r = r;
    t->k[i].s = s;
    t->k[i].ease = (unsigned char)ease;
}

int track_remove(Track *t, float f)
{
    int i = track_find(t, f);
    if (i < 0) return 0;
    memmove(t->k + i, t->k + i + 1, (t->n - i - 1) * sizeof(Key));
    t->n--;
    return 1;
}

int track_move(Track *t, float from, float to)
{
    int i = track_find(t, from);
    if (i < 0 || fabsf(from - to) < 0.5f) return 0;
    Key k = t->k[i];
    track_remove(t, from);
    track_set(t, to, k.t, k.r, k.s, k.ease);
    return 1;
}

int track_neighbor(const Track *t, float f, int dir, float *out)
{
    int found = 0;
    for (int i = 0; t && i < t->n; i++) {
        float kf = t->k[i].f;
        if (dir < 0 ? kf < f - 0.5f : kf > f + 0.5f) {
            if (!found || (dir < 0 ? kf > *out : kf < *out)) *out = kf;
            found = 1;
        }
    }
    return found;
}

/* slope at key j of one channel: Catmull-Rom, limited so the curve never overshoots (flat at peaks
   and at the ends) */
static float key_slope(const Track *t, int j, int ch)
{
    if (j <= 0 || j >= t->n - 1) return 0;
    const Key *p = &t->k[j - 1], *c = &t->k[j], *n = &t->k[j + 1];
    float vp = (&p->t.x)[ch], vc = (&c->t.x)[ch], vn = (&n->t.x)[ch];
    float d0 = (vc - vp) / (c->f - p->f), d1 = (vn - vc) / (n->f - c->f);
    if (d0 * d1 <= 0) return 0;
    float m = (vn - vp) / (n->f - p->f), lim = 3 * fminf(fabsf(d0), fabsf(d1));
    if (fabsf(m) > lim) m = m > 0 ? lim : -lim;
    return m;
}

int track_eval(const Track *t, float f, V3 *tt, V3 *r, V3 *s)
{
    if (!t || !t->n) return 0;
    float out[9];
    const Key *a = &t->k[0], *z = &t->k[t->n - 1];
    if (f <= a->f || t->n == 1) { *tt = a->t; *r = a->r; *s = a->s; return 1; }
    if (f >= z->f) { *tt = z->t; *r = z->r; *s = z->s; return 1; }
    int i = 0;
    while (i + 1 < t->n && t->k[i + 1].f <= f) i++;
    const Key *k0 = &t->k[i], *k1 = &t->k[i + 1];
    float h = k1->f - k0->f, u = h > 0 ? (f - k0->f) / h : 0;
    for (int ch = 0; ch < 9; ch++) {
        float v0 = (&k0->t.x)[ch], v1 = (&k1->t.x)[ch];
        if (k0->ease == EASE_STEP) out[ch] = v0;
        else if (k0->ease == EASE_LINEAR) out[ch] = v0 + (v1 - v0) * u;
        else {
            float m0 = key_slope(t, i, ch) * h, m1 = key_slope(t, i + 1, ch) * h, u2 = u * u, u3 = u2 * u;
            out[ch] = (2 * u3 - 3 * u2 + 1) * v0 + (u3 - 2 * u2 + u) * m0 + (-2 * u3 + 3 * u2) * v1 + (u3 - u2) * m1;
        }
    }
    *tt = v3(out[0], out[1], out[2]);
    *r = v3(out[3], out[4], out[5]);
    *s = v3(out[6], out[7], out[8]);
    return 1;
}

/* ---------------------------------------------------------------- skinning */

/* how far p is from a bone, capsule style: mostly how far past the bone's ends it lies along the
   bone, and only a little how far out to the side. That way a vertex belongs to the bone it sits
   beside, even on a chunky limb that is wider than its bones are long. */
static float seg_dist3(V3 p, V3 a, V3 b)
{
    V3 ab = v3sub(b, a);
    float l2 = v3dot(ab, ab);
    if (l2 <= 1e-12f) return v3len(v3sub(p, a));
    float t = v3dot(v3sub(p, a), ab) / l2, l = sqrtf(l2);
    float axial = t < 0 ? -t * l : t > 1 ? (t - 1) * l : 0;
    float radial = v3len(v3sub(p, v3add(a, v3mul(ab, t))));
    return sqrtf(axial * axial + 0.12f * radial * radial);
}

void skin_auto(Mesh *m, const M4 *mw, const Bone *bones, int nb, int iters)
{
    int nj = m->nsj, nv = m->nv;
    if (!nj || !nv || !m->ww) return;
    float *d = xalloc((size_t)nv * nj * sizeof(float)), *tmp = xalloc((size_t)nv * nj * sizeof(float));
    V3 lo = v3(1e30f, 1e30f, 1e30f), hi = v3(-1e30f, -1e30f, -1e30f);
    for (int v = 0; v < nv; v++) {
        V3 p = m4_pt(*mw, m->p[v]);
        lo = v3(fminf(lo.x, p.x), fminf(lo.y, p.y), fminf(lo.z, p.z));
        hi = v3(fmaxf(hi.x, p.x), fmaxf(hi.y, p.y), fmaxf(hi.z, p.z));
    }
    float eps = v3len(v3sub(hi, lo)) * 1e-3f + 1e-6f;
    for (int v = 0; v < nv; v++) {
        V3 p = m4_pt(*mw, m->p[v]);
        float *row = d + (size_t)v * nj, best = 1e30f;
        for (int b = 0; b < nb; b++) best = fminf(best, seg_dist3(p, bones[b].a, bones[b].b));
        for (int b = 0; b < nb; b++) {
            if (bones[b].owner < 0 || bones[b].owner >= nj) continue;
            float dd = seg_dist3(p, bones[b].a, bones[b].b);
            if (dd > best * 2 + eps * 4) continue;   /* only the bones about as near as the nearest */
            float q = (dd + eps) / (best + eps), q2 = q * q, w = 1 / (q2 * q2 * q2);
            if (w > row[bones[b].owner]) row[bones[b].owner] = w;
        }
        float s = 0;
        for (int j = 0; j < nj; j++) s += row[j];
        for (int j = 0; j < nj && s > 0; j++) row[j] /= s;
    }
    /* smooth over the mesh's edges so joints bend softly. Only short edges (against the bones) blend,
       so a low-poly mesh keeps crisp weights instead of smearing them across the whole model. */
    float blen = 0;
    int nbl = 0;
    for (int b = 0; b < nb; b++) { float l = v3len(v3sub(bones[b].b, bones[b].a)); if (l > 0) { blen += l; nbl++; } }
    blen = nbl ? blen / nbl : v3len(v3sub(hi, lo)) * 0.25f;
    float reach = 0.25f * blen + 1e-9f, *ek = xalloc((m->ne + 1) * sizeof(float)), *ksum = xalloc((nv + 1) * sizeof(float));
    for (int e = 0; e < m->ne; e++) {
        float l = v3len(v3sub(m4_pt(*mw, m->p[m->ev[2 * e]]), m4_pt(*mw, m->p[m->ev[2 * e + 1]])));
        ek[e] = l < reach ? 1 - l / reach : 0;
    }
    for (int it = 0; it < iters; it++) {
        memset(tmp, 0, (size_t)nv * nj * sizeof(float));
        memset(ksum, 0, (nv + 1) * sizeof(float));
        for (int e = 0; e < m->ne; e++) {
            int a = m->ev[2 * e], b = m->ev[2 * e + 1];
            if (ek[e] <= 0) continue;
            ksum[a] += ek[e];
            ksum[b] += ek[e];
            for (int j = 0; j < nj; j++) {
                tmp[(size_t)a * nj + j] += ek[e] * d[(size_t)b * nj + j];
                tmp[(size_t)b * nj + j] += ek[e] * d[(size_t)a * nj + j];
            }
        }
        for (int v = 0; v < nv; v++)
            for (int j = 0; j < nj && ksum[v] > 0; j++) {
                size_t i = (size_t)v * nj + j;
                d[i] = (d[i] + tmp[i]) / (1 + ksum[v]);
            }
    }
    free(ek);
    free(ksum);
    /* keep the strongest four */
    for (int v = 0; v < nv; v++) {
        memset(m->wj + 4 * v, 0, 4);
        memset(m->ww + 4 * v, 0, 4 * sizeof(float));
        float *row = d + (size_t)v * nj, mx = 0;
        for (int j = 0; j < nj; j++) mx = fmaxf(mx, row[j]);
        for (int j = 0; j < nj; j++)
            if (row[j] > mx * 0.02f) skin_add(m->wj, m->ww, v, j, row[j]);
        skin_normalize(m->wj, m->ww, v);
    }
    free(d);
    free(tmp);
}

void skin_deform(const Mesh *m, const M4 *jw, V3 *out)
{
    M4 *S = xalloc((m->nsj + 1) * sizeof(M4));
    for (int j = 0; j < m->nsj; j++) {
        M4 ibm;
        memcpy(ibm.m, m->ibm + 16 * j, sizeof ibm.m);
        S[j] = m4_mul(jw[j], ibm);
    }
    for (int v = 0; v < m->nv; v++) {
        V3 p = m->p[v], r = v3(0, 0, 0);
        float tw = 0;
        for (int k = 0; m->ww && k < 4; k++) {
            float w = m->ww[4 * v + k];
            int j = m->wj[4 * v + k];
            if (w <= 0 || j >= m->nsj) continue;
            r = v3add(r, v3mul(m4_pt(S[j], p), w));
            tw += w;
        }
        out[v] = tw > 0 ? v3mul(r, 1 / tw) : p;
    }
    free(S);
}

/* ---------------------------------------------------------------- IK */

void ik_two_bone(V3 a, float l1, float l2, V3 t, V3 pole, V3 *b, V3 *c)
{
    V3 d = v3sub(t, a);
    float L = v3len(d), mn = fabsf(l1 - l2) + 1e-5f, mx = l1 + l2 - 1e-5f;
    V3 dir = L > 1e-9f ? v3mul(d, 1 / L) : v3(0, -1, 0);
    if (L < mn) L = mn;
    if (L > mx) L = mx;
    float ca = (l1 * l1 + L * L - l2 * l2) / (2 * l1 * L);
    ca = ca < -1 ? -1 : ca > 1 ? 1 : ca;
    float sa = sqrtf(1 - ca * ca);
    V3 pp = v3sub(pole, v3mul(dir, v3dot(pole, dir)));
    if (v3len(pp) < 1e-7f) pp = v3cross(dir, fabsf(dir.x) < 0.9f ? v3(1, 0, 0) : v3(0, 0, 1));
    pp = v3norm(pp);
    *b = v3add(a, v3add(v3mul(dir, l1 * ca), v3mul(pp, l1 * sa)));
    *c = v3add(a, v3mul(dir, L));
}

/* ---------------------------------------------------------------- left / right names */

int name_mirror(const char *in, char *out, int n)
{
    static const char *pairs[][2] = { { "Left", "Right" }, { "left", "right" }, { "LEFT", "RIGHT" } };
    char b[64];
    snprintf(b, sizeof b, "%s", in);
    for (int i = 0; i < 3; i++)
        for (int k = 0; k < 2; k++) {
            const char *f = strstr(b, pairs[i][k]);
            if (!f) continue;
            int at = (int)(f - b);
            snprintf(out, n, "%.*s%s%s", at, b, pairs[i][!k], b + at + strlen(pairs[i][k]));
            return 1;
        }
    size_t l = strlen(b);
    static const char sides[][2] = { { 'L', 'R' }, { 'l', 'r' } };
    if (l >= 3 && (b[l - 2] == '_' || b[l - 2] == '.' || b[l - 2] == '-'))
        for (int i = 0; i < 2; i++)
            for (int k = 0; k < 2; k++)
                if (b[l - 1] == sides[i][k]) { b[l - 1] = sides[i][!k]; snprintf(out, n, "%s", b); return 1; }
    if (l >= 3 && (b[1] == '_' || b[1] == '.'))
        for (int i = 0; i < 2; i++)
            for (int k = 0; k < 2; k++)
                if (b[0] == sides[i][k]) { b[0] = sides[i][!k]; snprintf(out, n, "%s", b); return 1; }
    return 0;
}
