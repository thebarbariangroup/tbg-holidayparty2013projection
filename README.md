tbg-roofies-webbys-projection
=============================
Projection for the Webby Awards co-sponsored roofies party (August 2013)

## Dependencies
* OSX 10.8.4
* Cinder 0.8.5 and xcode
* Python 2.7
* beanstalkd

Be sure to set the CINDER_PATH to your local (built) version of Cinder

## Projection
* Built with cinder
* The app makes calls to the flask app to grab the photos at <code>http://localhost:5000</code>. Possible urls are:
	* <code>http://localhost:5000/getPhoto</code> (Photobooth photos)
	* <code>http://localhost:5000/getUserInstagram/tbg</code> (@barbariangroup instagram photos)
	* <code>http://localhost:5000/getInstagram</code> (#roofies2013 tagged instagram photos)

### To publish the final projection app
* The xcode project is in <code>projection/WebbysProjection/xcode/</code>
* Make sure the project is linked to wherever Cinder 0.8.5 is installed
* Run the app in xcode to get your final .app file

## Server notes:
* Python based
* To watch for photos added to dropbox, it uses beanstalkd
* The server uses flask
* Be sure pip is installed so that you can install a bunch of python dependencies


### Necessary python includes
(pip install _)

* pip (to install other dependencies) [Pip install directions](http://docs.python-guide.org/en/latest/starting/install/osx.html)
* virtual environment (not essential, but best if you want to use the venv environment in the python directory)
* beanstalkc (to add new photos from a directory to a queue) Be sure to [install beanstalkd](http://kr.github.io/beanstalkd/download.html)
* flask (to serve the json to an app)
* watchdog (to watch dir for changes)
* requests (makes making http requests easy)

To install the dependencies cd into the python dir:

    $ pip install -r stable-req.txt

## To run everything:

Terminal 1: (beanstalkd)

<pre>$ cd ~/[project directory]/python
$ . venv/bin/activate
$ beanstalkd</pre>

Terminal 2: (photo booth monitor)
<pre>$ cd ~/[project directory]/python
$ . venv/bin/activate
$ python photoboothMonitor.py ~/Dropbox/booth [swap out for final booth dropbox directory]</pre>

Terminal 3: (cinder web server)
<pre>$ cd ~/[project directory]/python
$ . venv/bin/activate
$ python cinderWebServer.py ~/Dropbox/booth [swap out for final booth dropbox directory]</pre>

Once all the python scripts and servers are running, launch the cinder app (WebbyProjection). That's it. You won't see anything until enough photos load, which may be a few seconds.
