import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstRtspServer', '1.0')
from gi.repository import Gst, GstRtspServer, GObject

# Initialize GStreamer
Gst.init(None)

class RTSPRelayServer(GstRtspServer.RTSPMediaFactory):
    def __init__(self, input_uri):
        super(RTSPRelayServer, self).__init__()
        self.input_uri = input_uri
        self.set_shared(True)

    def do_create_element(self, url):
        # Create each GStreamer element individually
        source = Gst.ElementFactory.make("rtspsrc", "source")
        depay = Gst.ElementFactory.make("rtph264depay", "depay")
        decoder = Gst.ElementFactory.make("avdec_h264", "decoder")
        videobalance = Gst.ElementFactory.make("videobalance", "videobalance")
        encoder = Gst.ElementFactory.make("x264enc", "encoder")
        pay = Gst.ElementFactory.make("rtph264pay", "pay")

        # Set properties for each element
        source.set_property("location", self.input_uri)
        source.set_property("latency", 200)
        videobalance.set_property("saturation", 0.0)
        encoder.set_property("tune", "zerolatency")
        pay.set_property("pt", 96)

        # Create the pipeline and add all elements
        pipeline = Gst.Pipeline.new("rtsp-pipeline")
        pipeline.add(source)
        pipeline.add(depay)
        pipeline.add(decoder)
        pipeline.add(videobalance)
        pipeline.add(encoder)
        pipeline.add(pay)

        # Link elements together
        source.connect("pad-added", self.on_pad_added, depay)
        depay.link(decoder)
        decoder.link(videobalance)
        videobalance.link(encoder)
        encoder.link(pay)

        return pipeline

    def on_pad_added(self, src, new_pad, depay):
        # Link dynamically added rtspsrc pad to the depayloader
        sink_pad = depay.get_static_pad("sink")
        if not sink_pad.is_linked():
            new_pad.link(sink_pad)

class Server:
    def __init__(self, input_uri, output_port):
        # Create the RTSP server
        self.server = GstRtspServer.RTSPServer()
        self.server.set_service(str(output_port))
        
        # Set up the relay factory
        relay_factory = RTSPRelayServer(input_uri)
        mount_points = self.server.get_mount_points()
        mount_points.add_factory("/relay", relay_factory)

        # Attach the server
        self.server.attach(None)
        print(f"RTSP server is streaming at rtsp://127.0.0.1:{output_port}/relay")

if __name__ == "__main__":
    # Define input RTSP stream URL and desired output port
    input_uri = "rtsp://your_rtsp_source_here"
    output_port = 8554

    # Start main loop
    server = Server(input_uri, output_port)
    loop = GObject.MainLoop()
    loop.run()
