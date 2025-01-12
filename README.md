> **WARNING**
> The project is not even under development—it is far from being called a "project." Currently, it is more like a "fancy, unreasonably expensive camera with an unhandy UI" rather than the planned product.
> The code is subpar, the idea is flawed; do not judge, as we had deadlines to meet.

# **LiveBoard**

An embedding development educational project

---

LiveBoard is a program supposed to run on a Raspberry Pi with a webcam or any similar setup. It provides a stream of the whiteboard (at least while you point the camera at it), with reduced noise, a precisely captured frame of the board, and unwanted objects removed. Its main feature is the ability to save a single screenshot of your whiteboard as a high-quality SVG image, ideally ready for vector picture editing.

---


## Dev notes

Run stream: `./mjpg_streamer -i "./input_uvc.so -d /dev/video4" -o "./output_http.so -w ./www -p 8080"`

Build and run server: `clang main.c -o main && ./main 8081 /dev/video4 "http://localhost:8080/?action=stream"`

List of available video devices: `v4l2-ctl --list-devices`
List available control settings: `v4l2-ctl -d /dev/video0 --list-ctrls`
List available video formats: `v4l2-ctl -d /dev/video0 --list-formats-ext`
Read the current setting: `v4l2-ctl -d /dev/video0 --get-ctrl=exposure_auto`
Change the setting value: `v4l2-ctl -d /dev/video0 --set-ctrl=exposure_auto=1`
