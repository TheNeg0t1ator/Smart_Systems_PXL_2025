#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

int main(int argc, char *argv[]) {
    GstRTSPServer *server;
    GstRTSPMountPoints *mounts;
    GstRTSPMediaFactory *factory;
    GMainLoop *loop;
    GstElement *bin, *rtspsrc, *rtph264depay, *h264parse, *nvv4l2decoder, *nvvideoconvert, *capsfilter, *nvv4l2h264enc, *rtph264pay;

    gst_init(&argc, &argv);

    // Create the main event loop
    loop = g_main_loop_new(NULL, FALSE);

    // Create the RTSP server
    server = gst_rtsp_server_new();
    gst_rtsp_server_set_service(server, "8554");

    // Get the mount points for the server
    mounts = gst_rtsp_server_get_mount_points(server);

    // Create a media factory
    factory = gst_rtsp_media_factory_new();

    // Create a bin for the pipeline elements
    bin = gst_bin_new("rtsp-bin");

    // Create elements and set properties
    rtspsrc = gst_element_factory_make("rtspsrc", "source");
    g_object_set(rtspsrc, "location", "rtsp://EdgeCam:EdgeCam@192.168.2.11:554/stream1", NULL);

    rtph264depay = gst_element_factory_make("rtph264depay", "depay");
    h264parse = gst_element_factory_make("h264parse", "parser");
    nvv4l2decoder = gst_element_factory_make("nvv4l2decoder", "decoder");
    nvvideoconvert = gst_element_factory_make("nvvideoconvert", "converter");
    
    capsfilter = gst_element_factory_make("capsfilter", "capsfilter");
    g_object_set(capsfilter, "caps", gst_caps_from_string("video/x-raw(memory:NVMM), width=256, height=256"), NULL);

    nvv4l2h264enc = gst_element_factory_make("nvv4l2h264enc", "encoder");
    rtph264pay = gst_element_factory_make("rtph264pay", "pay");
    g_object_set(rtph264pay, "pt", 96, NULL);

    // Add elements to the bin
    gst_bin_add_many(GST_BIN(bin), rtspsrc, rtph264depay, h264parse, nvv4l2decoder, nvvideoconvert, capsfilter, nvv4l2h264enc, rtph264pay, NULL);

    // Link elements in the bin
    gst_element_link_many(rtspsrc, rtph264depay, h264parse, nvv4l2decoder, nvvideoconvert, capsfilter, nvv4l2h264enc, rtph264pay, NULL);

    // Set the bin as the factory's element
    gst_rtsp_media_factory_set_element(factory, bin);

    // Attach the media factory to the /stream mount point
    gst_rtsp_mount_points_add_factory(mounts, "/stream", factory);

    // Start the server
    g_object_unref(mounts);
    gst_rtsp_server_attach(server, NULL);

    g_print("RTSP server is running at rtsp://192.168.2.52:8554/stream\n");

    // Run the loop
    g_main_loop_run(loop);

    // Cleanup
    g_object_unref(server);
    g_main_loop_unref(loop);

    return 0;
}
