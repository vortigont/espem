import os
import subprocess

# print("OS: %s" % os.name)
if os.name == 'nt':
  print("-DGIT_REV='\"n/a\"'")
  quit()


# PIO build flags
# https://docs.platformio.org/en/latest/projectconf/section_env_build.html#src-build-flags
try:
        #revision = subprocess.check_output(["git", "describe", "--abbrev=4", "--always", "--tags", "--long"], stderr=subprocess.DEVNULL).strip().decode("utf-8")
        revision = subprocess.check_output(["git", "describe", "--abbrev=7", "--always"], stderr=subprocess.DEVNULL).strip().decode("utf-8")
        print ("-DGIT_REV='\"%s\"'" % revision)
except:
        print ("-DGIT_REV='\"n/a\"'")
