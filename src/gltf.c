/* tuff3d - glTF 2.0 binary (.glb) export */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "gltf.h"

typedef struct { char *d; size_t n, cap; } Buf;

static void buf_put(Buf *b, const void *p, size_t n)
{
    if (b->n + n + 1 > b->cap) {
        while (b->n + n + 1 > b->cap) b->cap = b->cap ? b->cap * 2 : 4096;
        b->d = realloc(b->d, b->cap);
        if (!b->d) abort();
    }
    if (p) memcpy(b->d + b->n, p, n); else memset(b->d + b->n, 0, n);
    b->n += n;
    b->d[b->n] = 0;
}

static void buf_printf(Buf *b, const char *fmt, ...)
{
    char tmp[512];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(tmp, sizeof tmp, fmt, ap);
    va_end(ap);
    if (n < 0) return;
    if ((size_t)n < sizeof tmp) { buf_put(b, tmp, n); return; }
    char *big = malloc(n + 1);
    if (!big) abort();
    va_start(ap, fmt);
    vsnprintf(big, n + 1, fmt, ap);
    va_end(ap);
    buf_put(b, big, n);
    free(big);
}

static void buf_str(Buf *b, const char *s)   /* a JSON string */
{
    buf_put(b, "\"", 1);
    for (; s && *s; s++) {
        unsigned char c = (unsigned char)*s;
        if (c == '"' || c == '\\') { buf_put(b, "\\", 1); buf_put(b, s, 1); }
        else if (c < 0x20) buf_printf(b, "\\u%04x", c);
        else buf_put(b, s, 1);
    }
    buf_put(b, "\"", 1);
}

/* the binary chunk, its views and accessors, written as JSON fragments as they are made */
typedef struct { Buf bin, views, accs; int nviews, naccs; } Gl;

static void sep(Buf *b) { if (b->n) buf_put(b, ",", 1); }

static int add_view(Gl *g, const void *data, size_t n, int target)
{
    while (g->bin.n % 4) buf_put(&g->bin, NULL, 1);
    size_t off = g->bin.n;
    buf_put(&g->bin, data, n);
    sep(&g->views);
    buf_printf(&g->views, "{\"buffer\":0,\"byteOffset\":%lu,\"byteLength\":%lu", (unsigned long)off, (unsigned long)n);
    if (target) buf_printf(&g->views, ",\"target\":%d", target);
    buf_put(&g->views, "}", 1);
    return g->nviews++;
}

/* type: "SCALAR", "VEC3", ...; comps per element; ctype 5126 float, 5121 ubyte, 5123 ushort, 5125 uint */
static int add_acc(Gl *g, const void *data, int count, int ctype, const char *type, int comps, int target, int minmax)
{
    int csize = ctype == 5126 || ctype == 5125 ? 4 : ctype == 5123 ? 2 : 1;
    int view = add_view(g, data, (size_t)count * comps * csize, target);
    sep(&g->accs);
    buf_printf(&g->accs, "{\"bufferView\":%d,\"componentType\":%d,\"count\":%d,\"type\":\"%s\"", view, ctype, count, type);
    if (minmax && ctype == 5126 && count) {
        const float *f = data;
        float mn[16], mx[16];
        for (int c = 0; c < comps; c++) { mn[c] = mx[c] = f[c]; }
        for (int i = 1; i < count; i++)
            for (int c = 0; c < comps; c++) { float v = f[i * comps + c]; if (v < mn[c]) mn[c] = v; if (v > mx[c]) mx[c] = v; }
        buf_put(&g->accs, ",\"min\":[", 8);
        for (int c = 0; c < comps; c++) buf_printf(&g->accs, "%s%.9g", c ? "," : "", mn[c]);
        buf_put(&g->accs, "],\"max\":[", 9);
        for (int c = 0; c < comps; c++) buf_printf(&g->accs, "%s%.9g", c ? "," : "", mx[c]);
        buf_put(&g->accs, "]", 1);
    }
    buf_put(&g->accs, "}", 1);
    return g->naccs++;
}

/* one mesh: triangles, split where corners need different shading normals. Writes its JSON to meshes
   and returns 1, or 0 when it has no faces. */
static int write_mesh(Gl *g, Buf *meshes, const GNode *nd)
{
    Mesh m = mesh_copy(nd->mesh);
    mesh_triangulate(&m, NULL);
    int nc = m.fo[m.nf];
    if (!m.nf || nc < 3) { mesh_free(&m); return 0; }
    V3 *cn = malloc((nc + 1) * sizeof(V3));
    int *head = malloc((m.nv + 1) * sizeof(int)), *next = malloc((nc + 1) * sizeof(int)), *src = malloc((nc + 1) * sizeof(int));
    unsigned *idx = malloc((nc + 1) * sizeof(unsigned));
    V3 *nrm = malloc((nc + 1) * sizeof(V3));
    if (!cn || !head || !next || !src || !idx || !nrm) abort();
    mesh_cnormals(&m, nd->angle, cn);
    for (int v = 0; v < m.nv; v++) head[v] = -1;
    int nout = 0, colors = 0;
    for (int c = 0; c < nc; c++) {
        int v = m.fi[c], k = head[v];
        while (k >= 0 && memcmp(&nrm[k], &cn[c], sizeof(V3))) k = next[k];
        if (k < 0) {
            k = nout++;
            src[k] = v;
            nrm[k] = cn[c];
            next[k] = head[v];
            head[v] = k;
        }
        idx[c] = (unsigned)k;
    }
    for (int v = 0; v < m.nv && !colors; v++) colors = m.c[v].x != 1 || m.c[v].y != 1 || m.c[v].z != 1;
    float *pos = malloc(nout * 3 * sizeof(float)), *nr = malloc(nout * 3 * sizeof(float)), *col = malloc(nout * 3 * sizeof(float));
    if (!pos || !nr || !col) abort();
    for (int k = 0; k < nout; k++) {
        V3 p = m.p[src[k]], c = m.c[src[k]], n = v3norm(nrm[k]);
        if (v3len(n) < 0.5f) n = v3(0, 1, 0);
        memcpy(pos + 3 * k, &p, sizeof p);
        memcpy(nr + 3 * k, &n, sizeof n);
        memcpy(col + 3 * k, &c, sizeof c);
    }
    int aPos = add_acc(g, pos, nout, 5126, "VEC3", 3, 34962, 1);
    int aNrm = add_acc(g, nr, nout, 5126, "VEC3", 3, 34962, 0);
    int aCol = colors ? add_acc(g, col, nout, 5126, "VEC3", 3, 34962, 0) : -1;
    int aJ = -1, aW = -1;
    if (m.ww && nd->skinJoints) {
        unsigned char *jj = calloc(nout * 4, 1);
        float *ww = calloc(nout * 4, sizeof(float));
        if (!jj || !ww) abort();
        for (int k = 0; k < nout; k++) {
            float s = 0;
            for (int i = 0; i < 4; i++) {
                float w = m.ww[4 * src[k] + i];
                if (w > 0 && m.wj[4 * src[k] + i] < m.nsj) { jj[4 * k + i] = m.wj[4 * src[k] + i]; ww[4 * k + i] = w; s += w; }
            }
            if (s <= 0) { jj[4 * k] = 0; ww[4 * k] = 1; s = 1; }   /* unweighted: follow the first joint */
            for (int i = 0; i < 4; i++) ww[4 * k + i] /= s;
        }
        aJ = add_acc(g, jj, nout, 5121, "VEC4", 4, 34962, 0);
        aW = add_acc(g, ww, nout, 5126, "VEC4", 4, 34962, 0);
        free(jj);
        free(ww);
    }
    int aIdx;
    if (nout < 65536) {
        unsigned short *s16 = malloc(nc * sizeof(unsigned short));
        if (!s16) abort();
        for (int c = 0; c < nc; c++) s16[c] = (unsigned short)idx[c];
        aIdx = add_acc(g, s16, nc, 5123, "SCALAR", 1, 34963, 0);
        free(s16);
    } else {
        aIdx = add_acc(g, idx, nc, 5125, "SCALAR", 1, 34963, 0);
    }
    sep(meshes);
    buf_put(meshes, "{\"name\":", 8);
    char nm[80];
    snprintf(nm, sizeof nm, "%s_mesh", nd->name ? nd->name : "mesh");   /* its own name: Roblox adds _Node when they match */
    buf_str(meshes, nm);
    buf_printf(meshes, ",\"primitives\":[{\"attributes\":{\"POSITION\":%d,\"NORMAL\":%d", aPos, aNrm);
    if (aCol >= 0) buf_printf(meshes, ",\"COLOR_0\":%d", aCol);
    if (aJ >= 0) buf_printf(meshes, ",\"JOINTS_0\":%d,\"WEIGHTS_0\":%d", aJ, aW);
    buf_printf(meshes, "},\"indices\":%d,\"material\":0,\"mode\":4}]}", aIdx);
    free(pos); free(nr); free(col); free(cn); free(head); free(next); free(src); free(idx); free(nrm);
    mesh_free(&m);
    return 1;
}

static int is_default(V3 v, float d) { return v.x == d && v.y == d && v.z == d; }

int gltf_write(const char *path, const GNode *nodes, int nn, const GAnim *anims, int na)
{
    Gl g;
    Buf meshes = { 0 }, skins = { 0 }, nodej = { 0 }, animj = { 0 }, json = { 0 };
    memset(&g, 0, sizeof g);
    int *meshOf = malloc((nn + 1) * sizeof(int)), *skinOf = malloc((nn + 1) * sizeof(int)), nm = 0, ns = 0;
    if (!meshOf || !skinOf) abort();
    for (int i = 0; i < nn; i++) {
        meshOf[i] = skinOf[i] = -1;
        if (nodes[i].mesh && write_mesh(&g, &meshes, &nodes[i])) meshOf[i] = nm++;
        if (meshOf[i] >= 0 && nodes[i].skinJoints && nodes[i].mesh->ww && nodes[i].mesh->nsj) {
            const Mesh *m = nodes[i].mesh;
            int aIbm = add_acc(&g, m->ibm, m->nsj, 5126, "MAT4", 16, 0, 0);
            sep(&skins);
            buf_printf(&skins, "{\"inverseBindMatrices\":%d,\"joints\":[", aIbm);
            for (int j = 0; j < m->nsj; j++) buf_printf(&skins, "%s%d", j ? "," : "", nodes[i].skinJoints[j]);
            buf_put(&skins, "]}", 2);
            skinOf[i] = ns++;
        }
    }
    for (int i = 0; i < nn; i++) {
        const GNode *n = &nodes[i];
        sep(&nodej);
        buf_put(&nodej, "{\"name\":", 8);
        buf_str(&nodej, n->name ? n->name : "node");
        int nch = 0;
        for (int k = 0; k < nn; k++) {
            if (nodes[k].parent != i) continue;
            buf_printf(&nodej, nch++ ? ",%d" : ",\"children\":[%d", k);
        }
        if (nch) buf_put(&nodej, "]", 1);
        if (!is_default(n->t, 0)) buf_printf(&nodej, ",\"translation\":[%.9g,%.9g,%.9g]", n->t.x, n->t.y, n->t.z);
        if (n->r.x || n->r.y || n->r.z || n->r.w != 1) buf_printf(&nodej, ",\"rotation\":[%.9g,%.9g,%.9g,%.9g]", n->r.x, n->r.y, n->r.z, n->r.w);
        if (!is_default(n->s, 1)) buf_printf(&nodej, ",\"scale\":[%.9g,%.9g,%.9g]", n->s.x, n->s.y, n->s.z);
        if (meshOf[i] >= 0) buf_printf(&nodej, ",\"mesh\":%d", meshOf[i]);
        if (skinOf[i] >= 0) buf_printf(&nodej, ",\"skin\":%d", skinOf[i]);
        buf_put(&nodej, "}", 1);
    }
    for (int a = 0; a < na; a++) {
        const GAnim *an = &anims[a];
        if (an->nf < 1 || !an->nch) continue;
        float *times = malloc(an->nf * sizeof(float));
        if (!times) abort();
        for (int f = 0; f < an->nf; f++) times[f] = f / (an->fps > 0 ? an->fps : 24);
        int aIn = add_acc(&g, times, an->nf, 5126, "SCALAR", 1, 0, 1);
        free(times);
        Buf samp = { 0 }, chan = { 0 };
        int nsamp = 0;
        for (int c = 0; c < an->nch; c++) {
            const GChan *ch = &an->ch[c];
            const float *src[3] = { ch->t, ch->r, ch->s };
            static const char *path[3] = { "translation", "rotation", "scale" };
            for (int p = 0; p < 3; p++) {
                if (!src[p]) continue;
                int comps = p == 1 ? 4 : 3, aOut;
                if (p == 1) {
                    /* unit quaternions on the short way round from frame to frame */
                    float *q = malloc(an->nf * 4 * sizeof(float));
                    if (!q) abort();
                    for (int f = 0; f < an->nf; f++) {
                        float *o = q + 4 * f, l = 0;
                        memcpy(o, src[p] + 4 * f, 4 * sizeof(float));
                        for (int k = 0; k < 4; k++) l += o[k] * o[k];
                        l = l > 0 ? 1 / sqrtf(l) : 1;
                        for (int k = 0; k < 4; k++) o[k] *= l;
                        if (f && o[0] * o[-4] + o[1] * o[-3] + o[2] * o[-2] + o[3] * o[-1] < 0)
                            for (int k = 0; k < 4; k++) o[k] = -o[k];
                    }
                    aOut = add_acc(&g, q, an->nf, 5126, "VEC4", 4, 0, 0);
                    free(q);
                } else {
                    aOut = add_acc(&g, src[p], an->nf, 5126, "VEC3", comps, 0, 0);
                }
                sep(&samp);
                buf_printf(&samp, "{\"input\":%d,\"output\":%d,\"interpolation\":\"LINEAR\"}", aIn, aOut);
                sep(&chan);
                buf_printf(&chan, "{\"sampler\":%d,\"target\":{\"node\":%d,\"path\":\"%s\"}}", nsamp++, ch->node, path[p]);
            }
        }
        if (nsamp) {
            sep(&animj);
            buf_put(&animj, "{\"name\":", 8);
            buf_str(&animj, an->name ? an->name : "Take1");
            buf_printf(&animj, ",\"samplers\":[%s],\"channels\":[%s]}", samp.d ? samp.d : "", chan.d ? chan.d : "");
        }
        free(samp.d);
        free(chan.d);
    }
    while (g.bin.n % 4) buf_put(&g.bin, NULL, 1);
    buf_printf(&json, "{\"asset\":{\"version\":\"2.0\",\"generator\":\"tuff3d\"},\"scene\":0,\"scenes\":[{\"nodes\":[");
    int nr = 0;
    for (int i = 0; i < nn; i++) if (nodes[i].parent < 0) buf_printf(&json, nr++ ? ",%d" : "%d", i);
    buf_printf(&json, "]}],\"nodes\":[%s]", nodej.d ? nodej.d : "");
    if (nm) {
        buf_printf(&json, ",\"meshes\":[%s]", meshes.d);
        buf_printf(&json, ",\"materials\":[{\"name\":\"tuff3d\",\"pbrMetallicRoughness\":{\"baseColorFactor\":[1,1,1,1],"
                          "\"metallicFactor\":0,\"roughnessFactor\":0.8}}]");
    }
    if (ns) buf_printf(&json, ",\"skins\":[%s]", skins.d);
    if (animj.n) buf_printf(&json, ",\"animations\":[%s]", animj.d);
    if (g.bin.n) {
        buf_printf(&json, ",\"buffers\":[{\"byteLength\":%lu}]", (unsigned long)g.bin.n);
        buf_printf(&json, ",\"bufferViews\":[%s],\"accessors\":[%s]", g.views.d, g.accs.d);
    }
    buf_put(&json, "}", 1);
    while (json.n % 4) buf_put(&json, " ", 1);

    int ok = 0;
    FILE *f = fopen(path, "wb");
    if (f) {
        unsigned hdr[3] = { 0x46546C67u, 2, (unsigned)(12 + 8 + json.n + (g.bin.n ? 8 + g.bin.n : 0)) };
        unsigned jh[2] = { (unsigned)json.n, 0x4E4F534Au }, bh[2] = { (unsigned)g.bin.n, 0x004E4942u };
        ok = fwrite(hdr, 4, 3, f) == 3 && fwrite(jh, 4, 2, f) == 2 && fwrite(json.d, 1, json.n, f) == json.n;
        if (ok && g.bin.n) ok = fwrite(bh, 4, 2, f) == 2 && fwrite(g.bin.d, 1, g.bin.n, f) == g.bin.n;
        if (fclose(f)) ok = 0;
    }
    free(g.bin.d); free(g.views.d); free(g.accs.d);
    free(meshes.d); free(skins.d); free(nodej.d); free(animj.d); free(json.d);
    free(meshOf); free(skinOf);
    return ok ? 0 : -1;
}
