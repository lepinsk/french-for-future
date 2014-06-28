French for Future - Design
=================================
![Mockup](https://raw.github.com/lepinsk/akaoka-sans/master/Mockup.png)

### Font
* Avenir Book, small text is 18; clock numbers are 48

### Layout details (the pebble has a 144 x 168 pixel display)

* the date and time are centered text
* the top of the weekday is 22px from the top of the screen
* top of the time is 46px from the top
* top of the date is 95px from the top
* the dotted line is 126px from the top, 41px from the bottom
* the tops of the weather lines are 25px from the bottom of the screen (143px from the top)
* and the temperature is left-aligned, 10px from the left edge
* the conditions are right-aligned, 10px from the right edge

In practice, we've had to nudge our frames around quite a bit. Check ```layouts.h``` for the specific frame definitions.

### Weather strings

* tornado - 0
* sunny - 32
* clear - 31, 33, 34
* p. cloudy - 29, 30, 44
* cloudy - 26, 27, 28 
* cold - 25
* drizzle - 9
* foggy - 20
* hot - 36
* sleet - 18
* snowy - 13, 16, 41, 42, 43, 7, 14, 15, 46
* rainy - 11, 12, 40, 5, 6, 8, 10, 35
* stormy - 1, 3, 4, 37, 38, 39, 45, 47
* windy - 23, 24
* hail - 17
* dusty - 19
* hazy - 21
* smoky - 22


Yahoo's weather condition IDs are as follows:
```
0 tornado
1 tropical storm
2 hurricane
3 severe thunderstorms
4 thunderstorms
5 mixed rain and snow
6 mixed rain and sleet
7 mixed snow and sleet
8 freezing drizzle
9 drizzle
10  freezing rain
11  showers
12  showers
13  snow flurries
14  light snow showers
15  blowing snow
16  snow
17  hail
18  sleet
19  dust
20  foggy
21  haze
22  smoky
23  blustery
24  windy
25  cold
26  cloudy
27  mostly cloudy (night)
28  mostly cloudy (day)
29  partly cloudy (night)
30  partly cloudy (day)
31  clear (night)
32  sunny
33  fair (night)
34  fair (day)
35  mixed rain and hail
36  hot
37  isolated thunderstorms
38  scattered thunderstorms
39  scattered thunderstorms
40  scattered showers
41  heavy snow
42  scattered snow showers
43  heavy snow
44  partly cloudy
45  thundershowers
46  snow showers
47  isolated thundershowers
```