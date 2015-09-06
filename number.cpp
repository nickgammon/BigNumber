/*
* this file is originally from GNU bc-1.06. it was trimmed down by lhf to fix
* a memory leak in bc_raisemod and to remove the free list, as in php bcmath.
*/

/* number.c: Implements arbitrary precision numbers. */
/*
    Copyright (C) 1991, 1992, 1993, 1994, 1997, 2000 Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License , or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING.  If not, write to:

      The Free Software Foundation, Inc.
      59 Temple Place, Suite 330
      Boston, MA 02111-1307 USA.


    You may contact the author by:
       e-mail:  philnelson@acm.org
      us-mail:  Philip A. Nelson
                Computer Science Department, 9062
                Western Washington University
                Bellingham, WA 98226-9062
       
*************************************************************************/

#include <stdio.h>
#include "bcconfig.h"
#include "number.h"
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>/* Prototypes needed for external utility routines. */

/* Storage used for special numbers. */
bc_num _zero_;
bc_num _one_;
bc_num _two_;

/* new_num allocates a number and sets fields to known values. */

bc_num
bc_new_num (uint16_t length, uint16_t scale)
{
  bc_num temp;

  temp = (bc_num) malloc (sizeof(bc_struct));
  if (temp == NULL) bc_out_of_memory ();
  temp->n_sign = PLUS;
  temp->n_len = length;
  temp->n_scale = scale;
  temp->n_refs = 1;
  temp->n_ptr = (char *) malloc (length + scale);
  if (temp->n_ptr == NULL) bc_out_of_memory();
  temp->n_value = temp->n_ptr;
  memset (temp->n_ptr, 0, length + scale);
  return temp;
}

/* "Frees" a bc_num NUM.  Actually decreases reference count and only
   frees the storage if reference count is zero. */

void
bc_free_num (bc_num *num)
{
  if (num == NULL || *num == NULL) return;
  (*num)->n_refs--;
  if ((*num)->n_refs == 0) {
    if ((*num)->n_ptr)
      free ((*num)->n_ptr);
    free (*num);
  }
  *num = NULL;
}


/* Intitialize the number package! */

void
bc_init_numbers ()
{
  _zero_ = bc_new_num (1, 0);
  _one_  = bc_new_num (1, 0);
  _one_->n_value[0] = 1;
  _two_  = bc_new_num (1, 0);
  _two_->n_value[0] = 2;
}


/* Make a copy of a number!  Just increments the reference count! */

bc_num bc_copy_num (bc_num num)
{
  num->n_refs++;
  return num;
}


/* Initialize a number NUM by making it a copy of zero. */

void
bc_init_num (bc_num *num)
{
  *num = bc_copy_num (_zero_);
}

/* For many things, we may have leading zeros in a number NUM.
   _bc_rm_leading_zeros just moves the data "value" pointer to the
   correct place and adjusts the length. */

static void
_bc_rm_leading_zeros (bc_num num)
{
  /* We can move n_value to point to the first non zero digit! */
  while (*num->n_value == 0 && num->n_len > 1) {
    num->n_value++;
    num->n_len--;
  }
}


/* Compare two bc numbers.  Return value is 0 if equal, -1 if N1 is less
   than N2 and +1 if N1 is greater than N2.  If USE_SIGN is false, just
   compare the magnitudes. */

static int
_bc_do_compare (bc_num n1, bc_num n2, bool use_sign, int ignore_last)
{
  /* First, compare signs. */
  if (use_sign && n1->n_sign != n2->n_sign)
    {
      if (n1->n_sign == PLUS)
	return 1;	/* Positive N1 > Negative N2 */
      else
	return -1;	/* Negative N1 < Positive N1 */
    }

  /* Now compare the magnitude. */
  if (n1->n_len != n2->n_len)
    {
      if (n1->n_len > n2->n_len)
	{
	  /* Magnitude of n1 > n2. */
	  if (!use_sign || n1->n_sign == PLUS)
	    return 1;

          return -1;
	}
      else
	{
	  /* Magnitude of n1 < n2. */
	  if (!use_sign || n1->n_sign == PLUS)
	    return -1;

          return 1;
	}
    }

  /* If we get here, they have the same number of integer digits.
     check the integer part and the equal length part of the fraction. */
  uint16_t count = n1->n_len + MIN (n1->n_scale, n2->n_scale);
  char *n1ptr = n1->n_value;
  char *n2ptr = n2->n_value;

  while (count > 0 && *n1ptr == *n2ptr)
    {
      n1ptr++;
      n2ptr++;
      count--;
    }
  if (ignore_last && count == 1 && n1->n_scale == n2->n_scale)
    return 0;
  if (count != 0)
    {
      if (*n1ptr > *n2ptr)
	{
	  /* Magnitude of n1 > n2. */
	  if (!use_sign || n1->n_sign == PLUS)
	    return 1;

	  return -1;
	}
      else
	{
	  /* Magnitude of n1 < n2. */
	  if (!use_sign || n1->n_sign == PLUS)
	    return -1;

	  return 1;
	}
    }

  /* They are equal up to the last part of the equal part of the fraction. */
  if (n1->n_scale != n2->n_scale)
    {
      if (n1->n_scale > n2->n_scale)
	{
	  for (count = n1->n_scale-n2->n_scale; count>0; count--)
	    if (*n1ptr++ != 0)
	      {
		/* Magnitude of n1 > n2. */
		if (!use_sign || n1->n_sign == PLUS)
		  return 1;

		return -1;
	      }
	}
      else
	{
	  for (count = n2->n_scale-n1->n_scale; count>0; count--)
	    if (*n2ptr++ != 0)
	      {
		/* Magnitude of n1 < n2. */
		if (!use_sign || n1->n_sign == PLUS)
		  return -1;

		return 1;
	      }
	}
    }

  /* They must be equal! */
  return 0;
}


/* This is the "user callable" routine to compare numbers N1 and N2. */

int
bc_compare (bc_num n1, bc_num n2)
{
  return _bc_do_compare (n1, n2, TRUE, FALSE);
}

/* In some places we need to check if the number is negative. */

char
bc_is_neg (bc_num num)
{
  return num->n_sign == MINUS;
}

/* In some places we need to check if the number NUM is zero. */

char
bc_is_zero (bc_num num)
{
  /* Quick check. */
  if (num == _zero_) return TRUE;

  /* Initialize */
  uint16_t count = num->n_len + num->n_scale;
  char *nptr = num->n_value;

  /* The check */
  while (count > 0 && *nptr++ == 0) count--;

  if (count != 0)
    return FALSE;

  return TRUE;
}

/* In some places we need to check if the number NUM is almost zero.
   Specifically, all but the last digit is 0 and the last digit is 1.
   Last digit is defined by scale. */

char
bc_is_near_zero (bc_num num, uint16_t scale)
{
  /* Error checking */
  if (scale > num->n_scale)
    scale = num->n_scale;

  /* Initialize */
  uint16_t count = num->n_len + scale;
  char *nptr = num->n_value;

  /* The check */
  while (count > 0 && *nptr++ == 0) count--;

  if (count != 0 && (count != 1 || *--nptr != 1))
    return FALSE;

  return TRUE;
}


/* Perform addition: N1 is added to N2 and the value is
   returned.  The signs of N1 and N2 are ignored.
   SCALE_MIN is to set the minimum scale of the result. */

static bc_num
_bc_do_add (bc_num n1, bc_num n2, uint16_t scale_min)
{
  char *sumptr = NULL;

  /* Prepare sum. */
  uint16_t sum_scale = MAX (n1->n_scale, n2->n_scale);
  uint16_t sum_digits = MAX (n1->n_len, n2->n_len) + 1;
  bc_num sum = bc_new_num (sum_digits, MAX(sum_scale, scale_min));

  /* Zero extra digits made by scale_min. */
  if (scale_min > sum_scale)
    {
      sumptr = (char *) (sum->n_value + sum_scale + sum_digits);
      for (uint16_t count = scale_min - sum_scale; count > 0; count--)
	*sumptr++ = 0;
    }

  /* Start with the fraction part.  Initialize the pointers. */
  uint16_t n1bytes = n1->n_scale;
  uint16_t n2bytes = n2->n_scale;
  char *n1ptr = (char *) (n1->n_value + n1->n_len + n1bytes - 1);
  char *n2ptr = (char *) (n2->n_value + n2->n_len + n2bytes - 1);
  sumptr = (char *) (sum->n_value + sum_scale + sum_digits - 1);

  /* Add the fraction part.  First copy the longer fraction.*/
  if (n1bytes != n2bytes)
    {
      if (n1bytes > n2bytes)
	while (n1bytes>n2bytes)
	  { *sumptr-- = *n1ptr--; n1bytes--;}
      else
	while (n2bytes>n1bytes)
	  { *sumptr-- = *n2ptr--; n2bytes--;}
    }

  /* Now add the remaining fraction part and equal size integer parts. */
  n1bytes += n1->n_len;
  n2bytes += n2->n_len;
  int8_t carry = 0;
  while ((n1bytes > 0) && (n2bytes > 0))
    {
      *sumptr = *n1ptr-- + *n2ptr-- + carry;
      if (*sumptr > (BASE-1))
	{
	   carry = 1;
	   *sumptr -= BASE;
	}
      else
	carry = 0;
      sumptr--;
      n1bytes--;
      n2bytes--;
    }

  /* Now add carry the longer integer part. */
  if (n1bytes == 0)
    { n1bytes = n2bytes; n1ptr = n2ptr; }
  while (n1bytes-- > 0)
    {
      *sumptr = *n1ptr-- + carry;
      if (*sumptr > (BASE-1))
	{
	   carry = 1;
	   *sumptr -= BASE;
	 }
      else
	carry = 0;
      sumptr--;
    }

  /* Set final carry. */
  if (carry == 1)
    *sumptr += 1;

  /* Adjust sum and return. */
  _bc_rm_leading_zeros (sum);
  return sum;
}


/* Perform subtraction: N2 is subtracted from N1 and the value is
   returned.  The signs of N1 and N2 are ignored.  Also, N1 is
   assumed to be larger than N2.  SCALE_MIN is the minimum scale
   of the result. */

static bc_num
_bc_do_sub (bc_num n1, bc_num n2, uint16_t scale_min)
{
  bc_num diff;
  char  *diffptr;
  int8_t borrow, val;

  /* Allocate temporary storage. */
  uint16_t diff_len = MAX (n1->n_len, n2->n_len);
  uint16_t diff_scale = MAX (n1->n_scale, n2->n_scale);
  uint16_t min_len = MIN  (n1->n_len, n2->n_len);
  uint16_t min_scale = MIN (n1->n_scale, n2->n_scale);
  diff = bc_new_num (diff_len, MAX(diff_scale, scale_min));

  /* Zero extra digits made by scale_min. */
  if (scale_min > diff_scale)
    {
      diffptr = (char *) (diff->n_value + diff_len + diff_scale);
      for (uint16_t count = scale_min - diff_scale; count > 0; count--)
	*diffptr++ = 0;
    }

  /* Initialize the subtract. */
  char *n1ptr = (char *) (n1->n_value + n1->n_len + n1->n_scale -1);
  char *n2ptr = (char *) (n2->n_value + n2->n_len + n2->n_scale -1);
  diffptr = (char *) (diff->n_value + diff_len + diff_scale -1);

  /* Subtract the numbers. */
  borrow = 0;

  /* Take care of the longer scaled number. */
  if (n1->n_scale != min_scale)
    {
      /* n1 has the longer scale */
      for (uint16_t count = n1->n_scale - min_scale; count > 0; count--)
	*diffptr-- = *n1ptr--;
    }
  else
    {
      /* n2 has the longer scale */
      for (uint16_t count = n2->n_scale - min_scale; count > 0; count--)
	{
	  val = - *n2ptr-- - borrow;
	  if (val < 0)
	    {
	      val += BASE;
	      borrow = 1;
	    }
	  else
	    borrow = 0;
	  *diffptr-- = val;
	}
    }

  /* Now do the equal length scale and integer parts. */

  for (uint16_t count = 0; count < min_len + min_scale; count++)
    {
      val = *n1ptr-- - *n2ptr-- - borrow;
      if (val < 0)
	{
	  val += BASE;
	  borrow = 1;
	}
      else
	borrow = 0;
      *diffptr-- = val;
    }

  /* If n1 has more digits then n2, we now do that subtract. */
  if (diff_len != min_len)
    {
      for (uint16_t count = diff_len - min_len; count > 0; count--)
	{
	  val = *n1ptr-- - borrow;
	  if (val < 0)
	    {
	      val += BASE;
	      borrow = 1;
	    }
	  else
	    borrow = 0;
	  *diffptr-- = val;
	}
    }

  /* Clean up and return. */
  _bc_rm_leading_zeros (diff);
  return diff;
}


/* Here is the full subtract routine that takes care of negative numbers.
   N2 is subtracted from N1 and the result placed in RESULT.  SCALE_MIN
   is the minimum scale for the result. */

void
bc_sub (bc_num n1, bc_num n2, bc_num *result, uint16_t scale_min)
{
  bc_num diff = NULL;

  if (n1->n_sign != n2->n_sign)
    {
      diff = _bc_do_add (n1, n2, scale_min);
      diff->n_sign = n1->n_sign;
    }
  else
    {
      /* subtraction must be done. */
      /* Compare magnitudes. */
      int16_t cmp_res = _bc_do_compare (n1, n2, FALSE, FALSE);
      switch (cmp_res)
	{
	case -1:
	  /* n1 is less than n2, subtract n1 from n2. */
	  diff = _bc_do_sub (n2, n1, scale_min);
	  diff->n_sign = n2->n_sign == PLUS ? MINUS : PLUS;
	  break;
	case  0:
	  /* They are equal! return zero! */
	  {
		  uint16_t res_scale = MAX (scale_min, MAX(n1->n_scale, n2->n_scale));
		  diff = bc_new_num (1, res_scale);
		  memset (diff->n_value, 0, res_scale+1);
	  }
	  break;
	case  1:
	  /* n2 is less than n1, subtract n2 from n1. */
	  diff = _bc_do_sub (n1, n2, scale_min);
	  diff->n_sign = n1->n_sign;
	  break;
	}
    }

  /* Clean up and return. */
  bc_free_num (result);
  *result = diff;
}


/* Here is the full add routine that takes care of negative numbers.
   N1 is added to N2 and the result placed into RESULT.  SCALE_MIN
   is the minimum scale for the result. */

void
bc_add (bc_num n1, bc_num n2, bc_num *result, uint16_t scale_min)
{
  bc_num sum = NULL;

  if (n1->n_sign == n2->n_sign)
    {
      sum = _bc_do_add (n1, n2, scale_min);
      sum->n_sign = n1->n_sign;
    }
  else
    {
      /* subtraction must be done. */
      int16_t cmp_res = _bc_do_compare (n1, n2, FALSE, FALSE);  /* Compare magnitudes. */
      switch (cmp_res)
	{
	case -1:
	  /* n1 is less than n2, subtract n1 from n2. */
	  sum = _bc_do_sub (n2, n1, scale_min);
	  sum->n_sign = n2->n_sign;
	  break;
	case  0:
	  /* They are equal! return zero with the correct scale! */
          {
		  uint16_t res_scale = MAX (scale_min, MAX(n1->n_scale, n2->n_scale));
		  sum = bc_new_num (1, res_scale);
		  memset (sum->n_value, 0, res_scale+1);
          }
	  break;
	case  1:
	  /* n2 is less than n1, subtract n2 from n1. */
	  sum = _bc_do_sub (n1, n2, scale_min);
	  sum->n_sign = n1->n_sign;
	}
    }

  /* Clean up and return. */
  bc_free_num (result);
  *result = sum;
}

/* Recursive vs non-recursive multiply crossover ranges. */
#if defined(MULDIGITS)
#include "muldigits.h"
#else
#define MUL_BASE_DIGITS 80
#endif

int mul_base_digits = MUL_BASE_DIGITS;
#define MUL_SMALL_DIGITS mul_base_digits/4

/* Multiply utility routines */

static bc_num
new_sub_num (int length, uint16_t scale, char *value)
{
  bc_num temp;

  temp = (bc_num) malloc (sizeof(bc_struct));
  if (temp == NULL) bc_out_of_memory ();
  temp->n_sign = PLUS;
  temp->n_len = length;
  temp->n_scale = scale;
  temp->n_refs = 1;
  temp->n_ptr = NULL;
  temp->n_value = value;
  return temp;
}

static void
_bc_simp_mul (bc_num n1, int n1len, bc_num n2, int n2len, bc_num *prod, int full_scale)
{
  uint16_t prodlen = n1len + n2len + 1;

  *prod = bc_new_num (prodlen, 0);

  char *n1end = (char *) (n1->n_value + n1len - 1);
  char *n2end = (char *) (n2->n_value + n2len - 1);
  char *pvptr = (char *) ((*prod)->n_value + prodlen - 1);

  /* Here is the loop... */
  int16_t sum = 0;
  for (uint16_t indx = 0; indx < prodlen-1; indx++)
    {
      char *n1ptr = (char *) (n1end - MAX(0, indx-n2len+1));
      char *n2ptr = (char *) (n2end - MIN(indx, n2len-1));
      while ((n1ptr >= n1->n_value) && (n2ptr <= n2end))
	sum += *n1ptr-- * *n2ptr++;
      *pvptr-- = sum % BASE;
      sum = sum / BASE;
    }
  *pvptr = sum;
}


/* A special adder/subtractor for the recursive divide and conquer
   multiply algorithm.  Note: if sub is called, accum must
   be larger that what is being subtracted.  Also, accum and val
   must have n_scale = 0.  (e.g. they must look like integers. *) */
static void
_bc_shift_addsub (bc_num accum, bc_num val, int16_t shift, int16_t sub)
{
  uint16_t count = val->n_len;
  if (val->n_value[0] == 0)
    count--;
  assert (accum->n_len+accum->n_scale >= shift+count);
  
  /* Set up pointers and others */
  signed char *accp = (signed char *)(accum->n_value + accum->n_len + accum->n_scale - shift - 1);
  signed char *valp = (signed char *)(val->n_value + val->n_len - 1);
  int8_t carry = 0;

  if (sub) {
    /* Subtraction, carry is really borrow. */
    while (count--) {
      *accp -= *valp-- + carry;
      if (*accp < 0) {
	carry = 1;
        *accp-- += BASE;
      } else {
	carry = 0;
	accp--;
      }
    }
    while (carry) {
      *accp -= carry;
      if (*accp < 0)
	*accp-- += BASE;
      else
	carry = 0;
    }
  } else {
    /* Addition */
    while (count--) {
      *accp += *valp-- + carry;
      if (*accp > (BASE-1)) {
	carry = 1;
        *accp-- -= BASE;
      } else {
	carry = 0;
	accp--;
      }
    }
    while (carry) {
      *accp += carry;
      if (*accp > (BASE-1))
	*accp-- -= BASE;
      else
	carry = 0;
    }
  }
}

/* Recursive divide and conquer multiply algorithm.  
   Based on 
   Let u = u0 + u1*(b^n)
   Let v = v0 + v1*(b^n)
   Then uv = (B^2n+B^n)*u1*v1 + B^n*(u1-u0)*(v0-v1) + (B^n+1)*u0*v0

   B is the base of storage, number of digits in u1,u0 close to equal.
*/
static void _bc_rec_mul (bc_num u, int ulen, bc_num v, int vlen, bc_num *prod, int full_scale)
{ 
  bc_num u0, u1, v0, v1;
  int u0len, v0len;
  bc_num m1, m2, m3, d1, d2;
  int n, prodlen, m1zero;
  int d1len, d2len;

  /* Base case? */
  if ((ulen+vlen) < mul_base_digits
      || ulen < MUL_SMALL_DIGITS
      || vlen < MUL_SMALL_DIGITS ) {
    _bc_simp_mul (u, ulen, v, vlen, prod, full_scale);
    return;
  }

  /* Calculate n -- the u and v split point in digits. */
  n = (MAX(ulen, vlen)+1) / 2;

  /* Split u and v. */
  if (ulen < n) {
    u1 = bc_copy_num (_zero_);
    u0 = new_sub_num (ulen,0, u->n_value);
  } else {
    u1 = new_sub_num (ulen-n, 0, u->n_value);
    u0 = new_sub_num (n, 0, u->n_value+ulen-n);
  }
  if (vlen < n) {
    v1 = bc_copy_num (_zero_);
    v0 = new_sub_num (vlen,0, v->n_value);
  } else {
    v1 = new_sub_num (vlen-n, 0, v->n_value);
    v0 = new_sub_num (n, 0, v->n_value+vlen-n);
    }
  _bc_rm_leading_zeros (u1);
  _bc_rm_leading_zeros (u0);
  u0len = u0->n_len;
  _bc_rm_leading_zeros (v1);
  _bc_rm_leading_zeros (v0);
  v0len = v0->n_len;

  m1zero = bc_is_zero(u1) || bc_is_zero(v1);

  /* Calculate sub results ... */

  bc_init_num(&d1);
  bc_init_num(&d2);
  bc_sub (u1, u0, &d1, 0);
  d1len = d1->n_len;
  bc_sub (v0, v1, &d2, 0);
  d2len = d2->n_len;


  /* Do recursive multiplies and shifted adds. */
  if (m1zero)
    m1 = bc_copy_num (_zero_);
  else
    _bc_rec_mul (u1, u1->n_len, v1, v1->n_len, &m1, 0);

  if (bc_is_zero(d1) || bc_is_zero(d2))
    m2 = bc_copy_num (_zero_);
  else
    _bc_rec_mul (d1, d1len, d2, d2len, &m2, 0);

  if (bc_is_zero(u0) || bc_is_zero(v0))
    m3 = bc_copy_num (_zero_);
  else
    _bc_rec_mul (u0, u0->n_len, v0, v0->n_len, &m3, 0);

  /* Initialize product */
  prodlen = ulen+vlen+1;
  *prod = bc_new_num(prodlen, 0);

  if (!m1zero) {
    _bc_shift_addsub (*prod, m1, 2*n, 0);
    _bc_shift_addsub (*prod, m1, n, 0);
  }
  _bc_shift_addsub (*prod, m3, n, 0);
  _bc_shift_addsub (*prod, m3, 0, 0);
  _bc_shift_addsub (*prod, m2, n, d1->n_sign != d2->n_sign);

  /* Now clean up! */
  bc_free_num (&u1);
  bc_free_num (&u0);
  bc_free_num (&v1);
  bc_free_num (&m1);
  bc_free_num (&v0);
  bc_free_num (&m2);
  bc_free_num (&m3);
  bc_free_num (&d1);
  bc_free_num (&d2);
}

/* The multiply routine.  N2 times N1 is put int PROD with the scale of
   the result being MIN(N2 scale+N1 scale, MAX (SCALE, N2 scale, N1 scale)).
   */

void
bc_multiply (bc_num n1, bc_num n2, bc_num *prod, uint16_t scale)
{
  bc_num pval; 

  /* Initialize things. */
  uint16_t len1 = n1->n_len + n1->n_scale;
  uint16_t len2 = n2->n_len + n2->n_scale;
  uint16_t full_scale = n1->n_scale + n2->n_scale;
  uint16_t prod_scale = MIN(full_scale,MAX(scale,MAX(n1->n_scale,n2->n_scale)));

  /* Do the multiply */
  _bc_rec_mul (n1, len1, n2, len2, &pval, full_scale);

  /* Assign to prod and clean up the number. */
  pval->n_sign = n1->n_sign == n2->n_sign ? PLUS : MINUS;
  pval->n_value = pval->n_ptr;
  pval->n_len = len2 + len1 + 1 - full_scale;
  pval->n_scale = prod_scale;
  _bc_rm_leading_zeros (pval);
  if (bc_is_zero (pval))
    pval->n_sign = PLUS;
  bc_free_num (prod);
  *prod = pval;
}

/* Some utility routines for the divide:  First a one digit multiply.
   NUM (with SIZE digits) is multiplied by DIGIT and the result is
   placed into RESULT.  It is written so that NUM and RESULT can be
   the same pointers.  */

static void _one_mult (unsigned char *num, int size, int digit, unsigned char *result)
{
  if (digit == 0)
    memset (result, 0, size);
  else
    {
      if (digit == 1)
	memcpy (result, num, size);
      else
	{
	  /* Initialize */
	  unsigned char *nptr = (unsigned char *) (num+size-1);
	  unsigned char *rptr = (unsigned char *) (result+size-1);
	  int8_t carry = 0, value = 0;

	  while (size-- > 0)
	    {
	      value = *nptr-- * digit + carry;
	      *rptr-- = value % BASE;
	      carry = value / BASE;
	    }

	  if (carry != 0) *rptr = carry;
	}
    }
}


/* The full division routine. This computes N1 / N2.  It returns
   0 if the division is ok and the result is in QUOT.  The number of
   digits after the decimal point is SCALE. It returns -1 if division
   by zero is tried.  The algorithm is found in Knuth Vol 2. p237. */

int
bc_divide (bc_num n1, bc_num n2, bc_num *quot, uint16_t scale)
{
  bc_num qval;
  unsigned char *num1, *num2;
  unsigned char *ptr1, *ptr2, *n2ptr, *qptr;
  int val;
  uint16_t scale1, qdigits, len1, len2, scale2, extra, count;
  uint16_t qdig, qguess, borrow, carry;
  unsigned char *mval;
  char zero;
  unsigned int  norm;

  /* Test for divide by zero. */
  if (bc_is_zero (n2)) return -1;

  /* Test for divide by 1.  If it is we must truncate. */
  if (n2->n_scale == 0)
    {
      if (n2->n_len == 1 && *n2->n_value == 1)
	{
	  qval = bc_new_num (n1->n_len, scale);
	  qval->n_sign = n1->n_sign == n2->n_sign ? PLUS : MINUS;
	  memset (&qval->n_value[n1->n_len],0,scale);
	  memcpy (qval->n_value, n1->n_value,
		  n1->n_len + MIN(n1->n_scale,scale));
	  bc_free_num (quot);
	  *quot = qval;
	}
    }

  /* Set up the divide.  Move the decimal point on n1 by n2's scale.
     Remember, zeros on the end of num2 are wasted effort for dividing. */
  scale2 = n2->n_scale;
  n2ptr = (unsigned char *) n2->n_value+n2->n_len+scale2-1;
  while ((scale2 > 0) && (*n2ptr-- == 0)) scale2--;

  len1 = n1->n_len + scale2;
  scale1 = n1->n_scale - scale2;
  if (scale1 < scale)
    extra = scale - scale1;
  else
    extra = 0;
  num1 = (unsigned char *) malloc (n1->n_len+n1->n_scale+extra+2);
  if (num1 == NULL) bc_out_of_memory();
  memset (num1, 0, n1->n_len+n1->n_scale+extra+2);
  memcpy (num1+1, n1->n_value, n1->n_len+n1->n_scale);

  len2 = n2->n_len + scale2;
  num2 = (unsigned char *) malloc (len2+1);
  if (num2 == NULL) bc_out_of_memory();
  memcpy (num2, n2->n_value, len2);
  *(num2+len2) = 0;
  n2ptr = num2;
  while (*n2ptr == 0)
    {
      n2ptr++;
      len2--;
    }

  /* Calculate the number of quotient digits. */
  if (len2 > len1+scale)
    {
      qdigits = scale+1;
      zero = TRUE;
    }
  else
    {
      zero = FALSE;
      if (len2>len1)
	qdigits = scale+1;  	/* One for the zero integer part. */
      else
	qdigits = len1-len2+scale+1;
    }

  /* Allocate and zero the storage for the quotient. */
  qval = bc_new_num (qdigits - scale, scale);
  memset (qval->n_value, 0, qdigits);

  /* Allocate storage for the temporary storage mval. */
  mval = (unsigned char *) malloc (len2+1);
  if (mval == NULL) bc_out_of_memory ();

  /* Now for the full divide algorithm. */
  if (!zero)
    {
      /* Normalize */
      norm =  10 / ((int)*n2ptr + 1);
      if (norm != 1)
	{
	  _one_mult (num1, len1+scale1+extra+1, norm, num1);
	  _one_mult (n2ptr, len2, norm, n2ptr);
	}

      /* Initialize divide loop. */
      qdig = 0;
      if (len2 > len1)
	qptr = (unsigned char *) qval->n_value+len2-len1;
      else
	qptr = (unsigned char *) qval->n_value;

      /* Loop */
      while (qdig <= len1+scale-len2)
	{
	  /* Calculate the quotient digit guess. */
	  if (*n2ptr == num1[qdig])
	    qguess = 9;
	  else
	    qguess = (num1[qdig]*10 + num1[qdig+1]) / *n2ptr;

	  /* Test qguess. */
	  if (n2ptr[1]*qguess >
	      (num1[qdig]*10 + num1[qdig+1] - *n2ptr*qguess)*10
	       + num1[qdig+2])
	    {
	      qguess--;
	      /* And again. */
	      if (n2ptr[1]*qguess >
		  (num1[qdig]*10 + num1[qdig+1] - *n2ptr*qguess)*10
		  + num1[qdig+2])
		qguess--;
	    }

	  /* Multiply and subtract. */
	  borrow = 0;
	  if (qguess != 0)
	    {
	      *mval = 0;
	      _one_mult (n2ptr, len2, qguess, mval+1);
	      ptr1 = (unsigned char *) num1+qdig+len2;
	      ptr2 = (unsigned char *) mval+len2;
	      for (count = 0; count < len2+1; count++)
		{
		  val = (int) *ptr1 - (int) *ptr2-- - borrow;
		  if (val < 0)
		    {
		      val += 10;
		      borrow = 1;
		    }
		  else
		    borrow = 0;
		  *ptr1-- = val;
		}
	    }

	  /* Test for negative result. */
	  if (borrow == 1)
	    {
	      qguess--;
	      ptr1 = (unsigned char *) num1+qdig+len2;
	      ptr2 = (unsigned char *) n2ptr+len2-1;
	      carry = 0;
	      for (count = 0; count < len2; count++)
		{
		  val = (int) *ptr1 + (int) *ptr2-- + carry;
		  if (val > 9)
		    {
		      val -= 10;
		      carry = 1;
		    }
		  else
		    carry = 0;
		  *ptr1-- = val;
		}
	      if (carry == 1) *ptr1 = (*ptr1 + 1) % 10;
	    }

	  /* We now know the quotient digit. */
	  *qptr++ =  qguess;
	  qdig++;
	}
    }

  /* Clean up and return the number. */
  qval->n_sign = n1->n_sign == n2->n_sign ? PLUS : MINUS;
  if (bc_is_zero (qval)) qval->n_sign = PLUS;
  _bc_rm_leading_zeros (qval);
  bc_free_num (quot);
  *quot = qval;

  /* Clean up temporary storage. */
  free (mval);
  free (num1);
  free (num2);

  return 0;	/* Everything is OK. */
}


/* Division *and* modulo for numbers.  This computes both NUM1 / NUM2 and
   NUM1 % NUM2  and puts the results in QUOT and REM, except that if QUOT
   is NULL then that store will be omitted.
 */

int
bc_divmod (bc_num num1, bc_num num2, bc_num *quot, bc_num *rem, uint16_t scale)
{
  bc_num quotient = NULL;
  bc_num temp;

  /* Check for correct numbers. */
  if (bc_is_zero (num2)) return -1;

  /* Calculate final scale. */
  uint16_t rscale = MAX (num1->n_scale, num2->n_scale+scale);
  bc_init_num(&temp);

  /* Calculate it. */
  bc_divide (num1, num2, &temp, scale);
  if (quot)
    quotient = bc_copy_num (temp);
  bc_multiply (temp, num2, &temp, rscale);
  bc_sub (num1, temp, rem, rscale);
  bc_free_num (&temp);

  if (quot)
    {
      bc_free_num (quot);
      *quot = quotient;
    }

  return 0;	/* Everything is OK. */
}


/* Modulo for numbers.  This computes NUM1 % NUM2  and puts the
   result in RESULT.   */

int
bc_modulo (bc_num num1, bc_num num2, bc_num *result, uint16_t scale)
{
  return bc_divmod (num1, num2, NULL, result, scale);
}

/* Raise BASE to the EXPO power, reduced modulo MOD.  The result is
   placed in RESULT.  If a EXPO is not an integer,
   only the integer part is used.  */

int
bc_raisemod (bc_num base, bc_num expo, bc_num mod, bc_num *result, uint16_t scale)
{
  bc_num power, exponent, parity, temp;
  uint16_t rscale;

  /* Check for correct numbers. */
  if (bc_is_zero(mod)) return -1;
  if (bc_is_neg(expo)) return -1;

  /* Set initial values.  */
  power = bc_copy_num (base);
  exponent = bc_copy_num (expo);
  temp = bc_copy_num (_one_);
  bc_init_num(&parity);

  /* Check the base for scale digits. */
  if (base->n_scale != 0)
      bc_rt_warn (BC_WARNING_NON_ZERO_SCALE_IN_BASE);

  /* Check the exponent for scale digits. */
  if (exponent->n_scale != 0)
    {
      bc_rt_warn (BC_WARNING_NON_ZERO_SCALE_IN_EXPONENT);
      bc_divide (exponent, _one_, &exponent, 0); /*truncate */
    }

  /* Check the modulus for scale digits. */
  if (mod->n_scale != 0)
      bc_rt_warn (BC_WARNING_NON_ZERO_SCALE_IN_MODULUS);

  /* Do the calculation. */
  rscale = MAX(scale, base->n_scale);
  while ( !bc_is_zero(exponent) )
    {
      (void) bc_divmod (exponent, _two_, &exponent, &parity, 0);
      if ( !bc_is_zero(parity) )
	{
	  bc_multiply (temp, power, &temp, rscale);
	  (void) bc_modulo (temp, mod, &temp, scale);
	}

      bc_multiply (power, power, &power, rscale);
      (void) bc_modulo (power, mod, &power, scale);
    }

  /* Assign the value. */
  bc_free_num (&power);
  bc_free_num (&exponent);
  bc_free_num (result);
  bc_free_num (&parity);
  *result = temp;
  return 0;	/* Everything is OK. */
}

/* Raise NUM1 to the NUM2 power.  The result is placed in RESULT.
   Maximum exponent is LONG_MAX.  If a NUM2 is not an integer,
   only the integer part is used.  */

void
bc_raise (bc_num num1, bc_num num2, bc_num *result, uint16_t scale)
{
   bc_num temp, power;
   long exponent;
   uint16_t rscale;
   uint16_t pwrscale;
   uint16_t calcscale;

   /* Check the exponent for scale digits and convert to a long. */
   if (num2->n_scale != 0)
     bc_rt_warn (BC_WARNING_NON_ZERO_SCALE_IN_EXPONENT);
   exponent = bc_num2long (num2);
   if (exponent == 0 && (num2->n_len > 1 || num2->n_value[0] != 0))
       bc_rt_error (BC_ERROR_EXPONENT_TOO_LARGE_IN_RAISE);

   /* Special case if exponent is a zero. */
   if (exponent == 0)
     {
       bc_free_num (result);
       *result = bc_copy_num (_one_);
       return;
     }

   /* Other initializations. */
   bool neg = false;
   if (exponent < 0)
     {
       neg = true;
       exponent = -exponent;
       rscale = scale;
     }
   else
     {
       rscale = MIN (num1->n_scale*exponent, MAX(scale, num1->n_scale));
     }

   /* Set initial value of temp.  */
   power = bc_copy_num (num1);
   pwrscale = num1->n_scale;
   while ((exponent & 1) == 0)
     {
       pwrscale = 2*pwrscale;
       bc_multiply (power, power, &power, pwrscale);
       exponent = exponent >> 1;
     }
   temp = bc_copy_num (power);
   calcscale = pwrscale;
   exponent = exponent >> 1;

   /* Do the calculation. */
   while (exponent > 0)
     {
       pwrscale = 2*pwrscale;
       bc_multiply (power, power, &power, pwrscale);
       if ((exponent & 1) == 1) {
	 calcscale = pwrscale + calcscale;
	 bc_multiply (temp, power, &temp, calcscale);
       }
       exponent = exponent >> 1;
     }

   /* Assign the value. */
   if (neg)
     {
       bc_divide (_one_, temp, result, rscale);
       bc_free_num (&temp);
     }
   else
     {
       bc_free_num (result);
       *result = temp;
       if ((*result)->n_scale > rscale)
	 (*result)->n_scale = rscale;
     }
   bc_free_num (&power);
}

/* Take the square root NUM and return it in NUM with SCALE digits
   after the decimal place. */

int
bc_sqrt (bc_num *num, uint16_t scale)
{
  uint16_t rscale, cscale;
  bc_num guess, guess1, point5, diff;

  /* Initial checks. */
  int cmp_res = bc_compare (*num, _zero_);
  if (cmp_res < 0)
    return 0;		/* error */
  else
    {
      if (cmp_res == 0)
	{
	  bc_free_num (num);
	  *num = bc_copy_num (_zero_);
	  return 1;
	}
    }
  cmp_res = bc_compare (*num, _one_);
  if (cmp_res == 0)
    {
      bc_free_num (num);
      *num = bc_copy_num (_one_);
      return 1;
    }

  /* Initialize the variables. */
  rscale = MAX (scale, (*num)->n_scale);
  bc_init_num(&guess);
  bc_init_num(&guess1);
  bc_init_num(&diff);
  point5 = bc_new_num (1,1);
  point5->n_value[1] = 5;

  /* Calculate the initial guess. */
  if (cmp_res < 0)
    {
      /* The number is between 0 and 1.  Guess should start at 1. */
      bc_free_num (&guess); // fix memory leak
      guess = bc_copy_num (_one_);
      cscale = (*num)->n_scale;
    }
  else
    {
      /* The number is greater than 1.  Guess should start at 10^(exp/2). */
      bc_uint64_2num (&guess, 10);

      bc_uint64_2num (&guess1,(*num)->n_len);
      bc_multiply (guess1, point5, &guess1, 0);
      guess1->n_scale = 0;
      bc_raise (guess, guess1, &guess, 0);
      bc_free_num (&guess1);
      cscale = 3;
    }

  /* Find the square root using Newton's algorithm. */
  for(;;) {
      bc_free_num (&guess1);
      guess1 = bc_copy_num (guess);
      bc_divide (*num, guess, &guess, cscale);
      bc_add (guess, guess1, &guess, 0);
      bc_multiply (guess, point5, &guess, cscale);
      bc_sub (guess, guess1, &diff, cscale+1);
      if (bc_is_near_zero (diff, cscale))
	{
	  if (cscale < rscale+1)
	    cscale = MIN (cscale*3, rscale+1);
	  else
	    break;
	}
    }

  /* Assign the number and clean up. */
  bc_free_num (num);
  bc_divide (guess,_one_,num,rscale);
  bc_free_num (&guess);
  bc_free_num (&guess1);
  bc_free_num (&point5);
  bc_free_num (&diff);
  return 1;
}

/* Convert a number NUM to a long.  The function returns only the integer
   part of the number.  For numbers that are too large to represent as
   a long, this function returns a zero.  This can be detected by checking
   the NUM for zero after having a zero returned. */

long
bc_num2long (bc_num num)
{
  long val;
  char *nptr;

  /* Extract the int value, ignore the fraction. */
  val = 0;
  nptr = num->n_value;
  uint16_t index = num->n_len;
  for (; index > 0 && val <= LONG_MAX / BASE; index--)
    val = val*BASE + *nptr++;

  /* Check for overflow.  If overflow, return zero. */
  if (index>0) val = 0;
  if (val < 0) val = 0;

  /* Return the value. */
  if (num->n_sign == PLUS)
    return val;

 return -val;
}


/* Convert an integer VAL to a bc number NUM. */
void
bc_type2num(bc_num *num, uint64_t val, bool negative)
{
  char buffer[30];
  char *bptr, *vptr;
  uint16_t ix = 1;

  /* Get things going. */
  bptr = buffer;
  *bptr++ = val % BASE;
  val /= BASE;

  /* Extract remaining digits. */
  while (val != 0)
    {
      *bptr++ = val % BASE;
      val /= BASE;
      ix++; 		/* Count the digits. */
    }

  /* Make the number. */
  bc_free_num (num);
  *num = bc_new_num (ix, 0);

  if (negative) (*num)->n_sign = MINUS;

  /* Assign the digits. */
  vptr = (*num)->n_value;
  while (ix-- > 0)
    *vptr++ = *--bptr;
}

void bc_int32_2num(bc_num *num, int32_t val) {
  bool neg = val < 0;

  int64_t temp = val;

  if (neg)
    temp = -temp;

  bc_type2num(num, temp, neg);
}

void bc_uint32_2num(bc_num *num, uint32_t val) {
  bc_int64_2num(num, val);
}

void bc_int64_2num(bc_num *num, int64_t val) {
  bool neg = val < 0;

  if (neg)
    val = -val; // this will fail when val = min_int64
    
  bc_type2num(num, val, neg);
}

void bc_uint64_2num(bc_num *num, uint64_t val) {
  bc_type2num(num, val, false);
}


/* Convert a numbers to a string.  Base 10 only.*/

char *num2str (bc_num num)
{
  char *str, *sptr;
  char *nptr;
  uint16_t index;

  /* Allocate the string memory. */
  uint8_t signch = num->n_sign == PLUS ? 0 : 1;  /* Number of sign chars. */
  if (num->n_scale > 0)
    str = (char *) malloc (num->n_len + num->n_scale + 2 + signch);
  else
    str = (char *) malloc (num->n_len + 1 + signch);
  if (str == NULL) bc_out_of_memory();

  /* The negative sign if needed. */
  sptr = str;
  if (signch) *sptr++ = '-';

  /* Load the whole number. */
  nptr = num->n_value;
  for (index=num->n_len; index>0; index--)
    *sptr++ = BCD_CHAR(*nptr++);

  /* Now the fraction. */
  if (num->n_scale > 0)
    {
      *sptr++ = '.';
      for (index=0; index<num->n_scale; index++)
	*sptr++ = BCD_CHAR(*nptr++);
    }

  /* Terminate the string and return it! */
  *sptr = '\0';
  return str;
}
/* Convert strings to bc numbers.  Base 10 only.*/

void
bc_str2num (bc_num *num, const char *const str, uint16_t scale)
{
  uint16_t digits = 0, strscale = 0;
  const char *ptr = str;
  char *nptr;
  bool zero_int = false;

  /* Prepare num. */
  bc_free_num (num);

  /* Check for valid number and count digits. */
  if ( (*ptr == '+') || (*ptr == '-'))  ptr++;  /* Sign */
  while (*ptr == '0') ptr++;			/* Skip leading zeros. */
  while (isdigit((int)*ptr)) ptr++, digits++;	/* digits */
  if (*ptr == '.') ptr++;			/* decimal point */
  while (isdigit((int)*ptr)) ptr++, strscale++;	/* digits */
  if ((*ptr != '\0') || (digits+strscale == 0))
    {
      *num = bc_copy_num (_zero_);
      return;
    }

  /* Adjust numbers and allocate storage and initialize fields. */
  strscale = MIN(strscale, scale);
  if (digits == 0)
    {
      zero_int = true;
      digits = 1;
    }
  *num = bc_new_num (digits, strscale);

  /* Build the whole number. */
  ptr = str;
  if (*ptr == '-')
    {
      (*num) -> n_sign = MINUS;
      ptr++;
    }
  else
    {
      (*num) -> n_sign = PLUS;
      if (*ptr == '+') ptr++;
    }
  while (*ptr == '0') ptr++;			/* Skip leading zeros. */
  nptr = (*num) -> n_value;
  if (zero_int)
    {
      *nptr++ = 0;
      digits = 0;
    }
  for (;digits > 0; digits--)
    *nptr++ = CH_VAL(*ptr++);


  /* Build the fractional part. */
  if (strscale > 0)
    {
      ptr++;  /* skip the decimal point! */
      for (;strscale > 0; strscale--)
	*nptr++ = CH_VAL(*ptr++);
    }
}

// this method sucks
void bc_double2num(bc_num *num, double val) {
  char buffer[30];

#ifndef NOT_ARDUINO
  dtostrf(val, sizeof buffer - 1, sizeof buffer - 3, buffer);
#else
  snprintf(buffer, sizeof buffer, "%.*f", sizeof buffer - 3, val);
#endif

  bc_str2num(num, buffer, strlen(buffer));
}

/* Added by NJG to remove a memory leak */

void
bc_free_numbers (void)
{
  bc_free_num (&_zero_);
  bc_free_num (&_one_);
  bc_free_num (&_two_);
}

// error handler - replace this for different error handling
void bc_error (const int mesg)
{
  // warnings are negative
  if (mesg >= 0)
    while (1) // some IDEs don't have the exit() function
      { }
} // end of bc_error
