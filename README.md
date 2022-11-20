# mobile-events
Simple system to push notifications from a REST API to mobile phones

The system has 2 parts.

First a server written in in C++ that provides a simple API to the world, where events may be submitted. 

The second part is a trivial Android app that subscribes to events and 
shows them as they are received.

The API to submit events is a trivial HTTP/REST api with the events expressed in json.

The API used by the mobile app to receive events use HTML5/Server Side Events.

When this simple setup works, the idea is to add push notifications, so the server can
notify Androd devices about new events, even when the app is not running or in
the foreground. 

## Motivation:

I have a Rasberry Pi II monitoring some propertties in my home with it's IO ports. I want notifications on my
phone when something unusual happens, like when the power is lost. This project is for the notifications. 
