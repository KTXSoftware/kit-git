{
  "targets": [
    {
      "target_name": "git",
      "include_dirs": ["libgit2/src", "libgit2/include", "libgit2/deps/http-parser", "libgit2/deps/regex", "libgit2/deps/zlib"],
      "sources": [
      	'<!@(ls -1 libgit2/src/*.c)',
      	'libgit2/deps/http-parser/http_parser.c',
        'libgit2/deps/regex/regex.c',
      	'<!@(ls -1 libgit2/deps/zlib/*.c)',
      	'<!@(ls -1 libgit2/src/transports/*.c)',
      	'<!@(ls -1 libgit2/src/xdiff/*.c)',
      	'node_git.cpp'
      ],
      'conditions': [
          ['OS=="linux"', {
            
          }],
          ['OS=="win"', {
            "defines": ["WIN32_SHA1", "GIT_WINHTTP"],
            "sources": ['libgit2/src/hash/hash_win32.c', '<!@(ls -1 libgit2/src/win32/*.c)']
          }, { # OS != "win",
            "defines": ["STDC"],
            "sources": ['libgit2/src/hash/hash_generic.c', '<!@(ls -1 libgit2/src/unix/*.c)']
          }]
      ]
    }
  ]
}
