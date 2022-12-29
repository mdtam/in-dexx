# in-dexx
This is a GUI tool to search through a specified directory for some specified phrase.

# Functionality:

* A box for specifying a target directory.
* A prompt for specifying a phrase to search and a table for showing a list of found files.
* The tool reads the specified directory and files.
* uses boyer-moore algorithm to search through the files

# Improvements
* There is many many easy improvements still to be done: 
  * Don't read the files on every search. Only read all files when the directory changes
  * add option to search recursively in chosen directory. (feature already available but not enabled)
  * store history of searches. 

# How to Start
* install requirements: \
  `libglfw3-dev libglfw3 g++ pkg-config`
* build with
  ``` 
  cd src
  make all
  ```
* or run it with `make && ./in_dexx`
* `make clean` to cleanup

# References
* YT tutorial from https://www.youtube.com/watch?v=MWtHJTxgozQ.
* inspiration from https://github.com/gnunn1/vgrep
* GUI library from https://github.com/ocornut/imgui
* Boyer-Moore Implementation from https://github.com/harshil93/Boyer-Moore-Implementation

