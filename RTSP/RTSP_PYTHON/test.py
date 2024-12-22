import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstRtspServer', '1.0')
from gi.repository import Gst, GstRtspServer, GObject

def main():
    # Initialize GStreamer
    Gst.init(None)

    # Create the main event loop
    loop = GObject.MainLoop()

    # Create the RTSP server
    server = GstRtspServer.RTSPServer()
    server.set_service("8554")  # Set the port to 8554

    # Get the mount points for the server
    mounts = server.get_mount_points()

    # Create a media factory
    factory = GstRtspServer.RTSPMediaFactory()

    # Set the launch string for the media factory
    factory.set_launch(
        "( rtspsrc location=rtsp://EdgeCam:EdgeCam@192.168.2.11:554/stream2 ! "
        "rtph264depay ! h264parse ! nvv4l2decoder ! nvvideoconvert ! "
        "video/x-raw(memory:NVMM), width=640, height=480 ! "
        "nvv4l2h264enc ! rtph264pay name=pay0 pt=96 )"
    )

    # Attach the media factory to the /stream mount point
    mounts.add_factory("/stream", factory)

    # Start the server
    server.attach(None)
    print("RTSP server is running at rtsp://192.168.2.155:8554/stream")

    # Run the loop
    loop.run()

if __name__ == '__main__':
    main()