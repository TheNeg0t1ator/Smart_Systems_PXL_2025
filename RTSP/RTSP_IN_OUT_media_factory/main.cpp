#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

int main(int argc, char *argv[]) {
    GstRTSPServer *server;
    GstRTSPMountPoints *mounts;
    GstRTSPMediaFactory *factory;
    GMainLoop *loop;

    gst_init(&argc, &argv);

    // Create the main event loop
    loop = g_main_loop_new(NULL, FALSE);

    // Create the RTSP server
    server = gst_rtsp_server_new();
    gst_rtsp_server_set_service(server, "8554");  // Set the port to 8554

    // Get the mount points for the server
    mounts = gst_rtsp_server_get_mount_points(server);

    // Create a media factory
    factory = gst_rtsp_media_factory_new();

    // Use nvvideoconvert for video processing (scaling) before encoding
    gst_rtsp_media_factory_set_launch(factory,
        "( rtspsrc location=rtsp://EdgeCam:EdgeCam@192.168.2.11:554/stream2 ! rtph264depay ! h264parse ! nvv4l2decoder ! "
        "nvvideoconvert ! video/x-raw(memory:NVMM), width=640, height=480 ! "
        "nvv4l2h264enc ! rtph264pay name=pay0 pt=96 )");

    // Attach the media factory to the /stream mount point
    gst_rtsp_mount_points_add_factory(mounts, "/stream", factory);

    // Start the server
    g_object_unref(mounts);
    gst_rtsp_server_attach(server, NULL);

    g_print("RTSP server is running at rtsp://192.168.2.155:8554/stream\n");

    // Run the loop
    g_main_loop_run(loop);

    return 0;
}
