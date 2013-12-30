solution = Solution.new("kitgit")
project = Project.new("kitgit")

solution:cmd()

project:addDefine("NO_V8")
project:addDefine("WIN32_SHA1")
project:addDefine("GIT_WINHTTP")

project:addIncludeDir("libgit2/src")
project:addIncludeDir("libgit2/include")
project:addIncludeDir("libgit2/deps/http-parser")
project:addIncludeDir("libgit2/deps/regex")
project:addIncludeDir("libgit2/deps/zlib")

project:addFile("libgit2/src/*.c")
project:addFile("libgit2/src/hash/hash_win32.c")
project:addFile("libgit2/deps/http-parser/http_parser.c")
project:addFile("libgit2/deps/regex/regex.c")
project:addFile("libgit2/deps/zlib/*.c")
project:addFile("libgit2/src/transports/*.c")
project:addFile("libgit2/src/xdiff/*.c")
project:addFile("libgit2/src/win32/*.c")
project:addFile("node_git.cpp")

solution:addProject(project)
