#!/usr/bin/env python3

# [Apollo]GUI> git rev-parse --abbrev-ref HEAD
# master
# [Apollo]GUI> git describe --long --tags --abbrev=40 --match='v[0-9]*' --dirty
# v1.0.0-0-g5afbb1b9d5b4793fc4c6506e9f9bbe59fe6ea2a0-dirty
# [Apollo]GUI> git remote -v
# origin	git@github-tj.com:tjienta/TTauri.git (fetch)
# origin	git@github-tj.com:tjienta/TTauri.git (push)

import os
import subprocess

def run_with_return(*args):
    result = subprocess.run(args, encoding="UTF-8", check=True, stdout=subprocess.PIPE)
    return result.stdout

def git_get_branch(g):
    output = run_with_return("git", "rev-parse", "--abbrev-ref", "HEAD")
    g.branch = output.strip()

def git_get_describe(g):
    output = run_with_return("git", "describe", "--long", "--tags", "--abbrev=40", "--match=v[0-9]*", "--dirty")
    parts = output.strip().split("-")
    version_parts = parts[0].split(".")

    g.major_version = int(version_parts[0])
    if len(version_parts) >= 2:
        g.minor_version = int(version_parts[1])
    if len(version_parts) >= 3:
        g.patch_version = int(version_parts[2])
    g.nr_commits = int(parts[1])
    g.commit_sha = parts[2][1:] # Strip of the leading 'g'
    g.is_dirty = len(parts) >= 4 and parts[3] == "dirty"


class GitInformation:
    def __init__(self):
        self.branch = ""
        self.major_version = 0
        self.minor_version = 0
        self.patch_version = 0
        self.nr_commits = 0
        self.commit_sha = ""
        self.is_dirty = False
        self.remote = ""
        self.remote_short = ""


def git_information(directory):
    os.chdir(directory)

    g = GitInformation()
    git_get_branch(g)
    git_get_describe(g)


def main():
    print("branch='{}'".format(git_information(".")))
    print("branch='{}'".format(git_information("..")))

if __name__ == "__main__":
    main()

