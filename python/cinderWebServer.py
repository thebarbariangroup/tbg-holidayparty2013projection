from flask import Flask
import json
import beanstalkc
import requests
import os, random
from os import listdir
from os.path import isfile, join, basename
from os import walk
import sys
import urllib

# connect to the queue server
beanstalk = beanstalkc.Connection(host='localhost', port=11300)

# define our web server
app = Flask(__name__)
booth_files = None
path = None
instagramPath = None
global gif_suffix, jpg_suffix
gif_suffix = 'gif'
jpg_suffix = 'jpg'


# set up a route for photos
@app.route("/getPhoto")
def getPhoto():

	# if the master list of files doesn't exist yet, create it now
	global booth_files
	# Check to see if the file list has already been created
	if booth_files is None:
		# create the json file of image urls
		booth_files = make_master_json()
	else:
		# the json object already exists
		# print booth_files
		print "JSON OBJ EXISTS"

	# try to grab 4 photos from the photobooth, and if not, pad the list
	# with a random photo

	# connect to the beanstalk tube we want
	beanstalk.watch('photo')

	photo_count = 10

	jsonPacket = {}

	print beanstalk.stats_tube('photo')['current-jobs-ready']
	# only add photos to json if there are more than 4 photos in queue
	if (beanstalk.stats_tube('photo')['current-jobs-ready'] > 0):
		jsonPacket['photos'] = []
		for i in range(0, photo_count):
			photo_urls = {}
			# extract next queue item
			photo = beanstalk.reserve(timeout=.5)	
			if photo != None:	
				# check that gif or jpg is at end of string
				gif_suffix = 'gif'
				jpg_suffix = 'jpg'
				if photo.body.endswith( gif_suffix ) or photo.body.endswith( jpg_suffix ):
					# print path
					photo_urls['url'] = path + "/" + os.path.basename(photo.body)
					jsonPacket['photos'].append(photo_urls)

					# add photos to master list
					booth_files['photos'].append(photo_urls)

					# delete photo from queue
				photo.delete()
			else:
				break
	else:
		jsonPacket['photos'] = []
		# Pull x amount of random photos from the master json file
		photoIds = [] # list of ids already put in the array
		tries = 0
		totalPhotoAmt = len(booth_files["photos"])

		if totalPhotoAmt < photo_count:
			amt = totalPhotoAmt
		else:
			amt = photo_count

	
		# print "GET SOME RANDOM STUFF " + str(totalPhotoAmt)
		# while the jsonPacket length < amt, put a random photo into the jsonPacket json
		# let's only try up to 3 times so that it doesn't go on forever
		while len(jsonPacket['photos']) < amt and tries < 30:
			tries += 1
			randPhoto = booth_files['photos'][random.randint(0, totalPhotoAmt-1)]

			# check to make sure the randPhoto isn't already in the photoIds array. We don't want to add a photo twice
			dupe = False
			for url in photoIds:
				if url == randPhoto:
					dupe = True
					break
			if dupe:
				continue

			jsonPacket['photos'].append(randPhoto)
			photoIds.append(randPhoto)
		

	return json.dumps(jsonPacket)


@app.route("/getBoothPhotos")
def getBoothPhotos():
	booth_url = 'http://localhost/booth_test/booth_test.json?' + str( random.randint(0, 10000) );

	# load json from url
	r = requests.get(booth_url)
	booth_json = r.json()

	json_packet = {
		'photos' : []
	}

	print booth_json


	# image data is located in data attr
	for dic in booth_json["photos"]:
		photo_url = {}
		
		try:
			photo_url['url'] = dic["url"]
			print photo_url['url']
			saveImage(photo_url['url'], path);
		except:
			print "PROBLEM SAVING"
	
	
	# sort the files in the folder by most recent
	# os.chdir(payloadPath)
	# files = filter(os.path.isfile, os.listdir(payloadPath))
	# files = [os.path.join(payloadPath, f) for f in files] # add path to each file
	# sortedFiles = sorted(files, key=mtime)
	# sortedFiles.reverse()
	

	# grab the 10 most recent instagram photos
	# for i in range(10):
	# 	photo_urls = {}
	# 	filename = sortedFiles[i]
	# 	if filename.endswith( gif_suffix ) or filename.endswith( jpg_suffix ):
	# 		photo_urls['url'] = filename
	# 		json_packet['photos'].append(photo_urls)


	# return json.dumps(json_packet)
	# return "{}"
	return getPhoto()



@app.route("/getInstagram")
def getInstagram():
	# tag to use in url
	tag = "roofies2013"
	# params for instagram request
	payload = {
		'client_id' : 'def20410b5134f7d9b828668775aee4a'
	}

	instagram_url = 'https://api.instagram.com/v1/tags/%s/media/recent' %  tag

	# # get instagram url
	# r = requests.get(instagram_url, params=payload)
	# insta_json = r.json()

	# json_packet = {
	# 	'photos' : []
	# }

	# # image data is located in data attr
	# for dic in insta_json["data"]:
	# 	photo_url = {}
	# 	photo_url['url'] = dic["images"]["standard_resolution"]["url"]
	# 	json_packet['photos'].append(photo_url)
		
	# return json.dumps(json_packet)

	return getInstagramJson(instagram_url, payload, instagramPath + "_" + tag);


@app.route("/getUserInstagram/<payload>")
def getUserInstagram(payload):

	# user id to use in url
	if payload == 'tbg':
		user = "196514860" 	# @barbariangroup
	elif payload == 'webbys':
		user = "2876630"	# @thewebbyawards
	else:
		user = "196514860"
	
	# params for instagram request
	payload = {
		'client_id' : 'e9870ed9df4a4065a8108e111478bc72',
		'access_token' : '3877602.e9870ed.836e08ff4d104ec7934e0d0656434439'
	}
	
	instagram_url = 'https://api.instagram.com/v1/users/%s/media/recent' %  user

	return getInstagramJson(instagram_url, payload, instagramPath + "_" + user);


def getInstagramJson(instagram_url, payload, payloadPath):

	
	# # params for instagram request
	# payload = {
	# 	'client_id' : 'e9870ed9df4a4065a8108e111478bc72',
	# 	'access_token' : '3877602.e9870ed.836e08ff4d104ec7934e0d0656434439'
	# }
	
	# instagram_url = 'https://api.instagram.com/v1/users/%s/media/recent' %  user

	# get instagram url
	r = requests.get(instagram_url, params=payload)
	# return r
	insta_json = r.json()

	print " START ---------------"
	print instagramPath
	

	json_packet = {
		'photos' : []
	}

	# return json_packet

	# image data is located in data attr
	for dic in insta_json["data"]:
		photo_url = {}
		
		try:
			photo_url['url'] = dic["images"]["standard_resolution"]["url"]
			# json_packet['photos'].append(photo_url)
			saveImage(photo_url['url'], payloadPath);
		except:
			print "PROBLEM SAVING"
	
	# go through and start downloading those photos, they will automatically be added to the watched folder and added to the json file when downloaded
	# download photos (only if they don't already exist)
	

	# search_dir = "/mydir/"
	# sort the files in the folder by most recent
	os.chdir(payloadPath)
	files = filter(os.path.isfile, os.listdir(payloadPath))
	files = [os.path.join(payloadPath, f) for f in files] # add path to each file
	# files.sort(key=lambda x: os.path.st_mtime(x), reverse=True)
	sortedFiles = sorted(files, key=mtime)
	sortedFiles.reverse()
	

	# grab the 10 most recent instagram photos
	for i in range(10):
		photo_urls = {}
		filename = sortedFiles[i]
		if filename.endswith( gif_suffix ) or filename.endswith( jpg_suffix ):
			photo_urls['url'] = filename
			json_packet['photos'].append(photo_urls)


	return json.dumps(json_packet)
	# return getPhoto()
	# return "{}"

def mtime(filename):
	# print os.stat(filename).st_mtime
	return os.stat(filename).st_mtime 

def saveImage(path, dir):
	# print path
	# print instagramPath
	# print os.path.basename(path)
	directory = dir + "/" + os.path.basename(path)
	

	# # check if path exists yet. If not, then create it
	if not os.path.exists(dir):
		os.makedirs(dir)

	if not os.path.exists(directory):
		print directory + " -> DOES NOT EXIST"
		f = open(directory,'wb')
		f.write(urllib.urlopen(path).read())
		f.close()
	else:
		print "ALREADY EXISTS"

	
 

 

def make_master_json():
	
	# print "make master json of already downloaded booth files"
	
	jsonPacket = {}
	jsonPacket['photos'] = []
	for (dirpath, dirnames, filenames) in walk(path):
		# print filenames
		for (filename) in filenames:
			if filename.endswith( gif_suffix ) or filename.endswith( jpg_suffix ):
				
				photo_urls = {}
				photo_urls['url'] = dirpath + "/" + os.path.basename(filename)
				print photo_urls
				jsonPacket['photos'].append(photo_urls)
		# f.extend(filenames)
		# break
	
	# booth_files = json.dumps(jsonPacket)
	# return json.dumps(jsonPacket)
	return jsonPacket
	

def run_server():
	global path
	path = sys.argv[1] if len(sys.argv) > 1 else '.'
	global instagramPath
	instagramPath = path + '/instagram'
	# print instagramPath
	app.run(debug=True, use_reloader=True)

if __name__ == "__main__":
	#app.run( debug=True )
	run_server()