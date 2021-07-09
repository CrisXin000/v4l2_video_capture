[TOC]

# Video Capture(V4L2)

read video stream for usb camera and csi camera with v4l2

## Usage

```
mkdir build && cd build
cmake .. && make
./video_capture video_name1 video_name2 --format format1 format2 
```

**format number must match video number, the supported pixel format includes MJPEG, YUYV422, RG10(10 bit Bayer)**