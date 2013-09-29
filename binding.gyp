{
  "targets": [
    {
      "target_name": "git",
      "defines": ["WIN32_SHA1", "GIT_WINHTTP"],
      "include_dirs": ["libgit2/src", "libgit2/include", "libgit2/deps/regex", "libgit2/deps/zlib"],
      "sources": [
      	'<!@(ls -1 libgit2/src/*.c)',
      	'libgit2/src/hash/hash_win32.c',
      	'libgit2/deps/regex/regex.c',
      	'<!@(ls -1 libgit2/deps/zlib/*.c)',
      	'<!@(ls -1 libgit2/src/transports/*.c)',
      	'<!@(ls -1 libgit2/src/xdiff/*.c)',
      	'<!@(ls -1 libgit2/src/win32/*.c)',
      	'node_git.cpp'
      ]
    }
  ]
}
