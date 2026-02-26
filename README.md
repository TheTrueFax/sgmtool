# SGMTool
CLI tool to convert between the Rayne SGM file format and other 3D models.

SGMTool is maintained and developed solely by me, but feel free to create pull requests for optimisations, bugs, etc.

# Building
SGMTool is single-file as of now, so building is very simple.
It is reccomended if you are modifying SGMTool code, use valgrind on linux to check for memory leaks, and other bugs, for windows you can use other memory leak tools.
Building for windows:
- Navigate to /sgmtool/
- Run `gcc main.c -o sgmtool` (add `-g` flag if using debuggers)
Building for linux:
- Navigate to /sgmtool/
- Run `gcc main.c -o sgmtool -lm` (again, add `-g` flag if using valgrind)

# Usage
SGMTool currently only supports converting OBJ to SGM, usage is extremely simple, Run `./sgmtool --help` or `./sgmtool -h` for usage help as everything you need to know to use the tool is probably there.
Example OBJ to SGM: `./sgmtool -w example.obj -o result.sgm`
If on Windows, replace `./sgmtool` in each command with `.\sgmtool.exe`.
Also include the `--debug` or `-d` flag for debug printing when converting.

# Notes
SGMTool is a hobby project, developed for fun, my main motive is getting the program working and shipping, *not* having clean or fully optimised code. I should do these things but if it really bothers you so much, create a pull request for it.
I am hoping to support more file types and a wider range of tools as time goes on but this project will most likely die out once I have reached a certain point in development.
