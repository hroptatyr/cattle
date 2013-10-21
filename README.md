cattle (corporate actions on the command-line)
==============================================

[![Build Status](https://secure.travis-ci.org/hroptatyr/cattle.png?branch=master)](http://travis-ci.org/hroptatyr/cattle)
[![Build Status](https://drone.io/github.com/hroptatyr/cattle/status.png)](https://drone.io/github.com/hroptatyr/cattle/latest)

A simple library that handles corporate action events and applies them
to timeseries of market prices, outstanding securities and nominal
values.

In terms of that library a command-line tool `cattle` is built that
reads [ISO15022 mt564][1] messages, stack them and then apply a
timeseries of corporate action events to a timeseries of prices, nominal
values and outstanding securities.

Cattle is hosted primarily on github:

+ github: <https://github.com/hroptatyr/cattle>
+ issues: <https://github.com/hroptatyr/cattle/issues>
+ releases: <https://github.com/hroptatyr/cattle/releases>

At the moment cattle is in the planning phase, so little can be shown
or explained here.

Pronunciation
-------------
cattle is correctly pronounced SEE-AYE-tool.

  [1]: http://www.iso15022.org/uhb/uhb/finmt564.htm
