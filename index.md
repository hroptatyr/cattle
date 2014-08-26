---
title: cattle
project: cattle
layout: project
latest: https://github.com/hroptatyr/cattle/releases/download/v0.2.1/cattle-0.2.1.tar.xz
---

cattle (corporate actions on the command-line)
==============================================

A simple library that handles corporate action events and applies them
to time series of market prices, outstanding securities and nominal
values.

In terms of that library a command-line tool `cattle` is built that
reads [ISO15022 mt564][1] messages, stack them and then apply a
time series of corporate action events to a time series of prices, nominal
values and outstanding securities.

Cattle is hosted primarily on github:

+ github: <https://github.com/hroptatyr/cattle>
+ issues: <https://github.com/hroptatyr/cattle/issues>
+ releases: <https://github.com/hroptatyr/cattle/releases>


Pronunciation
-------------
cattle is correctly pronounced SEE-AYE-tool.

Red tape
--------
+ no external dependencies
+ licensed under BSD3c
+ contains GPLv2 code by Michael Abbott


Examples
========
Suppose we're interested in a backadjusted time series of ticker `XYZ`.
Observed market close prices (in GBP) are:

    2013-10-01	10.00
    2013-10-02	11.00
    2013-10-03	12.00
    2013-10-04	11.00
    2013-10-07	10.00
    2013-10-08	10.00
    2013-10-09	10.50

We know there was a dividend of GBP 2.00 ex'd on 04 Oct 2013, and
another dividend payment ex'd on 07 Oct 2013, also GBP 2.00.  The actual
mt564-messages are transformed into a JSON-like format; the ex-date is
the key column:

    2013-10-04	caev=DVCA .nett/GBP="2.00" .xdte="2013-10-04" .payd="2013-10-11"
    2013-10-07	caev=DVCA .nett/GBP="2.00" .payd="2013-10-21"

The observed market close prices already account for the corporate
action events so daily returns are perfectly explicable when both prices
as well as corporate action events are presented side by side.

However, many (if not most) time series analysis tools (and humans
alike) will be fooled into thinking that from 04 Oct the prices are
falling or stalling; to present the real picture (and hence allow
sensible analyses) we have to somehow incorporate the CA events into the
close price time series, i.e. as we say *adjust* the prices *for
corporate action events*.

Suppose the time series is stored, in the syntax above, in a file
`XYZ.tser`, the corporate action events in `XYZ.echs`:

    $ cattle apply XYZ.tser XYZ.echs
    2013-10-01	6.00
    2013-10-02	7.00
    2013-10-03	8.00
    2013-10-04	9.00
    2013-10-07	10.00
    2013-10-08	10.00
    2013-10-09	10.50
    $

will produce a *back-adjusted* price time series that pretends the
dividend payouts all happened at some time before the series begins.

As can be seen, at no point in time the prices are actually falling and
the daily price *differences* now accurately reflect what actually
happened to the ticker `XYZ`.

Back-adjustment leaves prices in the present unchanged while prices in
the past undergo the adjustment process.  This is helpful because
present prices can just be appended to the back-adjusted series provided
no corporate action has taken place.  A new adjustment run is only
necessary when a new corporate action event has been observed.

However, back-adjustment will interfere with records about position
openings as now the opening price does not correspond to the adjusted
price.  In such cases it can be helpful to *forward-adjust* a time
series as demonstrated by:

    $ cattle apply --forward XYZ.tser XYZ.echs
    2013-10-01	10.00
    2013-10-02	11.00
    2013-10-03	12.00
    2013-10-04	13.00
    2013-10-07	14.00
    2013-10-08	14.00
    2013-10-09	14.50
    $

This time series pretends the dividend payouts will happen at some point
in the future and that the ticker `XYZ` is still quoted cum dividend.

Had you opened a position on 01 Oct, and closed it on 08 Oct, you would
have made GBP 4.00, the real market price has gone from GBP 10.00 to
GBP 10.00 but there has been 2 dividend payments a GBP 2.00, so
totalling GBP 4.00.

Forward-adjustment leaves prices in the past (before the first corporate
action event) untouched but changes all prices afterwards.  That means a
new price can only be added to the series in adjusted form, i.e.
regardless whether or not a corporate action event has been observed the
adjustment tool must be run every day.


Reversing the adjustment
------------------------
Sometimes it is desirable to reproduce the original (raw) time series
from an adjusted series and a series of corporate action events.

The cattle tool provides the `--reverse` switch for this.  For the first
example (stored in a file `XYZ.adj`) the command line:

    $ cattle apply --reverse XYZ.adj XYZ.echs
    2013-10-01	10.00
    2013-10-02	11.00
    2013-10-03	12.00
    2013-10-04	11.00
    2013-10-07	10.00
    2013-10-08	10.00
    2013-10-09	10.50
    $

will reproduce exactly the original time series (`XYZ.tser`) that we
used to produce the adjusted series.

Similarly for the forward-adjusted time series (stored in `XYZ.fadj`):

    $ cattle apply --reverse --forward XYZ.fadj XYZ.echs
    2013-10-01	10.00
    2013-10-02	11.00
    2013-10-03	12.00
    2013-10-04	11.00
    2013-10-07	10.00
    2013-10-08	10.00
    2013-10-09	10.50
    $

reproduces the original time series, too.  The switch `--forward` in the
second example is necessary as information is lost about whether an
adjustment has been applied forwards or backwards.

Total returns
-------------
So far we considered corporate actions (especially those with payment
events) as if they could be moved in time so long as prices are adjusted
accordingly.  However, this inaccurately reflects situations where it's
not a concrete investment that needs to be tracked but rather a
benchmark or something abstract (like what would have happened to an
initial EUR 100 investment).

Under the assumption that we can always opt out of payouts (like
dividends, interest payments, etc.) cattle offers an adjustment mode
that keeps *true returns* unchanged.  Given above time series and
corporate action event file (`XYZ.tser`, `XYZ.echs`) the command line:

    $ cattle apply --total-return XYZ.tser XYZ.echs
    2013-10-01	6.82
    2013-10-02	7.50
    2013-10-03	8.18
    2013-10-04	9.00
    2013-10-07	10.00
    2013-10-08	10.00
    2013-10-09	10.50
    $

As in the total payout case, future values could simply be appended to
the series when no corporate action had taken place in the covered
period.  Unlike before, dividends account partially for past prices from
the moment the dividend has gone ex, i.e. the adjusted series does not
pretend the dividends have been paid out at some point in the past or
future but have been part of the observed prices all along.

Naturally and for the same reasons as given above, total return
adjustments can also be made in forward direction (`-F` is short for
`--forward`):

    $ cattle apply --total-return -F XYZ.tser XYZ.echs
    2013-10-01	10.00
    2013-10-02	11.00
    2013-10-03	12.00
    2013-10-04	13.20
    2013-10-07	14.67
    2013-10-08	14.67
    2013-10-09	15.40
    $

With total return adjustments it's very easy to answer questions like:
How much did my investment earn in percent since opening the position?

Moreover, total return adjustments can be reversed just like total
payout adjustments (using the `--reverse` switch).  However, because
adjusted prices are always quantised to market prices, the reversal
might suffer from *quantisation artefacts*.


Similar projects
================

None.

  [1]: http://www.iso15022.org/uhb/uhb/finmt564.htm


<!--
  Local variables:
  mode: auto-fill
  fill-column: 72
  filladapt-mode: t
  End:
-->
