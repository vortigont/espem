import subprocess

# PIO build flags
# https://docs.platformio.org/en/latest/projectconf/section_env_build.html#src-build-flags
revision = (
    subprocess.check_output(["git", "describe", "--abbrev=4", "--always", "--tags", "--long"])
    .strip()
    .decode("utf-8")
)
print("-DGIT_REV='\"%s\"'" % revision)