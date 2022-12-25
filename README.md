# in-dexx
This is a GUI tool to search through a specified directory for some specified phrase.

# Functionality:

* A prompt for specifying a target directory.
* A prompt for specifying a phrase to search and a pane for showing a list of found files.
* After the directory is specified, it builds an index and stores it on disk.
* The index gets updated once some files change.
* The index is reused after the app restart.

# How to Start
* install requirements: \
  `libglfw3-dev libglfw3 g++ pkg-config`
* ``` 
  cd src
  make all
  ```
* run it with `./in_dexx`
* `make clean` to cleanup

# References
* YT tutorial from https://www.youtube.com/watch?v=MWtHJTxgozQ.
* inspiration from https://github.com/gnunn1/vgrep
* GUI library from https://github.com/ocornut/imgui
