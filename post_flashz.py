#!/usr/bin/python

#https://docs.platformio.org/en/latest/scripting/actions.html
#https://trass3r.github.io/coding/2018/12/20/advanced-platformio.html
#https://github.com/platformio/bintray-secure-ota/blob/master/publish_firmware.py

from os.path import basename
from os.path import isfile
from os.path import getsize

import subprocess
import requests
import sys,zlib
import re

Import("env", "projenv")

# access to global build environment
#print(env)

# access to project build environment (is used source files in "src" folder)
#print(projenv.Dump())

#
# Dump build environment (for debug purpose)
# print(env.Dump())
#


def pigz_compress(source, target, env):
    firmware_path = str(target[0])      #.replace(".elf", ".bin")
    #firmware_name = basename(firmware_path)
    print("Compressing %s file..." % basename(firmware_path))
    subprocess.run(["pigz", "-fzk11", firmware_path])

#def zlib_compress(source, target, env):
def zlib_compress(source):
    imgfile = source
    print("Compressing %s file..." % basename(imgfile))
    with open(imgfile, 'rb') as img:
            with open(imgfile + '.zz', 'wb') as deflated:
                data = zlib.compress(img.read(), zlib.Z_BEST_COMPRESSION)
                deflated.write(data)
                compress_ratio = (float(getsize(imgfile)) - float(getsize(imgfile + '.zz'))) / float(getsize(imgfile)) * 100
                print("Compress ratio %d%%" % compress_ratio)

def ota_upload(source, target, env):
    file_path = str(source[0])
    print ("Found OTA_url option, will attempt over-the-air upload")

    try:
        compress = env.GetProjectOption('OTA_compress')
        if compress in ("yes", "true", "1"):
            print("Found OTA_compress option")
            zlib_compress(file_path)
            if (isfile(file_path + ".zz")):
                file_path += ".zz"
    except:
        print ("OTA_compress not found, NOT using compression")


    url = env.GetProjectOption('OTA_url')

    # check if we upload a firmware or FS image
    imgtype = None
    if (bool(re.search('firmware', file_path))):
        imgtype = 'fw'
    else:
        imgtype = 'fs'

    payload = {'img' : imgtype }
    f = {'file': open(file_path, 'rb')}
    req = None
    try:
        print("Uploading file %s to %s " % (file_path, url))
        req = requests.post(url, data = payload, files=f)
        req.raise_for_status()
    except requests.exceptions.RequestException as e:
        sys.stderr.write("Failed to upload file: %s\n" %
                         ("%s\n%s" % (req.status_code, req.text) if req else str(e)))
        env.Exit(1)

    print("The firmware has been successfuly uploaded!")

# available targets, i.e. buildprog, size, upload, program, buildfs, uploadfs, uploadfsota
#buildprog is a target when ALL files are compiled and PROGRAM is ready.

# autocompress all bin images (fw and fs)
#env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", pigz_compress)

# Custom upload handler
try:
    env.GetProjectOption('OTA_url')
    env.Replace(UPLOADCMD=ota_upload)
except:
    print ("OTA_url not found, fallback to serial flasher")
