import sys
import os
import json

def write_json( data ):
	with open('gifs.json', 'w') as f:
		f.write(json.dumps(data))

def loop( path_name ):
	json_packet = {'photos' : []}

	file_list = os.listdir(path)
	for f in file_list:
		photo_url = {}
		if f.endswith('gif'):
			photo_url['url'] = os.path.join(path_name, f)
			json_packet['photos'].append(photo_url)
	write_json(json_packet)

"""
Generates json of all the gifs in the folder provided
"""
if __name__ == "__main__":
	path = sys.argv[1]
	loop( path )
