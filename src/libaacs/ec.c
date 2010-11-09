/* ec.c -  Elliptic Curve functions
   Copyright (C) 2007 Free Software Foundation, Inc.

   This file is part of Libgcrypt.

   Libgcrypt is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   Libgcrypt is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.  */

#define mpi_alloc(m)        gcry_mpi_new(0)
#define mpi_alloc_like(m)   gcry_mpi_new(0)
#define mpi_alloc_set_ui(u) gcry_mpi_set_ui(gcry_mpi_new(0), u)
#define mpi_free(m)         gcry_mpi_release(m)
#define mpi_is_neg(m)      (mpi_cmp_ui(m, 0) < 0)
#define DIM(t)             (sizeof(t)/sizeof(t[0]))

/************************************ mpi.h ***********************************/

/* Object to represent a point in projective coordinates. */
typedef struct mpi_point_s mpi_point_t;
struct mpi_point_s
{
  gcry_mpi_t x;
  gcry_mpi_t y;
  gcry_mpi_t z;
};

/************************************ ec.c ************************************/

#include <config.h>
#include <stdio.h>
#include <stdlib.h>

#define point_init(a)  _gcry_mpi_ec_point_init ((a))
#define point_free(a)  _gcry_mpi_ec_point_free ((a))

/* This context is used with all our EC functions. */
typedef struct mpi_ec_ctx_s *mpi_ec_t;
struct mpi_ec_ctx_s
{
  /* Domain parameters.  */
  gcry_mpi_t p;   /* Prime specifying the field GF(p).  */
  gcry_mpi_t a;   /* First coefficient of the Weierstrass equation.  */

  int a_is_pminus3;  /* True if A = P - 3. */

  /* Some often used constants.  */
  gcry_mpi_t one;
  gcry_mpi_t two;
  gcry_mpi_t three;
  gcry_mpi_t four;
  gcry_mpi_t eight;
  gcry_mpi_t two_inv_p;

  /* Scratch variables.  */
  gcry_mpi_t scratch[11];
};


/* Initialized a point object.  gcry_mpi_ec_point_free shall be used
   to release this object.  */
static void
_gcry_mpi_ec_point_init (mpi_point_t *p)
{
  p->x = mpi_new (0);
  p->y = mpi_new (0);
  p->z = mpi_new (0);
}


/* Release a point object. */
static void
_gcry_mpi_ec_point_free (mpi_point_t *p)
{
  mpi_free (p->x); p->x = NULL;
  mpi_free (p->y); p->y = NULL;
  mpi_free (p->z); p->z = NULL;
}

/* Set the value from S into D.  */
static void
point_set (mpi_point_t *d, mpi_point_t *s)
{
  mpi_set (d->x, s->x);
  mpi_set (d->y, s->y);
  mpi_set (d->z, s->z);
}



static void
ec_addm (gcry_mpi_t w, gcry_mpi_t u, gcry_mpi_t v, mpi_ec_t ctx)
{
  mpi_addm (w, u, v, ctx->p);
}

static void
ec_subm (gcry_mpi_t w, gcry_mpi_t u, gcry_mpi_t v, mpi_ec_t ctx)
{
  mpi_subm (w, u, v, ctx->p);
}

static void
ec_mulm (gcry_mpi_t w, gcry_mpi_t u, gcry_mpi_t v, mpi_ec_t ctx)
{
    mpi_mulm (w, u, v, ctx->p);
}

static void
ec_powm (gcry_mpi_t w, const gcry_mpi_t b, const gcry_mpi_t e,
         mpi_ec_t ctx)
{
  mpi_powm (w, b, e, ctx->p);
}

static void
ec_invm (gcry_mpi_t x, gcry_mpi_t a, mpi_ec_t ctx)
{
  mpi_invm (x, a, ctx->p);
}



/* This function returns a new context for elliptic curve based on the
   field GF(p).  P is the prime specifying thuis field, A is the first
   coefficient.

   This context needs to be released using _gcry_mpi_ec_free.  */
static mpi_ec_t
_gcry_mpi_ec_init (gcry_mpi_t p, gcry_mpi_t a)
{
  unsigned i;
  mpi_ec_t ctx;
  gcry_mpi_t tmp;

#if 0
  mpi_normalize (p);
  mpi_normalize (a);
#endif

  /* Fixme: Do we want to check some constraints? e.g.
     a < p
  */

  ctx = gcry_xcalloc (1, sizeof *ctx);

  ctx->p = mpi_copy (p);
  ctx->a = mpi_copy (a);

  tmp = mpi_alloc_like (ctx->p);
  mpi_sub_ui (tmp, ctx->p, 3);
  ctx->a_is_pminus3 = !mpi_cmp (ctx->a, tmp);
  mpi_free (tmp);


  /* Allocate constants.  */
  ctx->one   = mpi_alloc_set_ui (1);
  ctx->two   = mpi_alloc_set_ui (2);
  ctx->three = mpi_alloc_set_ui (3);
  ctx->four  = mpi_alloc_set_ui (4);
  ctx->eight = mpi_alloc_set_ui (8);
  ctx->two_inv_p = mpi_alloc (0);
  ec_invm (ctx->two_inv_p, ctx->two, ctx);

  /* Allocate scratch variables.  */
  for (i=0; i< DIM(ctx->scratch); i++)
    ctx->scratch[i] = mpi_alloc_like (ctx->p);

  return ctx;
}

static void
_gcry_mpi_ec_free (mpi_ec_t ctx)
{
  unsigned i;

  if (!ctx)
    return;

  mpi_free (ctx->p);
  mpi_free (ctx->a);

  mpi_free (ctx->one);
  mpi_free (ctx->two);
  mpi_free (ctx->three);
  mpi_free (ctx->four);
  mpi_free (ctx->eight);

  mpi_free (ctx->two_inv_p);

  for (i=0; i< DIM(ctx->scratch); i++)
    mpi_free (ctx->scratch[i]);

  gcry_free (ctx);
}

/* Compute the affine coordinates from the projective coordinates in
   POINT.  Set them into X and Y.  If one coordinate is not required,
   X or Y may be passed as NULL.  CTX is the usual context. Returns: 0
   on success or !0 if POINT is at infinity.  */
static int
_gcry_mpi_ec_get_affine (gcry_mpi_t x, gcry_mpi_t y, mpi_point_t *point,
                         mpi_ec_t ctx)
{
  gcry_mpi_t z1, z2, z3;

  if (!mpi_cmp_ui (point->z, 0))
    return -1;

  z1 = mpi_new (0);
  z2 = mpi_new (0);
  ec_invm (z1, point->z, ctx);  /* z1 = z^(-1) mod p  */
  ec_mulm (z2, z1, z1, ctx);    /* z2 = z^(-2) mod p  */

  if (x)
    ec_mulm (x, point->x, z2, ctx);

  if (y)
    {
      z3 = mpi_new (0);
      ec_mulm (z3, z2, z1, ctx);      /* z3 = z^(-3) mod p  */
      ec_mulm (y, point->y, z3, ctx);
      mpi_free (z3);
    }

  mpi_free (z2);
  mpi_free (z1);
  return 0;
}


/*  RESULT = 2 * POINT  */
static void
_gcry_mpi_ec_dup_point (mpi_point_t *result, mpi_point_t *point, mpi_ec_t ctx)
{
#define x3 (result->x)
#define y3 (result->y)
#define z3 (result->z)
#define t1 (ctx->scratch[0])
#define t2 (ctx->scratch[1])
#define t3 (ctx->scratch[2])
#define l1 (ctx->scratch[3])
#define l2 (ctx->scratch[4])
#define l3 (ctx->scratch[5])

  if (!mpi_cmp_ui (point->y, 0) || !mpi_cmp_ui (point->z, 0))
    {
      /* P_y == 0 || P_z == 0 => [1:1:0] */
      mpi_set_ui (x3, 1);
      mpi_set_ui (y3, 1);
      mpi_set_ui (z3, 0);
    }
  else
    {
      if (ctx->a_is_pminus3)  /* Use the faster case.  */
        {
          /* L1 = 3(X - Z^2)(X + Z^2) */
          /*                          T1: used for Z^2. */
          /*                          T2: used for the right term.  */
          ec_powm (t1, point->z, ctx->two, ctx);
          ec_subm (l1, point->x, t1, ctx);
          ec_mulm (l1, l1, ctx->three, ctx);
          ec_addm (t2, point->x, t1, ctx);
          ec_mulm (l1, l1, t2, ctx);
        }
      else /* Standard case. */
        {
          /* L1 = 3X^2 + aZ^4 */
          /*                          T1: used for aZ^4. */
          ec_powm (l1, point->x, ctx->two, ctx);
          ec_mulm (l1, l1, ctx->three, ctx);
          ec_powm (t1, point->z, ctx->four, ctx);
          ec_mulm (t1, t1, ctx->a, ctx);
          ec_addm (l1, l1, t1, ctx);
        }
      /* Z3 = 2YZ */
      ec_mulm (z3, point->y, point->z, ctx);
      ec_mulm (z3, z3, ctx->two, ctx);

      /* L2 = 4XY^2 */
      /*                              T2: used for Y2; required later. */
      ec_powm (t2, point->y, ctx->two, ctx);
      ec_mulm (l2, t2, point->x, ctx);
      ec_mulm (l2, l2, ctx->four, ctx);

      /* X3 = L1^2 - 2L2 */
      /*                              T1: used for L2^2. */
      ec_powm (x3, l1, ctx->two, ctx);
      ec_mulm (t1, l2, ctx->two, ctx);
      ec_subm (x3, x3, t1, ctx);

      /* L3 = 8Y^4 */
      /*                              T2: taken from above. */
      ec_powm (t2, t2, ctx->two, ctx);
      ec_mulm (l3, t2, ctx->eight, ctx);

      /* Y3 = L1(L2 - X3) - L3 */
      ec_subm (y3, l2, x3, ctx);
      ec_mulm (y3, y3, l1, ctx);
      ec_subm (y3, y3, l3, ctx);
    }

#undef x3
#undef y3
#undef z3
#undef t1
#undef t2
#undef t3
#undef l1
#undef l2
#undef l3
}



/* RESULT = P1 + P2 */
static void
_gcry_mpi_ec_add_points (mpi_point_t *result,
                         mpi_point_t *p1, mpi_point_t *p2,
                         mpi_ec_t ctx)
{
#define x1 (p1->x    )
#define y1 (p1->y    )
#define z1 (p1->z    )
#define x2 (p2->x    )
#define y2 (p2->y    )
#define z2 (p2->z    )
#define x3 (result->x)
#define y3 (result->y)
#define z3 (result->z)
#define l1 (ctx->scratch[0])
#define l2 (ctx->scratch[1])
#define l3 (ctx->scratch[2])
#define l4 (ctx->scratch[3])
#define l5 (ctx->scratch[4])
#define l6 (ctx->scratch[5])
#define l7 (ctx->scratch[6])
#define l8 (ctx->scratch[7])
#define l9 (ctx->scratch[8])
#define t1 (ctx->scratch[9])
#define t2 (ctx->scratch[10])

  if ( (!mpi_cmp (x1, x2)) && (!mpi_cmp (y1, y2)) && (!mpi_cmp (z1, z2)) )
    {
      /* Same point; need to call the duplicate function.  */
      _gcry_mpi_ec_dup_point (result, p1, ctx);
    }
  else if (!mpi_cmp_ui (z1, 0))
    {
      /* P1 is at infinity.  */
      mpi_set (x3, p2->x);
      mpi_set (y3, p2->y);
      mpi_set (z3, p2->z);
    }
  else if (!mpi_cmp_ui (z2, 0))
    {
      /* P2 is at infinity.  */
      mpi_set (x3, p1->x);
      mpi_set (y3, p1->y);
      mpi_set (z3, p1->z);
    }
  else
    {
      int z1_is_one = !mpi_cmp_ui (z1, 1);
      int z2_is_one = !mpi_cmp_ui (z2, 1);

      /* l1 = x1 z2^2  */
      /* l2 = x2 z1^2  */
      if (z2_is_one)
        mpi_set (l1, x1);
      else
        {
          ec_powm (l1, z2, ctx->two, ctx);
          ec_mulm (l1, l1, x1, ctx);
        }
      if (z1_is_one)
        mpi_set (l2, x1);
      else
        {
          ec_powm (l2, z1, ctx->two, ctx);
          ec_mulm (l2, l2, x2, ctx);
        }
      /* l3 = l1 - l2 */
      ec_subm (l3, l1, l2, ctx);
      /* l4 = y1 z2^3  */
      ec_powm (l4, z2, ctx->three, ctx);
      ec_mulm (l4, l4, y1, ctx);
      /* l5 = y2 z1^3  */
      ec_powm (l5, z1, ctx->three, ctx);
      ec_mulm (l5, l5, y2, ctx);
      /* l6 = l4 - l5  */
      ec_subm (l6, l4, l5, ctx);

      if (!mpi_cmp_ui (l3, 0))
        {
          if (!mpi_cmp_ui (l6, 0))
            {
              /* P1 and P2 are the same - use duplicate function.  */
              _gcry_mpi_ec_dup_point (result, p1, ctx);
            }
          else
            {
              /* P1 is the inverse of P2.  */
              mpi_set_ui (x3, 1);
              mpi_set_ui (y3, 1);
              mpi_set_ui (z3, 0);
            }
        }
      else
        {
          /* l7 = l1 + l2  */
          ec_addm (l7, l1, l2, ctx);
          /* l8 = l4 + l5  */
          ec_addm (l8, l4, l5, ctx);
          /* z3 = z1 z2 l3  */
          ec_mulm (z3, z1, z2, ctx);
          ec_mulm (z3, z3, l3, ctx);
          /* x3 = l6^2 - l7 l3^2  */
          ec_powm (t1, l6, ctx->two, ctx);
          ec_powm (t2, l3, ctx->two, ctx);
          ec_mulm (t2, t2, l7, ctx);
          ec_subm (x3, t1, t2, ctx);
          /* l9 = l7 l3^2 - 2 x3  */
          ec_mulm (t1, x3, ctx->two, ctx);
          ec_subm (l9, t2, t1, ctx);
          /* y3 = (l9 l6 - l8 l3^3)/2  */
          ec_mulm (l9, l9, l6, ctx);
          ec_powm (t1, l3, ctx->three, ctx); /* fixme: Use saved value*/
          ec_mulm (t1, t1, l8, ctx);
          ec_subm (y3, l9, t1, ctx);
          ec_mulm (y3, y3, ctx->two_inv_p, ctx);
        }
    }

#undef x1
#undef y1
#undef z1
#undef x2
#undef y2
#undef z2
#undef x3
#undef y3
#undef z3
#undef l1
#undef l2
#undef l3
#undef l4
#undef l5
#undef l6
#undef l7
#undef l8
#undef l9
#undef t1
#undef t2
}



/* Scalar point multiplication - the main function for ECC.  If takes
   an integer SCALAR and a POINT as well as the usual context CTX.
   RESULT will be set to the resulting point. */
static void
_gcry_mpi_ec_mul_point (mpi_point_t *result,
                        gcry_mpi_t scalar, mpi_point_t *point,
                        mpi_ec_t ctx)
{
  gcry_mpi_t x1, y1, z1, k, h, yy;
  unsigned int i, loops;
  mpi_point_t p1, p2, p1inv;

  x1 = mpi_alloc_like (ctx->p);
  y1 = mpi_alloc_like (ctx->p);
  h  = mpi_alloc_like (ctx->p);
  k  = mpi_copy (scalar);
  yy = mpi_copy (point->y);

#if 0
  if ( mpi_is_neg (k) )
    {
      k->sign = 0;
      ec_invm (yy, yy, ctx);
    }
#endif

  if (!mpi_cmp_ui (point->z, 1))
    {                           
      mpi_set (x1, point->x);
      mpi_set (y1, yy);
    }
  else
    {
      gcry_mpi_t z2, z3;

      z2 = mpi_alloc_like (ctx->p);
      z3 = mpi_alloc_like (ctx->p);
      ec_mulm (z2, point->z, point->z, ctx);      
      ec_mulm (z3, point->z, z2, ctx);      
      ec_invm (z2, z2, ctx);          
      ec_mulm (x1, point->x, z2, ctx);
      ec_invm (z3, z3, ctx);          
      ec_mulm (y1, yy, z3, ctx);      
      mpi_free (z2);
      mpi_free (z3);
    }
  z1 = mpi_copy (ctx->one);

  mpi_mul (h, k, ctx->three); /* h = 3k */
  loops = mpi_get_nbits (h);

  mpi_set (result->x, point->x);
  mpi_set (result->y, yy); mpi_free (yy); yy = NULL;
  mpi_set (result->z, point->z); 

  p1.x = x1; x1 = NULL;
  p1.y = y1; y1 = NULL;
  p1.z = z1; z1 = NULL;
  point_init (&p2);
  point_init (&p1inv);

  for (i=loops-2; i > 0; i--)
    { 
      _gcry_mpi_ec_dup_point (result, result, ctx);
      if (mpi_test_bit (h, i) == 1 && mpi_test_bit (k, i) == 0)
        { 
          point_set (&p2, result);
          _gcry_mpi_ec_add_points (result, &p2, &p1, ctx); 
        }
      if (mpi_test_bit (h, i) == 0 && mpi_test_bit (k, i) == 1)
        {
          point_set (&p2, result);
          /* Invert point: y = p - y mod p  */
          point_set (&p1inv, &p1);
          ec_subm (p1inv.y, ctx->p, p1inv.y, ctx);
          _gcry_mpi_ec_add_points (result, &p2, &p1inv, ctx); 
        }
    }

  point_free (&p1);
  point_free (&p2);
  point_free (&p1inv);
  mpi_free (h);
  mpi_free (k);
}
