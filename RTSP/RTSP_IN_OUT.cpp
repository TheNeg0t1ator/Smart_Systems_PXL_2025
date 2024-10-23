#include <gst/gst.h>
#include <glib.h>
#include <stdio.h>
#include <cuda_runtime_api.h>
#include "gstnvdsmeta.h"

int main(int argc, char *argv[]) {
    GstElement *pipeline, *source, *depay, *sink;
    GstBus *bus;
    GstMessage *msg;
    GstStateChangeReturn ret;

    // Initialize GStreamer
    gst_init(&argc, &argv);

    // Create the elements
    source = gst_element_factory_make("rtspsrc", "source");
    if (!source) {
        g_printerr("Failed to create 'rtspsrc' element.\n");
        return -1;
    }

    depay = gst_element_factory_make("rtph264depay", "depay");
    if (!depay) {
        g_printerr("Failed to create 'rtph264depay' element.\n");
        return -1;
    }

    sink = gst_element_factory_make("autovideosink", "sink");
    if (!sink) {
        g_printerr("Failed to create 'autovideosink' element.\n");
        return -1;
    }

    // Create the empty pipeline
    pipeline = gst_pipeline_new("test-pipeline");
    if (!pipeline) {
        g_printerr("Failed to create 'pipeline' element.\n");
        return -1;
    }

    // Set the static RTSP URI
    g_object_set(G_OBJECT(source), "location", "rtsp://192.168.0.133/live", NULL);

    // Build the pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, depay, sink, NULL);
    if (!gst_element_link(depay, sink)) {
        g_printerr("Failed to link 'depay' and 'sink' elements.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    // Connect to the pad-added signal for the source element
    g_signal_connect(source, "pad-added", G_CALLBACK(+[](GstElement *src, GstPad *new_pad, GstElement *depay) {
        GstPad *sink_pad = gst_element_get_static_pad(depay, "sink");
        GstPadLinkReturn ret;
        GstCaps *new_pad_caps = NULL;
        GstStructure *new_pad_struct = NULL;
        const gchar *new_pad_type = NULL;

        g_print("Received new pad '%s' from '%s':\n", GST_PAD_NAME(new_pad), GST_ELEMENT_NAME(src));

        // Check the new pad's type
        new_pad_caps = gst_pad_get_current_caps(new_pad);
        new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
        new_pad_type = gst_structure_get_name(new_pad_struct);

        if (!g_str_has_prefix(new_pad_type, "application/x-rtp")) {
            g_printerr("It has type '%s' which is not 'application/x-rtp'. Ignoring.\n", new_pad_type);
            gst_caps_unref(new_pad_caps);
            gst_object_unref(sink_pad);
            return;
        }

        // Attempt to link the pads
        ret = gst_pad_link(new_pad, sink_pad);
        if (GST_PAD_LINK_FAILED(ret)) {
            g_printerr("Type is '%s' but link failed.\n", gst_caps_to_string(new_pad_caps));
        } else {
            g_print("Link succeeded (type '%s').\n", gst_caps_to_string(new_pad_caps));
        }

        // Unreference the caps and pads
        gst_caps_unref(new_pad_caps);
        gst_object_unref(sink_pad);
    }), depay);

    // Start playing the pipeline
    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    // Wait until error or EOS (End of Stream)
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    // Parse message
    if (msg != NULL) {
        GError *err;
        gchar *debug_info;

        switch (GST_MESSAGE_TYPE(msg)) {
            case GST_MESSAGE_ERROR:
                gst_message_parse_error(msg, &err, &debug_info);
                g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
                g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
                g_clear_error(&err);
                g_free(debug_info);
                break;
            case GST_MESSAGE_EOS:
                g_print("End-Of-Stream reached.\n");
                break;
            default:
                // We should not reach here because we only asked for ERRORs and EOS
                g_printerr("Unexpected message received.\n");
                break;
        }
        gst_message_unref(msg);
    }

    // Free resources
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}