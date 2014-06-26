//
//  pebble-js-app.js
//  French for Future
//
//  Created by Julian Lepinski on 2014-02-12
//  Based on Futura Weather by Niknam (https://github.com/Niknam/futura-weather-sdk2.0)
//

var temperatureOverride = true;
var temperatureInC = true;
var useYahooWeather = true;

Pebble.addEventListener("ready", function(e) {
    console.log("Starting ...");
    updateWeather();
});

Pebble.addEventListener("appmessage", function(e) {
    console.log("Got a message - Starting weather request...");
    updateWeather();
});

var updateInProgress = false;

function updateWeather() {
    if (!updateInProgress) {
        updateInProgress = true;
        var locationOptions = { "timeout": 15000, "maximumAge": 60000 };
        navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
    }
    else {
        console.log("Not starting a new request. Another one is in progress...");
    }
}

function locationSuccess(pos) {
    var coordinates = pos.coords;
    console.log("Got coordinates: " + JSON.stringify(coordinates));
    if (useYahooWeather){
        console.log("Fetching weather data from Yahoo");
        fetchWeatherYahoo(coordinates.latitude, coordinates.longitude);
    } else {
        console.log("Fetching weather data from OpenWeatherMap");
        fetchWeatherOWM(coordinates.latitude, coordinates.longitude);
    }
}

function locationError(err) {
    console.warn('Location error (' + err.code + '): ' + err.message);
    Pebble.sendAppMessage({ "error": "Loc unavailable" });
    updateInProgress = false;
}

function getJSON(url, callback){
    var req = new XMLHttpRequest();

    req.open('GET', url, true);
    req.onload = function(e) {
        if (req.readyState == 4) {
            if(req.status == 200) {
                if (callback) callback(JSON.parse(req.responseText));
            } else {
                console.log("getJSON error; req.status: " + req.status);
            }
        } else {
            console.log("getJSON error; req.readyState: " + req.readyState);
        }
    };

    req.send(null);
}

function callYQL(query, callback){
    var encodedQuery = 'http://query.yahooapis.com/v1/public/yql?q=' + encodeURIComponent(query) + '&format=json';

    console.log("callYQL calling getJSON with url " + encodedQuery);
    getJSON(encodedQuery, callback);
}

function fetchWeatherYahoo(latitude, longitude){
    callYQL('select * from geo.placefinder where text="' + latitude + ',' + longitude + '" and gflags="R"', function(json){
        var woeid = json.query.results.Result.woeid;
        console.log("callYQL called back; woeid: " + woeid);
        callYQL('select * from weather.forecast where woeid=' + woeid + ' and u="' + (temperatureInC ? "c" : "f") + '"' , function(json){
            var condition = parseInt(json.query.results.channel.item.condition.code);
            var temperature = parseInt(json.query.results.channel.item.condition.temp);
            var current_time = current_time = Date.now() / 1000;

            console.log("Temperature: " + temperature + " Condition: " + condition + " Now: " + Date.now() / 1000);
                              
            Pebble.sendAppMessage({
                "condition": condition,
                "temperature": temperature,
                "current_time": current_time
            });
            updateInProgress = false;
        });
    });
}

function fetchWeatherOWM(latitude, longitude) {
    var response;
    var req = new XMLHttpRequest();
    req.open('GET', "http://api.openweathermap.org/data/2.5/weather?" +
        "lat=" + latitude + "&lon=" + longitude + "&cnt=1", true);
    req.onload = function(e) {
        if (req.readyState == 4) {
            if(req.status == 200) {
                console.log(req.responseText);
                response = JSON.parse(req.responseText);
                var temperature, icon, city, sunrise, sunset, condition;
                var current_time = Date.now() / 1000;
                if (response) {
                    var tempResult = response.main.temp;
                    if (!temperatureOverride){
                        if (response.sys.country === "US") {
                            // Convert temperature to Fahrenheit if user is within the US
                            temperature = Math.round(((tempResult - 273.15) * 1.8) + 32);
                        } else {
                            // Otherwise, convert temperature to Celsius
                            temperature = Math.round(tempResult - 273.15);
                        }
                    } else {
                        if (!temperatureInC) {
                            temperature = Math.round(((tempResult - 273.15) * 1.8) + 32);
                        } else {
                            temperature = Math.round(tempResult - 273.15);
                        }
                    }		 
                    condition = response.weather[0].id;
                    sunrise = response.sys.sunrise;
                    sunset = response.sys.sunset;

                    console.log("Temperature: " + temperature + " Condition: " + condition + " Sunrise: " + sunrise +
                              " Sunset: " + sunset + " Now: " + Date.now() / 1000);
                              
                    Pebble.sendAppMessage({
                        "condition": condition,
                        "temperature": temperature,
                        "sunrise": sunrise,
                        "sunset": sunset,
                        "current_time": current_time
                    });
                    updateInProgress = false;
                }
            } else {
                console.log("Error");
                updateInProgress = false;
                Pebble.sendAppMessage({ "error": "HTTP Error" });
            }
        }
    };
    req.send(null);
}

Pebble.addEventListener("showConfiguration", function(e) {
    console.log("Showing configuration");
    Pebble.openURL('http://debaclesoftware.com/fff/index.html');
});

Pebble.addEventListener("webviewclosed", function(e) {
    console.log("Closing webview");
    var JSP = JSON.parse(e.response);
    var cs = JSP["colourScheme"];

    console.log(cs);
    Pebble.sendAppMessage({
    	"colourscheme" : cs
    });
});