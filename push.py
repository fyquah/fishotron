#!/usr/bin/env python
import os, json, subprocess, string, requests, tinys3, random, sys
from pymongo import MongoClient

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
    "width": obj["width"],
    "height": obj["length"],
    "location": {
        "lat": 5.476795,
        "lng": 100.247197
    },
    "image": aws_file_name,
    "description" : "This was a easy catch!",
    "name": "The Awesome Fish",
    "species": "Tuna"
}
headers = { "content-type": "application/json" }

# Remove the file
subprocess.call(["rm", sys.argv[1]])

# Connect and transfer data to Meteor
mongo_url = 'mongodb://client-9dfb4dde:e62f03e2-90ae-3bcb-7ca9-6ecbf52519d9@production-db-c2.meteor.io:27017/fishotron_meteor_com'
client = MongoClient(mongo_url)
db = client.fishotron_meteor_com
db.fish.insert_one(payload)