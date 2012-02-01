/* 
    Zz Dynamic Parser Library
    Copyright (C) 1989 - I.N.F.N - S.Cabasino, P.S.Paolucci, G.M.Todesco

    The Zz Library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    The Zz Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __PAN_AVL_H__
#define __PAN_AVL_H__

/*----------------------------------------------------------------------------*
 |                                                                            |
 |                                   avl.h                                    |
 |                                                                            |
 |                by W.T.  11-MAR-91, last revision 22-MAR-92                 |
 |                                                                            |
 |  Modification 5/98: Indroduced typedef void (*UsrFun)(void *, void *)      |
 *----------------------------------------------------------------------------*/

/* @(#)avl.h	8.111/27/93 */
/* $Id: avl.h,v 1.2 2002/01/11 11:52:02 brooks Exp $ */

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef __STDC__
#define AVL_PROTOTYPES
#else
#ifdef VAX
#define AVL_PROTOTYPES
#endif
#endif

#define AVL_MAX_SAFE_OFFSET 1024

#ifdef AVL_C
         double Avl_Dummy[(AVL_MAX_SAFE_OFFSET / sizeof(double)) + 1];
#else
  extern double Avl_Dummy[];
#endif

#define avl_offset(structure,member) \
  ((unsigned)((char *)&((structure *)Avl_Dummy)->member - (char *)Avl_Dummy))

#define AVL_NODUP_MBR (0 << 1)
#define AVL_NODUP_PTR (1 << 1)
#define AVL_NODUP_STR (2 << 1)
#define AVL_NODUP_LNG (3 << 1)
#define AVL_NODUP_INT (4 << 1)
#define AVL_NODUP_SHT (5 << 1)
#define AVL_NODUP_ULN (6 << 1)
#define AVL_NODUP_UIN (7 << 1)
#define AVL_NODUP_USH (8 << 1)
#define AVL_NODUP_CHR (9 << 1)

#define AVL_DUP_MBR (AVL_NODUP_MBR | 1)
#define AVL_DUP_PTR (AVL_NODUP_PTR | 1)
#define AVL_DUP_STR (AVL_NODUP_STR | 1)
#define AVL_DUP_LNG (AVL_NODUP_LNG | 1)
#define AVL_DUP_INT (AVL_NODUP_INT | 1)
#define AVL_DUP_SHT (AVL_NODUP_SHT | 1)
#define AVL_DUP_ULN (AVL_NODUP_ULN | 1)
#define AVL_DUP_UIN (AVL_NODUP_UIN | 1)
#define AVL_DUP_USH (AVL_NODUP_USH | 1)
#define AVL_DUP_CHR (AVL_NODUP_CHR | 1)

#define AVL_AVLCMP (int(*)(void *,void *))0

#define avl_tree_nodup(usrcmp)\
        avl__tree (AVL_NODUP_MBR, (unsigned)0, (usrcmp))
#define avl_tree_nodup_mbr(structure, member, usrcmp) \
        avl__tree (AVL_NODUP_MBR, avl_offset(structure,member), (usrcmp))
#define avl_tree_nodup_ptr(structure, member, usrcmp) \
        avl__tree (AVL_NODUP_PTR, avl_offset(structure,member), (usrcmp))
#define avl_tree_nodup_str(structure, member) \
        avl__tree (AVL_NODUP_STR, avl_offset(structure,member), AVL_AVLCMP)
#define avl_tree_nodup_long(structure, member) \
        avl__tree (AVL_NODUP_LNG, avl_offset(structure,member), AVL_AVLCMP)
#define avl_tree_nodup_int(structure, member) \
        avl__tree (AVL_NODUP_INT, avl_offset(structure,member), AVL_AVLCMP)
#define avl_tree_nodup_short(structure, member) \
        avl__tree (AVL_NODUP_SHT, avl_offset(structure,member), AVL_AVLCMP)
#define avl_tree_nodup_ulong(structure, member) \
        avl__tree (AVL_NODUP_ULN, avl_offset(structure,member), AVL_AVLCMP)
#define avl_tree_nodup_uint(structure, member) \
        avl__tree (AVL_NODUP_UIN, avl_offset(structure,member), AVL_AVLCMP)
#define avl_tree_nodup_ushort(structure, member) \
        avl__tree (AVL_NODUP_USH, avl_offset(structure,member), AVL_AVLCMP)
#define avl_tree_nodup_char(structure, member) \
        avl__tree (AVL_NODUP_CHR, avl_offset(structure,member), AVL_AVLCMP)

#define avl_tree_dup(usrcmp)\
        avl__tree (AVL_DUP_MBR, (unsigned)0, (usrcmp))
#define avl_tree_dup_mbr(structure, member, usrcmp) \
        avl__tree (AVL_DUP_MBR, avl_offset(structure,member), (usrcmp))
#define avl_tree_dup_ptr(structure, member, usrcmp) \
        avl__tree (AVL_DUP_PTR, avl_offset(structure,member), (usrcmp))
#define avl_tree_dup_str(structure, member) \
        avl__tree (AVL_DUP_STR, avl_offset(structure,member), AVL_AVLCMP)
#define avl_tree_dup_long(structure, member) \
        avl__tree (AVL_DUP_LNG, avl_offset(structure,member), AVL_AVLCMP)
#define avl_tree_dup_int(structure, member) \
        avl__tree (AVL_DUP_INT, avl_offset(structure,member), AVL_AVLCMP)
#define avl_tree_dup_short(structure, member) \
        avl__tree (AVL_DUP_SHT, avl_offset(structure,member), AVL_AVLCMP)
#define avl_tree_dup_ulong(structure, member) \
        avl__tree (AVL_DUP_ULN, avl_offset(structure,member), AVL_AVLCMP)
#define avl_tree_dup_uint(structure, member) \
        avl__tree (AVL_DUP_UIN, avl_offset(structure,member), AVL_AVLCMP)
#define avl_tree_dup_ushort(structure, member) \
        avl__tree (AVL_DUP_USH, avl_offset(structure,member), AVL_AVLCMP)
#define avl_tree_dup_char(structure, member) \
        avl__tree (AVL_DUP_CHR, avl_offset(structure,member), AVL_AVLCMP)

#define avl_nodes(tree) ((tree)->nodes)

#define avl_locate(tree,    key)  avl__locate    ((tree), (long)(key))
#define avl_locate_ge(tree, key)  avl__locate_ge ((tree), (long)(key))
#define avl_locate_gt(tree, key)  avl__locate_gt ((tree), (long)(key))
#define avl_locate_le(tree, key)  avl__locate_le ((tree), (long)(key))
#define avl_locate_lt(tree, key)  avl__locate_lt ((tree), (long)(key))
#define avl_remove(tree,    key)  avl__remove    ((tree), (long)(key))

#define avl_scan(tree,     usrfun)  avl__scan ((tree), (usrfun), 0)
#define avl_backscan(tree, usrfun)  avl__scan ((tree), (usrfun), 1)

#define avl_start(tree,     key)  avl__start ((tree), (long)(key), 0)
#define avl_backstart(tree, key)  avl__start ((tree), (long)(key), 1)

#define avl_link(tree, structure, member) \
        avl__link ((tree), avl_offset(structure,member), 0)
#define avl_backlink(tree, structure, member) \
        avl__link ((tree), avl_offset(structure,member), 1)

#define AVL_MAX_PATHDEPTH ((int)sizeof(long)*8 * 3/2 - 2)

union avl_key
  {
  void *ptr;
  long  val;
  };

struct avl_node
  {
  union  avl_key   key;
  void            *data;
  struct avl_node *left;
  struct avl_node *right;
  int              bal;
  };

struct avl_path
  {
  struct avl_node **pathnode;
  char             *pathright;
  struct avl_node  *node [AVL_MAX_PATHDEPTH + 2];
  char              right[AVL_MAX_PATHDEPTH + 2];
  };

typedef struct avl_tree
  {
  unsigned short   keyinfo;
  unsigned short   keyoffs;
  int            (*usrcmp)(void *,void *);
  long             nodes;
  struct avl_node *root;
  struct avl_path *path;
  } TREE;

#ifndef AVL_PROTOTYPES

TREE *avl__tree ();
TREE *avl_copy  ();

int   avl_insert       ();
void *avl__locate      ();
void *avl__locate_ge   ();
void *avl__locate_gt   ();
void *avl__locate_le   ();
void *avl__locate_lt   ();
void *avl_locate_first ();
void *avl_locate_last  ();
void *avl__remove      ();

void avl__scan   ();

void *avl_first  ();
void *avl_last   ();
void *avl__start ();
void *avl_next   ();
void *avl_prev   ();
void  avl_stop   ();

void *avl__link  ();

void avl_release ();
void avl_reset   ();
void avl_close   ();

int avl_dump ();

int avl_porting_problems ();

#else

typedef void (*UsrFun)(void *, void *);

TREE *avl__tree (int treetype, unsigned keyoffs, int (*usrcmp)(void *,void *));
TREE *avl_copy  (TREE *tree);

int   avl_insert       (TREE *tree, void *data);
void *avl__locate      (TREE *tree, long keyval);
void *avl__locate_ge   (TREE *tree, long keyval);
void *avl__locate_gt   (TREE *tree, long keyval);
void *avl__locate_le   (TREE *tree, long keyval);
void *avl__locate_lt   (TREE *tree, long keyval);
void *avl_locate_first (TREE *tree);
void *avl_locate_last  (TREE *tree);
void *avl__remove      (TREE *tree, long keyval);


void avl__scan   (TREE *tree, UsrFun, int back);

void *avl_first  (TREE *tree);
void *avl_last   (TREE *tree);
void *avl__start (TREE *tree, long keyval, int back);
void *avl_next   (TREE *tree);
void *avl_prev   (TREE *tree);
void  avl_stop   (TREE *tree);

void *avl__link  (TREE *tree, unsigned ptroffs, int back);

void avl_release (TREE *tree, UsrFun);
void avl_reset   (TREE *tree);
void avl_close   (TREE *tree);

int avl_dump (TREE *tree, void **dat_vect, int *lev_vect, int *pos_vect);

int avl_porting_problems (void);

#endif

#if defined(__cplusplus)
}
#endif

#endif

