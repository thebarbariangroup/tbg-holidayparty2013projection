import sys
import time
import beanstalkc
import json
from watchdog.observers import Observer
from watchdog.events import LoggingEventHandler, FileSystemEventHandler

# connect to the queue server
beanstalk = beanstalkc.Connection(host='localhost', port=11300)
# and use our 'photo' queue for photos
beanstalk.use('photo')

# queue new images that have been created to be displayed
class NewImageHandler(FileSystemEventHandler):
    """
    Monitor folder for new image files, and queue
    them for display on the screen
    """

    def on_created(self, event):
        "If any file or folder is changed"
        print('on created')
        if not event.is_directory:
            # add this photo to the beanstalk
            beanstalk.put(event.src_path);
            print "New Photo! %s" % event.src_path

# define the main function that'll 
if __name__ == "__main__":
    path = sys.argv[1] if len(sys.argv) > 1 else '.'
    event_handler = NewImageHandler()
    observer = Observer()
    observer.schedule(event_handler, path, recursive=True)
    observer.start()
    try:
        while True:
            print('sleep')
            time.sleep(1)
    except KeyboardInterrupt:
        observer.stop()
    observer.join()