/* tuff3d - glTF 2.0 binary (.glb) export: meshes with vertex colors and shading normals, the node
   hierarchy, skins (bones) and animation clips. Godot, Roblox Studio, Blender, Unity (with glTFast),
   three.js and most other tools read it. */
#ifndef TUFF_GLTF_H
#define TUFF_GLTF_H

#include "anim.h"

typedef struct {
    const char *name;
    int parent;              /* node index, -1 for a root */
    V3 t, s;                 /* local translation and scale */
    Quat r;                  /* local rotation */
    const Mesh *mesh;        /* NULL: a joint or an empty */
    float angle;             /* normal angle for the shading normals */
    const int *skinJoints;   /* skinned meshes: the node index of each of mesh->sj */
} GNode;

/* one animated node: a value per frame (3, 4 or 3 floats); NULL when that part does not move */
typedef struct { int node; const float *t, *r, *s; } GChan;
typedef struct { const char *name; int nf; float fps; int nch; const GChan *ch; } GAnim;

/* returns 0 on success, -1 if the file could not be written */
int gltf_write(const char *path, const GNode *nodes, int nn, const GAnim *anims, int na);

#endif
