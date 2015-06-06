#!/usr/bin/env python
import os, json, subprocess, string, requests, tinys3, random

dirname = os.path.dirname(os.path.realpath(__file__))
executable = dirname + "/json"
url = "http://localhost:3000/fish"

# j = subprocess.check_output([ executable, "output.jpeg"])
# s = string.split(j, "\n")[1]
# obj = json.loads(s)

obj = { 
    "width": 1,
    "length" : 2.0
}

conn = tinys3.Connection(os.environ["AWS_ACCESS_KEY_ID"], os.environ["AWS_SECRET_KEY"], 
    tls=True, endpoint='s3-eu-west-1.amazonaws.com')
file_name = str(random.random()) + "-pic.jpeg"
f = open("build/output.jpeg", "rb")
conn.upload(file_name, f, "fish-measurer")
aws_file_name = "https://s3-eu-west-1.amazonaws.com/fish-measurer/" + file_name

# Send the http request
payload = {
    "fish": {
        "width": obj["width"],
        "length": obj["length"],
        "lat": 5.476795,
        "lng": 100.247197
    }
}
headers = { "content-type": "application/json" }

print requests.post(url= url, data=json.dumps(payload), headers=headers)