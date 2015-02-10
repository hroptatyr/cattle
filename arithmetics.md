---
title: cattle arithmetics
project: cattle
layout: subpage
description: cattle arithmetics in detail
---

CAEV arithmetics
================

In general, the CAEV operation ADD is *NOT* commutative, instead you
must obey chronological order.  Same for SUB, the latest CAEV added
must be subbed first (LIFO), this is the right inverse of ADD, called
`ctl_caev_sub()`;  to get the left inverse of ADD, respectively, the
FIFO-sub, use `ctl_caev_sup()`.

The arithmetic we implement here favours additive operations over
multiplicative.  In our (raw) syntax we will write `x+p<-q` meaning
add `x` to the price, then, if `p` is non-zero, multiply by `p`, then
for non-zero `q` divide by `q`.

Add for additive operations is defined as:

    x1+0<-0 + x2+0<-0 = (x1+x2)+0<-0

and hence the inverse of x+0<-0 is (-x)+0<-0.

Similarly, add for multiplicative operations is defined as:

   0+p1<-q1 + 0+p2<-q2 = 0+(p1p2)<-(q1q2)

which makes `0+q<-p` the inverse of `0+p<-q`, with `0+1<-1` being the
neutral element.

For mixed operations the precedence rule means:

    x1+p1<-q1 + x2+p2<-q2 = (x1+x2q1/p1)+(p1p2)<-(q1q2)

So a dividend before a split becomes

    d+0<-0 + 0+p<-q = d+p<-q

and a split before a dividend is summed as

    0+p<-q + d+0<-0 = (dq/p)+p<-q

This arithmetic satisfies dilution properties just properly,
a dividend of 4 paid before a 2:1 split is the same as a
dividend of 2 paid after a 2:1 split:

    10@20         10@20
    20@10         10@16 + 10*4
    20@8 + 20*2   20@8  + 10*4
    =160 + 40     =160  + 40

And so DVCA/4 + SPLF/2:1 = SPLF/2:1 + DVCA/2 or in raw syntax
`-4+0<-0 + 0+1<-2  = -4+1<-2  vs  0+1<-2 + -2+0<-0 = -4+1<-2`

From this the two inverses of `+` become apparent, since the additive
part is always absolute in value, subtraction of additive operations
from the left is simply an addition.

    x1+p1<-q1 -l x2+p2<-q2 := (p2(x1 - x2)/q2)+(p1/p2)<-(q1/q2)

And since all multiplicative operations are aggregated into the
multiplicative cell of a sum, subtraction of additive operations from
the right is defined as:

    x1+p1<-q1 -r x2+p2<-q2 = (x1 - (q1/q2)x2/(p1/p2)+(p1/p2)<-(q1/q2)

The actual inverse elements can be computed via left or right
subtraction from the neutral operation 0+0<-0:

    0+0<-0  -l  x+p<-q = (-px/q)+q<-p

and

    0+0<-0  -r  x+p<-q = (-px/q)+q<-p

respectively, which we simply call inv or inverse.

Additionally, we provide a reverse operation defined by

    rev(x+p<-q) := -x+q<-p

and along with

    act(x+p<-q, P) := p(P + x)/q

which we call action of P under x+p<-q, or we say x+p<-q acts on P,
we can define the inverse in terms of act and rev as

    inv(x+p<-q) = rev(act(x+p<-q))

where act(x+p<-q) is the operator that maps x to px/q. */
