import json
import requests
from os.path import basename

# go to this url
# http://www.thebos.co/json/events/view/august-roofies
# load the json
# go through all of the nodes and find the link for hte raw file
# save the raw file to disk
#

def pullPhotos():
	print "PULL"

	url = 'http://www.thebos.co/json/events/view/august-roofies'

	# get instagram url
	r = requests.get(url)
	# return r
	json = r.json()
	# print json

	# json_packet = {
	# 	'photos' : []
	# }

	# # return json_packet


	# # image data is located in data attr
	for photo in json:
		# the raw filename of the gif
		filepath = json[photo]["raw"]["filename"]
		filename = basename(filepath)

		f = open('../../booth_photos/' + filename,'wb')
		f.write(requests.get(filepath).content)
		f.close()
	# 	photo_url = {}
	# 	photo_url['url'] = dic["images"]["standard_resolution"]["url"]
	# 	json_packet['photos'].append(photo_url)

if __name__ == "__main__":
	pullPhotos()