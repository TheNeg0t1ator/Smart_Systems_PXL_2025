import subprocess

def main():
    filepath = "C:/Users/kobed/Videos/Max/miami.mp4"
    source = f'filesrc location={filepath}'
    #sink = 'autovideosink'
    sink = 'openh264enc bitrate=5000 ! h264parse ! rtph264pay config-interval=10 pt=96 ! udpsink host=127.0.0.1 port=5000'
    pipe = 'decodebin ! videoconvert !'
    debug = ''
    pipeline = f'gst-launch-1.0 {debug} {source} ! {pipe} {sink}'
    print(pipeline)
    process = subprocess.Popen(pipeline, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)


    stdout, stderr = process.communicate()
    
    print("Output:", stdout.decode())
    print("Errors:", stderr.decode())
    
    return 0
    
    
if __name__ == '__main__':
    main()