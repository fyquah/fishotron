#!/usr/bin/env python
import os, json, subprocess, string, requests, tinys3, random, sys

url = "http://localhost:3000/fish"

obj = { 
    "length" : float(sys.argv[2]),
    "width": float(sys.argv[3])
}

print "length is " + str(float(sys.argv[2]))
print "width is " + str(float(sys.argv[3]))

conn = tinys3.Connection(os.environ["AWS_ACCESS_KEY_ID"], os.environ["AWS_SECRET_KEY"], 
    tls=True, endpoint='s3-eu-west-1.amazonaws.com')
file_name = str(random.random()) + "-pic.jpeg"
f = open(sys.argv[1], "rb")
conn.upload(file_name, f, "fish-measurer")
aws_file_name = "https://s3-eu-west-1.amazonaws.com/fish-measurer/" + file_name

# Send the http request
payload = {
    "fish": {
        "width": obj["width"],
        "length": obj["length"],
        "lat": 5.476795,
        "lng": 100.247197,
        "aws_image_url": aws_file_name
    }
}
headers = { "content-type": "application/json" }

print requests.post(url= url, data=json.dumps(payload), headers=headers)

# Remove the file
subprocess.call(["rm", sys.argv[1]])