/* tuff3d - keyframe animation, skinning and IK math (platform independent) */
#ifndef TUFF_ANIM_H
#define TUFF_ANIM_H

#include "mesh.h"

/* ---- rotations */
typedef struct { float x, y, z, w; } Quat;
Quat quat_from_m4(M4 m);                       /* the rotation of m (columns normalized first) */
M4 m4_from_quat(Quat q);
void m4_decompose(M4 m, V3 *t, V3 *rdeg, V3 *s);  /* inverse of m4_trs (no shear); rdeg: Maya xyz order */
M4 m4_rot_between(V3 from, V3 to);             /* the smallest rotation taking direction from to to */
/* the rotation taking the frame (d0, n0) to (d1, n1): d is a direction, n is turned to stay square to it */
M4 m4_align(V3 d0, V3 n0, V3 d1, V3 n1);
M4 m4_rot3(M4 m);                              /* m with its translation removed and columns normalized */

/* ---- keyframes. A key holds an object's whole transform at a frame; ease shapes the stretch from
   this key to the next one (smooth: an ease in / out curve that never overshoots, linear, step). */
enum { EASE_SMOOTH, EASE_LINEAR, EASE_STEP };
typedef struct { float f; V3 t, r, s; unsigned char ease; } Key;
typedef struct { unsigned obj; int n, cap; Key *k; } Track;
typedef struct { char name[32]; int start, end, nt, ct; Track *t; } Clip;
typedef struct { int n, cap, cur; float fps; Clip *c; } Anim;

void anim_init(Anim *a);                      /* one clip, "Take1", frames 0 - 48 at 24 fps */
void anim_free(Anim *a);
Anim anim_copy(const Anim *a);
int anim_add_clip(Anim *a, const char *name, int start, int end);   /* returns its index */
void anim_remove_clip(Anim *a, int i);
Clip *anim_clip(Anim *a);                     /* the current clip */
Track *clip_track(Clip *c, unsigned obj, int create);
void clip_drop_obj(Clip *c, unsigned obj);    /* forget an object's keys */
int track_find(const Track *t, float f);      /* the key at frame f (within half a frame), or -1 */
void track_set(Track *t, float f, V3 tt, V3 r, V3 s, int ease);
int track_remove(Track *t, float f);
/* moves the key at frame from to frame to (replacing one there); returns 1 if it moved */
int track_move(Track *t, float from, float to);
/* the transform at frame f; returns 0 when the track has no keys */
int track_eval(const Track *t, float f, V3 *tt, V3 *r, V3 *s);
int track_neighbor(const Track *t, float f, int dir, float *out);   /* previous (-1) / next (1) key frame */

/* ---- skinning */
typedef struct { V3 a, b; int owner; } Bone;  /* a bone segment a -> b (world), moving with skin joint 'owner' */
/* automatic weights for the vertices of m (in world space through meshWorld): the nearest bones win,
   blended by inverse distance, then smoothed over the mesh's edges. m must already have its skin
   arrays (mesh_skin_alloc). */
void skin_auto(Mesh *m, const M4 *meshWorld, const Bone *bones, int nb, int smoothIters);
/* linear blend skinning: out[v] = sum of weight * jointWorld[j] * ibm[j] * p[v] (world space) */
void skin_deform(const Mesh *m, const M4 *jointWorld, V3 *out);

/* ---- two bone IK: from the start joint at a with bone lengths l1, l2, reach for target t, bending
   toward the pole direction. Writes where the middle and end joints go. */
void ik_two_bone(V3 a, float l1, float l2, V3 t, V3 pole, V3 *b, V3 *c);

/* ---- names: "LeftArm" <-> "RightArm", "arm_L" <-> "arm_R", "L_arm", "arm.l" ...; 0 when the name
   has no side */
int name_mirror(const char *in, char *out, int n);

#endif
