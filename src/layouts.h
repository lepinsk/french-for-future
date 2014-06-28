//
//  config.h
//  French for Future
//
//  Created by Julian Lepinski on 2014-02-12
//  Based on Futura Weather by Niknam (https://github.com/Niknam/futura-weather-sdk2.0)
//

// Layouts

// Layout spec: looks like we're just running crazy-tall heights
// for our frames and not giving a damn... maybe this doesn't matter?
// everything's top-alightned, anyway. I can't see a cost, but it's a
// little inelegant. Anyway, origin = top left.
//
//							   X    Y     W    H
#define DAY_FRAME       (GRect(0,   16,   144, 168-62))
#define TIME_FRAME      (GRect(0,   32,   144, 168-20))
#define DATE_FRAME      (GRect(0,   90,   144, 168-62))
#define TEMP_FRAME      (GRect(10,  135,  134, 168-62))
#define COND_FRAME      (GRect(0,   135,  134, 168-62))