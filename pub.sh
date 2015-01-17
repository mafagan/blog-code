#!/bin/bash

hexo generate
cd public
python -m SimpleHTTPServer
